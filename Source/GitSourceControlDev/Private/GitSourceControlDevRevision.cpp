// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevRevision.h"
#include "GitSourceControlDevModule.h"
#include "GitSourceControlDevProvider.h"
#include "GitSourceControlDevUtils.h"
#include "SGitSourceControlDevSettings.h"

#define LOCTEXT_NAMESPACE "GitSourceControlDev"

bool FGitSourceControlDevRevision::Get( FString& InOutFilename ) const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	const FString& PathToGitBinary = GitSourceControlDev.AccessSettings().GetBinaryPath();
	const FString& PathToRepositoryRoot = GitSourceControlDev.GetProvider().GetPathToRepositoryRoot();

	// if a filename for the temp file wasn't supplied generate a unique-ish one
	if(InOutFilename.Len() == 0)
	{
		// create the diff dir if we don't already have it (Git wont)
		IFileManager::Get().MakeDirectory(*FPaths::DiffDir(), true);
		// create a unique temp file name based on the unique commit Id
		const FString TempFileName = FString::Printf(TEXT("%stemp-%s-%s"), *FPaths::DiffDir(), *CommitId, *FPaths::GetCleanFilename(Filename));
		InOutFilename = FPaths::ConvertRelativePathToFull(TempFileName);
	}

	// Diff against the revision
	const FString Parameter = FString::Printf(TEXT("%s:%s"), *CommitId, *Filename);

	bool bCommandSuccessful;
	if(FPaths::FileExists(InOutFilename))
	{
		bCommandSuccessful = true; // if the temp file already exists, reuse it directly
	}
	else
	{
		bCommandSuccessful = GitSourceControlDevUtils::RunDumpToFile(PathToGitBinary, PathToRepositoryRoot, Parameter, InOutFilename);
	}
	return bCommandSuccessful;
}

bool FGitSourceControlDevRevision::GetAnnotated( TArray<FAnnotationLine>& OutLines ) const
{
	return false;
}

bool FGitSourceControlDevRevision::GetAnnotated( FString& InOutFilename ) const
{
	return false;
}

const FString& FGitSourceControlDevRevision::GetFilename() const
{
	return Filename;
}

int32 FGitSourceControlDevRevision::GetRevisionNumber() const
{
	return RevisionNumber;
}

const FString& FGitSourceControlDevRevision::GetRevision() const
{
	return ShortCommitId;
}

const FString& FGitSourceControlDevRevision::GetDescription() const
{
	return Description;
}

const FString& FGitSourceControlDevRevision::GetUserName() const
{
	return UserName;
}

const FString& FGitSourceControlDevRevision::GetClientSpec() const
{
	static FString EmptyString(TEXT(""));
	return EmptyString;
}

const FString& FGitSourceControlDevRevision::GetAction() const
{
	return Action;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlDevRevision::GetBranchSource() const
{
	// @todo if this revision was copied from some other revision, then that source revision should
	//       be returned here (this should be determined when history is being fetched)
	return nullptr;
}

const FDateTime& FGitSourceControlDevRevision::GetDate() const
{
	return Date;
}

int32 FGitSourceControlDevRevision::GetCheckInIdentifier() const
{
	// in Git, revisions apply to the whole repository so (in Perforce terms) the revision *is* the changelist
	return RevisionNumber;
}

int32 FGitSourceControlDevRevision::GetFileSize() const
{
	return FileSize;
}

#undef LOCTEXT_NAMESPACE
