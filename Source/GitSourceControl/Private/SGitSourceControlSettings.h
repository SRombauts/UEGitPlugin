// Copyright (c) 2014-2017 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "SlateFwd.h"
#include "ISourceControlOperation.h"
#include "ISourceControlProvider.h"

enum class ECheckBoxState : uint8;

class SGitSourceControlSettings : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SGitSourceControlSettings) {}
	
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

	~SGitSourceControlSettings();

private:

	/** Delegates to get Git binary path from/to settings */
	FText GetBinaryPathText() const;
	void OnBinaryPathTextCommited(const FText& InText, ETextCommit::Type InCommitType) const;

	/** Delegate to get repository root, user name and email from provider */
	FText GetPathToRepositoryRoot() const;
	FText GetUserName() const;
	FText GetUserEmail() const;

	EVisibility MustInitializeGitRepository() const;
	bool CanInitializeGitRepository() const;
	bool CanInitializeGitLfs() const;
	bool CanUseGitLfsLocking() const;

	/** Delegate to initialize a new Git repository */
	FReply OnClickedInitializeGitRepository();

	void OnCheckedCreateGitIgnore(ECheckBoxState NewCheckedState);
	bool bAutoCreateGitIgnore;

	void OnCheckedCreateGitAttributes(ECheckBoxState NewCheckedState);
	bool bAutoCreateGitAttributes;

	void OnCheckedUseGitLfsLocking(ECheckBoxState NewCheckedState);
	ECheckBoxState IsUsingGitLfsLocking() const;
	bool GetIsUsingGitLfsLocking() const;

	void OnLfsUserNameCommited(const FText& InText, ETextCommit::Type InCommitType);
	FText GetLfsUserName() const;

	void OnCheckedInitialCommit(ECheckBoxState NewCheckedState);
	bool GetAutoInitialCommit() const;
	bool bAutoInitialCommit;
	void OnInitialCommitMessageCommited(const FText& InText, ETextCommit::Type InCommitType);
	FText GetInitialCommitMessage() const;
	FText InitialCommitMessage;

	void OnRemoteUrlCommited(const FText& InText, ETextCommit::Type InCommitType);
	FText GetRemoteUrl() const;
	FText RemoteUrl;

	/** Launch initial asynchronous add and commit operations */
	void LaunchMarkForAddOperation(const TArray<FString>& InFiles);
	void LaunchCheckInOperation();

	/** Delegate called when a source control operation has completed */
	void OnSourceControlOperationComplete(const FSourceControlOperationRef& InOperation, ECommandResult::Type InResult);

	/** Asynchronous operation progress notifications */
	TWeakPtr<SNotificationItem> OperationInProgressNotification;
	
	void DisplayInProgressNotification(const FSourceControlOperationRef& InOperation);
	void RemoveInProgressNotification();
	void DisplaySuccessNotification(const FSourceControlOperationRef& InOperation);
	void DisplayFailureNotification(const FSourceControlOperationRef& InOperation);
};
