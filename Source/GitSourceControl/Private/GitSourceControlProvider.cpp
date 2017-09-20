// Copyright (c) 2014-2017 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlProvider.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/QueuedThreadPool.h"
#include "Modules/ModuleManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "GitSourceControlCommand.h"
#include "ISourceControlModule.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlUtils.h"
#include "SGitSourceControlSettings.h"
#include "Logging/MessageLog.h"
#include "ScopedSourceControlProgress.h"
#include "IPluginManager.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

static FName ProviderName("Git LFS 2");

void FGitSourceControlProvider::Init(bool bForceConnection)
{
	// Init() is called multiple times at startup: do not check git each time
	if(!bGitAvailable)
	{
		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GitSourceControl"));
		if(Plugin.IsValid())
		{
			UE_LOG(LogSourceControl, Log, TEXT("Git plugin '%s'"), *(Plugin->GetDescriptor().VersionName));
		}

		CheckGitAvailability();
	}

	// bForceConnection: not used anymore
}

void FGitSourceControlProvider::CheckGitAvailability()
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>("GitSourceControl");
	FString PathToGitBinary = GitSourceControl.AccessSettings().GetBinaryPath();
	if(PathToGitBinary.IsEmpty())
	{
		// Try to find Git binary, and update settings accordingly
		PathToGitBinary = GitSourceControlUtils::FindGitBinaryPath();
		if(!PathToGitBinary.IsEmpty())
		{
			GitSourceControl.AccessSettings().SetBinaryPath(PathToGitBinary);
		}
	}

	if(!PathToGitBinary.IsEmpty())
	{
		UE_LOG(LogSourceControl, Log, TEXT("Using '%s'"), *PathToGitBinary);
		bGitAvailable = GitSourceControlUtils::CheckGitAvailability(PathToGitBinary, &GitVersion);
		if(bGitAvailable)
		{
			CheckRepositoryStatus(PathToGitBinary);
		}
	}
	else
	{
		bGitAvailable = false;
	}
}

void FGitSourceControlProvider::CheckRepositoryStatus(const FString& InPathToGitBinary)
{
	// Find the path to the root Git directory (if any, else uses the GameDir)
	const FString PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	bGitRepositoryFound = GitSourceControlUtils::FindRootDirectory(PathToGameDir, PathToRepositoryRoot);
	if(bGitRepositoryFound)
	{
		// Get branch name
		bGitRepositoryFound = GitSourceControlUtils::GetBranchName(InPathToGitBinary, PathToRepositoryRoot, BranchName);
		if(bGitRepositoryFound)
		{
			GitSourceControlUtils::GetRemoteUrl(InPathToGitBinary, PathToRepositoryRoot, RemoteUrl);
		}
		else
		{
			UE_LOG(LogSourceControl, Error, TEXT("'%s' is not a valid Git repository"), *PathToRepositoryRoot);
		}
	}
	else
	{
		UE_LOG(LogSourceControl, Warning, TEXT("'%s' is not part of a Git repository"), *FPaths::GameDir());
	}

	// Get user name & email (of the repository, else from the global Git config)
	GitSourceControlUtils::GetUserConfig(InPathToGitBinary, PathToRepositoryRoot, UserName, UserEmail);
}

void FGitSourceControlProvider::Close()
{
	// clear the cache
	StateCache.Empty();

	bGitAvailable = false;
	bGitRepositoryFound = false;
	UserName.Empty();
	UserEmail.Empty();
}

TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> FGitSourceControlProvider::GetStateInternal(const FString& Filename, const bool bUsingGitLfsLocking)
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
		TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> NewState = MakeShareable( new FGitSourceControlState(Filename, bUsingGitLfsLocking) );
		StateCache.Add(Filename, NewState);
		return NewState;
	}
}

FText FGitSourceControlProvider::GetStatusText() const
{
	// TODO LFS IsUsingGitLfsLocking() should be cached in the Provider to avoir doing this here so frequently
//	FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>("GitSourceControl");
//	const bool bUsingGitLfsLocking = GitSourceControl.AccessSettings().IsUsingGitLfsLocking();

	FFormatNamedArguments Args;
	Args.Add( TEXT("RepositoryName"), FText::FromString(PathToRepositoryRoot) );
	Args.Add( TEXT("RemoteUrl"), FText::FromString(RemoteUrl) );
	Args.Add( TEXT("BranchName"), FText::FromString(BranchName) );
	Args.Add( TEXT("UserName"), FText::FromString(UserName) );
	Args.Add( TEXT("UserEmail"), FText::FromString(UserEmail) );

	return FText::Format( NSLOCTEXT("Status", "Provider: Git\nEnabledLabel", "Local repository: {RepositoryName}\nRemote origin: {RemoteUrl}\nBranch: {BranchName}\nUser: {UserName}\nE-mail: {UserEmail}"), Args );
}

/** Quick check if source control is enabled */
bool FGitSourceControlProvider::IsEnabled() const
{
	return bGitRepositoryFound;
}

/** Quick check if source control is available for use (useful for server-based providers) */
bool FGitSourceControlProvider::IsAvailable() const
{
	return bGitRepositoryFound; // TODO LFS : AND is connected (!bWorkingOffline)
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

	TArray<FString> AbsoluteFiles = SourceControlHelpers::AbsoluteFilenames(InFiles);

	if(InStateCacheUsage == EStateCacheUsage::ForceUpdate)
	{
		Execute(ISourceControlOperation::Create<FUpdateStatus>(), AbsoluteFiles);
	}

	// TODO LFS IsUsingGitLfsLocking() should be cached in the Provider to avoir doing this here so frequently
	FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>("GitSourceControl");
	const bool bUsingGitLfsLocking = GitSourceControl.AccessSettings().IsUsingGitLfsLocking();

	for(const auto& AbsoluteFile : AbsoluteFiles)
	{
		OutState.Add(GetStateInternal(*AbsoluteFile, bUsingGitLfsLocking));
	}

	return ECommandResult::Succeeded;
}

TArray<FSourceControlStateRef> FGitSourceControlProvider::GetCachedStateByPredicate(TFunctionRef<bool(const FSourceControlStateRef&)> Predicate) const
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

bool FGitSourceControlProvider::RemoveFileFromCache(const FString& Filename)
{
	return StateCache.Remove(Filename) > 0;
}

FDelegateHandle FGitSourceControlProvider::RegisterSourceControlStateChanged_Handle( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	return OnSourceControlStateChanged.Add( SourceControlStateChanged );
}

void FGitSourceControlProvider::UnregisterSourceControlStateChanged_Handle( FDelegateHandle Handle )
{
	OnSourceControlStateChanged.Remove( Handle );
}

ECommandResult::Type FGitSourceControlProvider::Execute( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency, const FSourceControlOperationComplete& InOperationCompleteDelegate )
{
	if(!IsEnabled() && !(InOperation->GetName() == "Connect")) // Only Connect operation allowed while not Enabled (Connected)
	{
		return ECommandResult::Failed;
	}

	TArray<FString> AbsoluteFiles = SourceControlHelpers::AbsoluteFilenames(InFiles);

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

	FGitSourceControlCommand* Command = new FGitSourceControlCommand(InOperation, Worker.ToSharedRef());
	Command->Files = AbsoluteFiles;
	Command->OperationCompleteDelegate = InOperationCompleteDelegate;

	// fire off operation
	if(InConcurrency == EConcurrency::Synchronous)
	{
		Command->bAutoDelete = false;

		UE_LOG(LogSourceControl, Log, TEXT("ExecuteSynchronousCommand(%s)"), *InOperation->GetName().ToString());
		return ExecuteSynchronousCommand(*Command, InOperation->GetInProgressString());
	}
	else
	{
		Command->bAutoDelete = true;

		UE_LOG(LogSourceControl, Log, TEXT("IssueAsynchronousCommand(%s)"), *InOperation->GetName().ToString());
		return IssueCommand(*Command);
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
	// TODO LFS IsUsingGitLfsLocking() should be cached in the Provider to avoir doing this here so frequently
	FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return GitSourceControl.AccessSettings().IsUsingGitLfsLocking(); // Git LFS Lock uses read-only state
}

bool FGitSourceControlProvider::UsesChangelists() const
{
	return false;
}

bool FGitSourceControlProvider::UsesCheckout() const
{
	// TODO LFS IsUsingGitLfsLocking() should be cached in the Provider to avoir doing this here so frequently
	FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return GitSourceControl.AccessSettings().IsUsingGitLfsLocking(); // Git LFS Lock uses read-only state
}

TSharedPtr<IGitSourceControlWorker, ESPMode::ThreadSafe> FGitSourceControlProvider::CreateWorker(const FName& InOperationName) const
{
	const FGetGitSourceControlWorker* Operation = WorkersMap.Find(InOperationName);
	if(Operation != nullptr)
	{
		return Operation->Execute();
	}
		
	return nullptr;
}

void FGitSourceControlProvider::RegisterWorker( const FName& InName, const FGetGitSourceControlWorker& InDelegate )
{
	WorkersMap.Add( InName, InDelegate );
}

void FGitSourceControlProvider::OutputCommandMessages(const FGitSourceControlCommand& InCommand) const
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

void FGitSourceControlProvider::Tick()
{	
	bool bStatesUpdated = false;
	for(int32 CommandIndex = 0; CommandIndex < CommandQueue.Num(); ++CommandIndex)
	{
		FGitSourceControlCommand& Command = *CommandQueue[CommandIndex];
		if(Command.bExecuteProcessed)
		{
			// Remove command from the queue
			CommandQueue.RemoveAt(CommandIndex);

			// TODO LFS Update bWorkingOffline Disconnect flag

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

TArray< TSharedRef<ISourceControlLabel> > FGitSourceControlProvider::GetLabels( const FString& InMatchingSpec ) const
{
	TArray< TSharedRef<ISourceControlLabel> > Tags;

	return Tags;
}

#if SOURCE_CONTROL_WITH_SLATE
TSharedRef<class SWidget> FGitSourceControlProvider::MakeSettingsWidget() const
{
	return SNew(SGitSourceControlSettings);
}
#endif

ECommandResult::Type FGitSourceControlProvider::ExecuteSynchronousCommand(FGitSourceControlCommand& InCommand, const FText& Task)
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

ECommandResult::Type FGitSourceControlProvider::IssueCommand(FGitSourceControlCommand& InCommand)
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
