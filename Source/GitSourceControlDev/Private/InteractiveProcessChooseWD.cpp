// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "GitSourceControlPrivatePCH.h"
#include "InteractiveProcessChooseWD.h"

static FORCEINLINE bool CreatePipeWrite(void*& ReadPipe, void*& WritePipe)
{
#if PLATFORM_WINDOWS
	SECURITY_ATTRIBUTES Attr = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

	if (!::CreatePipe(&ReadPipe, &WritePipe, &Attr, 0))
	{
		return false;
	}

	if (!::SetHandleInformation(WritePipe, HANDLE_FLAG_INHERIT, 0))
	{
		return false;
	}

	return true;
#else
	return FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
#endif // PLATFORM_WINDOWS
}


DEFINE_LOG_CATEGORY(LogInteractiveProcessChooseWD);

FInteractiveProcessChooseWD::FInteractiveProcessChooseWD(const FString& InURL, const FString& InParams, const FString& InWorkingDir, bool InHidden, bool LongTime)
	: bCanceling(false)
	, bHidden(InHidden)
	, bKillTree(false)
	, URL(InURL)
	, Params(InParams)
	, WorkingDir(InWorkingDir)
	, ReadPipeParent(nullptr)
	, WritePipeParent(nullptr)
	, ReadPipeChild(nullptr)
	, WritePipeChild(nullptr)
	, Thread(nullptr)
	, ReturnCode(0)
	, StartTime(0)
	, EndTime(0)
{
	if(LongTime == true)
	{
		SleepTime = 0.0010f; ///< 10 milliseconds sleep
	}
	else
	{
		SleepTime = 0.f;
	}
}

FInteractiveProcessChooseWD::~FInteractiveProcessChooseWD()
{
	if (IsRunning() == true)
	{
		Cancel(false);
		Thread->WaitForCompletion();
		delete Thread;
	}
}

FTimespan FInteractiveProcessChooseWD::GetDuration() const
{
	if (IsRunning() == true)
	{
		return (FDateTime::UtcNow() - StartTime);
	}

	return (EndTime - StartTime);
}

bool FInteractiveProcessChooseWD::Launch()
{
	if (IsRunning() == true)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Warning, TEXT("The process is already running"));
		return false;
	}

	// For reading from child process
	if (FPlatformProcess::CreatePipe(ReadPipeParent, WritePipeChild) == false)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Failed to create pipes for parent process"));
		return false;
	}

	// For writing to child process
	if (CreatePipeWrite(ReadPipeChild, WritePipeParent) == false)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Failed to create pipes for parent process"));
		return false;
	}

	ProcessHandle = FPlatformProcess::CreateProc(*URL, *Params, false, bHidden, bHidden, nullptr, 0, *WorkingDir, WritePipeChild, ReadPipeChild);

	if (ProcessHandle.IsValid() == false)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Failed to create process"));
		return false;
	}

	// Creating name for the process
	static uint32 tempInteractiveProcessIndex = 0;
	ThreadName = FString::Printf(TEXT("FInteractiveProcess %d"), tempInteractiveProcessIndex);
	tempInteractiveProcessIndex++;

	Thread = FRunnableThread::Create(this, *ThreadName);
	if (Thread == nullptr)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Failed to create process thread!"));
		return false;
	}

	UE_LOG(LogInteractiveProcessChooseWD, Log, TEXT("Process creation succesfull %s"), *ThreadName);

	return true;
}

void FInteractiveProcessChooseWD::ProcessOutput(TArray<uint8> Output)
{
	if (Output.Num() > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessOutput: %i"), Output.Num());
		OutputArrayDelegate.ExecuteIfBound(Output);
	}

	/*
	Output.Append(FTCHARToUTF8('\0'));
	FString OutputString;
	OutputString += FUTF8ToTCHAR((const ANSICHAR*)Buffer).Get();

	TArray<FString> LogLines;

	OutputString.ParseIntoArray(LogLines, TEXT("\n"), false);

	for (int32 LogIndex = 0; LogIndex < LogLines.Num(); ++LogIndex)
	{
		// Don't accept if it is just an empty string
		if (LogLines[LogIndex].IsEmpty() == false)
		{
			OutputDelegate.ExecuteIfBound(LogLines[LogIndex]);
			UE_LOG(LogInteractiveProcessChooseWD, Log, TEXT("Child Process  -> %s"), *LogLines[LogIndex]);
		}
	}*/
}

void FInteractiveProcessChooseWD::SendMessageToProcessIf()
{
	// If there is not a message
	if (MessagesToProcess.IsEmpty() == true)
	{
		return;
	}

	if (WritePipeParent == nullptr)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Warning, TEXT("WritePipe is not valid"));
		return;
	}

	if (ProcessHandle.IsValid() == false)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Warning, TEXT("Process handle is not valid"));
		return;
	}

	// A string for original message and one for written message
	FString WrittenMessage, Message;
	MessagesToProcess.Dequeue(Message);

	FPlatformProcess::WritePipe(WritePipeParent, Message, &WrittenMessage);

	UE_LOG(LogInteractiveProcessChooseWD, Log, TEXT("Parent Process -> Original Message: %s , Written Message: %s"), *Message, *WrittenMessage);

	if (WrittenMessage.Len() == 0)
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Writing message through pipe failed"));
		return;
	}
	else if (Message.Len() > WrittenMessage.Len())
	{
		UE_LOG(LogInteractiveProcessChooseWD, Error, TEXT("Writing some part of the message through pipe failed"));
		return;
	}
}

void FInteractiveProcessChooseWD::SendWhenReady(const FString &Message)
{
	MessagesToProcess.Enqueue(Message);
}

// FRunnable interface
uint32 FInteractiveProcessChooseWD::Run()
{
	// control and interact with the process
	StartTime = FDateTime::UtcNow();
	{
		do
		{
			FPlatformProcess::Sleep(SleepTime);

			TArray<uint8> Output;
			FPlatformProcess::ReadPipeToArray(ReadPipeParent, Output);
			// Read pipe and redirect it to ProcessOutput function
			ProcessOutput(Output);

			// Write to process if there is a message
			SendMessageToProcessIf();

			// If wanted to stop program
			if (bCanceling == true)
			{
				FPlatformProcess::TerminateProc(ProcessHandle, bKillTree);
				CanceledDelegate.ExecuteIfBound();

				UE_LOG(LogInteractiveProcessChooseWD, Log, TEXT("The process is being canceled"));

				return 0;
			}
		} while (FPlatformProcess::IsProcRunning(ProcessHandle) == true);
	}

	// close pipes
	FPlatformProcess::ClosePipe(ReadPipeParent, WritePipeChild);
	ReadPipeParent = WritePipeChild = nullptr;
	FPlatformProcess::ClosePipe(ReadPipeChild, WritePipeParent);
	ReadPipeChild = WritePipeParent = nullptr;

	// get completion status
	if (FPlatformProcess::GetProcReturnCode(ProcessHandle, &ReturnCode) == false)
	{
		ReturnCode = -1;
	}

	EndTime = FDateTime::UtcNow();

	CompletedDelegate.ExecuteIfBound(ReturnCode, bCanceling);

	return 0;
}
