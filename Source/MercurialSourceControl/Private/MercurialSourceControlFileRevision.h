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

#include "ISourceControlRevision.h"

namespace MercurialSourceControl {

typedef TSharedPtr<ISourceControlRevision, ESPMode::ThreadSafe> FSourceControlRevisionPtr;

/**
 * Provides information relating to a revision of a file in a Mercurial repository.
 */
class FFileRevision 
	: public ISourceControlRevision
	, public TSharedFromThis<FFileRevision, ESPMode::ThreadSafe>
{
public:
	FFileRevision() : RevisionNumber(0) {}

	void SetFilename(const FString& InFilename)
	{
		AbsoluteFilename = InFilename;
	}

	void SetRevisionNumber(int32 InRevisionNumber)
	{
		RevisionNumber = InRevisionNumber;
	}

	void SetCommitId(const FString &commitId)
	{
		CommitId = commitId;
	}

	void SetUserName(const FString& InUserName)
	{
		UserName = InUserName;
	}

	void SetDate(const FDateTime& InDate)
	{
		Date = InDate;
	}

	void SetDescription(const FString& InDescription)
	{
		Description = InDescription;
	}

	void SetAction(const FString& InAction)
	{
		Action = InAction;
	}

public:
	// ISourceControlRevision methods

	/** 
	 * Copy this file revision into a temporary file.
	 * @param InOutFilename The filename that this revision should be written to. If this is empty
	 *        a temporary filename will be generated and returned in this string.
	 * @return true on success, false otherwise.
	 */
	virtual bool Get(FString& InOutFilename) const override;
	virtual bool GetAnnotated(TArray<FAnnotationLine>& OutLines) const override;
	virtual bool GetAnnotated(FString& InOutFilename) const override;
	virtual const FString& GetFilename() const override;
	virtual int32 GetRevisionNumber() const override;
	virtual const FString& GetRevision() const override;
	virtual const FString& GetDescription() const override;
	virtual const FString& GetUserName() const override;
	virtual const FString& GetClientSpec() const override;
	virtual const FString& GetAction() const override;
	virtual FSourceControlRevisionPtr GetBranchSource() const override;
	virtual const FDateTime& GetDate() const override;
	virtual int32 GetCheckInIdentifier() const override;
	virtual int32 GetFileSize() const override;

private:
	FString AbsoluteFilename;
	int32 RevisionNumber;
	FString CommitId;
	FString Description;
	FString UserName;
	FString Action;
	FDateTime Date;
};

typedef TSharedRef<FFileRevision, ESPMode::ThreadSafe> FFileRevisionRef;

} // namespace MercurialSourceControl
