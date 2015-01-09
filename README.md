Unreal Engine 4 Git Source Control Plugin
-----------------------------------------

UE4GitPlugin is a simple Git Source Control Plugin for Unreal Engine 4.7

### Status

[Download the latest release](https://github.com/SRombauts/UE4GitPlugin/releases)

Beta version 0.6.3:
- initialize a new Git local repository ('git init') to manage your UE4 Game Project.
- display status icons to show modified/added/deleted/untracked files
- show history of a file
- diff against depot or between previous versions of a file
- revert modifications of a file
- add a file
- delete a file
- checkin/commit a file (cannot handle atomically more than 20 files)
- show current branch name in status text

#### What *cannot* be done presently (TODO list for v1.0, ordered by priority):
- solve a merge conflict
- merge blueprints
- add localisation for git specific messages
- migrate an asset should add it to the destination project if also under Git (needs management of 'out of tree' files)
- displaying states of 'Engine' assets (also needs management of 'out of tree' files)
- tags: implement ISourceControlLabel to manage git tags
- .uproject file state si not visible in the current Editor
- Branch is not in the current Editor workflow (but on Epic Roadmap)
- Pull/Fetch/Push are not in the current Editor workflow
- Amend a commit is not in the current Editor workflow
- configure user name & email ('git config user.name' & git config user.email')

#### Known issues:
- the Editor does not show deleted files (only when deleted externaly?)
- the Editor does not show missing files
- issue #22: A Move/Rename leaves a redirector file behind
- issue #11: Add the "Resolve" operation introduced in Editor 4.3
- improve the 'Init' window text, hide it if connection is already done, auto connect
- reverting an asset does not seem to update content in Editor! Issue in Editor?
- file history show Revision as signed integer instead of hexadecimal SHA1
- file history does not report file size
- standard Editor commit dialog ask if user wants to "Keep Files Checked Out" => no use for Git or Mercurial CanCheckOut()==false
- Windows only (64 bits) -> Mac compiles but needs testing/debugging (Linux source control is not supported by Editor)

#### Wishlist (after v1.0):
- [git-annexe and/or git-media - #1 feature request](https://github.com/SRombauts/UE4GitPlugin/issues/1)

#### In-code TODO list (internal roadmap):

- FGitConnectWorker::Execute (While project not in Git source control)
  Improve error message "You should check out a working copy..."
  => double error message (and in reverse order) with "Project is not part of a Git working copy"

- FGitResolveWorker (GitSourceControlOperations.h)
  git add to mark a conflict as resolved
  
- FGitSourceControlRevision::GetBranchSource() const
  if this revision was copied from some other revision, then that source revision should
	be returned here (this should be determined when history is being fetched)
- FGitSourceControlState::GetBaseRevForMerge()
  get revision of the merge-base (https://www.kernel.org/pub/software/scm/git/docs/git-merge-base.html)
	
- FGitConnectWorker::Execute()
  popup to propose to initialize the git repository "git init + .gitignore"

- FGitSyncWorker (GitSourceControlOperations.h)
  git fetch remote(s) to be able to show files not up-to-date with the serveur
- FGitSourceControlState::IsCurrent() const
  check the state of the HEAD versus the state of tracked branch on remote

- FGitSourceControlRevision::GetFileSize() const
	git log does not give us the file size, but we could run a specific command

- GitSourceControlUtils::CheckGitAvailability
  also check Git config user.name & user.email

Windows:
- GitSourceControlUtils::FindGitBinaryPath
  use the Windows registry to find Git

Mac:
- GitSourceControlUtils::RunCommandInternalRaw
  Specifying the working copy (the root) of the git repository (before the command itself)
	does not work in UE4.1 on Mac if there is a space in the path ("/Users/xxx/Unreal Project/MyProject")

Bug reports?
- FGitSourceControlRevision::Get
  Bug report: a counter is needed to avoid overlapping files; temp files are not (never?) released by Editor!

- GitSourceControlUtils::UpdateCachedStates
  // State->TimeStamp = InState.TimeStamp; // Bug report: Workaround a bug with the Source Control Module not updating file state after a "Save"

### Getting started

#### Install Git

Under Windows 64bits, you could either:
- install a standalone Git, usually in "C:\Program Files (x86)\Git\bin\git.exe".
- or copy a [portable Git](https://code.google.com/p/msysgit/downloads/list?can=1&q=PortableGit)
inside "<UnrealEngine>/Engine/Binaries/ThirdParty/git/Win32".

#### Initialize your Game Project directory with Git

Use your favorite Git program to initialize and manage your whole game project directory.
For instance:

```
C:/Users/<username>/Documents/Unreal Projects/<YourGameProject>
```

#### Install this Git Plugin

There are a few ways to use a Plugin with UE4.

See also the [Plugins official Documentation](https://docs.unrealengine.com/latest/INT/Programming/Plugins/index.html)

##### Within a standard installed Unreal Engine binary release:

You can simply [download a ZIP of source code from the latest release](https://github.com/SRombauts/UE4GitPlugin/releases),
and unzip it under the "Plugins" directory of the Engine, beside PerforceSourceControl and SubversionSourceControl:

```
<UnrealEngineInstallation>/4.1/Engine/Plugins/Developer
```

Or you can clone the plugin repository, and as the name of the destination directory is unimportant, its just:

```bash
git clone https://github.com/SRombauts/UE4GitPlugin.git
```

##### Within an Unreal Engine source release from GitHub:

Donwload and unzip or clone the plugin repository under the "Plugins" directory of the Engine, beside PerforceSourceControl and SubversionSourceControl:

```
<UnrealEngineClone>/Engine/Plugins/Developer
```

Take care to use **GitSourceControl** as the name of the destination directory (same name as the "GitSourceControl.uplugin" file):

```bash
git clone https://github.com/SRombauts/UE4GitPlugin.git GitSourceControl
```

##### Within your Game Project only

Alternatively, you could choose to install the plugin into a subfolder or your Game Project "Plugins" directory:

```
<YourGameProject>/Plugins
```

In this case, you will obviously only be able to use the plugin within this project.

#### Enable Git Plugin within the Editor

Launch the UE4 Editor, then open:

```
Windows->Plugins, enable Editor->Source Control->Git
```

click Enable and restart the Editor.

#### Activate Git Source Control for your Game Project

Load your Game Project, then open:

```
File->Connect To Source Control... -> Git: Accept Settings
```

See also the [Source Control official Documentation](https://docs.unrealengine.com/latest/INT/Engine/UI/SourceControl/index.html)

### License

Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)

Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
or copy at http://opensource.org/licenses/MIT)

## How to contribute
### GitHub website
The most efficient way to help and contribute to this wrapper project is to
use the tools provided by GitHub:
- please fill bug reports and feature requests here: https://github.com/SRombauts/UE4GitPlugin/issues
- fork the repository, make some small changes and submit them with independent pull-requests

### Contact
You can also email me directly, I will answer any questions and requests.

### Coding Style Guidelines
The source code follow the UnreaEngine official [Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/index.html) :
- CamelCase naming convention, with a prefix letter to differentiate classes ('F'), interfaces ('I'), templates ('T')
- files (.cpp/.h) are named like the class they contains
- Doxygen comments, documentation is located with declaration, on headers
- Use portable common features of C++11 like nullptr, auto, range based for, override keyword
- Braces on their own line
- Tabs to indent code, with a width of 4 characters

## See also

- [Git Source Control Tutorial on the Wikis](https://wiki.unrealengine.com/Git_source_control_(Tutorial))
- [UE4 Git Plugin website](http://srombauts.github.com/UE4GitPlugin)

- [ue4-hg-plugin for Mercurial (and bigfiles)](https://github.com/enlight/ue4-hg-plugin)
