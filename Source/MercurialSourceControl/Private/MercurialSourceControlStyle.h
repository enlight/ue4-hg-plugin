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

class FSlateStyleSet;

/**
 * Extends EditorStyle with Mercurial source control icon overlays.
 */
namespace MercurialSourceControl {

class FMercurialStyle : public FEditorStyle
{
public:
	/** Replace the current EditorStyle with this one. */
	static void Initialize();
	/** Restore the previous EditorStyle. */
	static void Shutdown();

public:
	static const FName CleanStatusIcon32;
	static const FName CleanStatusIcon16;
	static const FName AddedStatusIcon32;
	static const FName AddedStatusIcon16;
	static const FName ModifiedStatusIcon32;
	static const FName ModifiedStatusIcon16;
	static const FName RemovedStatusIcon32;
	static const FName RemovedStatusIcon16;
	static const FName NotTrackedStatusIcon32;
	static const FName NotTrackedStatusIcon16;
	static const FName MissingStatusIcon32;
	static const FName MissingStatusIcon16;

private:
	FMercurialStyle() {}

	static TSharedRef<FSlateStyleSet> Create();

private:
	static TSharedPtr<FSlateStyleSet> Instance;
};

} // namespace MercurialSourceControl
