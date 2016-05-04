// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "GitSourceControlDevState.h"

#define LOCTEXT_NAMESPACE "GitSourceControlDev.State"

int32 FGitSourceControlDevState::GetHistorySize() const
{
	return History.Num();
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlDevState::GetHistoryItem( int32 HistoryIndex ) const
{
	check(History.IsValidIndex(HistoryIndex));
	return History[HistoryIndex];
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlDevState::FindHistoryRevision( int32 RevisionNumber ) const
{
	for(const auto& Revision : History)
	{
		if(Revision->GetRevisionNumber() == RevisionNumber)
		{
			return Revision;
		}
	}

	return nullptr;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlDevState::FindHistoryRevision(const FString& InRevision) const
{
	for(const auto& Revision : History)
	{
		if(Revision->GetRevision() == InRevision)
		{
			return Revision;
		}
	}

	return nullptr;
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlDevState::GetBaseRevForMerge() const
{
	for(const auto& Revision : History)
	{
		// look for the the SHA1 id of the file, not the commit id (revision)
		if(Revision->FileHash == PendingMergeBaseFileHash)
		{
			return Revision;
		}
	}

	return nullptr;
}

// @todo add Slate icons for git specific states (Added vs Modified, Copied vs Conflicted...)
FName FGitSourceControlDevState::GetIconName() const
{
	switch(WorkingCopyState) //-V719
	{
	case EWorkingCopyState::Modified:
		return FName("Subversion.CheckedOut");
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
		return FName("Subversion.OpenForAdd");
	case EWorkingCopyState::Deleted:
		return FName("Subversion.MarkedForDelete");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot");
	case EWorkingCopyState::Missing: // @todo Missing files does not currently show in Editor (but should probably)
		UE_LOG(LogSourceControl, Log, TEXT("EWorkingCopyState::Missing"));
//	case EWorkingCopyState::Unchanged:
		// Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	}

	return NAME_None;
}

FName FGitSourceControlDevState::GetSmallIconName() const
{
	switch(WorkingCopyState) //-V719
	{
	case EWorkingCopyState::Modified:
		return FName("Subversion.CheckedOut_Small");
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Renamed:
	case EWorkingCopyState::Copied:
		return FName("Subversion.OpenForAdd_Small");
	case EWorkingCopyState::Deleted:
		return FName("Subversion.MarkedForDelete_Small");
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision_Small");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot_Small");
	case EWorkingCopyState::Missing: // @todo Missing files does not currently show in Editor (but should probably)
		UE_LOG(LogSourceControl, Log, TEXT("EWorkingCopyState::Missing"));
//	case EWorkingCopyState::Unchanged:
		// Unchanged is the same as "Pristine" (not checked out) for Perforce, ie no icon
	}

	return NAME_None;
}

FText FGitSourceControlDevState::GetDisplayName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown", "Unknown");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Unchanged", "Unchanged");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added", "Added");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted", "Deleted");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified", "Modified");
	case EWorkingCopyState::Renamed:
		return LOCTEXT("Renamed", "Renamed");
	case EWorkingCopyState::Copied:
		return LOCTEXT("Copied", "Copied");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict", "Contents Conflict");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored", "Ignored");
	case EWorkingCopyState::Merged:
		return LOCTEXT("Merged", "Merged");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled", "Not Under Source Control");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing", "Missing");
	}

	return FText();
}

FText FGitSourceControlDevState::GetDisplayTooltip() const
{
	switch(WorkingCopyState) //-V719
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown_Tooltip", "Unknown source control state");
	case EWorkingCopyState::Unchanged:
		return LOCTEXT("Pristine_Tooltip", "There are no modifications");
	case EWorkingCopyState::Added:
		return LOCTEXT("Added_Tooltip", "Item is scheduled for addition");
	case EWorkingCopyState::Deleted:
		return LOCTEXT("Deleted_Tooltip", "Item is scheduled for deletion");
	case EWorkingCopyState::Modified:
		return LOCTEXT("Modified_Tooltip", "Item has been modified");
	case EWorkingCopyState::Conflicted:
		return LOCTEXT("ContentsConflict_Tooltip", "The contents (as opposed to the properties) of the item conflict with updates received from the repository.");
	case EWorkingCopyState::Ignored:
		return LOCTEXT("Ignored_Tooltip", "Item is being ignored.");
	case EWorkingCopyState::Merged:
		return LOCTEXT("Merged_Tooltip", "Item has been merged.");
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled_Tooltip", "Item is not under version control.");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing_Tooltip", "Item is missing (e.g., you moved or deleted it without using Git). This also indicates that a directory is incomplete (a checkout or update was interrupted).");
	}

	return FText();
}

const FString& FGitSourceControlDevState::GetFilename() const
{
	return LocalFilename;
}

const FDateTime& FGitSourceControlDevState::GetTimeStamp() const
{
	return TimeStamp;
}

// TODO : missing ?
// @todo Test: don't show deleted as they should not appear? 
//	case EWorkingCopyState::Deleted:
//	case EWorkingCopyState::Missing:
// Deleted and Missing assets cannot appear in the Content Browser
bool FGitSourceControlDevState::CanCheckIn() const
{
	return WorkingCopyState == EWorkingCopyState::Added
		|| WorkingCopyState == EWorkingCopyState::Deleted
		|| WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed;
}

bool FGitSourceControlDevState::CanCheckout() const
{
	return false; // With Git all tracked files in the working copy are always already checked-out (as opposed to Perforce)
}

bool FGitSourceControlDevState::IsCheckedOut() const
{
	return IsSourceControlled(); // With Git all tracked files in the working copy are always checked-out (as opposed to Perforce)
}

bool FGitSourceControlDevState::IsCheckedOutOther(FString* Who) const
{
	return false; // Git does not lock checked-out files as Perforce does
}

bool FGitSourceControlDevState::IsCurrent() const
{
	return true; // @todo check the state of the HEAD versus the state of tracked branch on remote
}

bool FGitSourceControlDevState::IsSourceControlled() const
{
	return WorkingCopyState != EWorkingCopyState::NotControlled && WorkingCopyState != EWorkingCopyState::Ignored && WorkingCopyState != EWorkingCopyState::Unknown;
}

bool FGitSourceControlDevState::IsAdded() const
{
	return WorkingCopyState == EWorkingCopyState::Added;
}

bool FGitSourceControlDevState::IsDeleted() const
{
	return WorkingCopyState == EWorkingCopyState::Deleted;
}

bool FGitSourceControlDevState::IsIgnored() const
{
	return WorkingCopyState == EWorkingCopyState::Ignored;
}

bool FGitSourceControlDevState::CanEdit() const
{
	return true; // With Git all files in the working copy are always editable (as opposed to Perforce)
}

bool FGitSourceControlDevState::IsUnknown() const
{
	return WorkingCopyState == EWorkingCopyState::Unknown;
}

bool FGitSourceControlDevState::IsModified() const
{
	// Warning: for Perforce, a checked-out file is locked for modification (whereas with Git all tracked files are checked-out),
	// so for a clean "check-in" (commit) checked-out files unmodified should be removed from the changeset (the index)
	// http://stackoverflow.com/questions/12357971/what-does-revert-unchanged-files-mean-in-perforce
	//
	// Thus, before check-in UE4 Editor call RevertUnchangedFiles() in PromptForCheckin() and CheckinFiles().
	//
	// So here we must take care to enumerate all states that need to be commited,
	// all other will be discarded :
	//  - Unknown
	//  - Unchanged
	//  - NotControlled
	//  - Ignored
	return WorkingCopyState == EWorkingCopyState::Added
		|| WorkingCopyState == EWorkingCopyState::Deleted
		|| WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed
		|| WorkingCopyState == EWorkingCopyState::Copied
		|| WorkingCopyState == EWorkingCopyState::Conflicted
		|| WorkingCopyState == EWorkingCopyState::Missing;
}


bool FGitSourceControlDevState::CanAdd() const
{
	return WorkingCopyState == EWorkingCopyState::NotControlled;
}

bool FGitSourceControlDevState::IsConflicted() const
{
	return WorkingCopyState == EWorkingCopyState::Conflicted;
}

#undef LOCTEXT_NAMESPACE
