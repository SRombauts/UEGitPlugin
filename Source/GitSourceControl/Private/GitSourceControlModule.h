// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "ISourceControlModule.h"
#include "GitSourceControlSettings.h"
#include "GitSourceControlProvider.h"
#include "SGitInitDialog.h"
#include "ModuleManager.h"

/**
* The modality of the init window.
*/
namespace EGitInitWindowMode
{
	enum Type
	{
		Modal,
		Modeless
	};
};


class FGitSourceControlModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Access the Git source control settings */
	FGitSourceControlSettings& AccessSettings();

	/** Save the Git source control settings */
	void SaveSettings();

	/** Access the Git source control provider */
	FGitSourceControlProvider& GetProvider()
	{
		return GitSourceControlProvider;
	}

	void ShowGitInitDialog(EGitInitWindowMode::Type InGitInitWindowMode);

	static FGitSourceControlModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	}

private:
	/** The login window we may be using */
	TSharedPtr<SWindow> GitSourceControlInitWindowPtr;

	/** The login window control we may be using */
	TSharedPtr<class SGitInitDialog> GitSourceControlInitPtr;

	/** The Git source control provider */
	FGitSourceControlProvider GitSourceControlProvider;

	/** The settings for Git source control */
	FGitSourceControlSettings GitSourceControlSettings;
};