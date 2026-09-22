using UnrealBuildTool;

public class FleetCommander : ModuleRules
{
    public FleetCommander(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "InputCore",
            "MeshDescription", "StaticMeshDescription", "Json", "JsonUtilities",
            "ImageWrapper"
        });
    }
}
