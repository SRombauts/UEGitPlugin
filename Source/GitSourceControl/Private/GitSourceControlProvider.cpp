// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlProvider.h"
#include "GitSourceControlCommand.h"
#include "ISourceControlModule.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlSettings.h"
#include "GitSourceControlOperations.h"
#include "GitSourceControlUtils.h"
#include "SGitSourceControlSettings.h"
#include "MessageLog.h"
#include "ScopedSourceControlProgress.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

static FName ProviderName("Git");

void FGitSourceControlProvider::Init(bool bForceConnection)
{
    PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
    PathToContentDir = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());

	if(!bGitAvailable)
	{
		bGitAvailable = GitSourceControlUtils::CheckGitAvailability();
	}
}

void FGitSourceControlProvider::Close()
{
	// clear the cache
	StateCache.Empty();
}

TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> FGitSourceControlProvider::GetStateInternal(const FString& Filename)
{
	TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe>* State = StateCache.Find(Filename);
	if(State != NULL)
	{
		// found cached item
		return (*State);
	}
	else
	{
		// cache an unknown state for this item
		TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> NewState = MakeShareable( new FGitSourceControlState(Filename) );
		StateCache.Add(Filename, NewState);
		return NewState;
	}
}

FString FGitSourceControlProvider::GetStatusText() const
{
	FString StatusText = LOCTEXT("ProviderName", "Provider: Git").ToString();
	StatusText += LINE_TERMINATOR;
	{
		FFormatNamedArguments Arguments;
		Arguments.Add( TEXT("YesOrNo"), IsEnabled() ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No") );
		StatusText += FText::Format(LOCTEXT("EnabledLabel", "Enabled: {YesOrNo}"), Arguments).ToString();
	}
	StatusText += LINE_TERMINATOR;
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Repository"), FText::FromString( PathToGameDir ));
		StatusText += FText::Format(LOCTEXT("RepositoryLabel", "Repository: {Repository}"), Arguments).ToString();
	}

	return StatusText;
}

bool FGitSourceControlProvider::IsEnabled() const
{
	return true;
}

bool FGitSourceControlProvider::IsAvailable() const
{
	return bGitAvailable;
}

const FName& FGitSourceControlProvider::GetName(void) const
{
	return ProviderName;
}

ECommandResult::Type FGitSourceControlProvider::GetState( const TArray<FString>& InFiles, TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, EStateCacheUsage::Type InStateCacheUsage )
{
	if(!IsEnabled())
	{
		return ECommandResult::Failed;
	}

    /* TODO: why absolute?
	TArray<FString> AbsoluteFiles;
	for( TArray<FString>::TConstIterator It(InFiles); It; It++)
	{
		AbsoluteFiles.Add(FPaths::ConvertRelativePathToFull(*It));
	}
	*/

	if(InStateCacheUsage == EStateCacheUsage::ForceUpdate)
	{
		Execute(ISourceControlOperation::Create<FUpdateStatus>(), InFiles);
	}

	for(TArray<FString>::TConstIterator It(InFiles); It; It++)
	{
		OutState.Add(GetStateInternal(*It));
	}

	return ECommandResult::Succeeded;
}

void FGitSourceControlProvider::RegisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	OnSourceControlStateChanged.Add( SourceControlStateChanged );
}

void FGitSourceControlProvider::UnregisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	OnSourceControlStateChanged.Remove( SourceControlStateChanged );
}

ECommandResult::Type FGitSourceControlProvider::Execute( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency, const FSourceControlOperationComplete& InOperationCompleteDelegate )
{
	if(!IsEnabled())
	{
		return ECommandResult::Failed;
	}

	/* TODO: why absolute?
	TArray<FString> AbsoluteFiles;
	for( TArray<FString>::TConstIterator It(InFiles); It; It++)
	{
		AbsoluteFiles.Add(FPaths::ConvertRelativePathToFull(*It));
	}
	*/

	// Query to see if we allow this operation
	TSharedPtr<IGitSourceControlWorker, ESPMode::ThreadSafe> Worker = CreateWorker(InOperation->GetName());
	if(!Worker.IsValid())
	{
		// this operation is unsupported by this source control provider
		FFormatNamedArguments Arguments;
		Arguments.Add( TEXT("OperationName"), FText::FromName(InOperation->GetName()) );
		Arguments.Add( TEXT("ProviderName"), FText::FromName(GetName()) );
		FMessageLog("SourceControl").Error(FText::Format(LOCTEXT("UnsupportedOperation", "Operation '{OperationName}' not supported by source control provider '{ProviderName}'"), Arguments));
		return ECommandResult::Failed;
	}
	else
	{
		// TODO Temporary debug code
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("OperationName"), FText::FromName(InOperation->GetName()));
		Arguments.Add(TEXT("ProviderName"), FText::FromName(GetName()));
		FMessageLog("SourceControl").Info(FText::Format(LOCTEXT("Execute", "Execute: Operation '{OperationName}' by source control provider '{ProviderName}'"), Arguments));
	}

	FGitSourceControlCommand* Command = new FGitSourceControlCommand(InOperation, Worker.ToSharedRef());
	Command->bAutoDelete = false;
	Command->Files = InFiles;
	// TODO:
//	GitSourceControlUtils::QuoteFilenames(Command->Files);
	Command->OperationCompleteDelegate = InOperationCompleteDelegate;

	// fire off operation
	if(InConcurrency == EConcurrency::Synchronous)
	{
		Command->bAutoDelete = false;
		// TODO:
	//	return ExecuteSynchronousCommand(*Command, InOperation->GetInProgressString(), true);
		// TODO Temporary debug code
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("OperationName"), FText::FromName(InOperation->GetName()));
		Arguments.Add(TEXT("ProviderName"), FText::FromName(GetName()));
		FMessageLog("SourceControl").Info(FText::Format(LOCTEXT("ExecuteSynchronousCommand", "Execute: Operation '{OperationName}' by source control provider '{ProviderName}'"), Arguments));
		return ECommandResult::Failed;
	}
	else
	{
		Command->bAutoDelete = true;
		return IssueCommand(*Command, false);
	}
}

bool FGitSourceControlProvider::CanCancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation ) const
{
	return false;
}

void FGitSourceControlProvider::CancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation )
{
}

bool FGitSourceControlProvider::UsesLocalReadOnlyState() const
{
	return false;
}

TSharedPtr<IGitSourceControlWorker, ESPMode::ThreadSafe> FGitSourceControlProvider::CreateWorker(const FName& InOperationName) const
{
	const FGetGitSourceControlWorker* Operation = WorkersMap.Find(InOperationName);
	if(Operation != NULL)
	{
		return Operation->Execute();
	}
		
	return NULL;
}

void FGitSourceControlProvider::RegisterWorker( const FName& InName, const FGetGitSourceControlWorker& InDelegate )
{
	WorkersMap.Add( InName, InDelegate );
}

void FGitSourceControlProvider::Tick()
{	
	bool bStatesUpdated = false;
	for (int32 CommandIndex = 0; CommandIndex < CommandQueue.Num(); ++CommandIndex)
	{
		FGitSourceControlCommand& Command = *CommandQueue[CommandIndex];
		if (Command.bExecuteProcessed)
		{
			// Remove command from the queue
			CommandQueue.RemoveAt(CommandIndex);

			// let command update the states of any files
			bStatesUpdated |= Command.Worker->UpdateStates();

			// TODO: update connection state
		//	UpdateConnectionState(Command);

			// TODO: dump any messages to output log
		//	OutputCommandMessages(Command);

			// run the completion delegate if we have one bound
			Command.OperationCompleteDelegate.ExecuteIfBound(Command.Operation, Command.bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed);

			//commands that are left in the array during a tick need to be deleted
			if(Command.bAutoDelete)
			{
				// Only delete commands that are not running 'synchronously'
				delete &Command;
			}
			
			// only do one command per tick loop, as we dont want concurrent modification 
			// of the command queue (which can happen in the completion delegate)
			break;
		}
	}

	if(bStatesUpdated)
	{
		OnSourceControlStateChanged.Broadcast();
	}
}

TArray< TSharedRef<ISourceControlLabel> > FGitSourceControlProvider::GetLabels( const FString& InMatchingSpec ) const
{
	TArray< TSharedRef<ISourceControlLabel> > Tags;

	return Tags;
}

TSharedRef<class SWidget> FGitSourceControlProvider::MakeSettingsWidget() const
{
	return SNew(SGitSourceControlSettings);
}

ECommandResult::Type FGitSourceControlProvider::IssueCommand(FGitSourceControlCommand& InCommand, const bool bSynchronous)
{
	if ( !bSynchronous && GThreadPool != NULL )
	{
		// Queue this to our worker thread(s) for resolving
		GThreadPool->AddQueuedWork(&InCommand);
		CommandQueue.Add(&InCommand);
		return ECommandResult::Succeeded;
	}
	else
	{
		InCommand.bCommandSuccessful = InCommand.DoWork();

		// TODO:
	//	UpdateConnectionState(InCommand);

		InCommand.Worker->UpdateStates();

		// TODO:
	//	OutputCommandMessages(InCommand);

		// Callback now if present. When asynchronous, this callback gets called from Tick().
		ECommandResult::Type Result = InCommand.bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed;
		InCommand.OperationCompleteDelegate.ExecuteIfBound(InCommand.Operation, Result);

		return Result;
	}
}
#undef LOCTEXT_NAMESPACE