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

namespace MercurialSourceControl {

/** Executes source control commands in a Mercurial repository by invoking hg.exe.  */
class FClient
{
public:
	/** Must be called before any of the other methods. */
	static bool Initialize();

	/** Check if the given directory is a Mercurial repository. */
	static bool IsDirectoryInRepository(const FString& InDirectory);

private:
	/**
	 * Invoke hg.exe with the given arguments and return the output.
	 * @param InCommand An hg command, e.g. add
	 * @param InOptions Zero or more options for the hg command.
	 * @param InWorkingDirectory The working directory to set for hg.exe.
	 * @param InFiles Zero or more filenames the hg command should operate on, all filenames should
	 *                be relative to InWorkingDirectory.
	 * @param OutResults Output from stdout of hg.exe.
	 * @param OutErrorMessages Output from stderr of hg.exe.
	 */
	static bool RunCommand(
		const FString& InCommand, const TArray<FString>& InOptions, 
		const FString& InWorkingDirectory, const TArray<FString>& InFiles,
		FString& OutResults, TArray<FString>& OutErrorMessages
	);

	/** Enclose the given filename in double-quotes. */
	static FString QuoteFilename(const FString& InFilename);

private:
	static FString MercurialExecutablePath;
};

} // namespace MercurialSourceControl
