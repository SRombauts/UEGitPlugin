// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "ISourceControlState.h"
#include "GitSourceControlRevision.h"

namespace EWorkingCopyState
{
	enum Type
	{
		Unknown,
		Unchanged, // often called "clean"
		Added,
		Deleted,
		Modified,
		Renamed,
		Copied,
		Missing,
		Conflicted,
		NotControlled,
		Ignored,
	};
}

class FGitSourceControlState : public ISourceControlState, public TSharedFromThis<FGitSourceControlState, ESPMode::ThreadSafe>
{
public:
	FGitSourceControlState( const FString& InLocalFilename )
		: LocalFilename(InLocalFilename)
		, WorkingCopyState(EWorkingCopyState::Unknown)
		, TimeStamp(0)
	{
	}

	/** ISourceControlState interface */
	virtual int32 GetHistorySize() const OVERRIDE;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetHistoryItem( int32 HistoryIndex ) const OVERRIDE;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FindHistoryRevision( int32 RevisionNumber ) const OVERRIDE;
	virtual FName GetIconName() const OVERRIDE;
	virtual FName GetSmallIconName() const OVERRIDE;
	virtual FText GetDisplayName() const OVERRIDE;
	virtual FText GetDisplayTooltip() const OVERRIDE;
	virtual const FString& GetFilename() const OVERRIDE;
	virtual const FDateTime& GetTimeStamp() const OVERRIDE;
	virtual bool CanCheckIn() const /* @todo for UE4.3: OVERRIDE */;
	virtual bool CanCheckout() const OVERRIDE;
	virtual bool IsCheckedOut() const OVERRIDE;
	virtual bool IsCheckedOutOther(FString* Who = nullptr) const OVERRIDE;
	virtual bool IsCurrent() const OVERRIDE;
	virtual bool IsSourceControlled() const OVERRIDE;
	virtual bool IsAdded() const OVERRIDE;
	virtual bool IsDeleted() const OVERRIDE;
	virtual bool IsIgnored() const OVERRIDE;
	virtual bool CanEdit() const OVERRIDE;
	virtual bool IsUnknown() const OVERRIDE;
	virtual bool IsModified() const OVERRIDE;
	virtual bool CanAdd() const OVERRIDE;

public:
	/** History of the item, if any */
	TGitSourceControlHistory History;

	/** Filename on disk */
	FString LocalFilename;

	/** State of the working copy */
	EWorkingCopyState::Type WorkingCopyState;

	/** The timestamp of the last update */
	FDateTime TimeStamp;
};
