// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class OpenPropagationLibrary : ModuleRules
{
	public OpenPropagationLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/OpenPL"));
        
        if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
        {
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Binaries", "Win64/OpenPL.dll"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Binaries", "Mac/OpenPL.dylib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Binaries", "Linux/libOpenPL.so"));
        }
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
            
        PrivateDependencyModuleNames.AddRange(new string[] { "FMODStudio" });
	}
}
