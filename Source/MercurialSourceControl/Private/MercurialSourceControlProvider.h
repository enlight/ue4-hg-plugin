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
#include <functional>

namespace MercurialSourceControl {

typedef std::function<FWorkerRef()> FCreateWorker;

class FProvider : public ISourceControlProvider
{
public:
	// ISourceControlProvider methods

	virtual void Init(bool bForceConnection = true) OVERRIDE;
	virtual void Close() OVERRIDE;
	virtual const FName& GetName() const OVERRIDE;
	virtual FString GetStatusText() const OVERRIDE;

	virtual bool IsEnabled() const OVERRIDE;

	virtual bool IsAvailable() const OVERRIDE;

	virtual ECommandResult::Type GetState(
		const TArray<FString>& InFiles, 
		TArray< TSharedRef<ISourceControlState, ESPMode::ThreadSafe> >& OutState, 
		EStateCacheUsage::Type InStateCacheUsage
	) OVERRIDE;

	virtual void RegisterSourceControlStateChanged(
		const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
	) OVERRIDE;

	virtual void UnregisterSourceControlStateChanged(
		const FSourceControlStateChanged::FDelegate& SourceControlStateChanged
	) OVERRIDE;

	virtual ECommandResult::Type Execute(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation, 
		const TArray<FString>& InFiles, 
		EConcurrency::Type InConcurrency = EConcurrency::Synchronous, 
		const FSourceControlOperationComplete& InOperationCompleteDelegate = FSourceControlOperationComplete()
	) OVERRIDE;

	virtual bool CanCancelOperation(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
	) const OVERRIDE;

	virtual void CancelOperation(
		const TSharedRef<ISourceControlOperation, ESPMode::ThreadSafe>& InOperation
	) OVERRIDE;

	virtual TArray< TSharedRef<class ISourceControlLabel> > GetLabels(
		const FString& InMatchingSpec
	) const OVERRIDE;

	virtual bool UsesLocalReadOnlyState() const OVERRIDE;
	virtual void Tick() OVERRIDE;
	virtual TSharedRef<class SWidget> MakeSettingsWidget() const OVERRIDE;

public:
	// FMercurialSourceControlProvider methods

	/**
	 * Register a delegate that creates a worker.
	 * Each worker performs a specific source control operation.
	 * @param InOperationName The name of the operation the worker will perform.
	 * @param InDelegate The delegate that will be called to create a worker.
	 */
	void RegisterWorkerCreator(const FName& InOperationName, const FCreateWorker& InDelegate);

private:
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

	/** Log all the info and error messages from the given command. */
	static void LogCommandMessages(const FCommand& InCommand);

	/** 
	 * Attempt to create a worker to perform the named operation, 
	 * if that fails return an invalid pointer.
	 */
	FWorkerPtr CreateWorker(const FName& InOperationName) const;

private:
	/** All the registered worker creation delegates. */
	TMap<FName, FCreateWorker> WorkerCreatorsMap;

	struct FCommandQueueEntry
	{
		FCommand* Command;
		bool bAutoDelete;
	};

	/** Queue of commands given by the main thread. */
	TArray<FCommandQueueEntry> CommandQueue;

	/** Used to notify when the state of an item (or group of items) has changed. */
	FSourceControlStateChanged OnSourceControlStateChanged;
};

} // namespace MercurialSourceControl
