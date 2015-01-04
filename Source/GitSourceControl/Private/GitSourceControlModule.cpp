// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlModule.h"
#include "ModuleManager.h"
#include "ISourceControlModule.h"
#include "GitSourceControlSettings.h"
#include "GitSourceControlOperations.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

template<typename Type>
static TSharedRef<IGitSourceControlWorker, ESPMode::ThreadSafe> CreateWorker()
{
	return MakeShareable( new Type() );
}

void FGitSourceControlModule::StartupModule()
{
	// Register our operations
	GitSourceControlProvider.RegisterWorker( "Connect", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitConnectWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Init", FGetGitSourceControlWorker::CreateStatic(&CreateWorker<FGitInitWorker>));
	// Note: this provider does not uses the "CheckOut" command, which is a Perforce "lock", as Git has no lock command (all tracked files in the working copy are always already checked-out).
	GitSourceControlProvider.RegisterWorker( "UpdateStatus", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitUpdateStatusWorker> ) );
	GitSourceControlProvider.RegisterWorker( "MarkForAdd", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitMarkForAddWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Delete", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitDeleteWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Revert", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitRevertWorker> ) );
	// @todo: git fetch remote(s) to be able to show files not up-to-date with the serveur
//	GitSourceControlProvider.RegisterWorker( "Sync", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitSyncWorker> ) );
	GitSourceControlProvider.RegisterWorker( "CheckIn", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitCheckInWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Copy", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitCopyWorker> ) );
	// @todo: git add to mark a conflict as resolved
//	GitSourceControlProvider.RegisterWorker( "Resolve", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitResolveWorker> ) );

	// load our settings
	GitSourceControlSettings.LoadSettings();

	// Bind our source control provider to the editor
	IModularFeatures::Get().RegisterModularFeature( "SourceControl", &GitSourceControlProvider );
}

void FGitSourceControlModule::ShutdownModule()
{
	// shut down the provider, as this module is going away
	GitSourceControlProvider.Close();

	// unbind provider from editor
	IModularFeatures::Get().UnregisterModularFeature("SourceControl", &GitSourceControlProvider);
}

FGitSourceControlSettings& FGitSourceControlModule::AccessSettings()
{
	return GitSourceControlSettings;
}

void FGitSourceControlModule::SaveSettings()
{
	if (FApp::IsUnattended() || IsRunningCommandlet())
	{
		return;
	}

	GitSourceControlSettings.SaveSettings();
}

void FGitSourceControlModule::ShowGitInitDialog(EGitInitWindowMode::Type InGitInitWindowMode)
{
#if SOURCE_CONTROL_WITH_SLATE
	
	// if we are showing a modal version of the dialog & a modeless version already exists, we must destroy the modeless dialog first
	if (InGitInitWindowMode == ELoginWindowMode::Modal && GitSourceControlInitPtr.IsValid())
	{
		// unhook the delegate so it doesn't fire in this case
		GitSourceControlInitWindowPtr->SetOnWindowClosed(FOnWindowClosed());
		GitSourceControlInitWindowPtr->RequestDestroyWindow();
		GitSourceControlInitWindowPtr = NULL;
		GitSourceControlInitPtr = NULL;
	}

	if (GitSourceControlInitWindowPtr.IsValid())
	{
		GitSourceControlInitWindowPtr->BringToFront();
	}
	else
	{
		// Create the window
		GitSourceControlInitWindowPtr = SNew(SWindow)
			.Title(LOCTEXT("GitSourceControlInitTitle", "Git Source Control Init"))
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.CreateTitleBar(false)
			.SizingRule(ESizingRule::Autosized);

		// Setup the content for the created login window.
		GitSourceControlInitWindowPtr->SetContent(
			SNew(SBox)
			.WidthOverride(300.0f)
			[
				SAssignNew(GitSourceControlInitPtr, SGitInitDialog)
				.ParentWindow(GitSourceControlInitWindowPtr)
			]
		);

		TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
		if (RootWindow.IsValid())
		{
			if (InGitInitWindowMode == ELoginWindowMode::Modal)
			{
				FSlateApplication::Get().AddModalWindow(GitSourceControlInitWindowPtr.ToSharedRef(), RootWindow);
			}
			else
			{
				FSlateApplication::Get().AddWindowAsNativeChild(GitSourceControlInitWindowPtr.ToSharedRef(), RootWindow.ToSharedRef());
			}
		}
		else
		{
			FSlateApplication::Get().AddWindow(GitSourceControlInitWindowPtr.ToSharedRef());
		}
	}
#else
	STUBBED("FGitSourceControlModule::ShowGitInitDialog - no Slate");
#endif // SOURCE_CONTROL_WITH_SLATE
}

IMPLEMENT_MODULE(FGitSourceControlModule, GitSourceControl);

#undef LOCTEXT_NAMESPACE
