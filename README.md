Unreal Engine 4 Git Source Control Plugin
-----------------------------------------

UE4GitPlugin is a simple Git Source Control Plugin for Unreal Engine 4.x

See the website http://srombauts.github.com/UE4GitPlugin on GitHub.

### Status

Pre-Alpha under early development: v0.0 only displays icons to show modified files for UE 4.1

### Getting started

#### Install into UnrealEngine or Build from sources

Clone this repository under "[UnrealEngine]\Engine\Plugins\Developer\":

```bash
git clone https://github.com/SRombauts/UE4GitPlugin.git GitSourceControl
```

#### Manage your Game Project with Git

Use your favorite Git program to manage your whole game project directory Unreal Projects\[YourGameProject].

#### Activating Source Control within the Editor

File->Connect To Source Control... -> Git: Accept Settings

[Source Control official Documentation](https://docs.unrealengine.com/latest/INT/Engine/UI/SourceControl/index.html)

### License

Copyright (c) 2014 Sébastien Rombauts (sebastien.rombauts@gmail.com)

Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
or copy at http://opensource.org/licenses/MIT)

## How to contribute
### GitHub website
The most efficient way to help and contribute to this wrapper project is to
use the tools provided by GitHub:
- please fill bug reports and feature requests here: https://github.com/SRombauts/SQLiteCpp/issues
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

## See also - Some other useful UE4 plugins:

 - [ue4-hg-plugin](https://github.com/enlight/ue4-hg-plugin)
