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

#include "MercurialSourceControlFileState.h"
#include "MercurialSourceControlFileRevision.h"

class FXmlFile;

namespace MercurialSourceControl {

typedef TSharedPtr<class FClient, ESPMode::ThreadSafe> FClientSharedPtr;

class FFileState;

/** Executes source control commands in a Mercurial repository by invoking hg.exe.  */
class FClient : public TSharedFromThis<FClient, ESPMode::ThreadSafe>
{
public:
	/**
	 * Check if the given filename corresponds to a valid Mercurial executable file.
	 * @note It's safe to call this method at any time, even before Initialize().
	 */
	static bool IsValidExecutable(const FString& InFilename);

	static bool FindExecutable(FString& OutFilename);

	/**
	 * Create and initialize the FClient singleton instance.
	 * @param InMercurialPath Absolute path to the Mercurial executable that should be invoked to
	 *                        manipulate a Mercurial repository.
	 * @param OutError Will contain an error message if this method returns false.
	 * @return true if the singleton instance was created and initialized successfully, 
	 *         false otherwise.
	 */
	static bool Create(const FString& InMercurialPath, FText& OutError);
	static const FClientSharedPtr& Get();
	static void Destroy();

public:
	/** Get the root directory of the repository in which the given working directory resides. */
	bool GetRepositoryRoot(const FString& InWorkingDirectory, FString& OutRepositoryRoot) const;

	bool GetFileStates(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
		TArray<FFileState>& OutFileStates, TArray<FString>& OutErrors
	) const;

	bool GetFileHistory(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
		TMap<FString, TArray<FFileRevisionRef> >& OutFileRevisionsMap, TArray<FString>& OutErrors
	) const;

	/** 
	 * Recreate a file as it was at the given revision.
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param RevisionNumber The local revision to recreate the file from.
	 * @param InFileToExtract The original absolute filename of the file to be recreated.
	 * @param InDestinationFile The absolute path at which the file should be recreated.
	 * @param OutErrors Output from stderr of hg.exe.
	 * @return true if the operation was successful, false otherwise.
	 */
	bool ExtractFileFromRevision(
		const FString& InWorkingDirectory, int32 RevisionNumber, const FString& InFileToExtract, 
		const FString& InDestinationFile, TArray<FString>& OutErrors
	) const;

	/** 
	 * Add files to the repository. 
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param InAbsoluteFiles Full filenames of files to add to the repository.
	 * @param bInAddAsLarge If true the files will be flagged as large when they're added to the 
	 *                      repository, otherwise they'll be added with no special flags.
	 * @param OutErrors Output from stderr of hg.exe.
	 * @return true if the operation was successful, false otherwise.
	 */
	bool AddFiles(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles, 
		bool bInAddAsLarge, TArray<FString>& OutErrors
	) const;

	/**
	 * Revert the given files to the contents they had in the parent of the working directory.
	 * The files will be restored to an unmodified state and any pending adds, removes, copies, 
	 * and renames will be undone. 
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param InAbsoluteFiles The absolute filenames of the files to revert.
	 * @param OutErrors Output from stderr of hg.exe.
	 * @return true if the operation was successful, false otherwise.
	 */
	bool RevertFiles(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
		TArray<FString>& OutErrors
	) const;

	/** Remove clean and missing files from the repository. */
	bool RemoveFiles(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles, 
		TArray<FString>& OutErrors
	) const;

	/** Remove added, clean, and missing files from the repository. */
	bool RemoveAllFiles(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
		TArray<FString>& OutErrors
	) const;

	bool CommitFiles(
		const FString& InWorkingDirectory, const TArray<FString>& InAbsoluteFiles,
		const FString& InCommitMessage, TArray<FString>& OutErrors
	) const;

	/** Get the local ID of the working directory's parent revision. */
	bool GetWorkingDirectoryParentRevisionID(
		const FString& InWorkingDirectory, FString& OutRevisionID, TArray<FString>& OutErrors
	) const;

private:
	static void AppendCommandOptions(
		FString& InOutCommand, const TArray<FString>& InOptions,
		const FString& InWorkingDirectory
	);
	static void AppendCommandFile(FString& InOutCommand, const FString& InFilename);
	static void AppendCommandFiles(FString& InOutCommand, const TArray<FString>& InFiles);
	static int32 GetFullCommandLength(const FString& InCommand, const TArray<FString>& InFiles);

	/** Enclose the given filename in double-quotes. */
	static FString QuoteFilename(const FString& InFilename);

	/** Convert a standard Mercurial status code character to the corresponding EFileStatus. */
	static EFileStatus StatusCodeToFileStatus(TCHAR StatusCode);

	static FString ActionCodeToString(TCHAR ActionCode);
	static FDateTime Rfc3339DateToDateTime(const FString& InDateString);

	/** 
	 * Extract file revisions from an XML log.
	 * @param InFilename The filename for which revisions should be extracted.
	 * @note The extracted revisions don't have a filename set!
	 */
	static void GetFileRevisionsFromXml(
		const FString& InFilename, const FXmlFile& InXmlFile,
		TArray<FFileRevisionRef>& OutFileRevisions
	);

	static FString UnescapeXMLEntities(const FString& InEscapedText);

	/** Convert all the given filenames to be relative to the specified path. */
	static bool ConvertFilesToRelative(
		const FString& InRelativeTo, const TArray<FString>& InFiles, TArray<FString>& OutFiles
	);

private:
	/**
	 * Constructor. 
	 * @param InMercurialPath Absolute valid path to hg.exe.
	 */
	FClient(const FString& InMercurialPath) : MercurialExecutablePath(InMercurialPath) {}

	/**
	 * Invoke hg.exe with the given command and return the output.
	 * @param InCommand A fully formed hg command, e.g. status --verbose Content/SomeFile.txt
	 * @param OutResults Output from stdout of hg.exe.
	 * @param OutErrorMessages Output from stderr of hg.exe.
	 * @return true if hg indicated the command was successful, false otherwise.
	 */
	bool RunCommand(
		const FString& InCommand, FString& OutResults, TArray<FString>& OutErrorMessages
	) const;

	/**
	 * Invoke hg.exe with the given arguments and return the output.
	 * @param InCommand An hg command, e.g. add
	 * @param InOptions Zero or more options for the hg command.
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param InFiles Zero or more filenames the hg command should operate on, all filenames should
	 *                be relative to InWorkingDirectory.
	 * @param bForceFileList If true force all filenames in InFiles to be written to a temporary
	 *                       file which is then passed in as a command argument instead of the
	 *                       individual filenames in InFiles.
	 *                       If false a temporary file will only be used when command line length 
	 *                       limits are exceeded.
	 * @param OutResults Output from stdout of hg.exe.
	 * @param OutErrorMessages Output from stderr of hg.exe.
	 * @return true if hg indicated the command was successful, false otherwise.
	 */
	bool RunCommand(
		const FString& InCommand, const TArray<FString>& InOptions, 
		const FString& InWorkingDirectory, const TArray<FString>& InFiles, bool bForceFileList,
		FString& OutResults, TArray<FString>& OutErrorMessages
	) const;

	/**
	 * Invoke hg.exe with the given arguments and return the output.
	 * @param InCommand An hg command, e.g. add
	 * @param InOptions Zero or more options for the hg command.
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param InFilename The filename the hg command should operate on, the filename should
	 *                   be relative to InWorkingDirectory.
	 * @param OutResults Output from stdout of hg.exe.
	 * @param OutErrorMessages Output from stderr of hg.exe.
	 * @return true if hg indicated the command was successful, false otherwise.
	 */
	bool RunCommand(
		const FString& InCommand, const TArray<FString>& InOptions,
		const FString& InWorkingDirectory, const FString& InFilename,
		FString& OutResults, TArray<FString>& OutErrorMessages
	) const;

private:
	FString MercurialExecutablePath;

private:
	static FClientSharedPtr Singleton;
};

} // namespace MercurialSourceControl
