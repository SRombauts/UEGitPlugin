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
	virtual int32 GetHistorySize() const final;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetHistoryItem( int32 HistoryIndex ) const final;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FindHistoryRevision( int32 RevisionNumber ) const final;
	virtual FName GetIconName() const final;
	virtual FName GetSmallIconName() const final;
	virtual FText GetDisplayName() const final;
	virtual FText GetDisplayTooltip() const final;
	virtual const FString& GetFilename() const final;
	virtual const FDateTime& GetTimeStamp() const final;
	virtual bool CanCheckIn() const /* @todo for UE4.3: final */;
	virtual bool CanCheckout() const final;
	virtual bool IsCheckedOut() const final;
	virtual bool IsCheckedOutOther(FString* Who = nullptr) const final;
	virtual bool IsCurrent() const final;
	virtual bool IsSourceControlled() const final;
	virtual bool IsAdded() const final;
	virtual bool IsDeleted() const final;
	virtual bool IsIgnored() const final;
	virtual bool CanEdit() const final;
	virtual bool IsUnknown() const final;
	virtual bool IsModified() const final;
	virtual bool CanAdd() const final;

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
