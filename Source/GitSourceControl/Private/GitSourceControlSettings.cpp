// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlSettings.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlProvider.h"
#include "GitSourceControlUtils.h"
#include "SourceControlHelpers.h"

namespace GitSettingsConstants
{

/** The section of the ini file we load our settings from */
static const FString SettingsSection = TEXT("GitSourceControl.GitSourceControlSettings");

}

const FString& FGitSourceControlSettings::GetBinaryPath() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return BinaryPath;
}

void FGitSourceControlSettings::SetBinaryPath(const FString& InString)
{
	FScopeLock ScopeLock(&CriticalSection);
	BinaryPath = InString;
}

// This is called at startup nearly before anything else in our module: BinaryPath will then be used by the provider
void FGitSourceControlSettings::LoadSettings()
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& IniFile = SourceControlHelpers::GetSettingsIni();
	bool bLoaded = GConfig->GetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), BinaryPath, IniFile);
	if(!bLoaded || BinaryPath.IsEmpty())
	{
		BinaryPath = GitSourceControlUtils::FindGitBinaryPath();
	}
}

void FGitSourceControlSettings::SaveSettings() const
{
	FScopeLock ScopeLock(&CriticalSection);

	// Re-Check provided git binary path for each change
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	GitSourceControl.GetProvider().CheckGitAvailability();
	if (GitSourceControl.GetProvider().IsAvailable())
	{
		const FString& IniFile = SourceControlHelpers::GetSettingsIni();
		GConfig->SetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), *BinaryPath, IniFile);
	}
}