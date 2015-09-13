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

#include "IMercurialSourceControlWorker.h"
#include "ISourceControlProvider.h"

namespace MercurialSourceControl {

typedef TSharedRef<class ISourceControlOperation, ESPMode::ThreadSafe> FSourceControlOperationRef;

/**
 * Executes a Mercurial command, the execution may be done on a worker thread.
 * The hard work is delegated to an IMercurialSourceControlWorker object. 
 */
class FCommand : public IQueuedWork
{
public:
	FCommand(
		const FString& InWorkingDirectory,
		const FString& InContentDirectory,
		const FSourceControlOperationRef& InOperation,
		const FWorkerRef& InWorker, 
		const FSourceControlOperationComplete& InCompleteDelegate = FSourceControlOperationComplete()
	);

	/** Execute the command. */
	bool DoWork();
	
	/** Return true iff the command has finished executing. */
	bool HasExecuted() const
	{
		return bExecuteProcessed != 0;
	}

	/** Update the state of any affected items after the command has executed. */
	bool UpdateStates()
	{
		check(bExecuteProcessed);

		return Worker->UpdateStates();
	}

	/** Get the result (succeeded/failed) of the command execution. */
	ECommandResult::Type GetResult() const
	{
		check(bExecuteProcessed);

		return bCommandSuccessful ? ECommandResult::Succeeded : ECommandResult::Failed;
	}

	/** Notify that the command has finished executing. */
	void NotifyOperationComplete()
	{
		OperationCompleteDelegate.ExecuteIfBound(Operation, GetResult());
	}

	/** Get the absolute path to the working directory of the command. */
	const FString& GetWorkingDirectory() const
	{
		return WorkingDirectory;
	}

	/** Get the absolute path to the current content directory. */
	const FString& GetContentDirectory() const
	{
		return ContentDirectory;
	}

	FSourceControlOperationRef GetOperation() const
	{
		return Operation;
	}

	void SetAbsoluteFiles(const TArray<FString>& InAbsoluteFiles)
	{
		Files = InAbsoluteFiles;
	}

	/** Get the absolute paths to the files the source control operation should be performed on. */
	const TArray<FString>& GetAbsoluteFiles() const
	{
		return Files;
	}

	void SetAbsoluteLargeFiles(const TArray<FString>& InAbsoluteLargeFiles)
	{
		LargeFiles = InAbsoluteLargeFiles;
	}

	const TArray<FString>& GetAbsoluteLargeFiles() const
	{
		return LargeFiles;
	}
	
public:
	// FQueuedWork methods
	virtual void DoThreadedWork() override;
	virtual void Abandon() override;

public:
	/** Descriptions of errors (if any) encountered while executing the command. */
	TArray<FString> ErrorMessages;

private:
	/** The source control operation to perform when the command is executed. */
	FSourceControlOperationRef Operation;

	/** The absolute paths to the files (if any) to perform the operation on. */
	TArray<FString> Files;

	/** The absolute paths to the large files (if any) to perform an 'add' operation on. */
	TArray<FString> LargeFiles;

	/** The worker that will actually perform the operation. */
	FWorkerRef Worker;

	/** Absolute path to the working directory for the command. */
	FString WorkingDirectory;

	/** Absolute path to the current content directory. */
	FString ContentDirectory;

	/** Will be set to true if the operation is performed successfully. */
	bool bCommandSuccessful;

	/** Executed after the operation completes. */
	FSourceControlOperationComplete OperationCompleteDelegate;

	/** Has the operation been completed? */
	volatile int32 bExecuteProcessed;

	/** Is this operation being performed synchronously or asynchronously? */
	EConcurrency::Type Concurrency;
};

} // namespace MercurialSourceControl
