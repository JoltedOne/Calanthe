using UnrealBuildTool;

public class CalantheEditorTarget : TargetRules
{
	public CalantheEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Calanthe");
	}
}
