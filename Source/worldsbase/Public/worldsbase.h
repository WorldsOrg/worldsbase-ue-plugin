// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class UWorldsbaseSettings;

class FWorldsbaseModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FWorldsbaseModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FWorldsbaseModule>("Worldsbase");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Worldsbase");
	}

	/** Getter for internal settings object to support runtime configuration changes */
	UWorldsbaseSettings* GetSettings() const;

protected:
	/** Module settings */
	UWorldsbaseSettings* ModuleSettings;
};
