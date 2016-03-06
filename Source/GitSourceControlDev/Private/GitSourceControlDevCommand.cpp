// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevCommand.h"
#include "GitSourceControlDevModule.h"
#include "GitSourceControlDevProvider.h"
#include "IGitSourceControlDevWorker.h"
#include "SGitSourceControlDevSettings.h"

FGitSourceControlDevCommand::FGitSourceControlDevCommand(const TSharedRef<class ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TSharedRef<class IGitSourceControlDevWorker, ESPMode::ThreadSafe>& InWorker, const FSourceControlOperationComplete& InOperationCompleteDelegate)
	: Operation(InOperation)
	, Worker(InWorker)
	, OperationCompleteDelegate(InOperationCompleteDelegate)
	, bExecuteProcessed(0)
	, bCommandSuccessful(false)
	, bAutoDelete(true)
	, Concurrency(EConcurrency::Synchronous)
{
	// grab the providers settings here, so we don't access them once the worker thread is launched
	check(IsInGameThread());
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>( "GitSourceControlDev" );
	PathToGitBinary = GitSourceControlDev.AccessSettings().GetBinaryPath();
	PathToRepositoryRoot = GitSourceControlDev.GetProvider().GetPathToRepositoryRoot();
}

bool FGitSourceControlDevCommand::DoWork()
{
	bCommandSuccessful = Worker->Execute(*this);
	FPlatformAtomics::InterlockedExchange(&bExecuteProcessed, 1);

	return bCommandSuccessful;
}

void FGitSourceControlDevCommand::Abandon()
{
	FPlatformAtomics::InterlockedExchange(&bExecuteProcessed, 1);
}

void FGitSourceControlDevCommand::DoThreadedWork()
{
	Concurrency = EConcurrency::Asynchronous;
	DoWork();
}
