// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlDevState.h"
#include "GitSourceControlDevRevision.h"

class FGitSourceControlDevCommand;

/**
 * Helper struct for maintaining temporary files for passing to commands
 */
class FScopedTempFile
{
public:

	/** Constructor - open & write string to temp file */
	FScopedTempFile(const FText& InText);

	/** Destructor - delete temp file */
	~FScopedTempFile();

	/** Get the filename of this temp file - empty if it failed to be created */
	const FString& GetFilename() const;

private:
	/** The filename we are writing to */
	FString Filename;
};

namespace GitSourceControlDevUtils
{

/**
 * Find the path to the Git binary, looking into a few places (standalone Git install, and other common tools embedding Git)
 * @returns the path to the Git binary if found, or an empty string.
 */
FString FindGitBinaryPath();

/**
 * Run a Git "version" command to check the availability of the binary.
 * @param InPathToGitBinary		The path to the Git binary
 * @returns true if the command succeeded and returned no errors
 */
bool CheckGitAvailability(const FString& InPathToGitBinary);

/**
 * Find the root of the Git repository, looking from the GameDir and upward in its parent directories
 * @param InPathToGameDir		The path to the Game Directory
 * @param OutRepositoryRoot		The path to the root directory of the Git repository if found, else the path to the GameDir
 * @returns true if the command succeeded and returned no errors
 */
bool FindRootDirectory(const FString& InPathToGameDir, FString& OutRepositoryRoot);

/**
 * Get Git config user.name & user.email
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	OutUserName			Name of the Git user configured for this repository (or globaly)
 * @param	OutEmailName		E-mail of the Git user configured for this repository (or globaly)
 */
void GetUserConfig(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutUserName, FString& OutUserEmail);

/**
 * Get Git current checked-out branch
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	OutBranchName		Name of the current checked-out branch (if any, ie. not in detached HEAD)
 */
void GetBranchName(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutBranchName);

/**
 * Run a Git command - output is a string TArray.
 *
 * @param	InCommand			The Git command - e.g. commit
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InParameters		The parameters to the Git command
 * @param	InFiles				The files to be operated on
 * @param	OutResults			The results (from StdOut) as an array per-line
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunCommand(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

/**
 * Run a Git "commit" command by batches.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InParameter			The parameters to the Git commit command
 * @param	InFiles				The files to be operated on
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunCommit(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

/**
 * Run a Git "status" command and parse it.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InFiles				The files to be operated on
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunUpdateStatus(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InFiles, TArray<FString>& OutErrorMessages, TArray<FGitSourceControlDevState>& OutStates);

/**
 * Run a Git "show" command to dump the binary content of a revision into a file.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InParameter			The parameters to the Git show command (rev:path)
 * @param	InDumpFileName		The temporary file to dump the revision
 * @returns true if the command succeeded and returned no errors
*/
bool RunDumpToFile(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InParameter, const FString& InDumpFileName);

/**
 * Run a Git "log" command and parse it.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InFile				The file to be operated on
 * @param	bMergeConflict		In case of a merge conflict, we also need to get the tip of the "remote branch" (MERGE_HEAD) before the log of the "current branch" (HEAD)
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @param	OutHistory			The history of the file
 */
bool RunGetHistory(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InFile, bool bMergeConflict, TArray<FString>& OutErrorMessages, TGitSourceControlDevHistory& OutHistory);

/**
 * Helper function for various commands to update cached states.
 * @returns true if any states were updated
 */
bool UpdateCachedStates(const TArray<FGitSourceControlDevState>& InStates);

/** 
 * Remove redundant errors (that contain a particular string) and also
 * update the commands success status if all errors were removed.
 */
void RemoveRedundantErrors(FGitSourceControlDevCommand& InCommand, const FString& InFilter);

}
