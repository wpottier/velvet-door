using UnrealBuildTool;

public class VelvetDoor : ModuleRules
{
	public VelvetDoor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "CommonUI",
            "CommonInput",
            "Slate",
            "SlateCore",
            "ApplicationCore",
            "ModelViewViewModel",
            "OnlineSubsystem",
            "RedpointEOSAPI",
            "RedpointEOSCore",
            "RedpointEOSPresence",
            "RedpointEOSFriends",
            "RedpointEOSUserCache",
            "RedpointEOSIdentity"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "OnlineSubsystemUtils"
        });
    }
}
