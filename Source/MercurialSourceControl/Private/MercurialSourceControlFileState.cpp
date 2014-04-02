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
#include "MercurialSourceControlFileState.h"

namespace MercurialSourceControl {

#define LOCTEXT_NAMESPACE "MercurialSourceControl.State"

int32 FFileState::GetHistorySize() const
{
	return History.Num();
}

FSourceControlRevisionPtr FFileState::GetHistoryItem(int32 HistoryIndex) const
{
	check(History.IsValidIndex(HistoryIndex));
	return History[HistoryIndex];
}

FSourceControlRevisionPtr FFileState::FindHistoryRevision(int32 RevisionNumber) const
{
	for (int32 i = 0; i < History.Num(); ++i)
	{
		if (History[i]->GetRevisionNumber() == RevisionNumber)
		{
			return History[i];
		}
	}
	return NULL;
}

FName FFileState::GetIconName() const
{
	if (!IsCurrent())
	{
		return FName("Subversion.NotAtHeadRevision");
	}

	// TODO: Moar icons?!
	switch (FileStatus)
	{
		case EFileStatus::Clean:
			return FName("Subversion.CheckedOut");
		case EFileStatus::Added:
			return FName("Subversion.OpenForAdd");
		case EFileStatus::NotTracked:
			return FName("Subversion.NotInDepot");
		default:
			return NAME_None;
	}
}

FName FFileState::GetSmallIconName() const
{
	if (!IsCurrent())
	{
		return FName("Subversion.NotAtHeadRevision_Small");
	}

	// TODO: Moar icons?!
	switch (FileStatus)
	{
		case EFileStatus::Clean:
			return FName("Subversion.CheckedOut_Small");
		case EFileStatus::Added:
			return FName("Subversion.OpenForAdd_Small");
		case EFileStatus::NotTracked:
			return FName("Subversion.NotInDepot_Small");
		default:
			return NAME_None;
	}
}

FText FFileState::GetDisplayName() const
{
	switch (FileStatus)
	{
		case EFileStatus::Unknown:
			return LOCTEXT("Unknown", "Uknown");

		case EFileStatus::Clean:
			return LOCTEXT("Clean", "Clean");

		case EFileStatus::Added:
			return LOCTEXT("Added", "Added");

		case EFileStatus::Removed:
			return LOCTEXT("Removed", "Removed");

		case EFileStatus::Modified:
			return LOCTEXT("Modified", "Modified");

		case EFileStatus::NotTracked:
			return LOCTEXT("NotTracked", "Not Tracked");

		case EFileStatus::Ignored:
			return LOCTEXT("Ignored", "Ignored");

		case EFileStatus::Missing:
			return LOCTEXT("Missing", "Missing");
	}
	return FText();
}

FText FFileState::GetDisplayTooltip() const
{
	switch (FileStatus)
	{
		case EFileStatus::Unknown:
			return LOCTEXT("Unknown_Tooltip", "Item status is unknown, or maybe hell froze over.");

		case EFileStatus::Clean:
			return LOCTEXT("Clean_Tooltip", "Item hasn't been modified.");

		case EFileStatus::Added:
			return LOCTEXT("Added_Tooltip", "Item has been added.");

		case EFileStatus::Removed:
			return LOCTEXT("Removed_Tooltip", "Item has been removed.");

		case EFileStatus::Modified:
			return LOCTEXT("Modified_Tooltip", "Item has been modified.");

		case EFileStatus::NotTracked:
			return LOCTEXT("NotTracked_Tooltip", "Item is not under source control.");

		case EFileStatus::Ignored:
			return LOCTEXT("Ignored_Tooltip", "Item is being ignored.");

		case EFileStatus::Missing:
			return LOCTEXT(
				"Missing_Tooltip", 
				"Mercurial is unable to locate the item on disk, this may occur when an item is deleted or moved by a non-Mercurial command."
			);
	}
	return FText();
}

const FString& FFileState::GetFilename() const
{
	return Filename;
}

const FDateTime& FFileState::GetTimeStamp() const
{
	return TimeStamp;
}

bool FFileState::CanCheckout() const
{
	return true;
}

bool FFileState::IsCheckedOut() const
{
	// TODO: figure out how this interacts with CanCheckout()
	return false;
}

bool FFileState::IsCheckedOutOther(FString* Who) const
{
	// Mercurial doesn't keep track of who checked what out
	return false;
}

bool FFileState::IsCurrent() const
{
	// TODO
	return true;
}

bool FFileState::IsSourceControlled() const
{
	return (FileStatus != EFileStatus::NotTracked) && (FileStatus != EFileStatus::Unknown);
}

bool FFileState::IsAdded() const
{
	return FileStatus == EFileStatus::Added;
}

bool FFileState::IsDeleted() const
{
	return FileStatus == EFileStatus::Removed;
}

bool FFileState::IsIgnored() const
{
	return FileStatus == EFileStatus::Ignored;
}

bool FFileState::CanEdit() const
{
	return true;
}

bool FFileState::IsUnknown() const
{
	return FileStatus == EFileStatus::Unknown;
}

bool FFileState::IsModified() const
{
	return FileStatus == EFileStatus::Modified;
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl