// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevModule.h"
#include "ModuleManager.h"
#include "ISourceControlModule.h"
#include "GitSourceControlDevSettings.h"
#include "GitSourceControlDevOperations.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"

#define LOCTEXT_NAMESPACE "GitSourceControlDev"

template<typename Type>
static TSharedRef<IGitSourceControlDevWorker, ESPMode::ThreadSafe> CreateWorker()
{
	return MakeShareable( new Type() );
}

void FGitSourceControlDevModule::StartupModule()
{
	// Register our operations
	GitSourceControlDevProvider.RegisterWorker( "Connect", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitConnectWorker> ) );
	// Note: this provider does not uses the "CheckOut" command, which is a Perforce "lock", as Git has no lock command (all tracked files in the working copy are always already checked-out).
	GitSourceControlDevProvider.RegisterWorker( "UpdateStatus", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitUpdateStatusWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "MarkForAdd", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitMarkForAddWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "Delete", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitDeleteWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "Revert", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitRevertWorker> ) );
	// @todo: git fetch remote(s) to be able to show files not up-to-date with the serveur
//	GitSourceControlDevProvider.RegisterWorker( "Sync", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitSyncWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "CheckIn", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitCheckInWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "Copy", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitCopyWorker> ) );
	GitSourceControlDevProvider.RegisterWorker( "Resolve", FGetGitSourceControlDevWorker::CreateStatic( &CreateWorker<FGitResolveWorker> ) );

	// load our settings
	GitSourceControlDevSettings.LoadSettings();

	// Bind our source control provider to the editor
	IModularFeatures::Get().RegisterModularFeature( "SourceControl", &GitSourceControlDevProvider );
}

void FGitSourceControlDevModule::ShutdownModule()
{
	// shut down the provider, as this module is going away
	GitSourceControlDevProvider.Close();

	// unbind provider from editor
	IModularFeatures::Get().UnregisterModularFeature("SourceControl", &GitSourceControlDevProvider);
}

void FGitSourceControlDevModule::SaveSettings()
{
	if (FApp::IsUnattended() || IsRunningCommandlet())
	{
		return;
	}

	GitSourceControlDevSettings.SaveSettings();
}

IMPLEMENT_MODULE(FGitSourceControlDevModule, GitSourceControlDev);

#undef LOCTEXT_NAMESPACE
