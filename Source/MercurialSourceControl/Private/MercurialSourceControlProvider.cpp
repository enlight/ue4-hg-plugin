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
#include "SMercurialSourceControlSettingsWidget.h"
#include "MercurialSourceControlCommand.h"
#include "MercurialSourceControlFileState.h"
#include "MercurialSourceControlClient.h"
#include "MessageLog.h"
#include "ScopedSourceControlProgress.h"
#include "MercurialSourceControlOperationNames.h"
#include "ISourceControlModule.h"

namespace MercurialSourceControl {

FName FProvider::SourceControlLogName("SourceControl");

// for LOCTEXT()
#define LOCTEXT_NAMESPACE "MercurialSourceControl"

void FProvider::Init(bool bForceConnection)
{
	Settings.Load();
	AbsoluteContentDirectory = FPaths::ConvertRelativePathToFull(FPaths::GameContentDir());
}

void FProvider::Close()
{
	// clear out the file state cache
	FileStateMap.Empty();
	// destroy the FClient singleton
	FClient::Destroy();
}

const FName& FProvider::GetName() const
{
	return ProviderName;
}

FText FProvider::GetStatusText() const
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("ProviderName"), LOCTEXT("Mercurial", "Mercurial"));
	Args.Add(TEXT("YesOrNo"), IsEnabled() ? LOCTEXT("Yes", "Yes") : LOCTEXT("No", "No"));
	Args.Add(TEXT("LocalPath"), FText::FromString(GetWorkingDirectory()));
	return FText::Format(
		LOCTEXT("Status", "Provider: {ProviderName}\nEnabled: {YesOrNo}\nRepository: {LocalPath}"), 
		Args
	);
}

bool FProvider::IsEnabled() const
{
	return FClient::Get().IsValid();
}

bool FProvider::IsAvailable() const
{
	return IsEnabled();
}

ECommandResult::Type FProvider::Login(
	const FString& InPassword, EConcurrency::Type InConcurrency,
	const FSourceControlOperationComplete& InOperationCompleteDelegate
)
{
	// UnrealEd occasionally likes to "login" even though the "connection" has already been
	// established, just to be sure. There isn't much point in doing so with Mercurial so ignore 
	// any pointless login requests. 
	if (IsAvailable())
	{
		TSharedRef<FConnect, ESPMode::ThreadSafe> ConnectOperation = 
			ISourceControlOperation::Create<FConnect>();
		ConnectOperation->SetPassword(InPassword);
		InOperationCompleteDelegate.ExecuteIfBound(ConnectOperation, ECommandResult::Succeeded);
	}
	else
	{
		// call the base class method
		return ISourceControlProvider::Login(InPassword, InConcurrency, InOperationCompleteDelegate);
	}
	return ECommandResult::Succeeded;
}

ECommandResult::Type FProvider::GetState(
	const TArray<FString>& InFiles, 
	TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, 
	EStateCacheUsage::Type InStateCacheUsage
)
{
	if (!IsEnabled())
	{
		return ECommandResult::Failed;
	}

	TArray<FString> AbsoluteFiles;
	for (const auto& Filename : InFiles)
	{
		AbsoluteFiles.Add(FPaths::ConvertRelativePathToFull(Filename));
	}
		
	// update the cache if requested to do so
	if (InStateCacheUsage == EStateCacheUsage::ForceUpdate)
	{
		// TODO: Should really check the return value, but the SVN/Perforce providers don't
		// this call will block until the operation is complete
		Execute(ISourceControlOperation::Create<FUpdateStatus>(), AbsoluteFiles);
	}

	// retrieve the states for the given files from the cache
	for (const auto& Filename : AbsoluteFiles)
	{
		OutState.Add(GetFileStateFromCache(Filename));
	}

	return ECommandResult::Succeeded;
}

FDelegateHandle FProvider::RegisterSourceControlStateChanged_Handle(
	const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
)
{
	return OnSourceControlStateChanged.Add(SourceControlStateChanged);
}

void FProvider::UnregisterSourceControlStateChanged_Handle(
	FDelegateHandle Handle
)
{
	OnSourceControlStateChanged.Remove(Handle);
}

ECommandResult::Type FProvider::Execute(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation,
	const TArray<FString>& InFiles, 
	EConcurrency::Type InConcurrency,
	const FSourceControlOperationComplete& InOperationCompleteDelegate
)
{
	// the "Connect" operation is the only operation that can be performed while the
	// provider is disabled, if the operation is successful the provider will be enabled
	if (!IsEnabled() && (InOperation->GetName() != OperationNames::Connect))
	{
		return ECommandResult::Failed;
	}

	// attempt to create a worker to perform the requested operation
	FWorkerPtr WorkerPtr = CreateWorker(InOperation->GetName());
	if (!WorkerPtr.IsValid())
	{
		// apparently we don't support this particular operation
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("OperationName"), FText::FromName(InOperation->GetName()));
		Arguments.Add(TEXT("ProviderName"), FText::FromName(GetName()));
		LogError(
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
	
	auto* Command = new FCommand(
		GetWorkingDirectory(), AbsoluteContentDirectory, InOperation,
		WorkerPtr.ToSharedRef(), InOperationCompleteDelegate
	);

	TArray<FString> AbsoluteFiles;
	if (InOperation->GetName() == OperationNames::Connect)
	{
		AbsoluteFiles.Add(Settings.GetMercurialPath());
	}
	else if (InOperation->GetName() == OperationNames::MarkForAdd)
	{
		TArray<FString> AbsoluteLargeFiles;
		PrepareFilenamesForAddCommand(InFiles, AbsoluteFiles, AbsoluteLargeFiles);
		
		if (AbsoluteLargeFiles.Num() > 0)
		{
			Command->SetAbsoluteLargeFiles(AbsoluteLargeFiles);
		}
	}
	else
	{
		for (const auto& Filename : InFiles)
		{
			AbsoluteFiles.Add(FPaths::ConvertRelativePathToFull(Filename));
		}
	}

	if (AbsoluteFiles.Num() > 0)
	{
		Command->SetAbsoluteFiles(AbsoluteFiles);
	}

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

bool FProvider::CanCancelOperation(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
) const
{
	// don't support cancellation
	return false;
}

void FProvider::CancelOperation(
	const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
)
{
	// nothing to do here
}

TArray< TSharedRef<class ISourceControlLabel> > FProvider::GetLabels(
	const FString& InMatchingSpec
) const
{
	TArray< TSharedRef<class ISourceControlLabel> > Labels;
	// TODO: figure out if this needs to be implemented
	return Labels;
}

bool FProvider::UsesLocalReadOnlyState() const
{
	return false;
}

bool FProvider::UsesChangelists() const
{
	return false;
}

void FProvider::Tick()
{
	bool bNotifyStateChanged = false;

	// remove commands that have finished executing from the queue
	for (int32 i = 0; i < CommandQueue.Num(); ++i)
	{
		FCommandQueueEntry CommandQueueEntry = CommandQueue[i];
		auto* Command = CommandQueueEntry.Command;
		if (Command->HasExecuted())
		{
			CommandQueue.RemoveAt(i);
			bNotifyStateChanged = Command->UpdateStates();
			LogErrors(Command->ErrorMessages);
			Command->NotifyOperationComplete();

			if (CommandQueueEntry.bAutoDelete)
			{
				delete Command;
			}

			// NotifyOperationComplete() may indirectly alter the command queue 
			// so the loop must stop here, the queue cleanup will resume on the
			// next tick.
			break;
		}
	}

	if (bNotifyStateChanged)
	{
		OnSourceControlStateChanged.Broadcast();
	}
}

#if SOURCE_CONTROL_WITH_SLATE
TSharedRef<class SWidget> FProvider::MakeSettingsWidget() const
{
	return SNew(SProviderSettingsWidget);
}
#endif // SOURCE_CONTROL_WITH_SLATE

void FProvider::RegisterWorkerCreator(
	const FName& InOperationName, const FCreateWorkerDelegate& InDelegate
)
{
	WorkerCreatorsMap.Add(InOperationName, InDelegate);
}

bool FProvider::UpdateFileStateCache(const TArray<FFileState>& InStates)
{
	for (auto StateIt(InStates.CreateConstIterator()); StateIt; StateIt++)
	{
		FFileStateRef FileState = GetFileStateFromCache(StateIt->GetFilename());
		FileState->SetFileStatus(StateIt->GetFileStatus());
		FileState->SetTimeStamp(StateIt->GetTimeStamp());
	}
	return InStates.Num() > 0;
}

bool FProvider::UpdateFileStateCache(
	const TMap<FString, TArray<FFileRevisionRef> >& InFileRevisionsMap
)
{
	for (auto It(InFileRevisionsMap.CreateConstIterator()); It; ++It)
	{
		FFileStateRef FileState = GetFileStateFromCache(It.Key());
		FileState->SetHistory(It.Value());
		FileState->SetTimeStamp(FDateTime::Now());
	}
	return InFileRevisionsMap.Num() > 0;
}

void FProvider::LogError(const FText& InErrorMessage)
{
	FMessageLog(SourceControlLogName).Error(InErrorMessage);
}

void FProvider::LogErrors(const TArray<FString>& ErrorMessages)
{
	FMessageLog SourceControlLog(SourceControlLogName);
	for (auto It(ErrorMessages.CreateConstIterator()); It; ++It)
	{
		SourceControlLog.Error(FText::FromString(*It));
	}
}

FFileStateRef FProvider::GetFileStateFromCache(const FString& Filename)
{
	FFileStateRef* StatePtr = FileStateMap.Find(Filename);
	if (StatePtr)
	{
		return *StatePtr;
	}
	else
	{
		FFileStateRef DefaultFileState = MakeShareable(new FFileState(Filename));
		FileStateMap.Add(Filename, DefaultFileState);
		return DefaultFileState;
	}
}

ECommandResult::Type FProvider::ExecuteSynchronousCommand(
	FCommand* Command, const FText& ProgressText
)
{
	// display a progress dialog if progress text was provided
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

	return Command->GetResult();
}

ECommandResult::Type FProvider::ExecuteCommand(FCommand* Command, bool bAutoDelete)
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
		LogErrors(Command->ErrorMessages);
		Command->NotifyOperationComplete();
		
		auto Result = Command->GetResult();
		if (bAutoDelete)
		{
			delete Command;
		}
		return Result;
	}
}

FWorkerPtr FProvider::CreateWorker(const FName& InOperationName) const
{
	const auto* CreateWorkerPtr = WorkerCreatorsMap.Find(InOperationName);
	if (CreateWorkerPtr)
	{
		return CreateWorkerPtr->Execute();
	}
	return nullptr;
}

void FProvider::PrepareFilenamesForAddCommand(
	const TArray<FString>& InFiles,
	TArray<FString>& OutAbsoluteFiles, TArray<FString>& OutAbsoluteLargeFiles
)
{
	if (Settings.IsLargefilesIntegrationEnabled())
	{
		FARFilter LargeAssetFilter;
		LargeAssetFilter.bRecursiveClasses = true;

		// convert filenames to long package names that can be used in the asset filter
		for (const auto& Filename : InFiles)
		{
			// currently only .uasset files can be auto-flagged as large
			if (!Filename.EndsWith(FPackageName::GetAssetPackageExtension()))
			{
				continue;
			}

			FString PackageName;
			if (!FPackageName::TryConvertFilenameToLongPackageName(Filename, PackageName))
			{
				UE_LOG(
					LogSourceControl, Error,
					TEXT("Failed to convert filename '%s' to package name"), *Filename
				);
				continue;
			}

			LargeAssetFilter.PackageNames.Add(*PackageName);
		}

		// add the asset types that the user has designated as "large" to the asset filter
		TArray<FString> LargeAssetTypes;
		Settings.GetLargeAssetTypes(LargeAssetTypes);
		for (const auto& ClassName : LargeAssetTypes)
		{
			LargeAssetFilter.ClassNames.Add(*ClassName);
		}

		FAssetRegistryModule& AssetRegistryModule = 
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// find all the assets matching the asset filter
		TArray<FAssetData> LargeAssets;
		AssetRegistryModule.Get().GetAssets(LargeAssetFilter, LargeAssets);

		// convert the long package names of all matching assets back to filenames
		for (const auto& Asset : LargeAssets)
		{
			FString RelativePath = FPackageName::LongPackageNameToFilename(
				Asset.PackageName.ToString(), FPackageName::GetAssetPackageExtension()
			);
			OutAbsoluteLargeFiles.Add(
				FPaths::ConvertRelativePathToFull(RelativePath)
			);
		}

		// any input file that didn't match the asset filter will be added with no special flags
		for (const auto& Filename : InFiles)
		{
			FString FullPath(FPaths::ConvertRelativePathToFull(Filename));
			if (!OutAbsoluteLargeFiles.Contains(FullPath))
			{
				OutAbsoluteFiles.Add(FullPath);
			}
		}
	}
	else
	{
		for (const auto& Filename : InFiles)
		{
			OutAbsoluteFiles.Add(FPaths::ConvertRelativePathToFull(Filename));
		}
	}
}

TArray<FSourceControlStateRef> FProvider::GetCachedStateByPredicate(
	TFunctionRef<bool(const FSourceControlStateRef&)> Predicate
) const
{
	TArray<FSourceControlStateRef> MatchingFileStates;
	for (const auto& FileStateMapEntry : FileStateMap)
	{
		auto FileState = FileStateMapEntry.Value;
		if (Predicate(FileState))
		{
			MatchingFileStates.Add(FileState);
		}
	}
	return MatchingFileStates;
}

#undef LOCTEXT_NAMESPACE

} // namespace namespace MercurialSourceControl
