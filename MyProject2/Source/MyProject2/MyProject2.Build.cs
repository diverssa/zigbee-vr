using UnrealBuildTool;

public class MyProject2 : ModuleRules
{
    public MyProject2(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate",
            "SlateCore"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ApplicationCore",
            "UMG",
            "Slate",
            "SlateCore"
        });
    }
}
