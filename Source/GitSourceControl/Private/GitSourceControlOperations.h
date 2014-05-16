// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "IGitSourceControlWorker.h"
#include "GitSourceControlState.h"
#include "GitSourceControlRevision.h"

/** @todo doc */
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

/** @todo
class FGitCheckInWorker : public IGitSourceControlWorker
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

/** @todo doc */
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

/** @todo doc */
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

/** @todo doc */
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

/** @todo doc */
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
