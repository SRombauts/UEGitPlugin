// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "SGitInitDialog.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlOperations.h"

#define LOCTEXT_NAMESPACE "SGitInitDialog"

#if SOURCE_CONTROL_WITH_SLATE

#include "SNotificationList.h"
#include "NotificationManager.h"
//InputCore is required for SButton
#include "InputCore.h"

class SGitInitDialogTitleBar : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SGitInitDialogTitleBar) {}

	/** A reference to the parent window */
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ParentWindowPtr = InArgs._ParentWindow;

		SBorder::Construct(SBorder::FArguments()
			.BorderImage(FEditorStyle::GetBrush("Window.Title.Active"))
			[
				SNew(SHorizontalBox)
				.Visibility(EVisibility::HitTestInvisible)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GitSourceControlInitTitle", "Git Source Control Init"))
					.TextStyle(FEditorStyle::Get(), "Window.TitleText")
					.Visibility(EVisibility::HitTestInvisible)
				]
			]
		);
	}

	virtual EWindowZone::Type GetWindowZoneOverride() const override
	{
		return EWindowZone::TitleBar;
	}

private:
	/** The parent window of this widget */
	TWeakPtr<SWindow> ParentWindowPtr;
};

void SGitInitDialog::Construct(const FArguments& InArgs)
{
	ParentWindowPtr = InArgs._ParentWindow;
	//SourceControlLoginClosed = InArgs._OnSourceControlLoginClosed;

	//ConnectionState = ELoginConnectionState::Disconnected;

	FGitSourceControlModule& SourceControlModule = FGitSourceControlModule::Get();

	ChildSlot
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("ChildWindow.Background"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0.0f, 0.0f, 0.0f, 10.0f)
					[
						SNew(SGitInitDialogTitleBar)
						.ParentWindow(InArgs._ParentWindow)
					]
				]
				+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(0.0f, 6.0f, 0.0f, 0.0f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Padding(4.0f)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 4.0f)
								[
									SNew(SBorder)
									.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryBottom"))
									.Padding(FMargin(4.0f, 12.0f))
									[
										SNew(STextBlock)
										.WrapTextAt(500.0f)
										.Text(LOCTEXT("GitSourceControlInitText", "Git is currently unititialized.\n\nTo initialize Git, please click Init."))
									]
								]
						]
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(8.0f, 16.0f, 8.0f, 8.0f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.HAlign(HAlign_Right)
							[
								SNew(SUniformGridPanel)
								.SlotPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
								+ SUniformGridPanel::Slot(0, 0)
								[
									SNew(SButton)
									.HAlign(HAlign_Center)
									.Text(LOCTEXT("GitInit", "Initialize Git"))
									.OnClicked(this, &SGitInitDialog::OnClickedInit)
								]
								+ SUniformGridPanel::Slot(1, 0)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.Text(LOCTEXT("Cancel", "Cancel"))
										.OnClicked(this, &SGitInitDialog::OnClickedCancel)
									]
							]
					]
			]
		];
}

FReply SGitInitDialog::OnClickedInit()
{
	FGitSourceControlModule& SourceControlModule = FGitSourceControlModule::Get();
	TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe> InitOperation = ISourceControlOperation::Create<FGitInit>();
	if (SourceControlModule.GetProvider().Execute(InitOperation, TArray<FString>(), EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateSP(this, &SGitInitDialog::SourceControlOperationComplete)) == ECommandResult::Succeeded){
		DisplayInitSuccess();
	}
	else
	{
		DisplayInitError();
	}

	return FReply::Handled();
}

FReply SGitInitDialog::OnClickedCancel()
{
	if (ParentWindowPtr.IsValid())
	{
		ParentWindowPtr.Pin()->RequestDestroyWindow();
	}
	return FReply::Handled();
}

void SGitInitDialog::SourceControlOperationComplete(const FSourceControlOperationRef& InOperation, ECommandResult::Type InResult)
{
	if (InResult == ECommandResult::Succeeded)
	{
		DisplayInitSuccess();
	}
	else
	{
		DisplayInitError();
	}
}

void SGitInitDialog::DisplayInitError() const
{
	FNotificationInfo Info(LOCTEXT("FailedToInit", "Git Source control initialization failed!"));
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	Info.Image = FEditorStyle::GetBrush(TEXT("NotificationList.FailureImage"));
	FSlateNotificationManager::Get().AddNotification(Info);

	if (ParentWindowPtr.IsValid())
	{
		ParentWindowPtr.Pin()->RequestDestroyWindow();
	}
}

void SGitInitDialog::DisplayInitSuccess() const
{
	FNotificationInfo Info(LOCTEXT("ConnectionSuccessful", "Source control initialization was successful!"));
	Info.bFireAndForget = true;
	Info.bUseSuccessFailIcons = true;
	Info.Image = FEditorStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
	FSlateNotificationManager::Get().AddNotification(Info);

	if (ParentWindowPtr.IsValid())
	{
		ParentWindowPtr.Pin()->RequestDestroyWindow();
	}
}

#endif SOURCE_CONTROL_WITH_SLATE

#undef LOCTEXT_NAMESPACE
