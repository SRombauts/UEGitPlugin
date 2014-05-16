// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlRevision.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

bool FGitSourceControlRevision::Get( FString& InOutFilename ) const
{
	// @todo git show file blob content into a temporary file
	return false;
}

bool FGitSourceControlRevision::GetAnnotated( TArray<FAnnotationLine>& OutLines ) const
{
	return false;
}

bool FGitSourceControlRevision::GetAnnotated( FString& InOutFilename ) const
{
	return false;
}

const FString& FGitSourceControlRevision::GetFilename() const
{
	return Filename;
}

int32 FGitSourceControlRevision::GetRevisionNumber() const
{
	return RevisionNumber;
}

const FString& FGitSourceControlRevision::GetDescription() const
{
	return Description;
}

const FString& FGitSourceControlRevision::GetUserName() const
{
	return UserName;
}

const FString& FGitSourceControlRevision::GetClientSpec() const
{
	static FString EmptyString(TEXT(""));
	return EmptyString;
}

const FString& FGitSourceControlRevision::GetAction() const
{
	return Action;
}

const FDateTime& FGitSourceControlRevision::GetDate() const
{
	return Date;
}

int32 FGitSourceControlRevision::GetCheckInIdentifier() const
{
	// in Git, revisions apply to the whole repository so (in Perforce terms) the revision *is* the changelist
	return RevisionNumber;
}

int32 FGitSourceControlRevision::GetFileSize() const
{
	// @todo git log does not give us the file size, but we could run a specifi command
	return 0;
}

#undef LOCTEXT_NAMESPACE
