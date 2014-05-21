// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "IGitSourceControlWorker.h"
#include "GitSourceControlState.h"
#include "GitSourceControlRevision.h"

/** Called when first activated on a project, and then at project load time.
 *  Look for the root directory of the git repository (where the ".git/" subdirectory is located). */
class FGitConnectWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;
};

/** @todo
class FGitCheckOutWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Temporary states for results *
	TArray<FGitSourceControlState> States;
};
*/

/** Commit (check-in) a set of file to the local depot. */
class FGitCheckInWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlState> States;
};

/** Add an untraked file to source control (so only a subset of the git add command). */
class FGitMarkForAddWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlState> States;
};

/** Delete a file and remove it from source control. */
class FGitDeleteWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Map of filenames to Git state */
	TArray<FGitSourceControlState> States;
};

/** Revert any change to a file to its state on the local depot. */
class FGitRevertWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Map of filenames to Git state */
	TArray<FGitSourceControlState> States;
};

/** @todo
class FGitSyncWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Map of filenames to Git state *
	TArray<FGitSourceControlState> States;
};
*/

/** Get source control status of files on local working copy. */
class FGitUpdateStatusWorker : public IGitSourceControlWorker
{
public:
	// IGitSourceControlWorker interface
	virtual FName GetName() const OVERRIDE;
	virtual bool Execute(class FGitSourceControlCommand& InCommand) OVERRIDE;
	virtual bool UpdateStates() const OVERRIDE;

public:
	/** Temporary states for results */
	TArray<FGitSourceControlState> States;

	/** Map of filenames to history */
	TMap<FString, TGitSourceControlHistory> Histories;
};
