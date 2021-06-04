// Copyright (c) 2014-2020 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlCommand.h"

#include "Modules/ModuleManager.h"
#include "GitSourceControlModule.h"

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
	FString PluginsRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectPluginsDir());
	// note this is not going to support operations where selected files are both in the root repo and the submodule/plugin's repo
	int NumPluginFiles = 0;

	for (auto& FilePath : AbsoluteFilePaths)
	{
		if (FilePath.Contains(PluginsRoot))
		{
			NumPluginFiles++;
		}
	}
	// if all plugins?
	// modify Source control base path
	if((NumPluginFiles == AbsoluteFilePaths.Num()) && (AbsoluteFilePaths.Num() > 0))
	{
		FString FullPath = AbsoluteFilePaths[0];

		FString PluginPart = FullPath.Replace(*PluginsRoot, *FString(""));
		PluginPart = PluginPart.Left(PluginPart.Find("/"));


		FString CandidateRepoRoot = PluginsRoot + PluginPart;

		FString IsItUsingGitPath = CandidateRepoRoot + "/.git";
		if (FPaths::FileExists(IsItUsingGitPath))
		{
			PathToRepositoryRoot = CandidateRepoRoot;
		}
	}
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
