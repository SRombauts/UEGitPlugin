// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "ISourceControlModule.h"
#include "GitSourceControlDevSettings.h"
#include "GitSourceControlDevProvider.h"

/**

UE4GitPlugin is a simple Git Source Control Plugin for Unreal Engine

Written and contributed by Sebastien Rombauts (sebastien.rombauts@gmail.com)

### Supported features
- initialize a new Git local repository ('git init') to manage your UE4 Game Project.
- display status icons to show modified/added/deleted/untracked files
- show history of a file
- diff against depot or between previous versions of a file
- revert modifications of a file
- add a file
- delete a file
- checkin/commit a file (cannot handle atomically more than 20 files)
- show current branch name in status text
- merge blueprints to solve a merge or rebase conflict

### What *cannot* be done presently
- tags: implement ISourceControlLabel to manage git tags
- .uproject file state si not visible in the current Editor
- Branch is not in the current Editor workflow (but on Epic Roadmap)
- Pull/Fetch/Push are not in the current Editor workflow
- Amend a commit is not in the current Editor workflow
- configure user name & email ('git config user.name' & git config user.email')
- Linux is not supported by the SourceControlProvider (#define SOURCE_CONTROL_WITH_SLATE	!PLATFORM_LINUX)
- git-annex and/or git-media

### Known issues:
- global menu "Submit to source control" leads to many lines of Logs "is outside repository"
- the Editor does not show deleted files (only when deleted externaly?)
- the Editor does not show missing files
- missing localisation for git specific messages
- migrate an asset should add it to the destination project if also under Git (needs management of 'out of tree' files)
- displaying states of 'Engine' assets (also needs management of 'out of tree' files)
- issue #22: A Move/Rename leaves a redirector file behind
- issue #11: Add the "Resolve" operation introduced in Editor 4.3
- reverting an asset does not seem to update content in Editor! Issue in Editor?
- renaming a Blueprint in Editor leaves a tracker file, AND modify too much the asset to enable git to track its history through renaming
- file history show Changelist as signed integer instead of hexadecimal SHA1
- standard Editor commit dialog ask if user wants to "Keep Files Checked Out" => no use for Git or Mercurial CanCheckOut()==false

 */
class FGitSourceControlDevModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Access the Git source control settings */
	FGitSourceControlDevSettings& AccessSettings()
	{
		return GitSourceControlDevSettings;
	}

	/** Save the Git source control settings */
	void SaveSettings();

	/** Access the Git source control provider */
	FGitSourceControlDevProvider& GetProvider()
	{
		return GitSourceControlDevProvider;
	}

private:
	/** The Git source control provider */
	FGitSourceControlDevProvider GitSourceControlDevProvider;

	/** The settings for Git source control */
	FGitSourceControlDevSettings GitSourceControlDevSettings;
};