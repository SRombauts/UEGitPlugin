// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevSettings.h"
#include "GitSourceControlDevModule.h"
#include "GitSourceControlDevProvider.h"
#include "GitSourceControlDevUtils.h"
#include "SourceControlHelpers.h"

namespace GitSettingsConstants
{

/** The section of the ini file we load our settings from */
static const FString SettingsSection = TEXT("GitSourceControlDev.GitSourceControlDevSettings");

}

const FString& FGitSourceControlDevSettings::GetBinaryPath() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return BinaryPath;
}

void FGitSourceControlDevSettings::SetBinaryPath(const FString& InString)
{
	FScopeLock ScopeLock(&CriticalSection);
	BinaryPath = InString;
}

// This is called at startup nearly before anything else in our module: BinaryPath will then be used by the provider
void FGitSourceControlDevSettings::LoadSettings()
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& IniFile = SourceControlHelpers::GetSettingsIni();
	bool bLoaded = GConfig->GetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), BinaryPath, IniFile);
	if(!bLoaded || BinaryPath.IsEmpty())
	{
		BinaryPath = GitSourceControlDevUtils::FindGitBinaryPath();
	}
}

void FGitSourceControlDevSettings::SaveSettings() const
{
	FScopeLock ScopeLock(&CriticalSection);

	// Re-Check provided git binary path for each change
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	GitSourceControlDev.GetProvider().CheckGitAvailability();
	if (GitSourceControlDev.GetProvider().IsAvailable())
	{
		const FString& IniFile = SourceControlHelpers::GetSettingsIni();
		GConfig->SetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), *BinaryPath, IniFile);
	}
}