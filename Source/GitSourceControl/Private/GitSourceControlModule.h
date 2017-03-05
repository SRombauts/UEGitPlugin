// Copyright (c) 2014-2017 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "GitSourceControlSettings.h"
#include "GitSourceControlProvider.h"

/**

UE4GitPlugin is a simple Git Source Control Plugin for Unreal Engine

Written and contributed by Sebastien Rombauts (sebastien.rombauts@gmail.com)

### Supported features
- initialize a new Git local repository ('git init') to manage your UE4 Game Project
  - can also create an appropriate .gitignore file as part of initialization
  - can also create a .gitattributes file to enable Git LFS (Large File System) as part of initialization
  - can also make the initial commit, with custom multi-line message
- display status icons to show modified/added/deleted/untracked files
- show history of a file
- visual diff of a blueprint against depot or between previous versions of a file
- revert modifications of a file (works best with UE4.15 "Content Hot-Reload" experimental option)
- add, delete, rename a file
- checkin/commit a file (cannot handle atomically more than 50 files)
- migrate an asset between two projects if both are using Git
- solve a merge conflict on a blueprint
- show current branch name in status text
- Sync to Pull (rebase) the current branch if there is no local modified files
- Git LFS (Github, Gitlab, Bitbucket), git-annex, git-fat and git-media are working with Git 2.10+
- Git LFS 2 File Locking is working with Git 2.10+ and Git LFS 2.0.0
- Windows, Mac and Linux

### What *cannot* be done presently
- Branch/Merge are not in the current Editor workflow
- Fetch/Push are not in the current Editor workflow
- Amend a commit is not in the current Editor workflow
- Revert All (using either "Stash" or "reset --hard")
- Configure user name & email ('git config user.name' & git config user.email')
- Configure remote origin URL ('git remote add origin url')

### Known issues
- the Editor does not show deleted files (only when deleted externaly?)
- the Editor does not show missing files
- missing localisation for git specific messages
- displaying states of 'Engine' assets (also needs management of 'out of tree' files)
- renaming a Blueprint in Editor leaves a redirector file, AND modify too much the asset to enable git to track its history through renaming
- standard Editor commit dialog asks if user wants to "Keep Files Checked Out" => no use for Git or Mercurial CanCheckOut()==false

 */
class FGitSourceControlModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Access the Git source control settings */
	FGitSourceControlSettings& AccessSettings()
	{
		return GitSourceControlSettings;
	}

	/** Save the Git source control settings */
	void SaveSettings();

	/** Access the Git source control provider */
	FGitSourceControlProvider& GetProvider()
	{
		return GitSourceControlProvider;
	}

private:
	/** The Git source control provider */
	FGitSourceControlProvider GitSourceControlProvider;

	/** The settings for Git source control */
	FGitSourceControlSettings GitSourceControlSettings;
};
