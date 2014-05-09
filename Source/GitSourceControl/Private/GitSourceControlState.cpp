// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlState.h"

#define LOCTEXT_NAMESPACE "GitSourceControl.State"

int32 FGitSourceControlState::GetHistorySize() const
{
	return History.Num();
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::GetHistoryItem( int32 HistoryIndex ) const
{
	check(History.IsValidIndex(HistoryIndex));
	return History[HistoryIndex];
}

TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FGitSourceControlState::FindHistoryRevision( int32 RevisionNumber ) const
{
	for(auto Iter(History.CreateConstIterator()); Iter; Iter++)
	{
		if((*Iter)->GetRevisionNumber() == RevisionNumber)
		{
			return *Iter;
		}
	}

	return NULL;
}

FName FGitSourceControlState::GetIconName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Modified:
	case EWorkingCopyState::Renamed:
		return FName("Subversion.OpenForAdd");
	case EWorkingCopyState::Missing:
	case EWorkingCopyState::Copied:
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot");
	}

	return NAME_None;
}

FName FGitSourceControlState::GetSmallIconName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Added:
	case EWorkingCopyState::Modified:
	case EWorkingCopyState::Renamed:
		return FName("Subversion.OpenForAdd_Small");
	case EWorkingCopyState::Missing:
	case EWorkingCopyState::Copied:
	case EWorkingCopyState::Conflicted:
		return FName("Subversion.NotAtHeadRevision_Small");
	case EWorkingCopyState::NotControlled:
		return FName("Subversion.NotInDepot_Small");
	}

	return NAME_None;
}

FText FGitSourceControlState::GetDisplayName() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown", "Unknown");
	case EWorkingCopyState::Pristine:
		return LOCTEXT("Pristine", "Pristine");
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
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled", "Not Under Source Control");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing", "Missing");
	}

	return FText();
}

FText FGitSourceControlState::GetDisplayTooltip() const
{
	switch(WorkingCopyState)
	{
	case EWorkingCopyState::Unknown:
		return LOCTEXT("Unknown_Tooltip", "Unknown source control state");
	case EWorkingCopyState::Pristine:
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
	case EWorkingCopyState::NotControlled:
		return LOCTEXT("NotControlled_Tooltip", "Item is not under version control.");
	case EWorkingCopyState::Missing:
		return LOCTEXT("Missing_Tooltip", "Item is missing (e.g., you moved or deleted it without using Git). This also indicates that a directory is incomplete (a checkout or update was interrupted).");
	}

	return FText();
}

const FString& FGitSourceControlState::GetFilename() const
{
	return LocalFilename;
}

const FDateTime& FGitSourceControlState::GetTimeStamp() const
{
	return TimeStamp;
}

bool FGitSourceControlState::CanCheckout() const
{
	return (WorkingCopyState == EWorkingCopyState::Pristine || WorkingCopyState == EWorkingCopyState::Modified);
}

bool FGitSourceControlState::IsCheckedOut() const
{
	return true;
}

bool FGitSourceControlState::IsCheckedOutOther(FString* Who) const
{
	return false;
}

bool FGitSourceControlState::IsCurrent() const
{
	return true;
}

bool FGitSourceControlState::IsSourceControlled() const
{
	return WorkingCopyState != EWorkingCopyState::NotControlled && WorkingCopyState != EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsAdded() const
{
	return WorkingCopyState == EWorkingCopyState::Added;
}

bool FGitSourceControlState::IsDeleted() const
{
	return WorkingCopyState == EWorkingCopyState::Deleted;
}

bool FGitSourceControlState::IsIgnored() const
{
	return WorkingCopyState == EWorkingCopyState::Ignored;
}

bool FGitSourceControlState::CanEdit() const
{
	return WorkingCopyState == EWorkingCopyState::Added;
}

bool FGitSourceControlState::IsUnknown() const
{
	return WorkingCopyState == EWorkingCopyState::Unknown;
}

bool FGitSourceControlState::IsModified() const
{
	return WorkingCopyState == EWorkingCopyState::Modified
		|| WorkingCopyState == EWorkingCopyState::Renamed
		|| WorkingCopyState == EWorkingCopyState::Copied
		|| WorkingCopyState == EWorkingCopyState::Conflicted;
}

#undef LOCTEXT_NAMESPACE