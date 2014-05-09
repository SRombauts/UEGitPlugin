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

	InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(TEXT("status"), TArray<FString>(), TArray<FString>(), InCommand.PathToGameDir, InCommand.InfoMessages, InCommand.ErrorMessages);
	if(InCommand.bCommandSuccessful)
	{
		if(InCommand.ErrorMessages.Num() > 0)
		{
			TSharedRef<FConnect, ESPMode::ThreadSafe> Operation = StaticCastSharedRef<FConnect>(InCommand.Operation);
			Operation->SetErrorText(LOCTEXT("NotAWorkingCopyError", "Project is not part of a Git working copy."));
			InCommand.ErrorMessages.Add(LOCTEXT("NotAWorkingCopyErrorHelp", "You should check out a working copy into your project directory.").ToString());
			InCommand.bCommandSuccessful = false;
		}	
	}
	return InCommand.bCommandSuccessful;
}

bool FGitConnectWorker::UpdateStates() const
{
	return false;
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
		Parameters.Add(TEXT("-z")); // similar to --porcelain, but use a NULL character to terminate file names instead of spaces
		Parameters.Add(TEXT("--ignored"));

		InCommand.bCommandSuccessful = GitSourceControlUtils::RunCommand(TEXT("status"), Parameters, InCommand.Files, InCommand.PathToGameDir, Results, InCommand.ErrorMessages);
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
