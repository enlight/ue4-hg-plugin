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
#include "MercurialSourceControlProvider.h"
#include "MercurialSourceControlSettingsWidget.h"

// for LOCTEXT()
#define LOCTEXT_NAMESPACE "MercurialSourceControl"

static FName ProviderName("Mercurial");

void FMercurialSourceControlProvider::Init(bool bForceConnection)
{
	// load setting from the command line or an INI file
	// we currently don't have any settings
}

void FMercurialSourceControlProvider::Close()
{
	// nothing to do here
}

const FName& FMercurialSourceControlProvider::GetName() const
{
	return ProviderName;
}

FString FMercurialSourceControlProvider::GetStatusText() const
{
	FString Text = LOCTEXT("ProviderName", "Provider: Mercurial").ToString();
	Text += LINE_TERMINATOR;
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("YesOrNo"), IsEnabled() ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"));
	Text += FText::Format(LOCTEXT("EnabledLabel", "Enabled: {YesOrNo}"), Arguments).ToString();
	return Text;
}

bool FMercurialSourceControlProvider::IsEnabled() const
{
	return true;
}

bool FMercurialSourceControlProvider::IsAvailable() const
{
	return true;
}

ECommandResult::Type FMercurialSourceControlProvider::GetState(
	const TArray<FString>& InFiles, 
	TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, 
	EStateCacheUsage::Type InStateCacheUsage
)
{
	// TODO: Implement this.
	return ECommandResult::Failed;
}

void FMercurialSourceControlProvider::RegisterSourceControlStateChanged(
	const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
)
{
	OnSourceControlStateChanged.Add(SourceControlStateChanged);
}

void FMercurialSourceControlProvider::UnregisterSourceControlStateChanged(
	const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
)
{
	OnSourceControlStateChanged.Remove(SourceControlStateChanged);
}

ECommandResult::Type FMercurialSourceControlProvider::Execute(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation,
	const TArray<FString>& InFiles, 
	EConcurrency::Type InConcurrency,
	const FSourceControlOperationComplete& InOperationCompleteDelegate
)
{
	if (!IsEnabled())
	{
		return ECommandResult::Failed;
	}

	// TODO: Implement this.
	return ECommandResult::Failed;
}

bool FMercurialSourceControlProvider::CanCancelOperation(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
) const
{
	// don't support cancellation
	return false;
}

void FMercurialSourceControlProvider::CancelOperation(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
)
{
	// nothing to do here
}

TArray< TSharedRef<class ISourceControlLabel> > FMercurialSourceControlProvider::GetLabels(
	const FString& InMatchingSpec
) const
{
	TArray< TSharedRef<class ISourceControlLabel> > Labels;
	// TODO: figure out if this needs to be implemented
	return Labels;
}

bool FMercurialSourceControlProvider::UsesLocalReadOnlyState() const
{
	return false;
}

void FMercurialSourceControlProvider::Tick()
{
	// TODO: do some work
}

TSharedRef<class SWidget> FMercurialSourceControlProvider::MakeSettingsWidget() const
{
	return SNew(SMercurialSourceControlSettingsWidget);
}

#undef LOCTEXT_NAMESPACE
