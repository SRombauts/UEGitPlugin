// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "SGitSourceControlSettings.h"
#include "Fonts/SlateFontInfo.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlUtils.h"
#include "SSeparator.h"
#include "SMultiLineEditableTextBox.h"

#define LOCTEXT_NAMESPACE "SGitSourceControlSettings"

void SGitSourceControlSettings::Construct(const FArguments& InArgs)
{
	FSlateFontInfo Font = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	bAutoCreateGitIgnore = true;
	bAutoInitialCommit = true;

	InitialCommitMessage = LOCTEXT("InitialCommitMessage", "Initial commit");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage( FEditorStyle::GetBrush("DetailsView.CategoryBottom"))
		.Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
		[
			SNew(SVerticalBox)
			// Path to the Git command line executable
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BinaryPathLabel", "Git Path"))
					.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Git binary"))
					.Font(Font)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SEditableTextBox)
					.Text(this, &SGitSourceControlSettings::GetBinaryPathText)
					.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Git binary"))
					.OnTextCommitted(this, &SGitSourceControlSettings::OnBinaryPathTextCommited)
					.Font(Font)
				]
			]
			// Root of the local repository
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RepositoryRootLabel", "Root of the repository"))
					.ToolTipText(LOCTEXT("RepositoryRootLabel_Tooltip", "Path to the root of the Git repository"))
					.Font(Font)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(STextBlock)
					.Text(this, &SGitSourceControlSettings::GetPathToRepositoryRoot)
					.ToolTipText(LOCTEXT("RepositoryRootLabel_Tooltip", "Path to the root of the Git repository"))
					.Font(Font)
				]
			]
			// User Name
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GitUserName", "User Name"))
					.ToolTipText(LOCTEXT("GitUserName_Tooltip", "User name configured for the Git repository"))
					.Font(Font)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(STextBlock)
					.Text(this, &SGitSourceControlSettings::GetUserName)
					.ToolTipText(LOCTEXT("GitUserName_Tooltip", "User name configured for the Git repository"))
					.Font(Font)
				]
			]
			// User e-mail
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GitUserEmail", "E-Mail"))
					.ToolTipText(LOCTEXT("GitUserEmail_Tooltip", "User e-mail configured for the Git repository"))
					.Font(Font)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(STextBlock)
					.Text(this, &SGitSourceControlSettings::GetUserEmail)
					.ToolTipText(LOCTEXT("GitUserEmail_Tooltip", "User e-mail configured for the Git repository"))
					.Font(Font)
				]
			]
			// Separator
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SSeparator)
			]
			// Explanation text
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RepositoryNotFound", "Current Project is not contained in a Git Repository. Fill the form below to initialize a new Repository."))
					.ToolTipText(LOCTEXT("RepositoryNotFound_Tooltip", "No Repository found at the level or above the current Project"))
					.Font(Font)
				]
			]
			// Option to add a proper .gitignore file (true by default)
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(0.1f)
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
					.IsChecked(ECheckBoxState::Checked)
					.OnCheckStateChanged(this, &SGitSourceControlSettings::OnCheckedCreateGitIgnore)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.9f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreateGitIgnore", "Add a .gitignore file"))
					.ToolTipText(LOCTEXT("CreateGitIgnore_Tooltip", "Create and add a standard '.gitignore' file"))
					.Font(Font)
				]
			]
			// Option to Make the initial Git commit with custom message
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(0.1f)
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("InitialGitCommit_Tooltip", "Make the initial Git commit"))
					.IsChecked(ECheckBoxState::Checked)
					.OnCheckStateChanged(this, &SGitSourceControlSettings::OnCheckedInitialCommit)
				]
				+SHorizontalBox::Slot()
				.FillWidth(0.9f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("InitialGitCommit", "Make the initial Git Commit"))
					.ToolTipText(LOCTEXT("InitialGitCommit_Tooltip", "Make the initial Git commit"))
					.Font(Font)
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				.Padding(2.0f)
				[
					SNew(SMultiLineEditableTextBox)
					.Text(this, &SGitSourceControlSettings::GetInitialCommitMessage)
					.ToolTipText(LOCTEXT("InitialCommitMessage_Tooltip", "Message of initial commit"))
					.OnTextCommitted(this, &SGitSourceControlSettings::OnInitialCommitMessageCommited)
					.Font(Font)
				]
			]
			// Button to initialize the project with Git, create the .gitignore, and make the first commit)
			+SVerticalBox::Slot()
			.FillHeight(2.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SGitSourceControlSettings::CanInitializeGitRepository)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("GitInitRepository", "Initialize project with Git"))
					.ToolTipText(LOCTEXT("GitInitRepository_Tooltip", "Initialize current project as a new Git repository"))
					.OnClicked(this, &SGitSourceControlSettings::OnClickedInitializeGitRepository)
					.HAlign(HAlign_Center)
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

FText SGitSourceControlSettings::GetPathToRepositoryRoot() const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return FText::FromString(GitSourceControl.GetProvider().GetPathToRepositoryRoot());
}

FText SGitSourceControlSettings::GetUserName() const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return FText::FromString(GitSourceControl.GetProvider().GetUserName());
}

FText SGitSourceControlSettings::GetUserEmail() const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	return FText::FromString(GitSourceControl.GetProvider().GetUserEmail());
}

EVisibility SGitSourceControlSettings::CanInitializeGitRepository() const
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	const bool bGitAvailable = GitSourceControl.GetProvider().IsGitAvailable();
	const bool bGitRepositoryFound = GitSourceControl.GetProvider().IsEnabled();
	return (bGitAvailable && !bGitRepositoryFound) ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SGitSourceControlSettings::OnClickedInitializeGitRepository()
{
	FGitSourceControlModule& GitSourceControl = FModuleManager::LoadModuleChecked<FGitSourceControlModule>("GitSourceControl");
	TArray<FString> InfoMessages;
	TArray<FString> ErrorMessages;
	const FString& PathToGitBinary = GitSourceControl.AccessSettings().GetBinaryPath();
	const FString PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	GitSourceControlUtils::RunCommand(TEXT("init"), PathToGitBinary, PathToGameDir, TArray<FString>(), TArray<FString>(), InfoMessages, ErrorMessages);
	// Check the new repository status to enable connection
	GitSourceControl.GetProvider().CheckGitAvailability();
	if(GitSourceControl.GetProvider().IsEnabled())
	{
		TArray<FString> ProjectFiles;
		ProjectFiles.Add(FPaths::GetCleanFilename(FPaths::GetProjectFilePath()));
		ProjectFiles.Add(FPaths::GetCleanFilename(FPaths::GameConfigDir()));
		ProjectFiles.Add(FPaths::GetCleanFilename(FPaths::GameContentDir()));
		if(FPaths::DirectoryExists(FPaths::GameSourceDir()))
		{
			ProjectFiles.Add(FPaths::GetCleanFilename(FPaths::GameSourceDir()));
		}
		if(bAutoCreateGitIgnore)
		{
			// Create a standard ".gitignore" file with common patterns for a typical Blueprint & C++ project
			const FString Filename = FString::Printf(TEXT("%s.gitignore"), *PathToGameDir);
			const FString GitIgnoreContent = TEXT("Binaries\nDerivedDataCache\nIntermediate\nSaved\n*.VC.db\n*.opensdf\n*.opendb\n*.sdf\n*.sln\n*.suo\n*.xcodeproj\n*.xcworkspace");
			if(FFileHelper::SaveStringToFile(GitIgnoreContent, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				ProjectFiles.Add(TEXT(".gitignore"));
			}
		}
		// Add .uproject, Config/, Content/ and Source/ files (and .gitignore if any)
		GitSourceControlUtils::RunCommand(TEXT("add"), PathToGitBinary, PathToGameDir, TArray<FString>(), ProjectFiles, InfoMessages, ErrorMessages);
		if(bAutoInitialCommit)
		{
			// optionnal initial git commit with custom message
			TArray<FString> Parameters;
			FString ParamCommitMsg = TEXT("--message=\"");
			ParamCommitMsg += InitialCommitMessage.ToString();
			ParamCommitMsg += TEXT("\"");
			Parameters.Add(ParamCommitMsg);
			GitSourceControlUtils::RunCommit(PathToGitBinary, PathToGameDir, Parameters, TArray<FString>(), InfoMessages, ErrorMessages);
		}
	}
	return FReply::Handled();
}

void SGitSourceControlSettings::OnCheckedCreateGitIgnore(ECheckBoxState NewCheckedState)
{
	bAutoCreateGitIgnore = (NewCheckedState == ECheckBoxState::Checked);
}

void SGitSourceControlSettings::OnCheckedInitialCommit(ECheckBoxState NewCheckedState)
{
	bAutoInitialCommit = (NewCheckedState == ECheckBoxState::Checked);
}

void SGitSourceControlSettings::OnInitialCommitMessageCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	InitialCommitMessage = InText;
}

FText SGitSourceControlSettings::GetInitialCommitMessage() const
{
	return InitialCommitMessage;
}

#undef LOCTEXT_NAMESPACE
