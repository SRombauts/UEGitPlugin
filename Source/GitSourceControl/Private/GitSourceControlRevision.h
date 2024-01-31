// Copyright (c) 2014-2022 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "ISourceControlRevision.h"

#include "Runtime/Launch/Resources/Version.h"

/** Revision of a file, linked to a specific commit */
class FGitSourceControlRevision : public ISourceControlRevision
{
public:

	/** ISourceControlRevision interface */
#if ENGINE_MAJOR_VERSION == 5
	virtual bool Get(FString& InOutFilename, EConcurrency::Type InConcurrency = EConcurrency::Synchronous) const override;
#else
	virtual bool Get(FString& InOutFilename) const override;
#endif
	virtual bool GetAnnotated(TArray<FAnnotationLine>& OutLines) const override;
	virtual bool GetAnnotated(FString& InOutFilename) const override;
	virtual const FString& GetFilename() const override;
	virtual int32 GetRevisionNumber() const override;
	virtual const FString& GetRevision() const override;
	virtual const FString& GetDescription() const override;
	virtual const FString& GetUserName() const override;
	virtual const FString& GetClientSpec() const override;
	virtual const FString& GetAction() const override;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetBranchSource() const override;
	virtual const FDateTime& GetDate() const override;
	virtual int32 GetCheckInIdentifier() const override;
	virtual int32 GetFileSize() const override;

public:

	/** The filename this revision refers to */
	FString Filename;

	/** The full hexadecimal SHA1 id of the commit this revision refers to */
	FString CommitId;

	/** The short hexadecimal SHA1 id (8 first hex char out of 40) of the commit: the string to display */
	FString ShortCommitId;

	/** The numeric value of the short SHA1 (8 first hex char out of 40) */
	int32 CommitIdNumber = 0;

	/** The index of the revision in the history (SBlueprintRevisionMenu assumes order for the "Depot" label) */
	int32 RevisionNumber = 0;

	/** The SHA1 identifier of the file at this revision */
	FString FileHash;

	/** The description of this revision */
	FString Description;

	/** The user that made the change */
	FString UserName;

	/** The action (add, edit, branch etc.) performed at this revision */
	FString Action;

	/** Source of move ("branch" in Perforce term) if any */
	TSharedPtr<FGitSourceControlRevision, ESPMode::ThreadSafe> BranchSource;

	/** The date this revision was made */
	FDateTime Date;

	/** The size of the file at this revision */
	int32 FileSize;

	FString PathToRepoRoot;
};

/** History composed of the last 100 revisions of the file */
typedef TArray< TSharedRef<FGitSourceControlRevision, ESPMode::ThreadSafe> >	TGitSourceControlHistory;
