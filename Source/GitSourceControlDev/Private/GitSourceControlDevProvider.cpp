// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevProvider.h"
#include "GitSourceControlDevCommand.h"
#include "ISourceControlModule.h"
#include "GitSourceControlDevModule.h"
#include "GitSourceControlDevSettings.h"
#include "GitSourceControlDevOperations.h"
#include "GitSourceControlDevUtils.h"
#include "SGitSourceControlDevSettings.h"
#include "MessageLog.h"
#include "ScopedSourceControlProgress.h"

#define LOCTEXT_NAMESPACE "GitSourceControlDev"

static FName ProviderName("Git (devel)");

void FGitSourceControlDevProvider::Init(bool bForceConnection)
{
	CheckGitAvailability();

	// bForceConnection: not used anymore
}

void FGitSourceControlDevProvider::CheckGitAvailability()
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	const FString& PathToGitBinary = GitSourceControlDev.AccessSettings().GetBinaryPath();
	if(!PathToGitBinary.IsEmpty())
	{
		bGitAvailable = GitSourceControlDevUtils::CheckGitAvailability(PathToGitBinary);
		if(bGitAvailable)
		{
			// Find the path to the root Git directory (if any)
			const FString PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
			const bool bRepositoryFound = GitSourceControlDevUtils::FindRootDirectory(PathToGameDir, PathToRepositoryRoot);
			// Get user name & email (of the repository, else from the global Git config)
			GitSourceControlDevUtils::GetUserConfig(PathToGitBinary, PathToRepositoryRoot, UserName, UserEmail);
			if (bRepositoryFound)
			{
				// Get branch name
				GitSourceControlDevUtils::GetBranchName(PathToGitBinary, PathToRepositoryRoot, BranchName);
				bGitRepositoryFound = true;
			}
			else
			{
				UE_LOG(LogSourceControl, Error, TEXT("'%s' is not part of a Git repository"), *FPaths::GameDir());
				bGitRepositoryFound = false;
			}
		}
	}
	else
	{
		bGitAvailable = false;
	}
}

void FGitSourceControlDevProvider::Close()
{
	// clear the cache
	StateCache.Empty();
}

TSharedRef<FGitSourceControlDevState, ESPMode::ThreadSafe> FGitSourceControlDevProvider::GetStateInternal(const FString& Filename)
{
	TSharedRef<FGitSourceControlDevState, ESPMode::ThreadSafe>* State = StateCache.Find(Filename);
	if(State != NULL)
	{
		// found cached item
		return (*State);
	}
	else
	{
		// cache an unknown state for this item
		TSharedRef<FGitSourceControlDevState, ESPMode::ThreadSafe> NewState = MakeShareable( new FGitSourceControlDevState(Filename) );
		StateCache.Add(Filename, NewState);
		return NewState;
	}
}

FText FGitSourceControlDevProvider::GetStatusText() const
{
	FFormatNamedArguments Args;
	Args.Add( TEXT("RepositoryName"), FText::FromString(PathToRepositoryRoot) );
	Args.Add( TEXT("BranchName"), FText::FromString(BranchName) );
	Args.Add( TEXT("UserName"), FText::FromString(UserName) );
	Args.Add( TEXT("UserEmail"), FText::FromString(UserEmail) );

	return FText::Format( NSLOCTEXT("Status", "Provider: Git\nEnabledLabel", "Repository: {RepositoryName}\nBranch: {BranchName}\nUser: {UserName}\nE-mail: {UserEmail}"), Args );
}

/** Quick check if source control is enabled */
bool FGitSourceControlDevProvider::IsEnabled() const
{
	return bGitRepositoryFound;
}

/** Quick check if source control is available for use (useful for server-based providers) */
bool FGitSourceControlDevProvider::IsAvailable() const
{
	return bGitRepositoryFound;
}

const FName& FGitSourceControlDevProvider::GetName(void) const
{
	return ProviderName;
}

ECommandResult::Type FGitSourceControlDevProvider::GetState( const TArray<FString>& InFiles, TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, EStateCacheUsage::Type InStateCacheUsage )
{
	if(!IsEnabled())
	{
		return ECommandResult::Failed;
	}

	TArray<FString> AbsoluteFiles = SourceControlHelpers::AbsoluteFilenames(InFiles);

	if(InStateCacheUsage == EStateCacheUsage::ForceUpdate)
	{
		Execute(ISourceControlOperation::Create<FUpdateStatus>(), AbsoluteFiles);
	}

	for(const auto& AbsoluteFile : AbsoluteFiles)
	{
		OutState.Add(GetStateInternal(*AbsoluteFile));
	}

	return ECommandResult::Succeeded;
}

TArray<FSourceControlStateRef> FGitSourceControlDevProvider::GetCachedStateByPredicate(TFunctionRef<bool(const FSourceControlStateRef&)> Predicate) const
{
	TArray<FSourceControlStateRef> Result;
	for(const auto& CacheItem : StateCache)
	{
		FSourceControlStateRef State = CacheItem.Value;
		if(Predicate(State))
		{
			Result.Add(State);
		}
	}
	return Result;
}

bool FGitSourceControlDevProvider::RemoveFileFromCache(const FString& Filename)
{
	return StateCache.Remove(Filename) > 0;
}

FDelegateHandle FGitSourceControlDevProvider::RegisterSourceControlStateChanged_Handle( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	return OnSourceControlStateChanged.Add( SourceControlStateChanged );
}

void FGitSourceControlDevProvider::UnregisterSourceControlStateChanged_Handle( FDelegateHandle Handle )
{
	OnSourceControlStateChanged.Remove( Handle );
}

ECommandResult::Type FGitSourceControlDevProvider::Execute( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency, const FSourceControlOperationComplete& InOperationCompleteDelegate )
{
	if(!IsEnabled() && !(InOperation->GetName() == "Connect")) // Only Connect operation allowed while not Enabled (Connected)
	{
		return ECommandResult::Failed;
	}

	TArray<FString> AbsoluteFiles = SourceControlHelpers::AbsoluteFilenames(InFiles);

	// Query to see if we allow this operation
	TSharedPtr<IGitSourceControlDevWorker, ESPMode::ThreadSafe> Worker = CreateWorker(InOperation->GetName());
	if(!Worker.IsValid())
	{
		// this operation is unsupported by this source control provider
		FFormatNamedArguments Arguments;
		Arguments.Add( TEXT("OperationName"), FText::FromName(InOperation->GetName()) );
		Arguments.Add( TEXT("ProviderName"), FText::FromName(GetName()) );
		FMessageLog("SourceControl").Error(FText::Format(LOCTEXT("UnsupportedOperation", "Operation '{OperationName}' not supported by source control provider '{ProviderName}'"), Arguments));
		return ECommandResult::Failed;
	}

	FGitSourceControlDevCommand* Command = new FGitSourceControlDevCommand(InOperation, Worker.ToSharedRef());
	Command->Files = AbsoluteFiles;
	Command->OperationCompleteDelegate = InOperationCompleteDelegate;

	// fire off operation
	if(InConcurrency == EConcurrency::Synchronous)
	{
		Command->bAutoDelete = false;
		return ExecuteSynchronousCommand(*Command, InOperation->GetInProgressString());
	}
	else
	{
		Command->bAutoDelete = true;
		return IssueCommand(*Command);
	}
}

bool FGitSourceControlDevProvider::CanCancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation ) const
{
	return false;
}

void FGitSourceControlDevProvider::CancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation )
{
}

bool FGitSourceControlDevProvider::UsesLocalReadOnlyState() const
{
	return false;
}

bool FGitSourceControlDevProvider::UsesChangelists() const
{
	return false;
}

TSharedPtr<IGitSourceControlDevWorker, ESPMode::ThreadSafe> FGitSourceControlDevProvider::CreateWorker(const FName& InOperationName) const
{
	const FGetGitSourceControlDevWorker* Operation = WorkersMap.Find(InOperationName);
	if(Operation != nullptr)
	{
		return Operation->Execute();
	}
		
	return nullptr;
}

void FGitSourceControlDevProvider::RegisterWorker( const FName& InName, const FGetGitSourceControlDevWorker& InDelegate )
{
	WorkersMap.Add( InName, InDelegate );
}

void FGitSourceControlDevProvider::OutputCommandMessages(const FGitSourceControlDevCommand& InCommand) const
{
	FMessageLog SourceControlLog("SourceControl");

	for(int32 ErrorIndex = 0; ErrorIndex < InCommand.ErrorMessages.Num(); ++ErrorIndex)
	{
		SourceControlLog.Error(FText::FromString(InCommand.ErrorMessages[ErrorIndex]));
	}

	for(int32 InfoIndex = 0; InfoIndex < InCommand.InfoMessages.Num(); ++InfoIndex)
	{
		SourceControlLog.Info(FText::FromString(InCommand.InfoMessages[InfoIndex]));
	}
}

void FGitSourceControlDevProvider::Tick()
{	
	bool bStatesUpdated = false;
	for(int32 CommandIndex = 0; CommandIndex < CommandQueue.Num(); ++CommandIndex)
	{
		FGitSourceControlDevCommand& Command = *CommandQueue[CommandIndex];
		if(Command.bExecuteProcessed)
		{
			// Remove command from the queue
			CommandQueue.RemoveAt(CommandIndex);

			// let command update the states of any files
			bStatesUpdated |= Command.Worker->UpdateStates();

			// dump any messages to output log
			OutputCommandMessages(Command);

			// run the completion delegate callback if we have one bound
			ECommandResult::Type Result = Command.bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed;
			Command.OperationCompleteDelegate.ExecuteIfBound(Command.Operation, Result);

			// commands that are left in the array during a tick need to be deleted
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

TArray< TSharedRef<ISourceControlLabel> > FGitSourceControlDevProvider::GetLabels( const FString& InMatchingSpec ) const
{
	TArray< TSharedRef<ISourceControlLabel> > Tags;

	return Tags;
}

#if SOURCE_CONTROL_WITH_SLATE
TSharedRef<class SWidget> FGitSourceControlDevProvider::MakeSettingsWidget() const
{
	return SNew(SGitSourceControlDevSettings);
}
#endif

ECommandResult::Type FGitSourceControlDevProvider::ExecuteSynchronousCommand(FGitSourceControlDevCommand& InCommand, const FText& Task)
{
	ECommandResult::Type Result = ECommandResult::Failed;

	// Display the progress dialog if a string was provided
	{
		FScopedSourceControlProgress Progress(Task);

		// Issue the command asynchronously...
		IssueCommand( InCommand );

		// ... then wait for its completion (thus making it synchrounous)
		while(!InCommand.bExecuteProcessed)
		{
			// Tick the command queue and update progress.
			Tick();
			
			Progress.Tick();

			// Sleep for a bit so we don't busy-wait so much.
			FPlatformProcess::Sleep(0.01f);
		}
	
		// always do one more Tick() to make sure the command queue is cleaned up.
		Tick();

		if(InCommand.bCommandSuccessful)
		{
			Result = ECommandResult::Succeeded;
		}
	}

	// Delete the command now (asynchronous commands are deleted in the Tick() method)
	check(!InCommand.bAutoDelete);

	// ensure commands that are not auto deleted do not end up in the command queue
	if ( CommandQueue.Contains( &InCommand ) ) 
	{
		CommandQueue.Remove( &InCommand );
	}
	delete &InCommand;

	return Result;
}

ECommandResult::Type FGitSourceControlDevProvider::IssueCommand(FGitSourceControlDevCommand& InCommand)
{
	if(GThreadPool != nullptr)
	{
		// Queue this to our worker thread(s) for resolving
		GThreadPool->AddQueuedWork(&InCommand);
		CommandQueue.Add(&InCommand);
		return ECommandResult::Succeeded;
	}
	else
	{
		return ECommandResult::Failed;
	}
}
#undef LOCTEXT_NAMESPACE
