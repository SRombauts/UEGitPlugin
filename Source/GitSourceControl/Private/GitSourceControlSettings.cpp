// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlSettings.h"

#include "Misc/ScopeLock.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"
#include "SourceControlHelpers.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlUtils.h"

namespace GitSettingsConstants
{

/** The section of the ini file we load our settings from */
static const FString SettingsSection = TEXT("GitSourceControl.GitSourceControlSettings");

}

const FString FGitSourceControlSettings::GetBinaryPath() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return BinaryPath; // Return a copy to be thread-safe
}

bool FGitSourceControlSettings::SetBinaryPath(const FString& InString)
{
	FScopeLock ScopeLock(&CriticalSection);
	const bool bChanged = (BinaryPath != InString);
	if(bChanged)
	{
		BinaryPath = InString;
	}
	return bChanged;
}

/** Tell if using the Git LFS file Locking workflow */
bool FGitSourceControlSettings::IsUsingGitLfsLocking() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return bUsingGitLfsLocking;
}

/** Configure the usage of Git LFS file Locking workflow */
bool FGitSourceControlSettings::SetUsingGitLfsLocking(const bool InUsingGitLfsLocking)
{
	FScopeLock ScopeLock(&CriticalSection);
	const bool bChanged = (bUsingGitLfsLocking != InUsingGitLfsLocking);
	bUsingGitLfsLocking = InUsingGitLfsLocking;
	return bChanged;
}

const FString FGitSourceControlSettings::GetLfsUserName() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return LfsUserName; // Return a copy to be thread-safe
}

bool FGitSourceControlSettings::SetLfsUserName(const FString& InString)
{
	FScopeLock ScopeLock(&CriticalSection);
	const bool bChanged = (LfsUserName != InString);
	if (bChanged)
	{
		LfsUserName = InString;
	}
	return bChanged;
}

// This is called at startup nearly before anything else in our module: BinaryPath will then be used by the provider
void FGitSourceControlSettings::LoadSettings()
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& IniFile = SourceControlHelpers::GetSettingsIni();
	GConfig->GetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), BinaryPath, IniFile);
	GConfig->GetBool(*GitSettingsConstants::SettingsSection, TEXT("UsingGitLfsLocking"), bUsingGitLfsLocking, IniFile);
	GConfig->GetString(*GitSettingsConstants::SettingsSection, TEXT("LfsUserName"), LfsUserName, IniFile);
}

void FGitSourceControlSettings::SaveSettings() const
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& IniFile = SourceControlHelpers::GetSettingsIni();
	GConfig->SetString(*GitSettingsConstants::SettingsSection, TEXT("BinaryPath"), *BinaryPath, IniFile);
	GConfig->SetBool(*GitSettingsConstants::SettingsSection, TEXT("UsingGitLfsLocking"), bUsingGitLfsLocking, IniFile);
	GConfig->SetString(*GitSettingsConstants::SettingsSection, TEXT("LfsUserName"), *LfsUserName, IniFile);
}
