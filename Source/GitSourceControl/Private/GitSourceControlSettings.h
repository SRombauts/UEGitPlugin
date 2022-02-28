// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"

class FGitSourceControlSettings
{
public:
	/** Get the Git Binary Path */
	const FString GetBinaryPath() const;

	/** Set the Git Binary Path */
	bool SetBinaryPath(const FString& InString);

	/** Tell if using the Git LFS file Locking workflow */
	bool IsUsingGitLfsLocking() const;

	/** Configure the usage of Git LFS file Locking workflow */
	bool SetUsingGitLfsLocking(const bool InUsingGitLfsLocking);

	/** Get the username used by the Git LFS 2 File Locks server */
	const FString GetLfsUserName() const;

	/** Set the username used by the Git LFS 2 File Locks server */
	bool SetLfsUserName(const FString& InString);

	/** Load settings from ini file */
	void LoadSettings();

	/** Save settings to ini file */
	void SaveSettings() const;

private:
	/** A critical section for settings access */
	mutable FCriticalSection CriticalSection;

	/** Git binary path */
	FString BinaryPath;

	/** Tells if using the Git LFS file Locking workflow */
	bool bUsingGitLfsLocking;

	/** Username used by the Git LFS 2 File Locks server */
	FString LfsUserName;
};
