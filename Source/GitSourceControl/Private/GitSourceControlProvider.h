// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "ISourceControlOperation.h"
#include "ISourceControlState.h"
#include "ISourceControlProvider.h"
#include "IGitSourceControlWorker.h"
#include "GitSourceControlState.h"
#include "GitSourceControlMenu.h"
#include "GitSourceControlConsole.h"

#include "Runtime/Launch/Resources/Version.h"

class FGitSourceControlCommand;

DECLARE_DELEGATE_RetVal(FGitSourceControlWorkerRef, FGetGitSourceControlWorker)

/// Git version and capabilites extracted from the string "git version 2.11.0.windows.3"
struct FGitVersion
{
	// Git version extracted from the string "git version 2.11.0.windows.3" (Windows) or "git version 2.11.0" (Linux/Mac/Cygwin/WSL)
	int Major;   // 2	Major version number
	int Minor;   // 11	Minor version number
	int Patch;   // 0	Patch/bugfix number
	int Windows; // 3	Windows specific revision number (under Windows only)

	uint32 bHasCatFileWithFilters : 1;
	uint32 bHasGitLfs : 1;
	uint32 bHasGitLfsLocking : 1;

	FGitVersion() 
		: Major(0)
		, Minor(0)
		, Patch(0)
		, Windows(0)
		, bHasCatFileWithFilters(false)
		, bHasGitLfs(false)
		, bHasGitLfsLocking(false)
	{
	}

	inline bool IsGreaterOrEqualThan(int InMajor, int InMinor) const
	{
		return (Major > InMajor) || (Major == InMajor && (Minor >= InMinor));
	}
};

class FGitSourceControlProvider : public ISourceControlProvider
{
public:
	/** Constructor */
	FGitSourceControlProvider()
	{
	}

	/* ISourceControlProvider implementation */
	virtual void Init(bool bForceConnection = true) override;
	virtual void Close() override;
	virtual FText GetStatusText() const override;
	virtual bool IsEnabled() const override;
	virtual bool IsAvailable() const override;
	virtual const FName& GetName(void) const override;
	virtual bool QueryStateBranchConfig(const FString& ConfigSrc, const FString& ConfigDest) override { return false; }
	virtual void RegisterStateBranches(const TArray<FString>& BranchNames, const FString& ContentRoot) override {}
	virtual int32 GetStateBranchIndex(const FString& InBranchName) const override { return INDEX_NONE; }
	virtual ECommandResult::Type GetState(const TArray<FString>& InFiles, TArray<FSourceControlStateRef>& OutState, EStateCacheUsage::Type InStateCacheUsage) override;
#if ENGINE_MAJOR_VERSION == 5
	virtual ECommandResult::Type GetState(const TArray<FSourceControlChangelistRef>& InChangelists, TArray<FSourceControlChangelistStateRef>& OutState, EStateCacheUsage::Type InStateCacheUsage) override;
#endif
	virtual TArray<FSourceControlStateRef> GetCachedStateByPredicate(TFunctionRef<bool(const FSourceControlStateRef&)> Predicate) const override;
	virtual FDelegateHandle RegisterSourceControlStateChanged_Handle(const FSourceControlStateChanged::FDelegate& SourceControlStateChanged) override;
	virtual void UnregisterSourceControlStateChanged_Handle(FDelegateHandle Handle) override;
#if ENGINE_MAJOR_VERSION == 5
	virtual ECommandResult::Type Execute(const FSourceControlOperationRef& InOperation, FSourceControlChangelistPtr InChangelist, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency = EConcurrency::Synchronous, const FSourceControlOperationComplete& InOperationCompleteDelegate = FSourceControlOperationComplete() ) override;
#else
	virtual ECommandResult::Type Execute(const FSourceControlOperationRef& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency = EConcurrency::Synchronous, const FSourceControlOperationComplete& InOperationCompleteDelegate = FSourceControlOperationComplete() ) override;
#endif
	virtual bool CanCancelOperation(const FSourceControlOperationRef& InOperation) const override;
	virtual void CancelOperation(const FSourceControlOperationRef& InOperation) override;
	virtual bool UsesLocalReadOnlyState() const override;
	virtual bool UsesChangelists() const override;
	virtual bool UsesCheckout() const override;
	virtual bool UsesFileRevisions() const; /* override				NOTE: added in UE5.1 */
	virtual TOptional<bool> IsAtLatestRevision() const; /* override	NOTE: added in UE5.1 */
	virtual TOptional<int> GetNumLocalChanges() const; /* override	NOTE: added in UE5.1 */
	virtual bool UsesUncontrolledChangelists() const; /* override   NOTE: added in UE5.2 */
	virtual bool UsesSnapshots() const; /* override   NOTE: added in UE5.2 */
	virtual bool AllowsDiffAgainstDepot() const; /* override   NOTE: added in UE5.2 */
	virtual void Tick() override;
	virtual TArray< TSharedRef<class ISourceControlLabel> > GetLabels(const FString& InMatchingSpec) const override;
#if ENGINE_MAJOR_VERSION == 5
	virtual TArray<FSourceControlChangelistRef> GetChangelists(EStateCacheUsage::Type InStateCacheUsage) override;
#endif
#if SOURCE_CONTROL_WITH_SLATE
	virtual TSharedRef<class SWidget> MakeSettingsWidget() const override;
#endif

	using ISourceControlProvider::Execute;

	/**
	 * Check configuration, else standard paths, and run a Git "version" command to check the availability of the binary.
	 */
	void CheckGitAvailability();

	/**
	 * Find the .git/ repository and check it's status.
	 */
	void CheckRepositoryStatus(const FString& InPathToGitBinary);

	/** Is git binary found and working. */
	inline bool IsGitAvailable() const
	{
		return bGitAvailable;
	}

	/** Git version for feature checking */
	inline const FGitVersion& GetGitVersion() const
	{
		return GitVersion;
	}

	/** Get the path to the root of the Git repository: can be the ProjectDir itself, or any parent directory */
	inline const FString& GetPathToRepositoryRoot() const
	{
		return PathToRepositoryRoot;
	}

	/** Git config user.name */
	inline const FString& GetUserName() const
	{
		return UserName;
	}

	/** Git config user.email */
	inline const FString& GetUserEmail() const
	{
		return UserEmail;
	}

	/** Git remote origin url */
	inline const FString& GetRemoteUrl() const
	{
		return RemoteUrl;
	}

	/** Helper function used to update state cache */
	TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> GetStateInternal(const FString& Filename);

	/**
	 * Register a worker with the provider.
	 * This is used internally so the provider can maintain a map of all available operations.
	 */
	void RegisterWorker( const FName& InName, const FGetGitSourceControlWorker& InDelegate );

	/** Remove a named file from the state cache */
	bool RemoveFileFromCache(const FString& Filename);

	/** Get files in cache */
	TArray<FString> GetFilesInCache();

private:

	/** Is git binary found and working. */
	bool bGitAvailable = false;

	/** Is git repository found. */
	bool bGitRepositoryFound = false;

	/** Is LFS File Locking enabled? */
	bool bUsingGitLfsLocking = false;

	/** Helper function for Execute() */
	TSharedPtr<class IGitSourceControlWorker, ESPMode::ThreadSafe> CreateWorker(const FName& InOperationName) const;

	/** Helper function for running command synchronously. */
	ECommandResult::Type ExecuteSynchronousCommand(class FGitSourceControlCommand& InCommand, const FText& Task);
	/** Issue a command asynchronously if possible. */
	ECommandResult::Type IssueCommand(class FGitSourceControlCommand& InCommand);

	/** Output any messages this command holds */
	void OutputCommandMessages(const class FGitSourceControlCommand& InCommand) const;

	/** Update repository status on Connect and UpdateStatus operations */
	void UpdateRepositoryStatus(const class FGitSourceControlCommand& InCommand);

	/** Path to the root of the Git repository: can be the ProjectDir itself, or any parent directory (found by the "Connect" operation) */
	FString PathToRepositoryRoot;

	/** Git config user.name (from local repository, else globally) */
	FString UserName;

	/** Git config user.email (from local repository, else globally) */
	FString UserEmail;

	/** Name of the current branch */
	FString BranchName;

	/** URL of the "origin" defaut remote server */
	FString RemoteUrl;

	/** Current Commit full SHA1 */
	FString CommitId;

	/** Current Commit description's Summary */
	FString CommitSummary;

	/** State cache */
	TMap<FString, TSharedRef<class FGitSourceControlState, ESPMode::ThreadSafe> > StateCache;

	/** The currently registered source control operations */
	TMap<FName, FGetGitSourceControlWorker> WorkersMap;

	/** Queue for commands given by the main thread */
	TArray < FGitSourceControlCommand* > CommandQueue;

	/** For notifying when the source control states in the cache have changed */
	FSourceControlStateChanged OnSourceControlStateChanged;

	/** Git version for feature checking */
	FGitVersion GitVersion;

	/** Source Control Menu Extension */
	FGitSourceControlMenu GitSourceControlMenu;

	/** Source Control Console commands */
	FGitSourceControlConsole GitSourceControlConsole;
};
