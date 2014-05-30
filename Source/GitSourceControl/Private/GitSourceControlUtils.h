// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlState.h"
#include "GitSourceControlRevision.h"

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

namespace GitSourceControlUtils
{

/**
 * Find the path to the Git binary, looking in a few standard place (ThirdParty subdirectory and standard sSystem paths)
 * @returns the path to the Git binary if found, or the last path tested if no git found.
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
 * @param OutRepositoryRoot		The path to the root directory of the Git repository if found
 * @returns true if the command succeeded and returned no errors
*/
bool FindRootDirectory(const FString& InPathToGameDir, FString& OutRepositoryRoot);

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
 * Run a Git commit command by batches.
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
 * Run a Git status command and parse it.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InFiles				The files to be operated on
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunUpdateStatus(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InFiles, TArray<FString>& OutErrorMessages, TArray<FGitSourceControlState>& OutStates);

/**
 * Run a Git show command to dump the binary content of a revision into a file.
 *
 * @param	InPathToGitBinary	The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InParameter			The parameters to the Git show command (rev:path)
 * @param	InDumpFileName		The temporary file to dump the revision
 * @returns true if the command succeeded and returned no errors
*/
bool RunDumpToFile(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InParameter, const FString& InDumpFileName);

/**
 * Parse the array of strings results of a 'git log' command
 * @param	InResults			The results (from StdOut) as an array per-line
 * @param	OutHistory			The history of the file
 */
void ParseLogResults(const TArray<FString>& InResults, TGitSourceControlHistory& OutHistory);

/**
 * Helper function for various commands to update cached states.
 * @returns true if any states were updated
 */
bool UpdateCachedStates(const TArray<FGitSourceControlState>& InStates);

/**
 * Reads all pending data from an anonymous pipe, such as STDOUT or STDERROR of a process.
 *
 * @param InReadPipe The handle to the pipe to read from.
 *
 * @return A dynamic array containing all data.
*/
TArray<uint8> ReadPipeToArray(void* InReadPipe);

}
