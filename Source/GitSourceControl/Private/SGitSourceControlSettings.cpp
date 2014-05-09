// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "SGitSourceControlSettings.h"
#include "GitSourceControlModule.h"

#define LOCTEXT_NAMESPACE "SGitSourceControlSettings"

void SGitSourceControlSettings::Construct(const FArguments& InArgs)
{
	FSlateFontInfo Font = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( FEditorStyle::GetBrush("DetailsView.CategoryBottom") )
		.Padding( FMargin( 0.0f, 3.0f, 0.0f, 0.0f ) )
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BinaryPathLabel", "Git Path"))
					.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Git binary"))
					.Font(Font)
				]
			]
			+SHorizontalBox::Slot()
			.FillWidth(2.0f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(2.0f)
				[
					SNew(SEditableTextBox)
					.Text(this, &SGitSourceControlSettings::GetBinaryPathText)
					.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Git binary"))
					.OnTextCommitted(this, &SGitSourceControlSettings::OnBinaryPathTextCommited)
					.OnTextChanged(this, &SGitSourceControlSettings::OnBinaryPathTextCommited, ETextCommit::Default)
					.Font(Font)
				]
			]
		]
	];
}

FText SGitSourceControlSettings::GetBinaryPathText() const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return FText::FromString(GitSourceControl.AccessSettings().GetBinaryPath());
}

void SGitSourceControlSettings::OnBinaryPathTextCommited(const FText& InText, ETextCommit::Type InCommitType) const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	GitSourceControl.AccessSettings().SetBinaryPath(InText.ToString());
	GitSourceControl.SaveSettings();
}

#undef LOCTEXT_NAMESPACE