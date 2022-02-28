// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
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
  - can also configure the default remote origin URL
- display status icons to show modified/added/deleted/untracked files
- show history of a file
- visual diff of a blueprint against depot or between previous versions of a file
- revert modifications of a file
- add, delete, rename a file
- checkin/commit a file (cannot handle atomically more than 50 files)
- migrate an asset between two projects if both are using Git
- solve a merge conflict on a blueprint
- show current branch name in status text
- Sync to Pull (rebase) the current branch
- Git LFS (Github, Gitlab, Bitbucket) is working with Git 2.10+ under Windows
- Git LFS 2 File Locking is working with Git 2.10+ and Git LFS 2.0.0
- Windows, Mac and Linux

### TODO 
1. configure the name of the remote instead of default "origin"

### TODO LFS 2.x File Locking

Known issues:
0. False error logs after a successful push:
To https://github.com/SRombauts/UE4GitLfs2FileLocks.git
   ee44ff5..59da15e HEAD -> master

Use "TODO LFS" in the code to track things left to do/improve/refactor:
1. IsUsingGitLfsLocking() should be cached in the Provider to avoid calling AccessSettings() too frequently
   it can not change without re-initializing (at least re-connect) the Provider!
2. Implement FGitSourceControlProvider::bWorkingOffline like the SubversionSourceControl plugin
3. Trying to deactivate Git LFS 2 file locking afterward on the "Login to Source Control" (Connect/Configure) screen
   is not working after Git LFS 2 has switched "read-only" flag on files (which needs the Checkout operation to be editable)!
   - temporarily deactivating locks may be required if we want to be able to work while not connected (do we really need this ???)
   - does Git LFS have a command to do this deactivation ?
     - perhaps should we rely on detection of such flags to detect LFS 2 usage (ie. the need to do a checkout)
       - see SubversionSourceControl plugin that deals with such flags
       - this would need a rework of the way the "bIsUsingFileLocking" si propagated, since this would no more be a configuration (or not only) but a file state
     - else we should at least revert those read-only flags when going out of "Lock mode"
4. Optimize usage of "git lfs locks", ie reduce the use of UdpateStatus() in Operations

### What *cannot* be done presently
- Branch/Merge are not in the current Editor workflow
- Fetch is not in the current Editor workflow
- Amend a commit is not in the current Editor workflow
- Configure user name & email ('git config user.name' & git config user.email')

### Known issues
- the Editor does not show deleted files (only when deleted externally?)
- the Editor does not show missing files
- missing localization for git specific messages
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
	const FGitSourceControlSettings& AccessSettings() const
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
	const FGitSourceControlProvider& GetProvider() const
	{
		return GitSourceControlProvider;
	}

private:
	/** The Git source control provider */
	FGitSourceControlProvider GitSourceControlProvider;

	/** The settings for Git source control */
	FGitSourceControlSettings GitSourceControlSettings;
};
