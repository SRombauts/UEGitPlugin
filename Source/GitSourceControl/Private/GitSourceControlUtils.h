// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlState.h"
#include "GitSourceControlRevision.h"

namespace GitSourceControlUtils
{

/**
 * Find the path to the Git binary, looking in a few standard place (ThirdParty subdirectory and standard sSystem paths)
 * @returns the path to the Git binary if found, or the last path tested if no git found.
 */
FString FindGitBinaryPath();

/**
 * Run a Git "version" command to check the availability of the binary.
 * @param InGitBinaryPath		The path to the Git binary
 * @returns true if the command succeeded and returned no errors
 */
bool CheckGitAvailability(const FString& InGitBinaryPath);

/**
 * Run a Git command - output is a string TArray.
 *
 * @param	InGitBinaryPath		The path to the Git binary
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	InCommand			The Git command - e.g. status
 * @param	InParameters		The parameters to the Git command
 * @param	InFiles				The files to be operated on
 * @param	OutResults			The results (from StdOut) as an array per-line
 * @param	OutErrorMessages	Any errors (from StdErr) as an array per-line
 * @returns true if the command succeeded and returned no errors
 */
bool RunCommand(const FString& InGitBinaryPath, const FString& InRepositoryRoot, const FString& InCommand, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

/**
 * Parse the array of strings results of a 'git log' command
 * @param	InResults			The results (from StdOut) as an array per-line
 * @param	OutHistory			The history of the file
 */
void ParseLogResults(const TArray<FString>& InResults, TGitSourceControlHistory& OutHistory);

/**
 * Parse the array of strings results of a 'git status' command
 * @param	InFiles				The files that have been operated on
 * @param	InResults			The results (from StdOut) as an array per-line
 * @param	OutStates			The current state of the files
 */
void ParseStatusResults(const TArray<FString>& InFiles, const TArray<FString>& InResults, TArray<FGitSourceControlState>& OutStates);

/**
 * Helper function for various commands to update cached states.
 * @returns true if any states were updated
 */
bool UpdateCachedStates(const TArray<FGitSourceControlState>& InStates);

}
