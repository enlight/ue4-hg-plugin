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
	// the class name that corresponds to this asset type
	FString AssetTypeClassName;

public:
	FLargeAssetTypeTreeItem(const FText& InTitle)
		: Title(InTitle)
		, bIsSelected(false)
	{
	}

	FLargeAssetTypeTreeItem(
		const IAssetType& InAssetType, const TWeakPtr<FLargeAssetTypeTreeItem>& InParent
	)
		: Parent(InParent)
		, bIsSelected(false)
	{
		Title = InAssetType.GetName();
		AssetTypeClassName = InAssetType.GetSupportedClass()->GetName();
	}

	bool IsSelected() const
	{
		if (SharedState.IsValid())
		{
			return SharedState->bIsSelected;
		}
		return bIsSelected;
	}

	void SetIsSelected(bool bInIsSelected)
	{
		if (SharedState.IsValid())
		{
			SharedState->bIsSelected = bInIsSelected;
		}
		else
		{
			bIsSelected = bInIsSelected;
		}
	}

	static void CreateSharedState(
		const TArray<FLargeAssetTypeTreeItemWeakPtr>& InAssetTypeItems, bool bIsSelected
	)
	{
		// An asset type may belong to multiple asset categories, however, 
		// all tree items are distinct, so multiple tree items may correspond to the same 
		// asset type. The checked state is shared between such duplicate items,
		// so that checking/unchecking an asset type in one category results in that
		// asset type being checked/unchecked in any other categories it may appear in.
		TSharedPtr<FSharedState> SharedState = MakeShareable(new FSharedState(bIsSelected));

		for (auto AssetTypeItem : InAssetTypeItems)
		{
			AssetTypeItem.Pin()->SharedState = SharedState;
		}
	}

private:
	// state shared by items that represent asset types belonging to multiple categories
	struct FSharedState : public TSharedFromThis<FSharedState>
	{
		bool bIsSelected;

		FSharedState(bool bInIsSelected) : bIsSelected(bInIsSelected) {}
	};

private:
	// only relevant for asset type items, and furthermore only when SharedState.IsValid() == false
	bool bIsSelected;
	// will only be valid for asset type items that correspond to asset types that belong to 
	// multiple categories
	TSharedPtr<FSharedState> SharedState;
};

#define LOCTEXT_NAMESPACE "MercurialSourceControl.SLargeAssetTypeTreeWidget"

void SLargeAssetTypeTreeWidget::Construct(const FArguments& InArgs)
{
	OnItemCheckStateChanged = InArgs._OnItemCheckStateChanged;

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

	Populate(InArgs._SelectedAssetTypeNames);
}

void SLargeAssetTypeTreeWidget::AddCategoriesToCategoryMap(
	TMap<EAssetTypeCategories::Type, FLargeAssetTypeTreeItemPtr>& OutCategoryMap
)
{
// reuse localized strings from the content browser
#define NS_CONTENT_BROWSER "ContentBrowser"

	OutCategoryMap.Add(
		EAssetTypeCategories::Basic, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "BasicFilter", "Basic")
			)
		)
	);
	OutCategoryMap.Add(
		EAssetTypeCategories::Animation, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "AnimationFilter", "Animation")
			)
		)
	);
	OutCategoryMap.Add(
		EAssetTypeCategories::MaterialsAndTextures, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "MaterialFilter", "Materials & Textures")
			)
		)
	);
	OutCategoryMap.Add(
		EAssetTypeCategories::Sounds, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "SoundFilter", "Sounds")
			)
		)
	);
	OutCategoryMap.Add(
		EAssetTypeCategories::Physics, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "PhysicsFilter", "Physics")
			)
		)
	);
	OutCategoryMap.Add(
		EAssetTypeCategories::Misc, 
		MakeShareable(
			new FLargeAssetTypeTreeItem(
				NSLOCTEXT(NS_CONTENT_BROWSER, "MiscFilter", "Miscellaneous")
			)
		)
	);

#undef NS_CONTENT_BROWSER 
}

void SLargeAssetTypeTreeWidget::Populate(const TArray<FString>& InSelectedAssetTypeClassNames)
{
	AssetCategories.Empty();

	TMap<EAssetTypeCategories::Type, FLargeAssetTypeTreeItemPtr> CategoryMap;
	AddCategoriesToCategoryMap(CategoryMap);
	
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
			TArray<FLargeAssetTypeTreeItemWeakPtr> AssetTypeItems;
			for (auto CategoryMapEntry : CategoryMap)
			{
				if (AssetType->GetCategories() & CategoryMapEntry.Key)
				{
					FLargeAssetTypeTreeItemPtr AssetTypeItem = 
						MakeShareable(
							new FLargeAssetTypeTreeItem(*AssetType.Get(), CategoryMapEntry.Value)
						);

					AssetTypeItems.Add(AssetTypeItem);
					CategoryMapEntry.Value->Children.Add(AssetTypeItem);
				}
			}

			if (AssetTypeItems.Num() == 1)
			{
				auto AssetTypeItem = AssetTypeItems[0].Pin();
				AssetTypeItem->SetIsSelected(
					InSelectedAssetTypeClassNames.Contains(AssetTypeItem->AssetTypeClassName)
				);
			}
			else if (AssetTypeItems.Num() > 1)
			{
				auto AssetTypeItem = AssetTypeItems[0].Pin();
				FLargeAssetTypeTreeItem::CreateSharedState(
					AssetTypeItems, 
					InSelectedAssetTypeClassNames.Contains(AssetTypeItem->AssetTypeClassName)
				);
			}
		}
	}

	for (const auto CategoryMapEntry : CategoryMap)
	{
		AssetCategories.Add(CategoryMapEntry.Value);
	}

	TreeView->RequestTreeRefresh();
}

void SLargeAssetTypeTreeWidget::SelectAssetTypesByClassName(
	const TArray<FString>& InAssetTypeClassNames
)
{
	for (const auto AssetCategory : AssetCategories)
	{
		for (const auto AssetType : AssetCategory->Children)
		{
			AssetType->SetIsSelected(InAssetTypeClassNames.Contains(AssetType->AssetTypeClassName));
		}
	}
}

void SLargeAssetTypeTreeWidget::GetSelectedAssetTypeClassNames(
	TArray<FString>& OutAssetTypeClassNames
) const
{
	for (const auto AssetCategory : AssetCategories)
	{
		for (const auto AssetType : AssetCategory->Children)
		{
			if (AssetType->IsSelected())
			{
				OutAssetTypeClassNames.AddUnique(AssetType->AssetTypeClassName);
			}
		}
	}
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
				.ToolTipText(
					!Item->AssetTypeClassName.IsEmpty() ?
						FText::FromString(FString(TEXT("Class: ")) + Item->AssetTypeClassName) : FText()
				)
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

ECheckBoxState SLargeAssetTypeTreeWidget::TreeView_IsChecked(
	FLargeAssetTypeTreeItemWeakPtr ItemWeakPtr
) const
{
	FLargeAssetTypeTreeItemPtr Item = ItemWeakPtr.Pin();
	if (Item.IsValid())
	{
		if (Item->Children.Num() > 0) // asset category item
		{
			// the checked state of an asset category item is determined by the checked states of 
			// the contained asset type items
			int32 NumberOfSelectedChildren = 0;
			for (const auto ChildItem : Item->Children)
			{
				if (ChildItem->IsSelected())
				{
					++NumberOfSelectedChildren;
				}
			}

			if (NumberOfSelectedChildren == Item->Children.Num())
			{
				return ECheckBoxState::Checked;
			}
			else if (NumberOfSelectedChildren == 0)
			{
				return ECheckBoxState::Unchecked;
			}
			else // some but not all asset type items in this asset category are checked
			{
				return ECheckBoxState::Undetermined;
			}
		}
		else // asset type item
		{
			return Item->IsSelected() ?
				ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}
	}
	return ECheckBoxState::Undetermined;
}

void SLargeAssetTypeTreeWidget::TreeView_OnCheckStateChanged(
	ECheckBoxState NewState, FLargeAssetTypeTreeItemWeakPtr ItemWeakPtr)
{
	FLargeAssetTypeTreeItemPtr Item = ItemWeakPtr.Pin();
	if (Item.IsValid())
	{
		bool bIsItemChecked = (NewState == ECheckBoxState::Checked);
		// propagate the checked state of the asset category to the asset types within it
		for (const auto ItemChild : Item->Children)
		{
			ItemChild->SetIsSelected(bIsItemChecked);
		}
		Item->SetIsSelected(bIsItemChecked);
		// the delegate will only be executed once, even if the user checks/unchecks an asset 
		// category and the checked state of multiple asset type items in that category changes, 
		// this is by design
		OnItemCheckStateChanged.ExecuteIfBound();
	}
}

#undef LOCTEXT_NAMESPACE

} // namespace MercurialSourceControl
