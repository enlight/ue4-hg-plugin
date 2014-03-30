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

class FMercurialSourceControlProvider : public ISourceControlProvider
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

private:
	/** Used to notify when the state of an item (or group of items) has changed. */
	FSourceControlStateChanged OnSourceControlStateChanged;
};