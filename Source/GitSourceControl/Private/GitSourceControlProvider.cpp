// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#include "GitSourceControlPrivatePCH.h"
#include "GitSourceControlProvider.h"
//#include "GitSourceControlCommand.h"
#include "ISourceControlModule.h"
#include "GitSourceControlModule.h"
#include "GitSourceControlSettings.h"
//#include "GitSourceControlOperations.h"
#include "SGitSourceControlSettings.h"
#include "MessageLog.h"
#include "ScopedSourceControlProgress.h"

#define LOCTEXT_NAMESPACE "GitSourceControl"

static FName ProviderName("Git");

void FGitSourceControlProvider::Init(bool bForceConnection)
{
    PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
    PathToContentDir = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());

    //TODO Chech Git Binary & User "login" (config user.name & user.email)
}

void FGitSourceControlProvider::Close()
{
}

FText FGitSourceControlProvider::GetStatusText() const
{
	FFormatNamedArguments Args;
	Args.Add( TEXT("IsEnabled"), IsEnabled() ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No") );
	Args.Add( TEXT("Repository"), FText::FromString( PathToGameDir ) );

	return FText::Format( NSLOCTEXT("Status", "Provider: Git\nEnabledLabel", "Enabled: {IsEnabled}\nRepository: {RepositoryName}"), Args );
}

bool FGitSourceControlProvider::IsEnabled() const
{
	return true;
}

bool FGitSourceControlProvider::IsAvailable() const
{
	//TODO Return result from check of Git Binary & User "login" (config user.name & user.email)
	return true;
}

const FName& FGitSourceControlProvider::GetName(void) const
{
	return ProviderName;
}

ECommandResult::Type FGitSourceControlProvider::GetState(const TArray<FString>& InFiles, TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, EStateCacheUsage::Type InStateCacheUsage)
{
	return ECommandResult::Failed;
}

void FGitSourceControlProvider::RegisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	OnSourceControlStateChanged.Add( SourceControlStateChanged );
}

void FGitSourceControlProvider::UnregisterSourceControlStateChanged( const FSourceControlStateChanged::FDelegate& SourceControlStateChanged )
{
	OnSourceControlStateChanged.Remove( SourceControlStateChanged );
}

ECommandResult::Type FGitSourceControlProvider::Execute( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, const TArray<FString>& InFiles, EConcurrency::Type InConcurrency, const FSourceControlOperationComplete& InOperationCompleteDelegate )
{
	return ECommandResult::Failed;
}

bool FGitSourceControlProvider::CanCancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation ) const
{
	return false;
}

void FGitSourceControlProvider::CancelOperation( const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation )
{
}

bool FGitSourceControlProvider::UsesLocalReadOnlyState() const
{
	return false;
}

void FGitSourceControlProvider::RegisterWorker( const FName& InName, const FGetGitSourceControlWorker& InDelegate )
{
	WorkersMap.Add( InName, InDelegate );
}

void FGitSourceControlProvider::Tick()
{	
}

TArray< TSharedRef<ISourceControlLabel> > FGitSourceControlProvider::GetLabels( const FString& InMatchingSpec ) const
{
	TArray< TSharedRef<ISourceControlLabel> > Tags;

	return Tags;
}

TSharedRef<class SWidget> FGitSourceControlProvider::MakeSettingsWidget() const
{
	return SNew(SGitSourceControlSettings);
}



#undef LOCTEXT_NAMESPACE