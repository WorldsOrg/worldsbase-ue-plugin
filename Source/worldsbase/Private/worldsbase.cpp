// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "worldsbase.h"

#include "WorldsbaseDefines.h"
#include "WorldsbaseLibrary.h"
#include "WorldsbaseSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "FWorldsbaseModule"

void FWorldsbaseModule::StartupModule()
{
	ModuleSettings = NewObject<UWorldsbaseSettings>(GetTransientPackage(), "WorldsbaseSettings", RF_Standalone);
	ModuleSettings->AddToRoot();

	// Register settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Worldsbase",
			LOCTEXT("RuntimeSettingsName", "Worldsbase"),
			LOCTEXT("RuntimeSettingsDescription", "Configure Worldsbase plugin settings"),
			ModuleSettings);
	}

	UE_LOG(LogWorldsbase, Log, TEXT("%s: Worldsbase (%s) module started"), *VA_FUNC_LINE, *UWorldsbaseLibrary::GetWorldsbaseVersion());
}

void FWorldsbaseModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Worldsbase");
	}

	if (!GExitPurge)
	{
		ModuleSettings->RemoveFromRoot();
	}
	else
	{
		ModuleSettings = nullptr;
	}
}

UWorldsbaseSettings* FWorldsbaseModule::GetSettings() const
{
	check(ModuleSettings);
	return ModuleSettings;
}

IMPLEMENT_MODULE(FWorldsbaseModule, Worldsbase)

DEFINE_LOG_CATEGORY(LogWorldsbase);

#undef LOCTEXT_NAMESPACE
