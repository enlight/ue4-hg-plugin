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
#include "MercurialSourceControlFileRevision.h"

namespace MercurialSourceControl {

class FCommand;
class FFileState;

/** 
 * Determines the location of the Mercurial repository root.
 * If the repository root is not found the Mercurial source control provider will not be enabled.
 */
class FConnectWorker : public IWorker
{
public:
	virtual FName GetName() const override;
	virtual bool Execute(FCommand& InCommand) override;
	virtual bool UpdateStates() const override;

private:
	FString RepositoryRoot;
};

/** 
 * Updates the file status and file revision history caches in the Mercurial 
 * source control provider. 
 */
class FUpdateStatusWorker : public IWorker
{
public:
	virtual FName GetName() const override;
	virtual bool Execute(FCommand& InCommand) override;
	virtual bool UpdateStates() const override;

private:
	TArray<FFileState> FileStates;
	TMap<FString, TArray<FFileRevisionRef> > FileRevisionsMap;
};

/** Reverts files back to the most recent revision in the repository. */
class FRevertWorker : public IWorker
{
public:
	virtual FName GetName() const;
	virtual bool Execute(FCommand& InCommand);
	virtual bool UpdateStates() const;

private:
	TArray<FFileState> FileStates;
};

/** Removes files from the repository. */
class FDeleteWorker : public IWorker
{
public:
	virtual FName GetName() const;
	virtual bool Execute(FCommand& InCommand);
	virtual bool UpdateStates() const;

private:
	TArray<FFileState> FileStates;
};

/** Marks files to be added to the repository. */
class FMarkForAddWorker : public IWorker
{
public:
	virtual FName GetName() const;
	virtual bool Execute(FCommand& InCommand);
	virtual bool UpdateStates() const;

private:
	TArray<FFileState> FileStates;
};

/** Commits files to the repository. */
class FCheckInWorker : public IWorker
{
public:
	virtual FName GetName() const;
	virtual bool Execute(FCommand& InCommand);
	virtual bool UpdateStates() const;

private:
	TArray<FFileState> FileStates;
};

} // namespace MercurialSourceControl
