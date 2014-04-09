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
#include "MercurialSourceControlStyle.h"
#include "SlateStyle.h"
#include "EditorStyle.h"

namespace MercurialSourceControl {

const FName FMercurialStyle::CleanStatusIcon32("Mercurial.CleanStatusIcon");
const FName FMercurialStyle::CleanStatusIcon16("Mercurial.CleanStatusIcon.Small");
const FName FMercurialStyle::AddedStatusIcon32("Mercurial.AddedStatusIcon");
const FName FMercurialStyle::AddedStatusIcon16("Mercurial.AddedStatusIcon.Small");
const FName FMercurialStyle::ModifiedStatusIcon32("Mercurial.ModifiedStatusIcon");
const FName FMercurialStyle::ModifiedStatusIcon16("Mercurial.ModifiedStatusIcon.Small");
const FName FMercurialStyle::RemovedStatusIcon32("Mercurial.RemovedStatusIcon");
const FName FMercurialStyle::RemovedStatusIcon16("Mercurial.RemovedStatusIcon.Small");
const FName FMercurialStyle::NotTrackedStatusIcon32("Mercurial.NotTrackedStatusIcon");
const FName FMercurialStyle::NotTrackedStatusIcon16("Mercurial.NotTrackedStatusIcon.Small");
const FName FMercurialStyle::MissingStatusIcon32("Mercurial.MissingStatusIcon");
const FName FMercurialStyle::MissingStatusIcon16("Mercurial.MissingStatusIcon.Small");

TSharedPtr<FSlateStyleSet> MercurialSourceControl::FMercurialStyle::Instance = nullptr;

void FMercurialStyle::Initialize()
{
	if (!Instance.IsValid())
	{
		Instance = Create();
	}
	SetStyle(Instance.ToSharedRef());
}

void FMercurialStyle::Shutdown()
{
	ResetToDefault(); // switch back to the previous style
	ensure(Instance.IsUnique());
	Instance.Reset();
}

TSharedRef<FSlateStyleSet> FMercurialStyle::Create()
{
	auto& EditorStyleModule = FModuleManager::LoadModuleChecked<IEditorStyleModule>(TEXT("EditorStyle"));
	TSharedRef<FSlateStyleSet> EditorStyleSetRef = EditorStyleModule.CreateEditorStyleInstance();
	FSlateStyleSet& EditorStyle = EditorStyleSetRef.Get();

	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon32x32(32.0f, 32.0f);

	const FString SlateBrushesPath = 
		FSlateBrush::UTextureIdentifier()
		// content root (i.e. parent directory of this plugin's Content folder), 
		// will be resolved to the location of the MercurialSourceControl.uplugin file,
		// but this only works if "CanContainContent" is set to true in the .uplugin file
		+ "/MercurialSourceControl"
		// asset path that's relative to <Content Root>/Content
		+ "/SlateBrushes";

	EditorStyle.Set(
		CleanStatusIcon32, 
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("CleanStatusIcon.CleanStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		CleanStatusIcon16, 
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("CleanStatusIcon.CleanStatusIcon"), Icon16x16
		)
	);
	EditorStyle.Set(
		AddedStatusIcon32,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("AddedStatusIcon.AddedStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		AddedStatusIcon16,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("AddedStatusIcon.AddedStatusIcon"), Icon16x16
		)
	);
	EditorStyle.Set(
		ModifiedStatusIcon32, 
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("ModifiedStatusIcon.ModifiedStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		ModifiedStatusIcon16, 
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("ModifiedStatusIcon.ModifiedStatusIcon"), Icon16x16
		)
	);
	EditorStyle.Set(
		RemovedStatusIcon32,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("RemovedStatusIcon.RemovedStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		RemovedStatusIcon16,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("RemovedStatusIcon.RemovedStatusIcon"), Icon16x16
		)
	);
	EditorStyle.Set(
		NotTrackedStatusIcon32,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("NotTrackedStatusIcon.NotTrackedStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		NotTrackedStatusIcon16,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("NotTrackedStatusIcon.NotTrackedStatusIcon"), Icon16x16
		)
	);
	EditorStyle.Set(
		MissingStatusIcon32,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("MissingStatusIcon.MissingStatusIcon"), Icon32x32
		)
	);
	EditorStyle.Set(
		MissingStatusIcon16,
		new FSlateImageBrush(
			SlateBrushesPath / TEXT("MissingStatusIcon.MissingStatusIcon"), Icon16x16
		)
	);

	return EditorStyleSetRef;
}

} // namespace MercurialSourceControl
