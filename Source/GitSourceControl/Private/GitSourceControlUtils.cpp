// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlUtils.h"
#include "GitSourceControlState.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlCommand.h"

#include "UniquePtr.h"

namespace GitSourceControlConstants
{
	/** The maximum number of files we submit in a single Git command */
	const int32 MaxFilesPerBatch = 50;
}

// Write the FString to a file in UTF-8 without BOM signature.
static bool SaveStringToFileUtf8(const FString& String, const TCHAR* Filename)
{
	bool bResult = false;
	auto Ar = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(Filename, 0));
	if(Ar)
	{
		if(!String.IsEmpty())
		{
			FTCHARToUTF8 UTF8String(*String);
			Ar->Serialize((UTF8CHAR*)UTF8String.Get(), UTF8String.Length() * sizeof(UTF8CHAR));
		}
		bResult = true;
	}

	return bResult;
}

FScopedTempFile::FScopedTempFile(const FText& InText)
{
	Filename = FPaths::CreateTempFilename(*FPaths::GameLogDir(), TEXT("Git-Temp"), TEXT(".txt"));
	// Write the file in UTF8 but without the BOM signature
	if(!SaveStringToFileUtf8(InText.ToString(), *Filename))
	{
		UE_LOG(LogSourceControl, Error, TEXT("Failed to write to temp file: %s"), *Filename);
	}
}

FScopedTempFile::~FScopedTempFile()
{
	if(FPaths::FileExists(Filename))
	{
		if(!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*Filename))
		{
			UE_LOG(LogSourceControl, Error, TEXT("Failed to delete temp file: %s"), *Filename);
		}
	}
}

const FString& FScopedTempFile::GetFilename() const
{
	return Filename;
}


namespace GitSourceControlUtils
{

// Launch the Git command line process and extract its results & errors
static bool RunCommandInternalRaw(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, FString& OutResults, FString& OutErrors)
{
	int32 ReturnCode = 0;
	FString FullCommand;
	FString LogableCommand; // short version of the command for logging purpose

	if (!InRepositoryRoot.IsEmpty())
	{
		// Specify the working copy (the root) of the git repository (before the command itself)
		FullCommand  = TEXT("--work-tree=\"");
		FullCommand += InRepositoryRoot;
		// and the ".git" subdirectory in it (before the command itself)
		FullCommand += TEXT("\" --git-dir=\"");
		FullCommand += InRepositoryRoot;
		FullCommand += TEXT(".git\" ");
	}
	// then the git command itself ("status", "log", "commit"...)
	LogableCommand += InCommand;

	// Append to the command all parameters, and then finally the files
	for(const auto& Parameter : InParameters)
	{
		LogableCommand += TEXT(" ");
		LogableCommand += Parameter;
	}
	for(const auto& File : InFiles)
	{
		LogableCommand += TEXT(" \"");
		LogableCommand += File;
		LogableCommand += TEXT("\"");
	}
	// Also, Git does not have a "--non-interactive" option, as it auto-detects when there are no connected standard input/output streams

	FullCommand += LogableCommand;

	UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternalRaw: 'git %s'"), *LogableCommand);
// @todo: temporary debug logs
//	UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternalRaw: 'git %s'"), *FullCommand);
	FPlatformProcess::ExecProcess(*InPathToGitBinary, *FullCommand, &ReturnCode, &OutResults, &OutErrors);
/*	UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternalRaw: ExecProcess ReturnCode=%d OutResults='%s'"), ReturnCode, *OutResults);
	if(!OutErrors.IsEmpty())
	{
		UE_LOG(LogSourceControl, Error, TEXT("RunCommandInternalRaw: ExecProcess ReturnCode=%d OutErrors='%s'"), ReturnCode, *OutErrors);
	}
*/
	return ReturnCode == 0;
}

// Basic parsing or results & errors from the Git command line process
static bool RunCommandInternal(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	bool bResult;
	FString Results;
	FString Errors;

	bResult = RunCommandInternalRaw(InCommand, InPathToGitBinary, InRepositoryRoot, InParameters, InFiles, Results, Errors);
	Results.ParseIntoArray(&OutResults, TEXT("\n"), true);
	Errors.ParseIntoArray(&OutErrorMessages, TEXT("\n"), true);

	return bResult;
}

FString FindGitBinaryPath()
{
	bool bFound = false;

#if PLATFORM_WINDOWS
	// 1) First of all, check for the ThirdParty directory as it may contain a specific version of Git for this plugin to work
	// NOTE using only "git" (or "git.exe") relying on the "PATH" envvar does not always work as expected, depending on the installation:
	// If the PATH is set with "git/cmd" instead of "git/bin",
	// "git.exe" launch "git/cmd/git.exe" that redirect to "git/bin/git.exe" and ExecProcess() is unable to catch its outputs streams.

	// Under Windows, we can use the third party "msysgit PortableGit" https://code.google.com/p/msysgit/downloads/list?can=1&q=PortableGit
	// NOTE: Win32 platform subdirectory as there is no Git 64bit build available
	FString GitBinaryPath(FPaths::EngineDir() / TEXT("Binaries/ThirdParty/git/Win32/bin") / TEXT("git.exe"));
	bFound = CheckGitAvailability(GitBinaryPath);
#endif

	// 2) If Git is not found in ThirdParty directory, look into standard install directory
	if(!bFound)
	{
#if PLATFORM_WINDOWS
		// @todo use the Windows registry to find Git
		GitBinaryPath = TEXT("C:/Program Files (x86)/Git/bin/git.exe");
#else
		GitBinaryPath = TEXT("/usr/bin/git");
#endif
	}

	return GitBinaryPath;
}

bool CheckGitAvailability(const FString& InPathToGitBinary)
{
	bool bGitAvailable = false;

	FString InfoMessages;
	FString ErrorMessages;
	bGitAvailable = RunCommandInternalRaw(TEXT("version"), InPathToGitBinary, FString(), TArray<FString>(), TArray<FString>(), InfoMessages, ErrorMessages);
	if(bGitAvailable)
	{
		if(InfoMessages.Contains("git"))
		{
			bGitAvailable = true;
		}
		else
		{
			bGitAvailable = false;
		}
	}

	// @todo also check Git config user.name & user.email

	return bGitAvailable;
}

bool FindRootDirectory(const FString& InPathToGameDir, FString& OutRepositoryRoot)
{
	bool bFound = false;
	FString PathToGitSubdirectory;
	OutRepositoryRoot = InPathToGameDir;

	while(!bFound && !OutRepositoryRoot.IsEmpty())
	{
		PathToGitSubdirectory = OutRepositoryRoot;
		PathToGitSubdirectory += TEXT(".git"); // Look for the ".git" subdirectory at the root of every Git repository
		bFound = IFileManager::Get().DirectoryExists(*PathToGitSubdirectory);
		if(!bFound)
		{
			int32 LastSlashIndex;
			OutRepositoryRoot = OutRepositoryRoot.LeftChop(5);
			if(OutRepositoryRoot.FindLastChar('/', LastSlashIndex))
			{
				OutRepositoryRoot = OutRepositoryRoot.Left(LastSlashIndex + 1);
			}
			else
			{
				OutRepositoryRoot.Empty();
			}
		}
	}

	return bFound;
}

bool RunCommand(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	bool bResult = true;

	if(InFiles.Num() > GitSourceControlConstants::MaxFilesPerBatch)
	{
		// Batch files up so we dont exceed command-line limits
		int32 FileCount = 0;
		while(FileCount < InFiles.Num())
		{
			TArray<FString> FilesInBatch;
			for(int32 FileIndex = 0; FileCount < InFiles.Num() && FileIndex < GitSourceControlConstants::MaxFilesPerBatch; FileIndex++, FileCount++)
			{
				FilesInBatch.Add(InFiles[FileCount]);
			}

			TArray<FString> BatchResults;
			TArray<FString> BatchErrors;
			bResult &= RunCommandInternal(InCommand, InPathToGitBinary, InRepositoryRoot, InParameters, FilesInBatch, BatchResults, BatchErrors);
			OutResults += BatchResults;
			OutErrorMessages += BatchErrors;
		}
	}
	else
	{
		bResult &= RunCommandInternal(InCommand, InPathToGitBinary, InRepositoryRoot, InParameters, InFiles, OutResults, OutErrorMessages);
	}

	return bResult;
}

bool RunCommit(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	bool bResult = true;

	if (InFiles.Num() > GitSourceControlConstants::MaxFilesPerBatch)
	{
		// Batch files up so we dont exceed command-line limits
		int32 FileCount = 0;
		TArray<FString> FilesInBatch;
		for (int32 FileIndex = 0; FileIndex < GitSourceControlConstants::MaxFilesPerBatch; FileIndex++, FileCount++)
		{
			FilesInBatch.Add(InFiles[FileCount]);
		}
		// First batch is a simple "git commit" command with only the first files
		bResult &= RunCommandInternal(TEXT("commit"), InPathToGitBinary, InRepositoryRoot, InParameters, InFiles, OutResults, OutErrorMessages);
		
		TArray<FString> Parameters;
		for (const auto& Parameter : InParameters)
		{
			Parameters.Add(Parameter);
		}
		Parameters.Add(TEXT("--amend"));

		while (FileCount < InFiles.Num())
		{
			TArray<FString> FilesInBatch;
			for (int32 FileIndex = 0; FileCount < InFiles.Num() && FileIndex < GitSourceControlConstants::MaxFilesPerBatch; FileIndex++, FileCount++)
			{
				FilesInBatch.Add(InFiles[FileCount]);
			}
			// Next batches "amend" the commit with some more files
			TArray<FString> BatchResults;
			TArray<FString> BatchErrors;
			bResult &= RunCommandInternal(TEXT("commit"), InPathToGitBinary, InRepositoryRoot, Parameters, FilesInBatch, BatchResults, BatchErrors);
			OutResults += BatchResults;
			OutErrorMessages += BatchErrors;
		}
	}
	else
	{
		RunCommandInternal(TEXT("commit"), InPathToGitBinary, InRepositoryRoot, InParameters, InFiles, OutResults, OutErrorMessages);
	}

	return bResult;
}

/** Match the relative filename of a Git status result with a provided absolute filename */
class FGitStatusFileMatcher
{
public:
	FGitStatusFileMatcher(const FString& InAbsoluteFilename)
		: AbsoluteFilename(InAbsoluteFilename)
	{
	}

	bool Matches(const FString& InResult) const
	{
		// Extract the relative filename from the Git status result
		// @todo this cannot work in case of a rename from -> to
		FString RelativeFilename = InResult.RightChop(3);
		return AbsoluteFilename.Contains(RelativeFilename);
	}

private:
	const FString& AbsoluteFilename;
};

/**
* Extract and interpret the file state from the given Git status result.
* @see file:///C:/Program%20Files%20(x86)/Git/doc/git/html/git-status.html
* ' ' = unmodified
* 'M' = modified
* 'A' = added
* 'D' = deleted
* 'R' = renamed
* 'C' = copied
* 'U' = updated but unmerged
* '?' = unknown/untracked
* '!' = ignored
*/
class FGitStatusParser
{
public:
	FGitStatusParser(const FString& InResult)
	{
		// @todo Get the second part of a rename "from -> to"
		//FString Filename = InResult.RightChop(3);

		TCHAR IndexState = InResult[0];
		TCHAR WCopyState = InResult[1];
		if((IndexState == 'U' || WCopyState == 'U')
		   || (IndexState == 'A' && WCopyState == 'A')
		   || (IndexState == 'D' && WCopyState == 'D'))
		{
			// "Unmerged" conflict cases are generally marked with a "U",
			// but there are also the special cases of both "A"dded, or both "D"eleted
			State = EWorkingCopyState::Conflicted;
		}
		else if(IndexState == 'A')
		{
			State = EWorkingCopyState::Added;
		}
		else if(IndexState == 'D')
		{
			State = EWorkingCopyState::Deleted;
		}
		else if(WCopyState == 'D')
		{
			State = EWorkingCopyState::Missing;
		}
		else if(IndexState == 'M' || WCopyState == 'M')
		{
			State = EWorkingCopyState::Modified;
		}
		else if(IndexState == 'R')
		{
			State = EWorkingCopyState::Renamed;
		}
		else if(IndexState == 'C')
		{
			State = EWorkingCopyState::Copied;
		}
		else if(IndexState == '?' || WCopyState == '?')
		{
			State = EWorkingCopyState::NotControlled;
		}
		else if(IndexState == '!' || WCopyState == '!')
		{
			State = EWorkingCopyState::Ignored;
		}
		else
		{
			// Unmodified never yield a status
			State = EWorkingCopyState::Unknown;
		}
	}

	EWorkingCopyState::Type State;
};

// Parse the array of strings results of a 'git status' command
static void ParseStatusResults(const TArray<FString>& InFiles, const TArray<FString>& InResults, TArray<FGitSourceControlState>& OutStates)
{
	for(const auto& File : InFiles)
	{
		FGitSourceControlState FileState(File);
		FGitStatusFileMatcher FileMatcher(File);
		int32 IdxResult = InResults.FindMatch(FileMatcher);
		if(IdxResult != INDEX_NONE)
		{
			// File found in status results; only the case for "changed" files
			FGitStatusParser StatusParser(InResults[IdxResult]);
			FileState.WorkingCopyState = StatusParser.State;
		}
		else
		{
			// File not found in status
			if(FPaths::FileExists(File))
			{
				// usually means the file is unchanged,
				FileState.WorkingCopyState = EWorkingCopyState::Unchanged;
			}
			else
			{
				// but also the case for newly created content: there is no file on disk until the content is saved for the first time
				FileState.WorkingCopyState = EWorkingCopyState::NotControlled;
			}
		}
		FileState.TimeStamp.Now();
		OutStates.Add(FileState);
	}
}

bool RunUpdateStatus(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InFiles, TArray<FString>& OutErrorMessages, TArray<FGitSourceControlState>& OutStates)
{
	bool bResults = true;
	TArray<FString> Results;
	TArray<FString> Parameters;
	Parameters.Add(TEXT("--porcelain"));
	Parameters.Add(TEXT("--ignored"));

	// Git status does not show any "untracked files" when called with files from different subdirectories! (issue #3)
	// 1) So here we group files by path (ie. by subdirectory)
	TMap<FString, TArray<FString>> GroupOfFiles;
	for(const auto& File : InFiles)
	{
		const FString Path = FPaths::GetPath(*File);
		TArray<FString>* Group = GroupOfFiles.Find(Path);
		if(Group != nullptr)
		{
			Group->Add(File);
		}
		else
		{
			TArray<FString> Group;
			Group.Add(File);
			GroupOfFiles.Add(Path, Group);
		}
	}

	// 2) then we can batch git status operation by subdirectory
	for(const auto& Files : GroupOfFiles)
	{
		bool bResult = RunCommand(TEXT("status"), InPathToGitBinary, InRepositoryRoot, Parameters, Files.Value, Results, OutErrorMessages);
		if(bResult)
		{
			ParseStatusResults(Files.Value, Results, OutStates);
		}
		else
		{
			bResults = false;
		}
	}

	return bResults;
}

/**
* Run a Git show command to dump the binary content of a revision into a file.
*/
bool RunDumpToFile(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InParameter, const FString& InDumpFileName)
{
	bool bResult = false;
	FString FullCommand;

	if(!InRepositoryRoot.IsEmpty())
	{
		// Specify the working copy (the root) of the git repository (before the command itself)
		FullCommand = TEXT("--work-tree=\"");
		FullCommand += InRepositoryRoot;
		// and the ".git" subdirectory in it (before the command itself)
		FullCommand += TEXT("\" --git-dir=\"");
		FullCommand += InRepositoryRoot;
		FullCommand += TEXT(".git\" ");
	}
	// then the git command itself
	FullCommand += TEXT("show ");

	// Append to the command the parameter
	FullCommand += InParameter;

	const bool bLaunchDetached = false;
	const bool bLaunchHidden = true;
	const bool bLaunchReallyHidden = bLaunchHidden;

	// Setup output redirection pipes, so that we can harvest compiler output and display it ourselves
#if PLATFORM_LINUX
	int pipefd[2];
	pipe(pipefd);
	void* PipeRead = &pipefd[0];
	void* PipeWrite = &pipefd[1];
#else
	void* PipeRead = NULL;
	void* PipeWrite = NULL;
#endif

	verify(FPlatformProcess::CreatePipe(PipeRead, PipeWrite));

	// @todo temp debug log
	//UE_LOG(LogSourceControl, Log, TEXT("RunDumpToFile: 'git %s'"), *FullCommand);
	FProcHandle ProcessHandle = FPlatformProcess::CreateProc(*InPathToGitBinary, *FullCommand, bLaunchDetached, bLaunchHidden, bLaunchReallyHidden, NULL, 0, NULL, PipeWrite);
	//UE_LOG(LogSourceControl, Log, TEXT("RunDumpToFile: ProcessHandle=%x"), ProcessHandle.Get());
	if(ProcessHandle.IsValid())
	{
		FPlatformProcess::Sleep(0.01);

		// @todo Append directly to the temp file whithout intermediate Array?
		TArray<uint8> BinaryFileContent;
		while(FPlatformProcess::IsProcRunning(ProcessHandle))
		{
			TArray<uint8> BinaryData = GitSourceControlUtils::ReadPipeToArray(PipeRead);
			if(BinaryData.Num() > 0)
			{
				BinaryFileContent.Append(BinaryData);
			}
		}
		TArray<uint8> BinaryData = GitSourceControlUtils::ReadPipeToArray(PipeRead);
		if(BinaryData.Num() > 0)
		{
			BinaryFileContent.Append(BinaryData);
		}
		// Save buffer into temp file
		if(FFileHelper::SaveArrayToFile(BinaryFileContent, *InDumpFileName))
		{
			UE_LOG(LogSourceControl, Log, TEXT("Writed '%s' (%do)"), *InDumpFileName, BinaryFileContent.Num());
			bResult = true;
		}
		else
		{
			UE_LOG(LogSourceControl, Error, TEXT("Could not write %s"), *InDumpFileName);
		}
	}
	else
	{
		UE_LOG(LogSourceControl, Error, TEXT("Failed to launch 'git show'"));
	}

#if PLATFORM_LINUX
	close(*(int*)ReadPipe);
	close(*(int*)PipeWrite);
#else
	FPlatformProcess::ClosePipe(PipeRead, PipeWrite);
#endif

	return bResult;
}


/**
* Extract and interpret the file state from the given Git log --name-status.
* @see https://www.kernel.org/pub/software/scm/git/docs/git-log.html
* ' ' = unmodified
* 'M' = modified
* 'A' = added
* 'D' = deleted
* 'R' = renamed
* 'C' = copied
* 'T' = type changed
* 'U' = updated but unmerged
* 'X' = unknown
* 'B' = broken pairing
*/
FString LogStatusToString(TCHAR InStatus)
{
	switch(InStatus)
	{
	case TEXT(' '):
		return FString("unmodified");
	case TEXT('M'):
		return FString("modified");
	case TEXT('A'):
		return FString("added");
	case TEXT('D'):
		return FString("deleted");
	case TEXT('R'):
		return FString("renamed");
	case TEXT('C'):
		return FString("copied");
	case TEXT('T'):
		return FString("type changed");
	case TEXT('U'):
		return FString("unmerged");
	case TEXT('X'):
		return FString("unknown");
	case TEXT('B'):
		return FString("broked pairing");
	}

	return FString();
}

/** Example git log results:
commit 97a4e7626681895e073aaefd68b8ac087db81b0b
Author: Sébastien Rombauts <sebastien.rombauts@gmail.com>
Date:   2014-05-15 21:32:27 +0200

    Another commit used to test History

     - with many lines
     - some <xml>
     - and strange characteres $*+

M	Content/Blueprints/Blueprint_CeilingLight.uasset

commit 355f0df26ebd3888adbb558fd42bb8bd3e565000
Author: Sébastien Rombauts <sebastien.rombauts@gmail.com>
Date:   2014-05-12 11:28:14 +0200

    Testing git status, edit, and revert

A		 Content/Blueprints/Blueprint_CeilingLight.uasset
*/
void ParseLogResults(const TArray<FString>& InResults, TGitSourceControlHistory& OutHistory)
{
	TSharedRef<FGitSourceControlRevision, ESPMode::ThreadSafe> SourceControlRevision = MakeShareable(new FGitSourceControlRevision);
	for(const auto& Result : InResults)
	{
		if(Result.StartsWith(TEXT("commit ")))
		{
			// End of the previous commit
			if(SourceControlRevision->RevisionNumber != 0)
			{
				SourceControlRevision->Description += TEXT("\nCommit Id: ");
				SourceControlRevision->Description += SourceControlRevision->CommitId;
				OutHistory.Add(SourceControlRevision);

				SourceControlRevision = MakeShareable(new FGitSourceControlRevision);
			}
			SourceControlRevision->CommitId = Result.RightChop(7);
			FString ShortCommitId = SourceControlRevision->CommitId.Right(8); // Short revision ; first 8 hex characters (max that can hold a 32 bit integer)
			SourceControlRevision->RevisionNumber = FParse::HexNumber(*ShortCommitId);
		}
		else if(Result.StartsWith(TEXT("Author: ")))
		{
			// Remove the 'email' part of the UserName
			FString UserNameEmail = Result.RightChop(8);
			int32 EmailIndex = 0;
			if(UserNameEmail.FindLastChar('<', EmailIndex))
			{
				SourceControlRevision->UserName = UserNameEmail.Left(EmailIndex - 1);
			}
		}
		else if(Result.StartsWith(TEXT("Date:   ")))
		{
			FString Date = Result.RightChop(8);
			SourceControlRevision->Date = FDateTime::FromUnixTimestamp(FCString::Atoi(*Date));
		}
	//	else if(Result.IsEmpty()) // empty line before/after commit message has already been taken care by FString::ParseIntoArray()
		else if(Result.StartsWith(TEXT("    ")))
		{
			SourceControlRevision->Description += Result.RightChop(4);
			SourceControlRevision->Description += TEXT("\n");
		}
		else
		{
			TCHAR Status = Result[0];
			SourceControlRevision->Action = LogStatusToString(Status); // Readable action string ("Added", Modified"...) instead of "A"/"M"...
			SourceControlRevision->Filename = Result.RightChop(2); // relative filename
		}
	}
	// End of the last commit
	if(SourceControlRevision->RevisionNumber != 0)
	{
		SourceControlRevision->Description += TEXT("\nCommit Id: ");
		SourceControlRevision->Description += SourceControlRevision->CommitId;
		OutHistory.Add(SourceControlRevision);
	}
}

bool UpdateCachedStates(const TArray<FGitSourceControlState>& InStates)
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>( "GitSourceControl" );
	FGitSourceControlProvider& Provider = GitSourceControl.GetProvider();
	int NbStatesUpdated = 0;

	for(const auto& InState : InStates)
	{
		TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> State = Provider.GetStateInternal(InState.LocalFilename);
		if(State->WorkingCopyState != InState.WorkingCopyState)
		{
			State->WorkingCopyState = InState.WorkingCopyState;
		//	State->TimeStamp = InState.TimeStamp; // @todo Bug report: Workaround a bug with the Source Control Module not updating file state after a "Save"
			NbStatesUpdated++;
		}
	}

	return (NbStatesUpdated > 0);
}

// @todo PullRequest to integrate this in FGenericPlatformProcess, FWindowsPlatformProcess, FMacPlatformProcess and FLinuxPlatformProcess
#if PLATFORM_WINDOWS
TArray<uint8> ReadPipeToArray(void* ReadPipe)
{
	TArray<uint8> Output;

	uint32 BytesAvailable = 0;
	if(::PeekNamedPipe(ReadPipe, NULL, 0, NULL, (::DWORD*)&BytesAvailable, NULL) && (BytesAvailable > 0))
	{
		Output.Init(BytesAvailable);
		uint32 BytesRead = 0;
		if(::ReadFile(ReadPipe, Output.GetData(), BytesAvailable, (::DWORD*)&BytesRead, NULL))
		{
			if(BytesRead < BytesAvailable)
			{
				Output.SetNum(BytesRead);
			}
		}
		else
		{
			Output.Empty();
		}
	}

	return Output;
}
#elif PLATFORM_MAC
TArray<uint8> ReadPipeToArray(void* ReadPipe)
{
	SCOPED_AUTORELEASE_POOL;

	TArray<uint8> Output;
   const int32 READ_SIZE = 32768;

	if(ReadPipe)
	{
	   Output.Init(READ_SIZE);
	   int32 BytesRead = 0;
		BytesRead = read([(NSFileHandle*)ReadPipe fileDescriptor], Output.GetData(), READ_SIZE);
		if (BytesRead > 0)
		{
			if (BytesRead < READ_SIZE)
			{
				Output.SetNum(BytesRead);
			}
		}
		else
		{
			Output.Empty();
		}
	}

	return Output;
}
#elif PLATFORM_LINUX
TArray<uint8> ReadPipeToArray(void* ReadPipe)
{
	TArray<uint8> Output;

	if (ReadPipe)
	{
		FPipeHandle * PipeHandle = reinterpret_cast< FPipeHandle* >(ReadPipe);
		int PipeDesc = PipeHandle->GetHandle();

		int BytesAvailable = 0;
		if (ioctl(PipeDesc, FIONREAD, &BytesAvailable) == 0)
		{
			if (BytesAvailable > 0)
			{
				Output.Init(BytesAvailable);
				int BytesRead = read(PipeDesc, Buffer, kBufferSize - 1);
				if (BytesRead > 0)
				{
					if(BytesRead < BytesAvailable)
					{
						Output.SetNum(BytesRead);
					}
				}
				else
				{
					Output.Empty();
				}
			}
		}
	}

	return Output;
}
#endif

}
