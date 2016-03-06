// Copyright (c) 2014-2016 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlDevPrivatePCH.h"
#include "SGitSourceControlDevSettings.h"
#include "GitSourceControlDevModule.h"
#include "GitSourceControlDevUtils.h"

#define LOCTEXT_NAMESPACE "SGitSourceControlDevSettings"

void SGitSourceControlDevSettings::Construct(const FArguments& InArgs)
{
	FSlateFontInfo Font = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	bAutoCreateGitIgnore = true;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage( FEditorStyle::GetBrush("DetailsView.CategoryBottom"))
		.Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
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
						.Text(this, &SGitSourceControlDevSettings::GetBinaryPathText)
						.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Git binary"))
						.OnTextCommitted(this, &SGitSourceControlDevSettings::OnBinaryPathTextCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
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
						.Text(LOCTEXT("RepositoryRootLabel", "Root of the repository"))
						.ToolTipText(LOCTEXT("RepositoryRootLabel_Tooltip", "Path to the root of the Git repository"))
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
						SNew(STextBlock)
						.Text(this, &SGitSourceControlDevSettings::GetPathToRepositoryRoot)
						.ToolTipText(LOCTEXT("RepositoryRootLabel_Tooltip", "Path to the root of the Git repository"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
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
						.Text(LOCTEXT("GitUserName", "User Name"))
						.ToolTipText(LOCTEXT("GitUserName_Tooltip", "User name configured for the Git repository"))
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
						SNew(STextBlock)
						.Text(this, &SGitSourceControlDevSettings::GetUserName)
						.ToolTipText(LOCTEXT("GitUserName_Tooltip", "User name configured for the Git repository"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
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
						.Text(LOCTEXT("GitUserEmail", "E-Mail"))
						.ToolTipText(LOCTEXT("GitUserEmail_Tooltip", "User e-mail configured for the Git repository"))
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
						SNew(STextBlock)
						.Text(this, &SGitSourceControlDevSettings::GetUserEmail)
						.ToolTipText(LOCTEXT("GitUserEmail_Tooltip", "User e-mail configured for the Git repository"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlDevSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(2.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					.AutoHeight()
					[
						SNew(SButton)
						.Text(LOCTEXT("GitInitRepository", "Initialize project with Git"))
						.ToolTipText(LOCTEXT("GitInitRepository_Tooltip", "Initialize current project as a new Git repository"))
						.OnClicked(this, &SGitSourceControlDevSettings::OnClickedInitializeGitRepository)
						.HAlign(HAlign_Center)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlDevSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(0.04f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SCheckBox)
						.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged(this, &SGitSourceControlDevSettings::OnCheckedCreateGitIgnore)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(0.96f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CreateGitIgnore", "Add a .gitignore file"))
						.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
						.Font(Font)
					]
				]
			]
		]
	];
}

FText SGitSourceControlDevSettings::GetBinaryPathText() const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	return FText::FromString(GitSourceControlDev.AccessSettings().GetBinaryPath());
}

void SGitSourceControlDevSettings::OnBinaryPathTextCommited(const FText& InText, ETextCommit::Type InCommitType) const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	GitSourceControlDev.AccessSettings().SetBinaryPath(InText.ToString());
	GitSourceControlDev.SaveSettings();
}

FText SGitSourceControlDevSettings::GetPathToRepositoryRoot() const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	return FText::FromString(GitSourceControlDev.GetProvider().GetPathToRepositoryRoot());
}

FText SGitSourceControlDevSettings::GetUserName() const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	return FText::FromString(GitSourceControlDev.GetProvider().GetUserName());
}

FText SGitSourceControlDevSettings::GetUserEmail() const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	return FText::FromString(GitSourceControlDev.GetProvider().GetUserEmail());
}

EVisibility SGitSourceControlDevSettings::CanInitializeGitRepository() const
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	const bool bGitAvailable = GitSourceControlDev.GetProvider().IsGitAvailable();
	const bool bGitRepositoryFound = GitSourceControlDev.GetProvider().IsEnabled();
	return (bGitAvailable && !bGitRepositoryFound) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SGitSourceControlDevSettings::OnClickedInitializeGitRepository()
{
	FGitSourceControlDevModule& GitSourceControlDev = FModuleManager::LoadModuleChecked<FGitSourceControlDevModule>("GitSourceControlDev");
	TArray<FString> InfoMessages;
	TArray<FString> ErrorMessages;
	const FString& PathToGitBinary = GitSourceControlDev.AccessSettings().GetBinaryPath();
	const FString PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	GitSourceControlDevUtils::RunCommand(TEXT("init"), PathToGitBinary, PathToGameDir, TArray<FString>(), TArray<FString>(), InfoMessages, ErrorMessages);
	// Check the new repository status to enable connection
	GitSourceControlDev.GetProvider().CheckGitAvailability();
	if(GitSourceControlDev.GetProvider().IsEnabled())
	{
		TArray<FString> ProjectFiles;
		const FString ProjectFile = FPaths::GetCleanFilename(FPaths::GetProjectFilePath());
		ProjectFiles.Add(ProjectFile);
		ProjectFiles.Add(TEXT("Config"));
		if(bAutoCreateGitIgnore)
		{
			// Create a standard ".gitignore" file with common patterns for a typical Blueprint & C++ project
			const FString Filename = FString::Printf(TEXT("%s.gitignore"), *PathToGameDir);
			const FString GitIgnoreContent = TEXT("Binaries\nDerivedDataCache\nIntermediate\nSaved\n*.opensdf\n*.sdf\n*.sln\n*.suo\n*.xcodeproj\n*.xcworkspace");
			if(FFileHelper::SaveStringToFile(GitIgnoreContent, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				ProjectFiles.Add(TEXT(".gitignore"));
			}
		}
		// Add .uproject and Config/ files (and .gitignore if any)
		GitSourceControlDevUtils::RunCommand(TEXT("add"), PathToGitBinary, PathToGameDir, TArray<FString>(), ProjectFiles, InfoMessages, ErrorMessages);
	}
	return FReply::Handled();
}

void SGitSourceControlDevSettings::OnCheckedCreateGitIgnore(ECheckBoxState NewCheckedState)
{
	bAutoCreateGitIgnore = (NewCheckedState == ECheckBoxState::Checked);
}

#undef LOCTEXT_NAMESPACE
