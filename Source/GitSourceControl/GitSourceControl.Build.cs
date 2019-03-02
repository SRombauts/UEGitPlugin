// Copyright (c) 2014-2019 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

using UnrealBuildTool;

public class GitSourceControl : ModuleRules
{
	public GitSourceControl(ReadOnlyTargetRules Target) : base(Target)
	{
		// Do not enforce "Include What You Use" UE4.15 policy
		// since it does not follow the same rules for In-Engine Plugins as for Game Project Plugins,
		// and as such prevents us to make a source code compiling as both.
		bEnforceIWYU = false;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"SourceControl",
				"Projects",
				"DesktopWidgets",
			}
		);
	}
}
