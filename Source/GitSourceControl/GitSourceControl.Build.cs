// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

using UnrealBuildTool;

public class GitSourceControl : ModuleRules
{
	public GitSourceControl(ReadOnlyTargetRules Target) : base(Target)
	{
		// Enable the Include-What-You-Use (IWYU) UE4.15 policy (see https://docs.unrealengine.com/en-us/Programming/UnrealBuildSystem/IWYUReferenceGuide)
		// "Shared PCHs may be used if an explicit private PCH is not set through PrivatePCHHeaderFile. In either case, none of the source files manually include a module PCH, and should include a matching header instead."
		bEnforceIWYU = true;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/GitSourceControlPrivatePCH.h";

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"InputCore",
				"DesktopWidgets",
				"EditorStyle",
				"UnrealEd",
				"SourceControl",
				"Projects",
			}
		);
	}
}
