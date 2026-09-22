using UnrealBuildTool;
using System.Collections.Generic;

public class FleetCommanderEditorTarget : TargetRules
{
    public FleetCommanderEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("FleetCommander");
    }
}
