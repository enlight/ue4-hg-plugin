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
#include "MercurialSourceControlClient.h"
#include "MercurialSourceControlFileState.h"
#include "ISourceControlModule.h"

namespace MercurialSourceControl {

FString FClient::MercurialExecutablePath;

bool FClient::Initialize()
{
	// TODO: Protect access to MercurialExecutablePath so we don't run into multi-threading issues.
	check(IsInGameThread());

	bool bHgFound = false;

	// TODO: let the user specify the path via settings
	
	// if the user didn't set the path look in ThirdParty directory
	MercurialExecutablePath = 
		FPaths::EngineDir() 
		/ TEXT("Binaries/ThirdParty/hg") 
		/ FPlatformProcess::GetBinariesSubdirectory() 
		/ TEXT("hg.exe");

	bHgFound = FPaths::FileExists(MercurialExecutablePath);

#ifdef PLATFORM_WINDOWS
	// look for the hg.exe that's shipped with TortoiseHg
	if (!bHgFound)
	{
		FString HgPath;
		if (FPlatformMisc::GetRegistryString(TEXT("TortoiseHg"), TEXT(""), true, HgPath) ||
			FPlatformMisc::GetRegistryString(TEXT("TortoiseHg"), TEXT(""), false, HgPath))
		{
			MercurialExecutablePath = HgPath / TEXT("hg.exe");
			bHgFound = FPaths::FileExists(MercurialExecutablePath);
		}
	}
#endif // PLATFORM_WINDOWS

	return bHgFound;
}

bool FClient::IsDirectoryInRepository(const FString& InDirectory)
{
	FString Result;
	TArray<FString> Errors;
	return RunCommand(
		TEXT("root"), TArray<FString>(), InDirectory, TArray<FString>(), Result, Errors
	);
}

bool FClient::GetFileStates(
	const FString& InWorkingDirectory, const TArray<FString>& InFiles, 
	TArray<FFileState>& OutFileStates, TArray<FString>& OutErrorMessages
)
{
	TArray<FString> Parameters;
	// show all modified, added, removed, deleted, unknown, clean, and ignored files
	Parameters.Add(TEXT("-marduci"));
	FString Output;

	if (RunCommand(TEXT("status"), Parameters, InWorkingDirectory, InFiles, Output, OutErrorMessages))
	{
		TArray<FString> Lines;
		Output.ParseIntoArray(&Lines, TEXT("\n"), true);
		for (auto It(Lines.CreateConstIterator()); It; It++)
		{
			// each line consists of a one character status code followed by a filename, 
			// a single space separates the status code from the filename
			FString Filename = (*It).RightChop(2);
			FPaths::NormalizeFilename(Filename);
			FFileState FileState(Filename);
			FileState.SetFileStatus(StatusCodeToFileStatus((*It)[0]));
			OutFileStates.Add(FileState);
		}
		return true;
	}
	return false;
}

bool FClient::RunCommand(
	const FString& InCommand, const TArray<FString>& InOptions, 
	const FString& InWorkingDirectory, const TArray<FString>& InFiles,
	FString& OutResults, TArray<FString>& OutErrorMessages
)
{
	FString Command = InCommand;
	
	for (int32 i = 0; i < InOptions.Num(); ++i)
	{
		Command += TEXT(" ");
		Command += InOptions[i];
	}

	// run in non-interactive mode 
	// (not strictly necessary as hg should detect the lack of a terminal, but just in case)
	Command += TEXT(" -y");

	// set the current working directory to the current game's content root
	Command += TEXT(" --cwd ");
	Command += QuoteFilename(InWorkingDirectory);

	for (int32 i = 0; i < InFiles.Num(); ++i)
	{
		Command += TEXT(" ");
		Command += InFiles[i];
	}

	UE_LOG(LogSourceControl, Log, TEXT("Executing hg %s"), *Command);

	int32 ReturnCode = 0;
	FString StdError;

	FPlatformProcess::ExecProcess(
		*MercurialExecutablePath, *Command, &ReturnCode, &OutResults, &StdError
	);

	TArray<FString> ErrorMessages;
	if (StdError.ParseIntoArray(&ErrorMessages, TEXT("\n"), true) > 0)
	{
		OutErrorMessages.Append(ErrorMessages);
	}

	return ReturnCode == 0;
}

FString FClient::QuoteFilename(const FString& InFilename)
{
	const FString Quote(TEXT("\""));
	return Quote + InFilename + Quote;
}

EFileStatus FClient::StatusCodeToFileStatus(TCHAR StatusCode)
{
	switch (StatusCode)
	{
		case TEXT('M'):
			return EFileStatus::Modified;

		case TEXT('A'):
			return EFileStatus::Added;

		case TEXT('R'):
			return EFileStatus::Removed;

		case TEXT('C'):
			return EFileStatus::Clean;

		case TEXT('!'):
			return EFileStatus::Missing;

		case TEXT('?'):
			return EFileStatus::NotTracked;

		case TEXT('I'):
			return EFileStatus::Ignored;

		default:
			return EFileStatus::Unknown;
	}
}

} // namespace MercurialSourceControl
