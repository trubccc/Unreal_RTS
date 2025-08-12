// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TopDown_Utilities : ModuleRules
{
    public TopDown_Utilities(ReadOnlyTargetRules Target) : base(Target)
    {
        // 这行设置了预编译头文件的使用方式。通常保持默认即可。
        //PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // 在这里添加你的模块需要公开引用的其他引擎模块。
        // 比如，如果你的.h文件里 #include 了引擎的Actor.h，就需要 "Engine" 模块。
        // "Core", "CoreUObject", "Engine" 是C++项目最基础的模块。
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine" 
            // 如果你用到了输入，可能还需要 "InputCore"
            // 如果你用到了UI (Widget), 就需要 "UMG"
        });

        // 在这里添加你的模块私下引用的模块。
        // 这些模块只在你的.cpp文件里使用。
        PrivateDependencyModuleNames.AddRange(new string[] {  
            // 例如 "Slate", "SlateCore" 等
        });
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"GameplayAbilities",
				"AIModule",
				"NavigationSystem"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"InputCore",
				"Slate",
				"SlateCore",
				"EnhancedInput",				
				"GameplayMessageRuntime",
				"GameplayTags",
				"GameplayTasks"
				// ... add private dependencies that you statically link with here ...	
			}
			);		
    }
}