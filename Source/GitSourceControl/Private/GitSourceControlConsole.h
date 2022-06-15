// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

/**
 * Editor only console commands.
 *
 * Such commands can be executed from the editor output log window, but also from command line arguments,
 * from Editor Blueprints utilities, or from C++ Code using. eg. GEngine->Exec("git status Content/");
 */
class FGitSourceControlConsole
{
public:
	void Register();
	void Unregister();

private:
	// Git Command Line Interface: Run 'git' commands directly from the Unreal Editor Console.
	void ExecuteGitConsoleCommand(const TArray<FString>& a_args);

	/** Console command for interacting with 'git' CLI directly */
	TUniquePtr<FAutoConsoleCommand> GitConsoleCommand;
};