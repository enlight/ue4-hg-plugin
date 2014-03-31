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
#include "MercurialSourceControlCommand.h"
#include "MessageLog.h"
#include "ScopedSourceControlProgress.h"

// for LOCTEXT()
#define LOCTEXT_NAMESPACE "MercurialSourceControl"

static const char* SourceControl = "SourceControl";
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

	// check if we can create a worker to perform this operation
	const FCreateMercurialSourceControlWorker CreateWorker = *(WorkerCreatorsMap.Find(InOperation->GetName()));
	if (!CreateWorker)
	{
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("OperationName"), FText::FromName(InOperation->GetName()));
		Arguments.Add(TEXT("ProviderName"), FText::FromName(GetName()));
		FMessageLog(SourceControl).Error(
			FText::Format(
				LOCTEXT(
					"UnsupportedOperation", 
					"Operation '{OperationName}' not supported by source control provider '{ProviderName}'"
				),
				Arguments
			)
		);
		return ECommandResult::Failed;
	}
	
	auto Command = new FMercurialSourceControlCommand(InOperation, CreateWorker(), InOperationCompleteDelegate);
	if (InConcurrency == EConcurrency::Synchronous)
	{
		auto Result = ExecuteSynchronousCommand(Command, InOperation->GetInProgressString());
		delete Command;
		return Result;
	}
	else
	{
		return ExecuteCommand(Command, true);
	}
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

void FMercurialSourceControlProvider::RegisterWorkerCreator(
	const FName& InOperationName, 
	const FCreateMercurialSourceControlWorker& InDelegate
)
{
	WorkerCreatorsMap.Add(InOperationName, InDelegate);
}

ECommandResult::Type FMercurialSourceControlProvider::ExecuteSynchronousCommand(
	FMercurialSourceControlCommand* Command, const FText& ProgressText
)
{
	auto Result = ECommandResult::Failed;

	// display a progress dialog if progress text was provided
	{
		FScopedSourceControlProgress Progress(ProgressText);

		// attempt to execute the command asynchronously
		ExecuteCommand(Command, false);

		// wait for the command to finish executing
		while (!Command->HasExecuted())
		{
			Tick();
			Progress.Tick();
			FPlatformProcess::Sleep(0.01f);
		}

		// make sure the command queue is cleaned up
		Tick();

		if (Command->bCommandSuccessful)
		{
			Result = ECommandResult::Succeeded;
		}
	}

	return Result;
}

ECommandResult::Type FMercurialSourceControlProvider::ExecuteCommand(
	FMercurialSourceControlCommand* Command, bool bAutoDelete
)
{
	if (GThreadPool)
	{
		GThreadPool->AddQueuedWork(Command);
		CommandQueue.Add({Command, bAutoDelete});
		return ECommandResult::Succeeded;
	}
	else // fall back to synchronous execution
	{
		Command->DoWork();
		Command->UpdateStates();
		LogCommandMessages(*Command);
		Command->NotifyOperationComplete();
		
		auto Result = Command->GetResult();
		if (bAutoDelete)
		{
			delete Command;
		}
		return Result;
	}
}

void FMercurialSourceControlProvider::LogCommandMessages(
	const FMercurialSourceControlCommand& InCommand
)
{
	// TODO
}

#undef LOCTEXT_NAMESPACE
