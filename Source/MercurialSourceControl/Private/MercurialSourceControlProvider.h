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

#include "ISourceControlProvider.h"
#include "IMercurialSourceControlWorker.h"
#include "MercurialSourceControlFileState.h"
#include "MercurialSourceControlProviderSettings.h"

namespace MercurialSourceControl {

DECLARE_DELEGATE_RetVal(FWorkerRef, FCreateWorkerDelegate)

class FCommand;

/** 
 * Provides access to the file revision history stored in a Mercurial repository.
 * 
 * This source control provider works with files that are stored in the project Content directory.
 * The project Content directory must be located within a Mercurial repository, 
 * or be the root of the repository itself.
 */
class FProvider : public ISourceControlProvider
{
public:
	// ISourceControlProvider methods

	virtual void Init(bool bForceConnection = true) override;
	virtual void Close() override;
	virtual const FName& GetName() const override;
	virtual FText GetStatusText() const override;
	virtual bool IsEnabled() const override;
	virtual bool IsAvailable() const override;

	virtual ECommandResult::Type Login(
		const FString& InPassword, EConcurrency::Type InConcurrency, 
		const FSourceControlOperationComplete& InOperationCompleteDelegate
	) override;

	virtual ECommandResult::Type GetState(
		const TArray<FString>& InFiles, 
		TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, 
		EStateCacheUsage::Type InStateCacheUsage
	) override;

	virtual TArray<FSourceControlStateRef> GetCachedStateByPredicate(
		const TFunctionRef<bool(const FSourceControlStateRef&)>& Predicate
	) const override;

	
	virtual FDelegateHandle RegisterSourceControlStateChanged_Handle(
		const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
	) override;

	virtual void UnregisterSourceControlStateChanged_Handle(
		FDelegateHandle Handle
	) override;

	virtual ECommandResult::Type Execute(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, 
		const TArray<FString>& InFiles, 
		EConcurrency::Type InConcurrency = EConcurrency::Synchronous, 
		const FSourceControlOperationComplete& InOperationCompleteDelegate = FSourceControlOperationComplete()
	) override;

	virtual bool CanCancelOperation(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
	) const override;

	virtual void CancelOperation(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
	) override;

	virtual TArray< TSharedRef<class ISourceControlLabel> > GetLabels(
		const FString& InMatchingSpec
	) const override;

	virtual bool UsesLocalReadOnlyState() const override;
	virtual bool UsesChangelists() const override;
	virtual void Tick() override;

#if SOURCE_CONTROL_WITH_SLATE
	virtual TSharedRef<class SWidget> MakeSettingsWidget() const override;
#endif // SOURCE_CONTROL_WITH_SLATE

public:
	FProvider() : ProviderName("Mercurial") {}

	/**
	 * Register a delegate that creates a worker.
	 * Each worker performs a specific source control operation.
	 * @param InOperationName The name of the operation the worker will perform.
	 * @param InDelegate The delegate that will be called to create a worker.
	 */
	void RegisterWorkerCreator(const FName& InOperationName, const FCreateWorkerDelegate& InDelegate);

	/** Update the file status cache with the content of the given file states. */
	bool UpdateFileStateCache(const TArray<FFileState>& InStates);

	/** Update the file status cache with the content of the given file revisions. */
	bool UpdateFileStateCache(const TMap<FString, TArray<FFileRevisionRef> >& InFileRevisionsMap);

	static void LogError(const FText& InErrorMessage);
	static void LogErrors(const TArray<FString>& ErrorMessages);

	/** 
	 * Set the absolute path to the repository root.
	 * @note The path must end in a '/'.
	 */
	void SetRepositoryRoot(const FString& InRepositoryRoot)
	{
		check(!FPaths::IsRelative(InRepositoryRoot));

		RepositoryRoot = InRepositoryRoot;
	}

	/** Get the working directory that will be used when hg.exe is invoked. */
	const FString& GetWorkingDirectory() const
	{
		// repository root will only be set after a successful "Connect" command
		return (RepositoryRoot.Len() > 0) ? RepositoryRoot : AbsoluteContentDirectory;
	}

	FProviderSettings& GetSettings()
	{
		return Settings;
	}
		
private:
	/** 
	 * Attempt to retrieve the state of the given file from the cache, if that fails create a 
	 * default state for the file. 
	 */
	FFileStateRef GetFileStateFromCache(const FString& Filename);

	/** 
	 * Execute a command synchronously.
	 * @param ProgressText Text to be displayed on the progress dialog while the command is 
	 *                     executing.
	 */
	ECommandResult::Type ExecuteSynchronousCommand(FCommand* Command, const FText& ProgressText);
	
	/** 
	 * Execute a command asynchronously if possible, 
	 * fall back to synchronous execution if necessary.
	 * @param bAutoDelete If true the command will be deleted after it finishes executing,
	 *                    assuming it's executed asynchronously this will happen in Tick().
	 */
	ECommandResult::Type ExecuteCommand(FCommand* Command, bool bAutoDelete);

	/** 
	 * Attempt to create a worker to perform the named operation, 
	 * if that fails return an invalid pointer.
	 */
	FWorkerPtr CreateWorker(const FName& InOperationName) const;

	/**
	 * Split out the given files into two sets, regular, and large.
	 * @param InFiles Either relative or absolute filenames that should be tracked by Mercurial.
	 * @param OutAbsoluteFiles Will be filled in with the absolute filenames of any files in 
	 *                         InFiles that are not in OutAbsoluteFiles.
	 * @param OutAbsoluteLargeFiles Will be filled in with the absolute filenames of any files in
	 *                              InFiles that match one of the asset types specified by the user.
	 */
	void PrepareFilenamesForAddCommand(
		const TArray<FString>& InFiles, 
		TArray<FString>& OutAbsoluteFiles, TArray<FString>& OutAbsoluteLargeFiles
	);

private:
	/** All the registered worker creation delegates. */
	TMap<FName, FCreateWorkerDelegate> WorkerCreatorsMap;

	struct FCommandQueueEntry
	{
		FCommand* Command;
		bool bAutoDelete;
	};

	/** Queue of commands given by the main thread. */
	TArray<FCommandQueueEntry> CommandQueue;

	/** Cache of file states. */
	TMap<FString, FFileStateRef> FileStateMap;

	/** Used to notify when the state of an item (or group of items) has changed. */
	FSourceControlStateChanged OnSourceControlStateChanged;

	/** Absolute path to the current project's content directory. */
	FString AbsoluteContentDirectory;

	/** Absolute path to the repository root directory. */
	FString RepositoryRoot;

	/** User accessible settings. */
	FProviderSettings Settings;

	FName ProviderName;

private:
	static FName SourceControlLogName;
};

} // namespace MercurialSourceControl
