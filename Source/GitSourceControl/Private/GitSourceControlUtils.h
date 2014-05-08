// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlState.h"

namespace GitSourceControlUtils
{

/**
 * Run a Git command - output is a string TArray.
 *
 * @param	InCommand			The Git command - e.g. status
 * @param	InParameters		The parameters to the Git command
 * @param	InFiles				The files to be operated on
 * @param	InRepositoryRoot	The Git repository from where to run the command - usually the Game directory (can be empty)
 * @param	OutResults			The results (from StdOut) as an array per-line
 * @param	OutErrorMessages	Any errors (from StdErr)
 * @returns true if the command succeeded and returned no errors
 */
bool RunCommand(const FString& InCommand, const TArray<FString>& InParameters, const TArray<FString>& InFiles, const FString& InRepositoryRoot, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages);

}
