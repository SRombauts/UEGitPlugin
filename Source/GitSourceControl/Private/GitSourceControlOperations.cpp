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

	InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToGameDir, TEXT("status"), TArray<FString>(), TArray<FString>(), InCommand.InfoMessages, InCommand.ErrorMessages);
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

FName FGitRevertWorker::GetName() const
{
	return "Revert";
}

bool FGitRevertWorker::Execute(FGitSourceControlCommand& InCommand)
{
    // reset any changes already added in index
    {
        TArray<FString> Parameters;
        InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToGameDir, TEXT("reset"), Parameters, InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);
    }

    // revert any changes in working copy
	{
		TArray<FString> Parameters;
        InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToGameDir, TEXT("checkout"), Parameters, InCommand.Files, InCommand.InfoMessages, InCommand.ErrorMessages);
	}

	// now update the status of our files
	{
		TArray<FString> Results;
		TArray<FString> Parameters;
		Parameters.Add(TEXT("--porcelain"));
		Parameters.Add(TEXT("--ignored"));

		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToGameDir, TEXT("status"), Parameters, InCommand.Files, Results, InCommand.ErrorMessages);
		GitSourceControlUtils::ParseStatusResults(InCommand.Files, Results, InCommand.ErrorMessages, States);
	}

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

	if(InCommand.Files.Num() > 0)
	{
		TArray<FString> Results;
		TArray<FString> Parameters;
		Parameters.Add(TEXT("--porcelain"));
		Parameters.Add(TEXT("--ignored"));

		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(InCommand.PathToGitBinary, InCommand.PathToGameDir, TEXT("status"), Parameters, InCommand.Files, Results, InCommand.ErrorMessages);
		GitSourceControlUtils::ParseStatusResults(InCommand.Files, Results, InCommand.ErrorMessages, States);
	}
	else
	{
		InCommand.bCommandSuccessful = true; // nothing to do
	}

	// @todo update using any special hints passed in via the operation


	return InCommand.bCommandSuccessful;
}

bool FGitUpdateStatusWorker::UpdateStates() const
{
	bool bUpdated = GitSourceControlUtils::UpdateCachedStates(States);

	// @todo add history, if any

	return bUpdated;
}

#undef LOCTEXT_NAMESPACE
