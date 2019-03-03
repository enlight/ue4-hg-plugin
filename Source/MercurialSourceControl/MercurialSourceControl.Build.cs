namespace UnrealBuildTool.Rules
{
	public class MercurialSourceControl : ModuleRules
	{
		public MercurialSourceControl(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivatePCHHeaderFile = "Private/MercurialSourceControlPrivatePCH.h";
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
                    "Slate",
					"SlateCore",
					"EditorStyle",
					"SourceControl",
					"XmlParser",
                    "InputCore",
                    "DesktopPlatform",
                    "AssetTools",
                    "CoreUObject",
                    "AssetRegistry",
                    "UnrealEd"
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);
		}
	}
}