// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlUtils.h"
#include "GitSourceControlState.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlCommand.h"

namespace GitSourceControlConstants
{
	/** The maximum number of files we submit in a single Git command */
	const int32 MaxFilesPerBatch = 50;
}

namespace GitSourceControlUtils
{

static bool RunCommandInternal(const FString& InCommand, const FString& InRepositoryRoot, const TArray<FString>& InFiles, const TArray<FString>& InParameters, FString& OutResults, TArray<FString>& OutErrorMessages)
{
	int32 ReturnCode = 0;
	FString StdError;
	FString FullCommand;

	// Specify the working copy (the root) of the git repository (before the command itself)
	FullCommand  = TEXT(" --work-tree=\"");
	FullCommand += InRepositoryRoot;
	// and the ".git" subdirectory in it (before the command itself)
	FullCommand += TEXT("\" --git-dir=\"");
	FullCommand += InRepositoryRoot;
	FullCommand += TEXT(".git\" ");
    // then the git command itself ("status", "log", "commit"...)
    FullCommand += InCommand;

    // Append to the command all parameters, and then finalla the files
	for (int32 Index = 0; Index < InParameters.Num(); Index++)
	{
		FullCommand += TEXT(" ");
		FullCommand += InParameters[Index];
	}
	for (int32 Index = 0; Index < InFiles.Num(); Index++)
	{
		FullCommand += TEXT(" ");
		FullCommand += InFiles[Index];
	}

	// Git auto-detect non-interactive condition (no connected standard input/output streams)
	
#if PLATFORM_WINDOWS
	// NOTE using only "git" (or "git.exe") relying on the "PATH" envvar does not always work as expected,
	// depending on the option selected at installation time:
	// If the PATH contains "git/cmd" instead of "git/bin/" (with all Linux shell commands),
	// "git.exe" launch "git/cmd/git.exe" that redirect to "git/bin/git.exe" and ExecProcess() is unable to catch its outputs streams.
	//  const FString GitBinaryPath = TEXT("git");	// NOK, start a shell! match git.exe on Windows

	// Under Windows, we use the third party "msysgit PortableGit" https://code.google.com/p/msysgit/downloads/list?can=1&q=PortableGit
	// NOTE: Win32 platform subdirectory as there is no Git 64bit build available
	const FString GitBinaryPath = FPaths::EngineDir() / TEXT("Binaries/ThirdParty/git/Win32/bin") / TEXT("git.exe");
#else
	const FString GitBinaryPath = TEXT("/usr/bin/git");
#endif

	// TODO: test log
	UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternal: Attempting '%s %s'"), *GitBinaryPath, *FullCommand);
	FPlatformProcess::ExecProcess(*GitBinaryPath, *FullCommand, &ReturnCode, &OutResults, &StdError);
    UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternal: ExecProcess ReturnCode=%d OutResults='%s'"), ReturnCode, *OutResults);
	if (!StdError.IsEmpty())
	{
		UE_LOG(LogSourceControl, Error, TEXT("RunCommandInternal: ExecProcess ReturnCode=%d StdError='%s'"), ReturnCode, *StdError);

		// parse output & errors
		TArray<FString> Errors;
		StdError.ParseIntoArray(&Errors, TEXT("\n"), true);
		OutErrorMessages.Append(Errors);
	}

	return ReturnCode == 0;
}

bool RunCommand(const FString& InCommand, const FString& InRepositoryRoot, const TArray<FString>& InFiles, const TArray<FString>& InParameters, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	bool bResult = true;

	if(InFiles.Num() > 0)
	{
		// Batch files up so we dont exceed command-line limits
		int32 FileCount = 0;
		while(FileCount < InFiles.Num())
		{
			TArray<FString> FilesInBatch;
			for(int32 FileIndex = 0; FileIndex < InFiles.Num() && FileIndex < GitSourceControlConstants::MaxFilesPerBatch; FileIndex++, FileCount++)
			{
				FilesInBatch.Add(InFiles[FileIndex]);
			}

			FString Results;
            bResult &= RunCommandInternal(InCommand, InRepositoryRoot, FilesInBatch, InParameters, Results, OutErrorMessages);
			Results.ParseIntoArray(&OutResults, TEXT("\n"), true);
		}
	}
	else
	{
		FString Results;
        bResult &= RunCommandInternal(InCommand, InRepositoryRoot, InFiles, InParameters, Results, OutErrorMessages);
		Results.ParseIntoArray(&OutResults, TEXT("\n"), true);
	}

	return bResult;
}

}
