// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "ISourceControlRevision.h"

/** Revision of a file, linked to a specific commit */
class FGitSourceControlRevision : public ISourceControlRevision, public TSharedFromThis<FGitSourceControlRevision, ESPMode::ThreadSafe>
{
public:
	FGitSourceControlRevision()
		: RevisionNumber(0)
	{
	}

	/** ISourceControlRevision interface */
	virtual bool Get( FString& InOutFilename ) const final;
	virtual bool GetAnnotated( TArray<FAnnotationLine>& OutLines ) const final;
	virtual bool GetAnnotated( FString& InOutFilename ) const final;
	virtual const FString& GetFilename() const final;
	virtual int32 GetRevisionNumber() const final;
	virtual const FString& GetDescription() const final;
	virtual const FString& GetUserName() const final;
	virtual const FString& GetClientSpec() const final;
	virtual const FString& GetAction() const final;
	virtual const FDateTime& GetDate() const final;
	virtual int32 GetCheckInIdentifier() const final;
	virtual int32 GetFileSize() const final;

public:

	/** The filename this revision refers to */
	FString Filename;

	/** The SHA1 id of the commit this revision refers to */
	FString CommitId;

	/** The revision number */
	int32 RevisionNumber;

	/** The description of this revision */
	FString Description;

	/** The user that made the change */
	FString UserName;

	/** The action (add, edit etc.) performed at this revision */
	FString Action;

	/** The date this revision was made */
	FDateTime Date;
};

/** History composed of the last 100 revisions of the file */
typedef TArray< TSharedRef<FGitSourceControlRevision, ESPMode::ThreadSafe> >	TGitSourceControlHistory;
