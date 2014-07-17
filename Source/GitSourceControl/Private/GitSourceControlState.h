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
	virtual int32 GetHistorySize() const override final;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetHistoryItem(int32 HistoryIndex) const override final;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> FindHistoryRevision(int32 RevisionNumber) const override final;
	virtual TSharedPtr<class ISourceControlRevision, ESPMode::ThreadSafe> GetBaseRevForMerge() const override final;
	virtual FName GetIconName() const override final;
	virtual FName GetSmallIconName() const override final;
	virtual FText GetDisplayName() const override final;
	virtual FText GetDisplayTooltip() const override final;
	virtual const FString& GetFilename() const override final;
	virtual const FDateTime& GetTimeStamp() const override final;
	virtual bool CanCheckIn() const override final;
	virtual bool CanCheckout() const override final;
	virtual bool IsCheckedOut() const override final;
	virtual bool IsCheckedOutOther(FString* Who = nullptr) const override final;
	virtual bool IsCurrent() const override final;
	virtual bool IsSourceControlled() const override final;
	virtual bool IsAdded() const override final;
	virtual bool IsDeleted() const override final;
	virtual bool IsIgnored() const override final;
	virtual bool CanEdit() const override final;
	virtual bool IsUnknown() const override final;
	virtual bool IsModified() const override final;
	virtual bool CanAdd() const override final;
	virtual bool IsConflicted() const override final;

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
