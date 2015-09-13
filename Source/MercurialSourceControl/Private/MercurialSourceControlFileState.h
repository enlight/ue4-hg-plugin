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
	Removed,
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
		: AbsoluteFilename(InFilename)
		, FileStatus(EFileStatus::Unknown)
		, TimeStamp(0)
	{
	}

	void SetFileStatus(EFileStatus InFileStatus)
	{
		FileStatus = InFileStatus;
	}

	EFileStatus GetFileStatus() const
	{
		return FileStatus;
	}

	void SetTimeStamp(const FDateTime& InTimeStamp)
	{
		TimeStamp = InTimeStamp;
	}

	void SetHistory(const TArray<FFileRevisionRef>& InFileRevisions)
	{
		History = InFileRevisions;
	}

	// kill ambiguous error message:
	TSharedRef< FFileState, ESPMode::ThreadSafe> AsShared()
	{
		return TSharedFromThis<FFileState, ESPMode::ThreadSafe>::AsShared();
	}

public:
	// ISourceControlState methods

	virtual int32 GetHistorySize() const;
	virtual FSourceControlRevisionPtr GetHistoryItem(int32 HistoryIndex) const override;
	virtual FSourceControlRevisionPtr FindHistoryRevision(int32 RevisionNumber) const override;
	virtual FSourceControlRevisionPtr FindHistoryRevision(const FString& InRevision) const override;
	virtual FSourceControlRevisionPtr GetBaseRevForMerge() const override;
	virtual FName GetIconName() const override;
	virtual FName GetSmallIconName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetDisplayTooltip() const override;
	virtual const FString& GetFilename() const override;
	virtual const FDateTime& GetTimeStamp() const override;
	virtual bool CanCheckIn() const override;
	virtual bool CanCheckout() const override;
	virtual bool IsCheckedOut() const override;
	virtual bool IsCheckedOutOther(FString* Who = nullptr) const override;
	virtual bool IsCurrent() const override;
	virtual bool IsSourceControlled() const override;
	virtual bool IsAdded() const override;
	virtual bool IsDeleted() const override;
	virtual bool IsIgnored() const override;
	virtual bool CanEdit() const override;
	virtual bool IsUnknown() const override;
	virtual bool IsModified() const override;
	virtual bool CanAdd() const override;
	virtual bool IsConflicted() const override;

private:
	/** All the revisions of the file */
	TArray<FFileRevisionRef> History;

	FString AbsoluteFilename;
	EFileStatus FileStatus;

	/** 
	 * Last time the state was updated.
	 * @note This is not the last modified time of the file, just the last time
	 *       the FileStatus etc. member fields were updated.
	 */
	FDateTime TimeStamp;
};

typedef TSharedRef<FFileState, ESPMode::ThreadSafe> FFileStateRef;

} // MercurialSourceControl
