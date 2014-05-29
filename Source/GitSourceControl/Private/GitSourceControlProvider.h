// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "ISourceControlProvider.h"
#include "IGitSourceControlWorker.h"
#include "GitSourceControlState.h"

DECLARE_DELEGATE_RetVal(FGitSourceControlWorkerRef, FGetGitSourceControlWorker)

class FGitSourceControlProvider : public ISourceControlProvider
{
public:
	/** Constructor */
	FGitSourceControlProvider() 
		: bGitAvailable(false)
	{
	}

	/* ISourceControlProvider implementation */
	virtual void Init(bool bForceConnection = true) OVERRIDE;
	virtual void Close() OVERRIDE;
	virtual FText GetStatusText() const OVERRIDE;
	virtual bool IsEnabled() const OVERRIDE;
	virtual bool IsAvailable() const OVERRIDE;
	virtual const FName& GetName(void) const OVERRIDE;
	virtual ECommandResult::Type GetState( const TArray<FString>& InFiles, TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, EStateCacheUsage::Type InStateCacheUsage ) OVERRIDE;
	virtual void RegisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged ) OVERRIDE;
	virtual void UnregisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged ) OVERRIDE;
	virtual ECommandResult::Type Execute( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency = EConcurrency::Synchronous, const FSourceControlOperationComplete& InOperationCompleteDelegate = FSourceControlOperationComplete() ) OVERRIDE;
	virtual bool CanCancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation ) const OVERRIDE;
	virtual void CancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation ) OVERRIDE;
	virtual bool UsesLocalReadOnlyState() const OVERRIDE;
	virtual void Tick() OVERRIDE;
	virtual TArray< TSharedRef<class ISourceControlLabel> > GetLabels( const FString& InMatchingSpec ) const OVERRIDE;
	virtual TSharedRef<class SWidget> MakeSettingsWidget() const OVERRIDE;

	/**
	 * Run a Git "version" command to check the availability of the binary.
	 */
	void CheckGitAvailability();

	/** Get the path to the root of the Git repository: can be the GameDir itself, or any parent directory */
	inline const FString& GetPathToRepositoryRoot() const
	{
		return PathToRepositoryRoot;
	}

	/** Get the path to the Game directory: shall be inside of the Git repository */
	inline const FString& GetPathToGameDir() const
	{
		return PathToGameDir;
	}

	/** Helper function used to update state cache */
	TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> GetStateInternal(const FString& Filename);

	/**
	 * Register a worker with the provider.
	 * This is used internally so the provider can maintain a map of all available operations.
	 */
	void RegisterWorker( const FName& InName, const FGetGitSourceControlWorker& InDelegate );

private:

	/** Is git binary found and working. */
	bool bGitAvailable;

	/** Helper function for Execute() */
	TSharedPtr<class IGitSourceControlWorker, ESPMode::ThreadSafe> CreateWorker(const FName& InOperationName) const;

	/** Helper function for running command synchronously. */
	ECommandResult::Type ExecuteSynchronousCommand(class FGitSourceControlCommand& InCommand, const FText& Task);
	/** Issue a command asynchronously if possible. */
	ECommandResult::Type IssueCommand(class FGitSourceControlCommand& InCommand);

	/** Output any messages this command holds */
	void OutputCommandMessages(const class FGitSourceControlCommand& InCommand) const;

	/** Path to the root of the Git repository: can be the GameDir itself, or any parent directory (found by the "Connect" operation) */
	FString PathToRepositoryRoot;

	/** Path to the Game directory: shall be inside of the Git repository */
	FString PathToGameDir;

	/** Name of the current branch (found by the "Connect" operation) */
	FString BranchName;

	/** State cache */
	TMap<FString, TSharedRef<class FGitSourceControlState, ESPMode::ThreadSafe> > StateCache;

	/** The currently registered source control operations */
	TMap<FName, FGetGitSourceControlWorker> WorkersMap;

	/** Queue for commands given by the main thread */
	TArray < FGitSourceControlCommand* > CommandQueue;

	/** For notifying when the source control states in the cache have changed */
	FSourceControlStateChanged OnSourceControlStateChanged;
};
