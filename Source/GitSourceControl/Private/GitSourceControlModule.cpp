// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
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
	// Note: this provider does not uses the "CheckOut" command, which is a Perforce "lock", as Git has no lock command.
	GitSourceControlProvider.RegisterWorker( "UpdateStatus", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitUpdateStatusWorker> ) );
	GitSourceControlProvider.RegisterWorker( "MarkForAdd", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitMarkForAddWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Delete", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitDeleteWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Revert", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitRevertWorker> ) );
	// @todo: Git fetch
//	GitSourceControlProvider.RegisterWorker( "Sync", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitSyncWorker> ) );
	GitSourceControlProvider.RegisterWorker( "CheckIn", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitCheckInWorker> ) );
	GitSourceControlProvider.RegisterWorker( "Copy", FGetGitSourceControlWorker::CreateStatic( &CreateWorker<FGitCopyWorker> ) );
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

void FGitSourceControlModule::ShowInitDialog(ELoginWindowMode::Type InLoginWindowMode, EOnLoginWindowStartup::Type InOnLoginWindowStartup)
{
#if SOURCE_CONTROL_WITH_SLATE
	
	// if we are showing a modal version of the dialog & a modeless version already exists, we must destroy the modeless dialog first
	if (InLoginWindowMode == ELoginWindowMode::Modal && SourceControlInitPtr.IsValid())
	{
		// unhook the delegate so it doesn't fire in this case
		SourceControlInitWindowPtr->SetOnWindowClosed(FOnWindowClosed());
		SourceControlInitWindowPtr->RequestDestroyWindow();
		SourceControlInitWindowPtr = NULL;
		SourceControlInitPtr = NULL;
	}

	if (SourceControlInitWindowPtr.IsValid())
	{
		SourceControlInitWindowPtr->BringToFront();
	}
	else
	{
		// Create the window
		SourceControlInitWindowPtr = SNew(SWindow)
			.Title(LOCTEXT("SourceControlInitTitle", "Git Source Control Init"))
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.CreateTitleBar(false)
			.SizingRule(ESizingRule::Autosized);

		// Setup the content for the created login window.
		SourceControlInitWindowPtr->SetContent(
			SNew(SBox)
			.WidthOverride(700.0f)
			[
				SAssignNew(SourceControlInitPtr, SGitInitDialog)
				.ParentWindow(SourceControlInitWindowPtr)
			]
		);

		TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
		if (RootWindow.IsValid())
		{
			if (InLoginWindowMode == ELoginWindowMode::Modal)
			{
				FSlateApplication::Get().AddModalWindow(SourceControlInitWindowPtr.ToSharedRef(), RootWindow);
			}
			else
			{
				FSlateApplication::Get().AddWindowAsNativeChild(SourceControlInitWindowPtr.ToSharedRef(), RootWindow.ToSharedRef());
			}
		}
		else
		{
			FSlateApplication::Get().AddWindow(SourceControlInitWindowPtr.ToSharedRef());
		}
	}
#else
	STUBBED("FSourceControlModule::ShowLoginDialog - no Slate");
#endif // SOURCE_CONTROL_WITH_SLATE
}

IMPLEMENT_MODULE(FGitSourceControlModule, GitSourceControl);

#undef LOCTEXT_NAMESPACE
