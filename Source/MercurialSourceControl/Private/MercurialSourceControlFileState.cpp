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
#include "MercurialSourceControlStyle.h"

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

FSourceControlRevisionPtr FFileState::FindHistoryRevision(const FString& InRevision) const
{
	for (int32 i = 0; i < History.Num(); ++i)
	{
		if (History[i]->GetRevision() == InRevision)
		{
			return History[i];
		}
	}
	return NULL;
}

FSourceControlRevisionPtr FFileState::GetBaseRevForMerge() const
{
	// TODO: return the revision of the common ancestor when there is a conflict
	return nullptr;
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
			return FMercurialStyle::CleanStatusIcon32;
		case EFileStatus::Added:
			return FMercurialStyle::AddedStatusIcon32;
		case EFileStatus::NotTracked:
			return FMercurialStyle::NotTrackedStatusIcon32;
		case EFileStatus::Modified:
			return FMercurialStyle::ModifiedStatusIcon32;
		case EFileStatus::Removed:
			return FMercurialStyle::RemovedStatusIcon32;
		case EFileStatus::Missing:
			return FMercurialStyle::MissingStatusIcon32;
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
			return FMercurialStyle::CleanStatusIcon16;
		case EFileStatus::Added:
			return FMercurialStyle::AddedStatusIcon16;
		case EFileStatus::NotTracked:
			return FMercurialStyle::NotTrackedStatusIcon16;
		case EFileStatus::Modified:
			return FMercurialStyle::ModifiedStatusIcon16;
		case EFileStatus::Removed:
			return FMercurialStyle::RemovedStatusIcon16;
		case EFileStatus::Missing:
			return FMercurialStyle::MissingStatusIcon16;
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
	return AbsoluteFilename;
}

const FDateTime& FFileState::GetTimeStamp() const
{
	return TimeStamp;
}

bool FFileState::CanCheckIn() const
{
	return !IsConflicted()
		&& ((FileStatus == EFileStatus::Added)
		|| (FileStatus == EFileStatus::Modified)
		|| (FileStatus == EFileStatus::Removed));
}

bool FFileState::CanCheckout() const
{
	// the check-out operation is not supported by the Mercurial provider
	return false;
}

bool FFileState::IsCheckedOut() const
{
	// since Mercurial has no concept of exclusive checkouts (unlike Perforce & SVN) 
	// any file being tracked by Mercurial is always considered checked out so that the end user
	// doesn't have to perform a pointless check-out operation before they can edit a file
	return IsSourceControlled();
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
	// In case you're wondering why we check for EFileStatus::Added in here, it's because 
	// UnrealEd makes certain assumptions about source control providers, and those assumptions are
	// based on Perfoce. In this particular case we're working around the assumption that it's 
	// a good idea to revert unchanged files before a commit (see 
	// FSourceControlWindows::PromptForCheckin() for details), with that in mind...
	//
	// What is an unchanged file? Anything that IsCheckedOut() && !IsModified(). In the Mercurial
	// provider all the files are checked out all the time, so it all comes down to !IsModified().
	// Here's what would happen if we didn't account for EFileStatus::Added in IsModified():
	// 1. User creates a new file and marks it for add, its status is now EFileStatus::Added.
	// 2. User tries to commit the added file (Check In in UnrealEd).
	// 3. UnrealEd reverts the file because IsModified() == false, so its status is now 
	//    EFileStatus::NotTracked.
	// 4. UnrealEd tries to commit the file it just reverted, but that fails since Mercurial is
	//    no longer tracking it.

	return (FileStatus == EFileStatus::Modified) || (FileStatus == EFileStatus::Added);
}

bool FFileState::CanAdd() const
{
	return FileStatus == EFileStatus::NotTracked;
}

bool FFileState::IsConflicted() const
{
	// TODO: Figure out if the file is actually in conflict or not when retrieving the file status
	return false;
}

bool FFileState::CanDelete() const
{
	// TODO: Stub for 4.14 build
	return false;
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
