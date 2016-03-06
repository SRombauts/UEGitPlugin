// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "IGitSourceControlDevWorker.h"
#include "GitSourceControlDevState.h"
#include "GitSourceControlDevRevision.h"

/** Called when first activated on a project, and then at project load time.
 *  Look for the root directory of the git repository (where the ".git/" subdirectory is located). */
class FGitConnectWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitConnectWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;
};

/** Commit (check-in) a set of file to the local depot. */
class FGitCheckInWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitCheckInWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlDevState> States;
};

/** Add an untraked file to source control (so only a subset of the git add command). */
class FGitMarkForAddWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitMarkForAddWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlDevState> States;
};

/** Delete a file and remove it from source control. */
class FGitDeleteWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitDeleteWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Map of filenames to Git state */
	TArray<FGitSourceControlDevState> States;
};

/** Revert any change to a file to its state on the local depot. */
class FGitRevertWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitRevertWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Map of filenames to Git state */
	TArray<FGitSourceControlDevState> States;
};

/** @todo: git fetch remote(s) to be able to show files not up-to-date with the serveur
class FGitSyncWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitSyncWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/// Map of filenames to Git state
	TArray<FGitSourceControlDevState> States;
};
*/

/** Get source control status of files on local working copy. */
class FGitUpdateStatusWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitUpdateStatusWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlDevState> States;

	/** Map of filenames to history */
	TMap<FString, TGitSourceControlDevHistory> Histories;
};

/** Copy or Move operation on a single file */
class FGitCopyWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitCopyWorker() {}
	// IGitSourceControlDevWorker interface
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlDevState> OutStates;
};

/** git add to mark a conflict as resolved */
class FGitResolveWorker : public IGitSourceControlDevWorker
{
public:
	virtual ~FGitResolveWorker() {}
	virtual FName GetName() const override;
	virtual bool Execute(class FGitSourceControlDevCommand& InCommand) override;
	virtual bool UpdateStates() const override;
	
private:
	/** Temporary states for results */
	TArray<FGitSourceControlDevState> States;
};
