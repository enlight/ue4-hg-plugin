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

#include "MercurialSourceControlPrivatePCH.h"
#include "MercurialSourceControlFileRevision.h"

namespace MercurialSourceControl {

bool FFileRevision::Get(FString& InOutFilename) const
{
	// TODO
	return false;
}

bool FFileRevision::GetAnnotated(TArray<FAnnotationLine>& OutLines) const
{
	// TODO
	return false;
}

bool FFileRevision::GetAnnotated(FString& InOutFilename) const
{
	// TODO
	return false;
}

const FString& FFileRevision::GetFilename() const
{
	return Filename;
}

int32 FFileRevision::GetRevisionNumber() const
{
	return RevisionNumber;
}

const FString& FFileRevision::GetDescription() const
{
	return Description;
}

const FString& FFileRevision::GetUserName() const
{
	return UserName;
}

const FString& FFileRevision::GetClientSpec() const
{
	static const FString EmptyString(TEXT(""));
	return EmptyString;
}

const FString& FFileRevision::GetAction() const
{
	return Action;
}

const FDateTime& FFileRevision::GetDate() const
{
	return Date;
}

int32 FFileRevision::GetCheckInIdentifier() const
{
	return RevisionNumber;
}

int32 FFileRevision::GetFileSize() const
{
	// TODO: maybe implement this later
	return 0;
}

} // namespace MercurialSourceControl
