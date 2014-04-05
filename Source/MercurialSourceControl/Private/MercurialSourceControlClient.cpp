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
#include "ISourceControlModule.h"
#include "XmlParser.h"

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

bool FClient::GetRepositoryRoot(const FString& InWorkingDirectory, FString& OutRepositoryRoot)
{
	FString Output;
	TArray<FString> None;
	TArray<FString> Errors;
	if (RunCommand(TEXT("root"), None, InWorkingDirectory, None, Output, Errors))
	{
		Output.RemoveFromEnd(TEXT("\n"));
		OutRepositoryRoot = Output;
		FPaths::NormalizeDirectoryName(OutRepositoryRoot);
		OutRepositoryRoot += TEXT("/");
		return true;
	}
	return false;
}

bool FClient::GetFileStates(
	const FString& InWorkingDirectory, const TArray<FString>& InFiles, 
	TArray<FFileState>& OutFileStates, TArray<FString>& OutErrorMessages
)
{
	TArray<FString> Options;
	// show all modified, added, removed, deleted, unknown, clean, and ignored files
	Options.Add(TEXT("-marduci"));
	FString Output;

	if (RunCommand(TEXT("status"), Options, InWorkingDirectory, InFiles, Output, OutErrorMessages))
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

bool FClient::GetFileHistory(
	const FString& InWorkingDirectory, const TArray<FString>& InFiles,
	TMap<FString, TArray<FFileRevisionRef> >& OutFileRevisionsMap,
	TArray<FString>& OutErrorMessages
)
{
	bool bResult = true;
	TArray<FString> Options;
	Options.Add(TEXT("--style xml"));
	// verbose: all changes and full commit messages
	Options.Add(TEXT("-v"));
	FString Output;
	FXmlFile XmlFile;
	
	for (auto It(InFiles.CreateConstIterator()); It;  It++)
	{
		if (RunCommand(TEXT("log"), Options, InWorkingDirectory, *It, Output, OutErrorMessages))
		{
			if (XmlFile.LoadFile(Output, EConstructMethod::ConstructFromBuffer))
			{
				TArray<FFileRevisionRef> FileRevisions;
				GetFileRevisionsFromXml(*It, XmlFile, FileRevisions);
				if (FileRevisions.Num() > 0)
				{
					OutFileRevisionsMap.Add(*It, FileRevisions);
				}
			}
		}
		else
		{
			bResult = false;
		}
	}
	return bResult;
}

bool FClient::ExtractFileFromRevision(
	const FString& InWorkingDirectory, int32 RevisionNumber, const FString& InFileToExtract,
	const FString& InDestinationFile, TArray<FString>& OutErrorMessages
)
{
	TArray<FString> Options;
	Options.Add(FString::Printf(TEXT("--rev %d"), RevisionNumber));
	Options.Add(FString(TEXT("--output ")) + QuoteFilename(InDestinationFile));
	FString Output;

	return RunCommand(
		TEXT("cat"), Options, InWorkingDirectory, InFileToExtract, Output, OutErrorMessages
	);
}

void FClient::AppendCommandOptions(
	FString& InOutCommand, const TArray<FString>& InOptions, const FString& InWorkingDirectory
)
{
	for (int32 i = 0; i < InOptions.Num(); ++i)
	{
		InOutCommand += TEXT(" ");
		InOutCommand += InOptions[i];
	}

	// run in non-interactive mode 
	// (not strictly necessary as hg should detect the lack of a terminal, but just in case)
	InOutCommand += TEXT(" -y");

	// set the current working directory to the current game's content root
	InOutCommand += TEXT(" --cwd ");
	InOutCommand += QuoteFilename(InWorkingDirectory);
}

void FClient::AppendCommandFile(FString& InOutCommand, const FString& InFilename)
{
	InOutCommand += TEXT(" ");
	InOutCommand += QuoteFilename(InFilename);
}

void FClient::AppendCommandFiles(FString& InOutCommand, const TArray<FString>& InFiles)
{
	for (int32 i = 0; i < InFiles.Num(); ++i)
	{
		InOutCommand += TEXT(" ");
		InOutCommand += QuoteFilename(InFiles[i]);
	}
}

bool FClient::RunCommand(
	const FString& InCommand, FString& OutResults, TArray<FString>& OutErrorMessages
)
{
	UE_LOG(LogSourceControl, Log, TEXT("Executing hg %s"), *InCommand);

	int32 ReturnCode = 0;
	FString StdError;

	FPlatformProcess::ExecProcess(
		*MercurialExecutablePath, *InCommand, &ReturnCode, &OutResults, &StdError
	);

	TArray<FString> ErrorMessages;
	if (StdError.ParseIntoArray(&ErrorMessages, TEXT("\n"), true) > 0)
	{
		OutErrorMessages.Append(ErrorMessages);
	}

	return ReturnCode == 0;
}

bool FClient::RunCommand(
	const FString& InCommand, const TArray<FString>& InOptions,
	const FString& InWorkingDirectory, const TArray<FString>& InFiles,
	FString& OutResults, TArray<FString>& OutErrorMessages
)
{
	FString Command(InCommand);
	AppendCommandOptions(Command, InOptions, InWorkingDirectory);
	AppendCommandFiles(Command, InFiles);
	return RunCommand(Command, OutResults, OutErrorMessages);
}

bool FClient::RunCommand(
	const FString& InCommand, const TArray<FString>& InOptions,
	const FString& InWorkingDirectory, const FString& InFilename,
	FString& OutResults, TArray<FString>& OutErrorMessages
)
{
	FString Command(InCommand);
	AppendCommandOptions(Command, InOptions, InWorkingDirectory);
	AppendCommandFile(Command, InFilename);
	return RunCommand(Command, OutResults, OutErrorMessages);
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

FString FClient::ActionCodeToString(TCHAR ActionCode)
{
	switch (ActionCode)
	{
		case TEXT('M'):
			return TEXT("edit");

		case TEXT('A'):
			return TEXT("add");

		case TEXT('R'):
			return TEXT("remove");

		default:
			return TEXT("unknown");
	}
}

FDateTime FClient::Rfc3339DateToDateTime(const FString& InDateString)
{
	// There are some slight variations but the variant Mercurial seems to use by default is:
	// YYYY-MM-DDTHH:MM:SS[+,-]HH:MM
	const TCHAR* Space = TEXT(" ");
	FString Buffer = InDateString.Replace(TEXT("T"), Space);
	Buffer.ReplaceInline(TEXT("Z"), Space);
	Buffer.ReplaceInline(TEXT("-"), Space);
	Buffer.ReplaceInline(TEXT(":"), Space);

	TArray<FString> Segments;
	Buffer.ParseIntoArray(&Segments, Space, true);

	int32 Year = FMath::Clamp((Segments.Num() > 0) ? FCString::Atoi(*Segments[0]) : 0, 0, 9999);
	int32 Month = FMath::Clamp((Segments.Num() > 1) ? FCString::Atoi(*Segments[1]) : 0, 1, 12);
	int32 Day = FMath::Clamp(
		(Segments.Num() > 2) ? FCString::Atoi(*Segments[2]) : 0, 
		1, FDateTime::DaysInMonth(Year, Month)
	);
	int32 Hour = FMath::Clamp((Segments.Num() > 3) ? FCString::Atoi(*Segments[3]) : 0, 0, 23);
	int32 Minute = FMath::Clamp((Segments.Num() > 4) ? FCString::Atoi(*Segments[4]) : 0, 0, 59);
	int32 Second = FMath::Clamp((Segments.Num() > 5) ? FCString::Atoi(*Segments[5]) : 0, 0, 59);

	return FDateTime(Year, Month, Day, Hour, Minute, Second);
}

void FClient::GetFileRevisionsFromXml(
	const FString& InFilename, const FXmlFile& InXmlFile, TArray<FFileRevisionRef>& OutFileRevisions
)
{
	static const FString LogTag(TEXT("log"));
	static const FString LogEntryTag(TEXT("logentry"));
	static const FString RevisionTag(TEXT("revision"));
	static const FString AuthorTag(TEXT("author"));
	static const FString DateTag(TEXT("date"));
	static const FString MsgTag(TEXT("msg"));
	static const FString PathsTag(TEXT("paths"));
	static const FString PathTag(TEXT("path"));
	static const FString ActionTag(TEXT("action"));

	const FXmlNode* LogNode = InXmlFile.GetRootNode();
	check(LogNode && (LogNode->GetTag() == LogTag));
	if (!LogNode || (LogNode->GetTag() != LogTag))
		return;

	const TArray<FXmlNode*> LogEntries = LogNode->GetChildrenNodes();
	for (auto LogEntryIt(LogEntries.CreateConstIterator()); LogEntryIt; LogEntryIt++)
	{
		const FXmlNode* LogEntryNode = *LogEntryIt;
		check(LogEntryNode && (LogEntryNode->GetTag() == LogEntryTag));
		if (!LogEntryNode || (LogEntryNode->GetTag() != LogEntryTag))
		{
			continue;
		}

		FFileRevisionRef FileRevision = MakeShareable(new FFileRevision());
		FileRevision->SetFilename(InFilename);
		FileRevision->SetRevisionNumber(FCString::Atoi(*LogEntryNode->GetAttribute(RevisionTag)));

		const FXmlNode* AuthorNode = LogEntryNode->FindChildNode(AuthorTag);
		if (AuthorNode)
		{
			FileRevision->SetUserName(AuthorNode->GetContent());
		}

		const FXmlNode* DateNode = LogEntryNode->FindChildNode(DateTag);
		if (DateNode)
		{
			FileRevision->SetDate(Rfc3339DateToDateTime(DateNode->GetContent()));
		}

		const FXmlNode* MsgNode = LogEntryNode->FindChildNode(MsgTag);
		if (MsgNode)
		{
			FileRevision->SetDescription(MsgNode->GetContent());
		}

		// the paths node contains path nodes indicating the operations that were performed, 
		// e.g.
		// <paths>
		//     <path action="A">foo/bar/Test.txt</path>
		//     <path action="R">foo/Test.txt</path>
		// </paths>
		// In the example above Test.txt was moved from directory foo to foo/bar.
		const FXmlNode* PathsNode = LogEntryNode->FindChildNode(PathsTag);
		if (PathsNode)
		{
			const TArray<FXmlNode*> Paths = PathsNode->GetChildrenNodes();
			for (auto PathIt(Paths.CreateConstIterator()); PathIt; PathIt++)
			{
				const FXmlNode* PathNode = *PathIt;
				if (PathNode && (PathNode->GetTag() == PathTag))
				{
					FString Filename(PathNode->GetContent());
					if (Filename == InFilename)
					{
						FString ActionCode = PathNode->GetAttribute(ActionTag);
						if (ActionCode.Len() > 0)
						{
							FileRevision->SetAction(ActionCodeToString(ActionCode[0]));
						}
						else
						{
							FileRevision->SetAction(TEXT("unknown"));
						}
					}
				}
			}
		}

		OutFileRevisions.Add(FileRevision);
	}
}

} // namespace MercurialSourceControl
