// Copyright (c) 2014-2015 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

#include "GitSourceControlModule.h"


#if SOURCE_CONTROL_WITH_SLATE

class SGitInitDialog : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SGitInitDialog) {}

	/** A reference to the parent window */
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

private:

	/** Delegate called when the user clicks the 'Accept Settings' button */
	FReply OnClickedInit();

	/** Delegate called when the user clicks the 'Disable Source Control' button */
	FReply OnClickedCancel();

	/** Called when a connection attempt fails */
	void DisplayInitError() const;

	/** Called when a connection attempt succeeds */
	void DisplayInitSuccess() const;

	/** Delegate called form the source control system when a login attempt has completed */
	void SourceControlOperationComplete(const FSourceControlOperationRef& InOperation, ECommandResult::Type InResult);

private:

	/** The parent window of this widget */
	TWeakPtr<SWindow> ParentWindowPtr;

	/** Holds the details view. */
	TSharedPtr<class IDetailsView> DetailsView;

	/** Delegate called when the window is closed */
	FSourceControlLoginClosed SourceControlLoginClosed;

	/** The currently displayed settings widget container */
	TSharedPtr<class SBorder> SettingsBorder;
};

#endif // SOURCE_CONTROL_WITH_SLATE