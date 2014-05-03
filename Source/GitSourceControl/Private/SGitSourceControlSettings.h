// Copyright (c) 2014 Sebastien Rombauts (sebastien.rombauts@gmail.com)
//
// Distributed under the MIT License (MIT) (See accompanying file LICENSE.txt
// or copy at http://opensource.org/licenses/MIT)

#pragma once

class SGitSourceControlSettings : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SGitSourceControlSettings) {}
	
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

private:

	/** Delegate to get binary path from settings */
	FText GetBinaryPathText() const;

	/** Delegate to commit repository text to settings */
	void OnBinaryPathTextCommited(const FText& InText, ETextCommit::Type InCommitType) const;
};