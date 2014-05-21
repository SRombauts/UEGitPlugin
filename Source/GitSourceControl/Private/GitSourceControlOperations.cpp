// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlOperations.h"
#include "GitSourceControlState.h"
#include "GitSourceControlCommand.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlUtils.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

FName FGitConnectWorker::GetName() const
{
	return "Connect";
}

bool FGitConnectWorker::Execute(FGitSourceControlCommand& InCommand)
{
	check(InCommand.Operation->GetName() == "Connect");

	InCommand.bCommandSuccessful = GitSourceControlUtils::FindRootDirectory(InCommand.PathToGameDir, InCommand.PathToRepositoryRoot);
	if(InCommand.bCommandSuccessful)
	{
		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("status"), TArray<FString>(), TArray<FString>(), InCommand.InfoMessages, InCommand.ErrorMessages);
	}
	// @todo Get current branche name
	if(!InCommand.bCommandSuccessful || InCommand.ErrorMessages.Num() > 0 || InCommand.InfoMessages.Num() == 0)
	{
		// @todo popup to propose to initialize the git repository
		TSharedRef<FConnect, ESPMode::ThreadSafe> Operation = StaticCastSharedRef<FConnect>(InCommand.Operation);
		Operation->SetErrorText(LOCTEXT("NotAWorkingCopyError", "Project is not part of a Git working copy."));
		InCommand.ErrorMessages.Add(LOCTEXT("NotAWorkingCopyErrorHelp", "You should check out a working copy into your project directory.").ToString());
		InCommand.bCommandSuccessful = false;
	}

	return InCommand.bCommandSuccessful;
}

bool FGitConnectWorker::UpdateStates() const
{
	return false;
}

FName FGitCheckInWorker::GetName() const
{
	return "CheckIn";
}

bool FGitCheckInWorker::Execute(FGitSourceControlCommand& InCommand)
{
	check(InCommand.Operation->GetName() == "CheckIn");

	TSharedRef<FCheckIn, ESPMode::ThreadSafe> Operation = StaticCastSharedRef<FCheckIn>(InCommand.Operation);

	// make a temp file to place our commit message in
	FScopedTempFile CommitMsgFile(Operation->GetDescription());
	if(CommitMsgFile.GetFilename().Len() > 0)
	{
		TArray<FString> Parameters;
		FString ParamCommitMsgFilename = TEXT("--file=\"");
		ParamCommitMsgFilename += FPaths::ConvertRelativePathToFull(CommitMsgFile.GetFilename());
		ParamCommitMsgFilename += TEXT("\"");
		Parameters.Add(ParamCommitMsgFilename);

		// @todo: use "git commit --amend <files>" for batch commit
		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("commit"), Parameters, InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);
		if(InCommand.bCommandSuccessful)
		{
			// @todo SetSuccessMessage ParseCommitResults
			// Operation->SetSuccessMessage(ParseCommitResults(InCommand.InfoMessages));
			UE_LOG(LogSourceControl, Log, TEXT("FGitCheckInWorker: commit successful"));
		}
	}

	// now update the status of our files
	GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, InCommand.Files, InCommand.ErrorMessages, States);

	return InCommand.bCommandSuccessful;
}

bool FGitCheckInWorker::UpdateStates() const
{
	return GitSourceControlUtils::UpdateCachedStates(States);
}

FName FGitMarkForAddWorker::GetName() const
{
	return "MarkForAdd";
}

bool FGitMarkForAddWorker::Execute(FGitSourceControlCommand& InCommand)
{
	InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("add"), TArray<FString>(), InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);

	// now update the status of our files
	GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, InCommand.Files, InCommand.ErrorMessages, States);

	return InCommand.bCommandSuccessful;
}

bool FGitMarkForAddWorker::UpdateStates() const
{
	return GitSourceControlUtils::UpdateCachedStates(States);
}

FName FGitDeleteWorker::GetName() const
{
	return "Delete";
}

bool FGitDeleteWorker::Execute(FGitSourceControlCommand& InCommand)
{
	InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("rm"), TArray<FString>(), InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);

	// now update the status of our files
	GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, InCommand.Files, InCommand.ErrorMessages, States);

	return InCommand.bCommandSuccessful;
}

bool FGitDeleteWorker::UpdateStates() const
{
	return GitSourceControlUtils::UpdateCachedStates(States);
}

FName FGitRevertWorker::GetName() const
{
	return "Revert";
}

bool FGitRevertWorker::Execute(FGitSourceControlCommand& InCommand)
{
	// reset any changes already added in index
	{
		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("reset"), TArray<FString>(), InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);
	}

	// revert any changes in working copy
	{
		TArray<FString> Parameters;
		// @todo do not alarm when reverting newly added file
		//Parameters.Add(TEXT("--quiet"));
		//Parameters.Add(TEXT("--force"));
		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("checkout"), Parameters, InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);
	}

	// now update the status of our files
	GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, InCommand.Files, InCommand.ErrorMessages, States);

	return InCommand.bCommandSuccessful;
}

bool FGitRevertWorker::UpdateStates() const
{
	return GitSourceControlUtils::UpdateCachedStates(States);
}

FName FGitUpdateStatusWorker::GetName() const
{
	return "UpdateStatus";
}

bool FGitUpdateStatusWorker::Execute(FGitSourceControlCommand& InCommand)
{
	check(InCommand.Operation->GetName() == "UpdateStatus");

	TSharedRef<FUpdateStatus, ESPMode::ThreadSafe> Operation = StaticCastSharedRef<FUpdateStatus>(InCommand.Operation);

	if(InCommand.Files.Num() > 0)
	{
		InCommand.bCommandSuccessful = GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, InCommand.Files, InCommand.ErrorMessages, States);

		if(Operation->ShouldUpdateHistory())
		{
			for(const auto& ItFile : InCommand.Files)
			{
				TArray<FString> Results;
				TArray<FString> Parameters;

				Parameters.Add(TEXT("--max-count 100"));
				Parameters.Add(TEXT("--follow")); // follow file renames
				Parameters.Add(TEXT("--date=raw"));
				Parameters.Add(TEXT("--name-status")); // relative filename at this revision, preceded by a status character

				TArray<FString> Files;
				Files.Add(*ItFile);

				InCommand.bCommandSuccessful &= GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, TEXT("log"), Parameters, Files, Results, InCommand.ErrorMessages);
				TGitSourceControlHistory History;
				GitSourceControlUtils::ParseLogResults(Results, History);
				Histories.Add(*ItFile, History);
			}
		}
	}
	else
	{
		// Perforce "opened files" are those that have been modified (or added/deleted): that is what we get with a simple Git status from the root
		if(Operation->ShouldGetOpenedOnly())
		{
			// @todo Cannot work before "Connect" operation!
			if(!InCommand.PathToRepositoryRoot.IsEmpty())
			{
				TArray<FString> Files;
				Files.Add(FPaths::ConvertRelativePathToFull(FPaths::GameDir()));
				InCommand.bCommandSuccessful = GitSourceControlUtils::RunUpdateStatus(InCommand.PathToGitBinary, InCommand.PathToRepositoryRoot, Files, InCommand.ErrorMessages, States);
			}
		}
		else
		{
			// @todo is this normal/possible?
			UE_LOG(LogSourceControl, Error, TEXT("FGitUpdateStatusWorker: InCommand.Files.Num() == 0"));
			InCommand.bCommandSuccessful = true; // nothing to do
		}

		// @todo Re-check current branche name
	}

	// don't use the ShouldUpdateModifiedState() hint here as it is specific to Perforce: the above normal Git status has already told us this information (like SVN and Mercurial)

	return InCommand.bCommandSuccessful;
}

bool FGitUpdateStatusWorker::UpdateStates() const
{
	bool bUpdated = GitSourceControlUtils::UpdateCachedStates(States);

	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>( "GitSourceControl" );
	FGitSourceControlProvider& Provider = GitSourceControl.GetProvider();

	// add history, if any
	for(const auto& ItHistory : Histories)
	{
		TSharedRef<FGitSourceControlState, ESPMode::ThreadSafe> State = Provider.GetStateInternal(ItHistory.Key);
		State->History = ItHistory.Value;
		State->TimeStamp = FDateTime::Now();
		bUpdated = true;
	}

	return bUpdated;
}

#undef LOCTEXT_NAMESPACE
