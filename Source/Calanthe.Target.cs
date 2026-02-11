using UnrealBuildTool;

public class CalantheTarget : TargetRules
{
	public CalantheTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Calanthe");
	}
}
