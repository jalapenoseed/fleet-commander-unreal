using UnrealBuildTool;
using System.Collections.Generic;

public class FleetCommanderTarget : TargetRules
{
    public FleetCommanderTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("FleetCommander");
    }
}
