// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlCommand.h"

#include "Modules/ModuleManager.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlUtils.h"

FGitSourceControlCommand::FGitSourceControlCommand(const TSharedRef<class ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TSharedRef<class IGitSourceControlWorker, ESPMode::ThreadSafe>& InWorker, const FSourceControlOperationComplete& InOperationCompleteDelegate)
	: Operation(InOperation)
	, Worker(InWorker)
	, OperationCompleteDelegate(InOperationCompleteDelegate)
	, bExecuteProcessed(0)
	, bCommandSuccessful(false)
	, bConnectionDropped(false)
	, bAutoDelete(true)
	, Concurrency(EConcurrency::Synchronous)
{
	// grab the providers settings here, so we don't access them once the worker thread is launched
	check(IsInGameThread());
	const FGitSourceControlModule& GitSourceControl = FModuleManager::GetModuleChecked<FGitSourceControlModule>( "GitSourceControl" );
	PathToGitBinary = GitSourceControl.AccessSettings().GetBinaryPath();
	bUsingGitLfsLocking = GitSourceControl.AccessSettings().IsUsingGitLfsLocking();
	PathToRepositoryRoot = GitSourceControl.GetProvider().GetPathToRepositoryRoot();
}
void FGitSourceControlCommand::UpdateRepositoryRootIfSubmodule(const TArray<FString>& AbsoluteFilePaths)
{
	PathToRepositoryRoot = GitSourceControlUtils::ChangeRepositoryRootIfSubmodule(AbsoluteFilePaths, PathToRepositoryRoot);
}
bool FGitSourceControlCommand::DoWork()
{
	bCommandSuccessful = Worker->Execute(*this);
	FPlatformAtomics::InterlockedExchange(&bExecuteProcessed, 1);

	return bCommandSuccessful;
}

void FGitSourceControlCommand::Abandon()
{
	FPlatformAtomics::InterlockedExchange(&bExecuteProcessed, 1);
}

void FGitSourceControlCommand::DoThreadedWork()
{
	Concurrency = EConcurrency::Asynchronous;
	DoWork();
}

ECommandResult::Type FGitSourceControlCommand::ReturnResults()
{
	// Save any messages that have accumulated
	for (FString& String : InfoMessages)
	{
		Operation->AddInfoMessge(FText::FromString(String));
	}
	for (FString& String : ErrorMessages)
	{
		Operation->AddErrorMessge(FText::FromString(String));
	}

	// run the completion delegate if we have one bound
	ECommandResult::Type Result = bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed;
	OperationCompleteDelegate.ExecuteIfBound(Operation, Result);

	return Result;
}
