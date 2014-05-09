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
	const int32 MaxFilesPerBatch = 20;
}

namespace GitSourceControlUtils
{

#if PLATFORM_WINDOWS
	// NOTE using only "git" (or "git.exe") relying on the "PATH" envvar does not always work as expected,
	// depending on the option selected at installation time:
	// If the PATH contains "git/cmd" instead of "git/bin/" (with all Linux shell commands),
	// "git.exe" launch "git/cmd/git.exe" that redirect to "git/bin/git.exe" and ExecProcess() is unable to catch its outputs streams.
	//  const FString GitBinaryPath = TEXT("git");	// NOK, start a shell! match git.exe on Windows

	// Under Windows, we use the third party "msysgit PortableGit" https://code.google.com/p/msysgit/downloads/list?can=1&q=PortableGit
	// NOTE: Win32 platform subdirectory as there is no Git 64bit build available
	static FString GitBinaryPath(FPaths::EngineDir() / TEXT("Binaries/ThirdParty/git/Win32/bin") / TEXT("git.exe"));
#else
	static FString GitBinaryPath(TEXT("/usr/bin/git"));
#endif

const FString& GetGitBinaryPath()
{
	return GitBinaryPath;
}

void SetGitBinaryPath(const FString& InGitBinaryPath)
{
	GitBinaryPath = InGitBinaryPath;
}


static bool RunCommandInternal(const FString& InCommand, const TArray<FString>& InParameters, const TArray<FString>& InFiles, const FString& InRepositoryRoot, FString& OutResults, FString& OutErrors)
{
	int32	ReturnCode = 0;
	FString FullCommand;

	if (!InRepositoryRoot.IsEmpty())
	{
		// Specify the working copy (the root) of the git repository (before the command itself)
		FullCommand  = TEXT("--work-tree=\"");
		FullCommand += InRepositoryRoot;
		// and the ".git" subdirectory in it (before the command itself)
		FullCommand += TEXT("\" --git-dir=\"");
		FullCommand += InRepositoryRoot;
		FullCommand += TEXT(".git\" ");
	}
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
		FullCommand += TEXT(" \"");
		FullCommand += InFiles[Index];
		FullCommand += TEXT("\"");
	}
	// Also, fit auto-detect non-interactive condition (no connected standard input/output streams)

	// @todo: temporary debug logs
	UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternal: Attempting '%s %s'"), *GitBinaryPath, *FullCommand);
	FPlatformProcess::ExecProcess(*GitBinaryPath, *FullCommand, &ReturnCode, &OutResults, &OutErrors);
    UE_LOG(LogSourceControl, Log, TEXT("RunCommandInternal: ExecProcess ReturnCode=%d OutResults='%s'"), ReturnCode, *OutResults);
	if(!OutErrors.IsEmpty())
	{
		UE_LOG(LogSourceControl, Error, TEXT("RunCommandInternal: ExecProcess ReturnCode=%d OutErrors='%s'"), ReturnCode, *OutErrors);
	}

	return ReturnCode == 0;
}

bool CheckGitAvailability()
{
	bool bGitAvailable = false;

	FString InfoMessages;
	FString ErrorMessages;
	bGitAvailable = GitSourceControlUtils::RunCommandInternal(TEXT("version"), TArray<FString>(), TArray<FString>(), FString(), InfoMessages, ErrorMessages);
	if (bGitAvailable && ErrorMessages.IsEmpty() && (!InfoMessages.IsEmpty()))
	{
		if(InfoMessages.Contains("git"))
		{
			bGitAvailable = true;
		}
	}

	// @todo also check Git config user.name & user.email

	return bGitAvailable;
}

bool RunCommand(const FString& InCommand, const TArray<FString>& InParameters, const TArray<FString>& InFiles, const FString& InRepositoryRoot, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	bool bResult = true;

	if(InFiles.Num() > GitSourceControlConstants::MaxFilesPerBatch)
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
			FString Errors;
			bResult &= RunCommandInternal(InCommand, InParameters, FilesInBatch, InRepositoryRoot, Results, Errors);
			Results.ParseIntoArray(&OutResults, TEXT("\n"), true);
			Errors.ParseIntoArray(&OutErrorMessages, TEXT("\n"), true);
		}
	}
	else
	{
		FString Results;
		FString Errors;
		bResult &= RunCommandInternal(InCommand, InParameters, InFiles, InRepositoryRoot, Results, Errors);
		Results.ParseIntoArray(&OutResults, TEXT("\n"), true);
		Errors.ParseIntoArray(&OutErrorMessages, TEXT("\n"), true);
	}

	return bResult;
}

/** Match the relative filename of a Git status result with a provided absolute filename */
class FGitStatusFileMatcher
{
public:
	FGitStatusFileMatcher(const FString& InAbsoluteFilename)
		: AbsoluteFilename(InAbsoluteFilename)
	{
	}

	bool Matches(const FString& InResult) const
	{
		// Extract the relative filename from the Git status result
		FString RelativeFilename = InResult.RightChop(3);
		return AbsoluteFilename.Contains(RelativeFilename);
	}

private:
	const FString& AbsoluteFilename;
};

/** 
 * Extract and interpret the file state from the given Git status result.
 * @see file:///C:/Program%20Files%20(x86)/Git/doc/git/html/git-status.html
 * ' ' = unmodified
 * 'M' = modified
 * 'A' = added
 * 'D' = deleted
 * 'R' = renamed
 * 'C' = copied
 * 'U' = updated but unmerged
 * '?' = unknown/untracked
 * '!' = ignored
 */
class FGitStatusParser
{
public:
	FGitStatusParser(const FString& InResult)
	{
		//FString Filename = InResult.RightChop(3);
		TCHAR IndexState = InResult[0];
		TCHAR WCopyState = InResult[1];
		if(	  (IndexState == 'U' || WCopyState == 'U')
		   || (IndexState == 'A' && WCopyState == 'A')
		   || (IndexState == 'D' && WCopyState == 'D'))
		{
			// "Unmerged" conflict cases are generaly marked with a "U", 
			// but there are also the special cases of both "A"dded, or both "D"eleted
			State = EWorkingCopyState::Conflicted;
		}
		else if(IndexState == 'A')
		{
			State = EWorkingCopyState::Added;
		}
		else if(IndexState == 'D')
		{
			State = EWorkingCopyState::Deleted;
		}
		else if(WCopyState == 'D')
		{
			State = EWorkingCopyState::Missing;
		}
		else if(IndexState == 'M' || WCopyState == 'M')
		{
			State = EWorkingCopyState::Modified;
		}
		else if(IndexState == 'R')
		{
			State = EWorkingCopyState::Renamed;
		}
		else if(IndexState == 'C')
		{
			State = EWorkingCopyState::Copied;
		}
		else if(IndexState == '?' || WCopyState == '?')
		{
			State = EWorkingCopyState::NotControlled;
		}
		else if(IndexState == '!' || WCopyState == '!')
		{
			State = EWorkingCopyState::Ignored;
		}
		else
		{
			// "Pristine"/Clean/Unmodified never yield a status
			State = EWorkingCopyState::Unknown;
		}
	}

	EWorkingCopyState::Type State;
};


void ParseStatusResults(const TArray<FString>& InFiles, const TArray<FString>& InResults, TArray<FString>& OutErrorMessages, TArray<FGitSourceControlState>& OutStates)
{
	for(int32 IdxFile = 0; IdxFile < InFiles.Num(); IdxFile++)
	{
		FGitSourceControlState FileState(InFiles[IdxFile]);
		FGitStatusFileMatcher FileMatcher(InFiles[IdxFile]);
		int32 IdxResult = InResults.FindMatch(FileMatcher);
		if(IdxResult != INDEX_NONE)
		{
			// File found in status results; only the case for "changed" files
			FGitStatusParser StatusParser(InResults[IdxResult]);
			FileState.WorkingCopyState = StatusParser.State;
		}
		else
		{
			// File not found in status
			if(FPaths::FileExists(InFiles[IdxFile]))
			{
				// usually means the file is unchanged,
				FileState.WorkingCopyState = EWorkingCopyState::Pristine;
			}
			else
			{
				// but also the case for newly created content: there is no file on disk until the content is saved for the first time
				FileState.WorkingCopyState = EWorkingCopyState::NotControlled;
			}
		}
		FileState.TimeStamp.Now();
		OutStates.Add(FileState);
	}
}

bool UpdateCachedStates(const TArray<FGitSourceControlState>& InStates)
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>( "GitSourceControl" );
	FGitSourceControlProvider& Provider = GitSourceControl.GetProvider();
	int NbStatesUpdated = 0;

	for(int StatusIndex = 0; StatusIndex < InStates.Num(); StatusIndex++)
	{
		const FGitSourceControlState& InState = InStates[StatusIndex];
		TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> State = Provider.GetStateInternal(InState.LocalFilename);
		if(State->WorkingCopyState != InState.WorkingCopyState)
		{
			State->WorkingCopyState = InState.WorkingCopyState;
			State->TimeStamp = FDateTime::Now();
			NbStatesUpdated++;
		}
	}

	return (NbStatesUpdated > 0);
}
}
