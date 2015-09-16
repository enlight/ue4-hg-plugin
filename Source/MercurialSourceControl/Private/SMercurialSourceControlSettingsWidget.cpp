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
#include "SLargeAssetTypeTreeWidget.h"

namespace MercurialSourceControl {

#define LOCTEXT_NAMESPACE "MercurialSourceControl.SSettingsWidget"

void SProviderSettingsWidget::Construct(const FArguments& InArgs)
{
	const FProviderSettings& ProviderSettings = FModule::GetProvider().GetSettings();
	FString MercurialPath = ProviderSettings.GetMercurialPath();
	if (MercurialPath.IsEmpty())
	{
		FClient::FindExecutable(MercurialPath);
	}
	MercurialPathText = FText::FromString(MercurialPath);
	bEnableLargefilesIntegration = ProviderSettings.IsLargefilesIntegrationEnabled();
	TArray<FString> LargeAssetTypes;
	ProviderSettings.GetLargeAssetTypes(LargeAssetTypes);
	
	FSlateFontInfo TextFont = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	ChildSlot
	[
		SNew(SVerticalBox)
		// Mercurial Executable
		+SVerticalBox::Slot()
		.AutoHeight()
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
					.Text(LOCTEXT("MercurialExecutableLabel", "Mercurial Executable"))
					.ToolTipText(
						LOCTEXT(
							"MercurialExecutableLabel_ToolTip", "Path to Mercurial executable (hg.exe)"
						)
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
						.OnTextCommitted(this, &SProviderSettingsWidget::MercurialPath_OnTextCommitted)
						.OnTextChanged(
							this, &SProviderSettingsWidget::MercurialPath_OnTextCommitted, 
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
						.OnClicked(this, &SProviderSettingsWidget::MercurialPathBrowse_OnClicked)
					]
				]
			]
		]
		// Enable Largefiles Integration Checkbox
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SCheckBox)
			.IsChecked(EnableLargefilesIntegration_IsChecked())
			.OnCheckStateChanged(this, &SProviderSettingsWidget::EnableLargefilesIntegration_OnCheckStateChanged)
			.ToolTipText(
				LOCTEXT(
					"EnableLargefiles_Tooltip",
					"When enabled the editor will always flag certain asset types as \"large\" when adding them to a repository."
				)
			)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EnableLargefilesLabel", "Enable Largefiles Integration"))
				.Font(TextFont)
			]
		]
		// Largefiles Integration Settings
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			// wrap the asset type tree in a fixed height box to prevent excessive
			// flickering when toggling the visibility of this section
			SAssignNew(LargefilesSettingsBox, SBox)
			.Padding(FMargin(0.0f, 10.0f))
			.HeightOverride(400.0f)
			.Visibility(GetLargeAssetTypeTreeVisibility())
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
					.Text(
						LOCTEXT(
							"LargeAssetTypeTreeLabel", 
							"Select the asset types the editor should always mark as \"large\" when adding them to a repository."
						)
					)
					.Font(TextFont)
					.AutoWrapText(true)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryMiddle"))
					.Padding(FMargin(5.0f))
					[
						SAssignNew(LargeAssetTypeTreeWidget, SLargeAssetTypeTreeWidget)
						.SelectedAssetTypeNames(LargeAssetTypes)
						.OnItemCheckStateChanged(
							this, 
							&SProviderSettingsWidget::LargeAssetTypeTree_OnItemCheckStateChanged
						)
					]
				]
			]
		]
	];
}

FText SProviderSettingsWidget::GetMercurialPathText() const
{
	return MercurialPathText;
}

void SProviderSettingsWidget::MercurialPath_OnTextCommitted(
	const FText& InText, ETextCommit::Type InCommitType
)
{
	FProviderSettings& Settings = FModule::GetProvider().GetSettings();
	Settings.SetMercurialPath(InText.ToString());
	Settings.Save();
	// update the backing field for the SEditableTextBox
	MercurialPathText = InText;
}

FReply SProviderSettingsWidget::MercurialPathBrowse_OnClicked()
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

ECheckBoxState SProviderSettingsWidget::EnableLargefilesIntegration_IsChecked() const
{
	return bEnableLargefilesIntegration ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

EVisibility SProviderSettingsWidget::GetLargeAssetTypeTreeVisibility() const
{
	return bEnableLargefilesIntegration ? EVisibility::Visible : EVisibility::Collapsed;
}

void SProviderSettingsWidget::EnableLargefilesIntegration_OnCheckStateChanged(
	ECheckBoxState NewState
)
{
	bEnableLargefilesIntegration = (NewState == ECheckBoxState::Checked);
	
	if (LargefilesSettingsBox.IsValid())
	{
		LargefilesSettingsBox->SetVisibility(GetLargeAssetTypeTreeVisibility());
	}
	
	FProviderSettings& Settings = FModule::GetProvider().GetSettings();
	Settings.EnableLargefilesIntegration(bEnableLargefilesIntegration);
	Settings.Save();
}

void SProviderSettingsWidget::LargeAssetTypeTree_OnItemCheckStateChanged()
{
	TArray<FString> LargeAssetTypes;
	LargeAssetTypeTreeWidget->GetSelectedAssetTypeClassNames(LargeAssetTypes);

	FProviderSettings& Settings = FModule::GetProvider().GetSettings();
	Settings.SetLargeAssetTypes(LargeAssetTypes);
	Settings.Save();
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
