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
#include "PlatformFilemanager.h"

namespace MercurialSourceControl {

#define LOCTEXT_NAMESPACE "MercurialSourceControl.Client"

FClientSharedPtr FClient::Singleton;

/** 
 * Creates a temp file on disk that is bound to the lifetime of an FScopedTempFile instance.
 * When an FScopedTempFile instance is destroyed the temp file it created is deleted from disk.
 */
class FScopedTempFile
{
public:
	FScopedTempFile(const FString& InExtension)
	: Extension(InExtension)
	{
	}

	~FScopedTempFile()
	{
		if (!Filename.IsEmpty())
		{
			IFileManager::Get().Delete(*Filename);
		}
	}

	const FString& GetFilename()
	{
		if (Filename.IsEmpty())
		{
			FString OutputDir = FPaths::GameLogDir();
			FPaths::NormalizeDirectoryName(OutputDir);
			Filename = FPaths::CreateTempFilename(*OutputDir, TEXT("hg-"), *Extension);
			Filename = FPaths::ConvertRelativePathToFull(Filename);
		}
		return Filename;
	}

private:
	FString Filename;
	FString Extension;
};

bool FClient::IsValidExecutable(const FString& InFilename)
{
	if (FPaths::FileExists(InFilename))
	{
		int32 ReturnCode = 0;
		FString Output;
		FString Error;

		FPlatformProcess::ExecProcess(*InFilename, TEXT("version"), &ReturnCode, &Output, &Error);
		return (ReturnCode == 0) && Output.Contains(TEXT("Mercurial"));
	}
	return false;
}

bool FClient::FindExecutable(FString& OutFilename)
{
	OutFilename.Empty();

#if PLATFORM_WINDOWS
	// look for the hg.exe that's shipped with TortoiseHg
	const TCHAR* SubKey = TEXT("Software\\TortoiseHg");
	const TCHAR* ValueName = TEXT("");
	FString HgPath;

	if (FPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, SubKey, ValueName, HgPath) ||
		FPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, SubKey, ValueName, HgPath))
	{
		HgPath /= TEXT("hg.exe");
		if (IsValidExecutable(HgPath))
		{
			OutFilename = HgPath;
		}
	}
#endif // PLATFORM_WINDOWS

	return !OutFilename.IsEmpty();
}

bool FClient::Create(const FString& InMercurialPath, FText& OutError)
{
	if (ensure(!Singleton.IsValid()))
	{
		FString ExePath = InMercurialPath;

		bool bExeFound = ExePath.IsEmpty() ? 
			FClient::FindExecutable(ExePath) : FClient::IsValidExecutable(ExePath);
		
		if (bExeFound)
		{
			Singleton = MakeShareable(new FClient(ExePath));
		}
		else
		{
			OutError = LOCTEXT("ExeNotFound", "Failed to locate a valid Mercurial executable.");
		}
	}
	return Singleton.IsValid();
}

const FClientSharedPtr& FClient::Get()
{
	return Singleton;
}

void FClient::Destroy()
{
	Singleton.Reset();
}

bool FClient::GetRepositoryRoot(const FString& InWorkingDirectory, FString& OutRepositoryRoot) const
{
	FString Output;
	TArray<FString> None;
	TArray<FString> Errors;
	if (RunCommand(TEXT("root"), None, InWorkingDirectory, None, false, Output, Errors))
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
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles, 
	TArray<FFileState>& OutFileStates, TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	// convert absolute paths to be relative to the working directory
	for (const auto& AbsoluteFilename : InAbsoluteFiles)
	{
		// TODO: Consider logging a warning if the relative path can't be deduced. 
		//       Unfortunately UnrealEd has a tendency to pass in paths to built-in engine content,
		//       and if the end user creates their project on a different drive to the one the 
		//       engine is installed on those paths can't be converted to be relative to the 
		//       project's repository working directory.
		FString Filename = AbsoluteFilename;
		if (FPaths::MakePathRelativeTo(Filename, *InWorkingDirectory))
		{
			RelativeFiles.Add(Filename);
		}
	}
	
	if (RelativeFiles.Num() == 0)
	{
		return true;
	}

	TArray<FString> Options;
	// show all modified, added, removed, deleted, unknown, clean, and ignored files
	Options.Add(TEXT("-marduci"));
	FString Output;
	
	if (RunCommand(TEXT("status"), Options, InWorkingDirectory, RelativeFiles, false, Output, OutErrors))
	{
		TArray<FString> Lines;
		Output.ParseIntoArray(Lines, TEXT("\n"), true);
		for (const auto& Line : Lines)
		{
			// each line consists of a one character status code followed by a filename, 
			// a single space separates the status code from the filename
			FString Filename = Line.RightChop(2);
			FPaths::NormalizeFilename(Filename);
			FFileState FileState(InWorkingDirectory / Filename);
			FileState.SetFileStatus(StatusCodeToFileStatus(Line[0]));
			OutFileStates.Add(FileState);
		}
		return true;
	}
	return false;
}

bool FClient::GetFileHistory(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
	TMap<FString, TArray<FFileRevisionRef> >& OutFileRevisionsMap, TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	if (!ConvertFilesToRelative(InWorkingDirectory, InAbsoluteFiles, RelativeFiles))
	{
		// FIXME: Instead of quiting as soon as we get an invalid filename keep going!
		return false;
	}

	bool bResult = true;
	TArray<FString> Options;
	Options.Add(TEXT("--encoding utf-8"));
	Options.Add(TEXT("--style xml"));
	// verbose: all changes and full commit messages
	Options.Add(TEXT("-v"));
	FXmlFile XmlFile;

	for (const auto& RelativeFile : RelativeFiles)
	{
		FString Output;
		if (RunCommand(TEXT("log"), Options, InWorkingDirectory, RelativeFile, Output, OutErrors))
		{
			if (XmlFile.LoadFile(Output, EConstructMethod::ConstructFromBuffer))
			{
				TArray<FFileRevisionRef> FileRevisions;
				GetFileRevisionsFromXml(RelativeFile, XmlFile, FileRevisions);
				if (FileRevisions.Num() > 0)
				{
					FString AbsoluteFile = InWorkingDirectory / RelativeFile;
					for (const auto& Revision : FileRevisions)
					{
						Revision->SetFilename(AbsoluteFile);
					}
					OutFileRevisionsMap.Add(AbsoluteFile, FileRevisions);
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
	const FString& InDestinationFile, TArray<FString>& OutErrors
) const
{
	FString Filename = InFileToExtract;
	if (!FPaths::MakePathRelativeTo(Filename, *InWorkingDirectory))
	{
		return false;
	}

	TArray<FString> Options;
	Options.Add(FString::Printf(TEXT("--rev %d"), RevisionNumber));
	Options.Add(FString(TEXT("--output ")) + QuoteFilename(InDestinationFile));
	FString Output;

	return RunCommand(
		TEXT("cat"), Options, InWorkingDirectory, Filename, Output, OutErrors
	);
}

bool FClient::AddFiles(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles, bool bInAddAsLarge,
	TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	if (!ConvertFilesToRelative(InWorkingDirectory, InAbsoluteFiles, RelativeFiles))
	{
		return false;
	}

	TArray<FString> Options;
	if (bInAddAsLarge)
	{
		Options.Add("--large");
	}
	FString Output;

	return RunCommand(TEXT("add"), Options, InWorkingDirectory, RelativeFiles, false, Output, OutErrors);
}

bool FClient::RevertFiles(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
	TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	if (!ConvertFilesToRelative(InWorkingDirectory, InAbsoluteFiles, RelativeFiles))
	{
		return false;
	}

	TArray<FString> Options;
	// TODO: It would be a good idea to allow users to toggle this option via the source control
	//       provider settings panel/dialog.
	Options.Add(TEXT("--no-backup"));
	FString Output;

	return RunCommand(
		TEXT("revert"), Options, InWorkingDirectory, RelativeFiles, false, Output, OutErrors
	);
}

bool FClient::RemoveFiles(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
	TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	if (!ConvertFilesToRelative(InWorkingDirectory, InAbsoluteFiles, RelativeFiles))
	{
		return false;
	}

	TArray<FString> Options;
	FString Output;

	return RunCommand(
		TEXT("remove"), Options, InWorkingDirectory, RelativeFiles, false, Output, OutErrors
	);
}

bool FClient::RemoveAllFiles(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
	TArray<FString>& OutErrors
) const
{
	// The idea here is to emulate the functionality of "svn delete", which works slightly
	// differently to "hg remove". The difference being SVN will delete files with a status of 
	// "added" from the disk, but HG will not (it expects you to use "hg forget" first and then
	// delete the file from disk manually).

	// first we need to figure out what the status of each file we need to remove is
	TArray<FFileState> FileStates;
	if (!GetFileStates(InWorkingDirectory, InAbsoluteFiles, FileStates, OutErrors))
	{
		return false;
	}

	// now we can split out the "added" files that need special handling from the rest
	TArray<FString> AddedFiles;
	TArray<FString> RemovableFiles;
	for (const auto& FileState : FileStates)
	{
		switch (FileState.GetFileStatus())
		{
			case EFileStatus::Added:
				AddedFiles.Add(FileState.GetFilename());
				break;

			case EFileStatus::Clean:
			case EFileStatus::Missing:
				RemovableFiles.Add(FileState.GetFilename());
				break;
		}
	}

	bool bResult = true;

	// forget and delete added files
	if (AddedFiles.Num() > 0)
	{
		TArray<FString> RelativeFiles;
		if (!ConvertFilesToRelative(InWorkingDirectory, AddedFiles, RelativeFiles))
		{
			return false;
		}
			
		TArray<FString> Options;
		FString Output;

		bResult &= RunCommand(
			TEXT("forget"), Options, InWorkingDirectory, RelativeFiles, false, Output, OutErrors
		);
		
		for (const auto& Filename : AddedFiles)
		{
			bResult &= IFileManager::Get().Delete(*Filename);
		}
	}
	
	// remove any other removable files
	if (RemovableFiles.Num() > 0)
	{
		bResult &= RemoveFiles(InWorkingDirectory, RemovableFiles, OutErrors);
	}

	return bResult;
}

bool FClient::CommitFiles(
	const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
	const FString& InCommitMessage, TArray<FString>& OutErrors
) const
{
	TArray<FString> RelativeFiles;
	if (!ConvertFilesToRelative(InWorkingDirectory, InAbsoluteFiles, RelativeFiles))
	{
		return false;
	}

	auto Encoding = FCString::IsPureAnsi(*InCommitMessage) ? 
		FFileHelper::EEncodingOptions::ForceAnsi : FFileHelper::EEncodingOptions::ForceUTF8;

	// write the commit message to a temp file
	FScopedTempFile CommitMessageFile(TEXT(".txt"));
	bool bResult = FFileHelper::SaveStringToFile(
		InCommitMessage, *CommitMessageFile.GetFilename(), Encoding
	);

	if (!bResult)
	{
		OutErrors.Add(FString::Printf(
			TEXT("Failed to write to temp file: %s"), *CommitMessageFile.GetFilename()
		));
		return false;
	}

	TArray<FString> Options;
	if (Encoding == FFileHelper::EEncodingOptions::ForceUTF8)
	{
		Options.Add(FString(TEXT("--encoding utf-8")));
	}
	Options.Add(FString(TEXT("--logfile ")) + QuoteFilename(CommitMessageFile.GetFilename()));
	FString Output;

	return RunCommand(
		TEXT("commit"), Options, InWorkingDirectory, RelativeFiles, true, Output, OutErrors
	);
}

bool FClient::GetWorkingDirectoryParentRevisionID(
	const FString& InWorkingDirectory, FString& OutRevisionID, TArray<FString>& OutErrors
) const
{
	TArray<FString> Options;
	// just grab the local revision number
	Options.Add(FString(TEXT("--template \"{rev}\"")));
	FString Command(TEXT("parents"));
	AppendCommandOptions(Command, Options, InWorkingDirectory);
	return RunCommand(Command, OutRevisionID, OutErrors);
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

int32 FClient::GetFullCommandLength(const FString& InCommand, const TArray<FString>& InFiles)
{
	int32 Length = InCommand.Len();
	for (const auto& Filename : InFiles)
	{
		Length += Filename.Len();
		Length += 3; // 1 space + 2 double-quotes
	}
	return Length;
}

bool FClient::RunCommand(
	const FString& InCommand, FString& OutResults, TArray<FString>& OutErrorMessages
) const
{
	UE_LOG(LogSourceControl, Log, TEXT("Executing hg %s"), *InCommand);

	int32 ReturnCode = 0;
	FString StdError;

	FPlatformProcess::ExecProcess(
		*MercurialExecutablePath, *InCommand, &ReturnCode, &OutResults, &StdError
	);

	TArray<FString> ErrorMessages;
	if (StdError.ParseIntoArray(ErrorMessages, TEXT("\n"), true) > 0)
	{
		OutErrorMessages.Append(ErrorMessages);
	}

	return ReturnCode == 0;
}

bool FClient::RunCommand(
	const FString& InCommand, const TArray<FString>& InOptions,
	const FString& InWorkingDirectory, const TArray<FString>& InFiles, bool bForceFileList,
	FString& OutResults, TArray<FString>& OutErrorMessages
) const
{
	FString Command(InCommand);
	AppendCommandOptions(Command, InOptions, InWorkingDirectory);

	// on Windows 7+ this number is actually around 32,000, but we'll pick something lower in case
	// other platforms are less generous
	const int32 MaxCommandLineLength = 16000;

	if (bForceFileList
		|| ((InFiles.Num() > 0) && (GetFullCommandLength(Command, InFiles) > MaxCommandLineLength)))
	{
		// Write all the filenames to be committed to a temp file that will be passed in to hg,
		// this gets around command-line argument length limitations.
		FString FileList;
		for (const auto& RelativeFilename : InFiles)
		{
			FileList += TEXT("path:");
			FileList += RelativeFilename + TEXT("\n");
		}

		// The file list must be saved using the system's default encoding, because that's the
		// encoding hg will always use when reading in the file list. For future reference:
		// http://mercurial.selenic.com/wiki/EncodingStrategy
		// http://en.it-usenet.org/thread/16853/40385/
		FScopedTempFile ListFile(TEXT(".lst"));
		bool bResult = FFileHelper::SaveStringToFile(
			FileList, *ListFile.GetFilename(), FFileHelper::EEncodingOptions::ForceAnsi
		);

		if (!bResult)
		{
			OutErrorMessages.Add(
				FString::Printf(TEXT("Failed to write to temp file: '%s'"), *ListFile.GetFilename())
			);
			return false;
		}

		AppendCommandFile(Command, FString::Printf(TEXT("listfile:%s"), *ListFile.GetFilename()));
		// ListFile must be in-scope when this call is made
		return RunCommand(Command, OutResults, OutErrorMessages);
	}
	else
	{
		AppendCommandFiles(Command, InFiles);
		return RunCommand(Command, OutResults, OutErrorMessages);
	}
}

bool FClient::RunCommand(
	const FString& InCommand, const TArray<FString>& InOptions,
	const FString& InWorkingDirectory, const FString& InFilename,
	FString& OutResults, TArray<FString>& OutErrorMessages
) const
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
	Buffer.ParseIntoArray(Segments, Space, true);

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
	static const FString CommitIdTag(TEXT("node"));
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

		// note: we don't set the filename for the created revision, this is because the filename
		// must be absolute and we only have the relative filename at this point
		FFileRevisionRef FileRevision = MakeShareable(new FFileRevision());
		FileRevision->SetRevisionNumber(FCString::Atoi(*LogEntryNode->GetAttribute(RevisionTag)));
		FileRevision->SetCommitId(*LogEntryNode->GetAttribute(CommitIdTag));

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
			FileRevision->SetDescription(UnescapeXMLEntities(MsgNode->GetContent()));
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
					if (PathNode->GetContent() == InFilename)
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

FString FClient::UnescapeXMLEntities(const FString& InEscapedText)
{
	FString Text(InEscapedText);
	
	Text.ReplaceInline(TEXT("&lt;"), TEXT("<"));
	Text.ReplaceInline(TEXT("&#60;"), TEXT("<"));

	Text.ReplaceInline(TEXT("&gt;"), TEXT(">"));
	Text.ReplaceInline(TEXT("&#62;"), TEXT(">"));

	Text.ReplaceInline(TEXT("&quot;"), TEXT("\""));
	Text.ReplaceInline(TEXT("&#34;"), TEXT("\""));

	Text.ReplaceInline(TEXT("&apos;"), TEXT("'"));
	Text.ReplaceInline(TEXT("&#39;"), TEXT("'"));

	Text.ReplaceInline(TEXT("&amp;"), TEXT("&"));
	Text.ReplaceInline(TEXT("&#38;"), TEXT("&"));

	return Text;
}

bool FClient::ConvertFilesToRelative(
	const FString& InRelativeTo, const TArray<FString>& InFiles, TArray<FString>& OutFiles
)
{
	for (const auto& AbsoluteFilename : InFiles)
	{
		FString Filename = AbsoluteFilename;
		if (FPaths::MakePathRelativeTo(Filename, *InRelativeTo))
		{
			OutFiles.Add(Filename);
		}
		else
		{
			return false;
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
