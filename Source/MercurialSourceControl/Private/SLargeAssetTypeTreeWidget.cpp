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
#include "AssetToolsModule.h"
#include "SLargeAssetTypeTreeWidget.h"

namespace MercurialSourceControl {

typedef IAssetTypeActions IAssetType; // sorry, but the original name was horrid
typedef TWeakPtr<IAssetType> FAssetTypeWeakPtr;

/**
 * A tree item in an SLargeAssetTypeTreeWidget.
 *
 * Tree items of this type can represent either an asset category or an asset type.
 * Asset categories are the top level tree items and can contain asset type items.
 */
class FLargeAssetTypeTreeItem : public TSharedFromThis<FLargeAssetTypeTreeItem>
{
public:
	// text that will be displayed for this item in the tree view
	FText Title;
	// parents store shared pointers to children, 
	// so children must only store weak pointers to their parents to ensure proper cleanup
	FLargeAssetTypeTreeItemWeakPtr Parent;
	// only asset category items will actually have any children
	TArray<FLargeAssetTypeTreeItemPtr> Children;
	// only relevant for asset type items, the checked state of asset categories is deduced from
	// the contained asset type items
	bool bIsChecked;

public:
	FLargeAssetTypeTreeItem(const FText& InTitle)
		: Title(InTitle)
		, bIsChecked(false)
	{
	}

	FLargeAssetTypeTreeItem(
		const FAssetTypeWeakPtr& InAssetType, const TWeakPtr<FLargeAssetTypeTreeItem>& InParent
	)
		: Parent(InParent)
		, bIsChecked(false)
	{
		TSharedPtr<IAssetType> AssetType = InAssetType.Pin();
		if (AssetType.IsValid())
		{
			Title = AssetType->GetName();
		}
	}
};

#define LOCTEXT_NAMESPACE "MercurialSourceControl.SLargeAssetTypeTreeWidget"

void SLargeAssetTypeTreeWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SAssignNew(TreeView, SLargeAssetTypeTreeView)
		.SelectionMode(ESelectionMode::None)
		.TreeItemsSource(&AssetCategories)
		// get child items for any given parent item
		.OnGetChildren(this, &SLargeAssetTypeTreeWidget::TreeView_OnGetChildren)
		// generate a widget for each item
		.OnGenerateRow(this, &SLargeAssetTypeTreeWidget::TreeView_OnGenerateRow)
	];

	Populate();
}

void SLargeAssetTypeTreeWidget::Populate()
{
	AssetCategories.Empty();

// reuse localized strings from the content browser
#define NS_CONTENT_BROWSER "ContentBrowser"

	TMap<EAssetTypeCategories::Type, FLargeAssetTypeTreeItemPtr> CategoryMap;
	CategoryMap.Add(
		EAssetTypeCategories::Basic, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "BasicFilter", "Basic")
			)
		)
	);
	CategoryMap.Add(
		EAssetTypeCategories::Animation, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "AnimationFilter", "Animation")
			)
		)
	);
	CategoryMap.Add(
		EAssetTypeCategories::MaterialsAndTextures, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "MaterialFilter", "Materials & Textures")
			)
		)
	);
	CategoryMap.Add(
		EAssetTypeCategories::Sounds, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "SoundFilter", "Sounds")
			)
		)
	);
	CategoryMap.Add(
		EAssetTypeCategories::Physics, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "PhysicsFilter", "Physics")
			)
		)
	);
	CategoryMap.Add(
		EAssetTypeCategories::Misc, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "MiscFilter", "Miscellaneous")
			)
		)
	);

#undef NS_CONTENT_BROWSER 
	
	FAssetToolsModule& AssetToolsModule = 
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	TArray<FAssetTypeWeakPtr> AssetTypes;
	AssetToolsModule.Get().GetAssetTypeActionsList(AssetTypes);
		
	struct FOrderAssetTypesByNameAsc
	{
		// return true if A should appear before B, false otherwise
		bool operator()(const FAssetTypeWeakPtr& A, const FAssetTypeWeakPtr& B) const
		{
			return A.Pin()->GetName().CompareTo(B.Pin()->GetName()) < 0;
		}
	};

	AssetTypes.Sort(FOrderAssetTypesByNameAsc());

	// assign all the asset types to the corresponding category tree items
	for (const auto AssetTypeWeakPtr : AssetTypes)
	{
		TSharedPtr<IAssetType> AssetType = AssetTypeWeakPtr.Pin();
		// for consistency ignore asset types that can't be filtered by in the content browser,
		// usually this is because the asset type is not fully supported
		if (AssetType.IsValid() && AssetType->CanFilter())
		{
			for (auto CategoryMapEntry : CategoryMap)
			{
				if (AssetType->GetCategories() & CategoryMapEntry.Key)
				{
					CategoryMapEntry.Value->Children.Add(
						MakeShareable(
							new FLargeAssetTypeTreeItem(AssetTypeWeakPtr, CategoryMapEntry.Value)
						)
					);
				}
			}
		}
	}

	for (auto CategoryMapEntry : CategoryMap)
	{
		AssetCategories.Add(CategoryMapEntry.Value);
	}

	TreeView->RequestTreeRefresh();
}

TSharedRef<ITableRow> SLargeAssetTypeTreeWidget::TreeView_OnGenerateRow(
	FLargeAssetTypeTreeItemPtr Item, const TSharedRef<STableViewBase>& OwnerTable
)
{
	return 
		SNew(STableRow<FLargeAssetTypeTreeItemPtr>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SCheckBox)
				.IsChecked(
					this, &SLargeAssetTypeTreeWidget::TreeView_IsChecked,
					FLargeAssetTypeTreeItemWeakPtr(Item)
				)
				.OnCheckStateChanged(
					this, &SLargeAssetTypeTreeWidget::TreeView_OnCheckStateChanged,
					FLargeAssetTypeTreeItemWeakPtr(Item)
				)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(Item->Title)
			]
		];
}

void SLargeAssetTypeTreeWidget::TreeView_OnGetChildren(
	FLargeAssetTypeTreeItemPtr Parent, TArray<FLargeAssetTypeTreeItemPtr>& OutChildren
)
{
	if (Parent.IsValid())
	{
		OutChildren = Parent->Children;
	}
}

ESlateCheckBoxState::Type SLargeAssetTypeTreeWidget::TreeView_IsChecked(
	FLargeAssetTypeTreeItemWeakPtr ItemWeakPtr
) const
{
	FLargeAssetTypeTreeItemPtr Item = ItemWeakPtr.Pin();
	if (Item.IsValid())
	{
		bool bIsChecked = Item->bIsChecked;
		// an asset category is only checked if all the contained asset types are checked
		if (Item->Children.Num() > 0)
		{
			bIsChecked = true;
			for (const auto ChildItem : Item->Children)
			{
				bIsChecked &= ChildItem->bIsChecked;
			}
		}
		return bIsChecked ? ESlateCheckBoxState::Checked : ESlateCheckBoxState::Unchecked;
	}
	return ESlateCheckBoxState::Undetermined;
}

void SLargeAssetTypeTreeWidget::TreeView_OnCheckStateChanged(
	ESlateCheckBoxState::Type NewState, FLargeAssetTypeTreeItemWeakPtr ItemWeakPtr)
{
	FLargeAssetTypeTreeItemPtr Item = ItemWeakPtr.Pin();
	if (Item.IsValid())
	{
		bool bIsItemChecked = (NewState == ESlateCheckBoxState::Checked);
		// propagate the checked state of the asset category to the asset types within it
		for (const auto ItemChild : Item->Children)
		{
			ItemChild->bIsChecked = bIsItemChecked;
		}
		Item->bIsChecked = bIsItemChecked;
	}
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
