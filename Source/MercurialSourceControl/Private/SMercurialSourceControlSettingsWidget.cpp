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
#include "SMercurialSourceControlSettingsWidget.h"
#include "DesktopPlatformModule.h"
#include "MercurialSourceControlModule.h"
#include "MercurialSourceControlClient.h"

namespace MercurialSourceControl {

#define LOCTEXT_NAMESPACE "MercurialSourceControl.SSettingsWidget"

void SProviderSettingsWidget::Construct(const FArguments& InArgs)
{
	 FString MercurialPath = FModule::GetProvider().GetSettings().GetMercurialPath();
	if (MercurialPath.IsEmpty())
	{
		FClient::FindExecutable(MercurialPath);
	}
	MercurialPathText = FText::FromString(MercurialPath);

	FSlateFontInfo TextFont = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MercurialExecutableLabel", "Mercurial Executable").ToString())
				.ToolTipText(
					LOCTEXT(
						"MercurialExecutableLabel_ToolTip", "Path to Mercurial executable (hg.exe)"
					).ToString()
				)
				.Font(TextFont)
			]
		]
		+SHorizontalBox::Slot()
		.FillWidth(2.0f)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(3.0f)
				[
					SNew(SEditableTextBox)
					.Text(this, &SProviderSettingsWidget::GetMercurialPathText)
					.OnTextCommitted(this, &SProviderSettingsWidget::OnMercurialPathTextCommitted)
					.OnTextChanged(
						this, &SProviderSettingsWidget::OnMercurialPathTextCommitted, 
						ETextCommit::Default
					)
					.Font(TextFont)
				]
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(5.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Text(LOCTEXT("MercurialExecutableBrowseButtonLabel", "Browse"))
					.OnClicked(this, &SProviderSettingsWidget::OnMercurialPathBrowseButtonClicked)
				]
			]
		]
	];
}

FText SProviderSettingsWidget::GetMercurialPathText() const
{
	return MercurialPathText;
}

void SProviderSettingsWidget::OnMercurialPathTextCommitted(
	const FText& InText, ETextCommit::Type InCommitType
)
{
	FProviderSettings& Settings = FModule::GetProvider().GetSettings();
	Settings.SetMercurialPath(InText.ToString());
	Settings.Save();
	// update the backing field for the SEditableTextBox
	MercurialPathText = InText;
}

FReply SProviderSettingsWidget::OnMercurialPathBrowseButtonClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		FString MercurialPath = MercurialPathText.ToString();

#ifdef PLATFORM_WINDOWS
		const FString Filter(TEXT("Executable files (*.exe;*.bat;*.cmd)|*.exe;*.bat;*.cmd"));
#else
		const FString Filter(TEXT("All files (*.*)|*.*"));
#endif
		
		TArray<FString> SelectedFiles;

		bool bFileSelected = DesktopPlatform->OpenFileDialog(
			nullptr, 
			LOCTEXT("ChooseExecutableDialogTitle", "Choose a Mercurial executable").ToString(),
			*FPaths::GetPath(MercurialPath), TEXT(""), Filter, EFileDialogFlags::None, SelectedFiles
		);
		
		if (bFileSelected)
		{
			check(SelectedFiles.Num() == 1);
			
			MercurialPath = FPaths::ConvertRelativePathToFull(SelectedFiles[0]);
			if (FClient::IsValidExecutable(MercurialPath))
			{
				FProviderSettings& Settings = FModule::GetProvider().GetSettings();
				Settings.SetMercurialPath(MercurialPath);
				Settings.Save();
				// update the backing field for the SEditableTextBox
				MercurialPathText = FText::FromString(MercurialPath);
			}
			else
			{
				FMessageDialog::Open(
					EAppMsgType::Ok, 
					LOCTEXT(
						"WrongExecutablePath", 
						"The file you selected is not a Mercurial executable."
					)
				);
			}
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
