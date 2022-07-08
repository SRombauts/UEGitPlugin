// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "ISourceControlProvider.h"

#include "Runtime/Launch/Resources/Version.h"

/** Git extension of the Source Control toolbar menu */
class FGitSourceControlMenu
{
public:
	void Register();
	void Unregister();
	
	/** This functions will be bound to appropriate Command. */
	void PushClicked();
	void SyncClicked();
	void RevertClicked();
	void RefreshClicked();

private:
	bool HaveRemoteUrl() const;

	bool				SaveDirtyPackages();
	TArray<FString>		ListAllPackages();
	TArray<UPackage*>	UnlinkPackages(const TArray<FString>& InPackageNames);
	void				ReloadPackages(TArray<UPackage*>& InPackagesToReload);

	bool StashAwayAnyModifications();
	void ReApplyStashedModifications();

#if ENGINE_MAJOR_VERSION == 5
	void AddMenuExtension(struct FToolMenuSection& Builder);
#else
	void AddMenuExtension(class FMenuBuilder& Builder);
	TSharedRef<class FExtender> OnExtendLevelEditorViewMenu(const TSharedRef<class FUICommandList> CommandList);
#endif

	void DisplayInProgressNotification(const FText& InOperationInProgressString);
	void RemoveInProgressNotification();
	void DisplaySucessNotification(const FName& InOperationName);
	void DisplayFailureNotification(const FName& InOperationName);

private:
#if ENGINE_MAJOR_VERSION == 4
	FDelegateHandle ViewMenuExtenderHandle;
#endif

	/** Was there a need to stash away modifications before Sync? */
	bool bStashMadeBeforeSync;

	/** Loaded packages to reload after a Sync or Revert operation */
	TArray<UPackage*> PackagesToReload;

	/** Current source control operation from extended menu if any */
	TWeakPtr<class SNotificationItem> OperationInProgressNotification;

	/** Delegate called when a source control operation has completed */
	void OnSourceControlOperationComplete(const FSourceControlOperationRef& InOperation, ECommandResult::Type InResult);
};
