Unreal Engine 4 Git Source Control Plugin
-----------------------------------------

UE4GitPlugin is a simple Git Source Control Plugin for Unreal Engine 4.2

### Status

[Download the last release](https://github.com/SRombauts/UE4GitPlugin/releases)

Beta version 0.5:
- display status icons to show modified/added/deleted/untracked files
- show history of a file
- diff against depot or between previous versions of a file
- revert modifications of a file
- add a file
- delete a file
- checkin/commit a file (cannot handle atomically more than 20 files)
- show current branch name in status text

What *cannot* be done presently:
- initialize a new Git local repository ('git init') to manager your UE4 Game Project.
- configure user name & email ('git config user.name' & git config user.email')
- commit description message cannot take more than one line (Editor limitation)
- Pull/Fetch/Push are not in the current Editor workflow
- Branch and Merge are not in the current Editor workflow (but on Epic Roadmap)
- Amend a commit & Add file to Index are not in the current Editor workflow

Wishlist:
- [git-annexe and/or git-media - #1 feature request](https://github.com/SRombauts/UE4GitPlugin/issues/1)

Known issues:
- reverting an asset does not seem to update content in Editor! Issue in Editor?
- renaming an asset does not seem to be handled correctly by the Editor...
- renamed file may not be tracked correctly (not yet tested, see above)
- file history does not report file size
- Windows only (64bits)

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

Take care of using the name **GitSourceControl** as destination directory (same name as the "GitSourceControl.uplugin" file):

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

Copyright (c) 2014 Sébastien Rombauts (sebastien.rombauts@gmail.com)

Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
or copy at http://opensource.org/licenses/MIT)

## How to contribute
### GitHub website
The most efficient way to help and contribute to this wrapper project is to
use the tools provided by GitHub:
- please fill bug reports and feature requests here: https://github.com/SRombauts/UE4GitPlugin/issues
- fork the repository, make some small changes and submit them with independant pull-request

### Contact
You can also email me directly, I will answer any questions and requests.

### Coding Style Guidelines
The source code follow the UnreaEngine official [Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/index.html) :
- CamelCase naming convention, with a prefix letter to differentiate classes ('F'), interfaces ('I'), templates ('T')
- files (.cpp/.h) are named like the class they contains
- Doxygen comments, documentation is located with declaration, on headers
- Use portable common features of C++11 like nullptr, auto, range based for, OVERRIDE macro
- Braces on their own line
- Tabs to indent code, with a width of 4 characters

## See also

- [ue4-hg-plugin for Mercurial (and bigfiles)](https://github.com/enlight/ue4-hg-plugin)
- [UE4 Git Plugin website](http://srombauts.github.com/UE4GitPlugin).
