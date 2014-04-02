//-------------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2014 Vadim Macagon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-------------------------------------------------------------------------------
#pragma once

#include "ISourceControlState.h"
#include "MercurialSourceControlFileRevision.h"

namespace MercurialSourceControl {

enum class EFileStatus
{
	Unknown,
	Clean,
	Added,
	Deleted,
	Modified,
	NotTracked,
	Ignored,
	Missing,
};

/**
 * Provides information relating to the current status of a file in a Mercurial repository,
 * and the revision history of that file.
 */
class FFileState
	: public ISourceControlState
	, public TSharedFromThis<FFileState, ESPMode::ThreadSafe>
{
public:
	FFileState(const FString& InFilename)
		: Filename(InFilename)
		, Status(EFileStatus::Unknown)
		, TimeStamp(0)
	{
	}

public:
	// ISourceControlState methods

	virtual int32 GetHistorySize() const;
	virtual FSourceControlRevisionPtr GetHistoryItem(int32 HistoryIndex) const OVERRIDE;
	virtual FSourceControlRevisionPtr FindHistoryRevision(int32 RevisionNumber) const OVERRIDE;
	virtual FName GetIconName() const OVERRIDE;
	virtual FName GetSmallIconName() const OVERRIDE;
	virtual FText GetDisplayName() const OVERRIDE;
	virtual FText GetDisplayTooltip() const OVERRIDE;
	virtual const FString& GetFilename() const OVERRIDE;
	virtual const FDateTime& GetTimeStamp() const OVERRIDE;
	virtual bool CanCheckout() const OVERRIDE;
	virtual bool IsCheckedOut() const OVERRIDE;
	virtual bool IsCheckedOutOther(FString* Who = nullptr) const OVERRIDE;
	virtual bool IsCurrent() const OVERRIDE;
	virtual bool IsSourceControlled() const OVERRIDE;
	virtual bool IsAdded() const OVERRIDE;
	virtual bool IsDeleted() const OVERRIDE;
	virtual bool IsIgnored() const OVERRIDE;
	virtual bool CanEdit() const OVERRIDE;
	virtual bool IsUnknown() const OVERRIDE;
	virtual bool IsModified() const OVERRIDE;

private:
	/** All the revisions of the file */
	TArray<FFileRevisionRef> History;

	FString Filename;
	EFileStatus Status;

	/** Last update time */
	FDateTime TimeStamp;
};

typedef TSharedRef<FFileState, ESPMode::ThreadSafe> FFileStateRef;

} // MercurialSourceControl
