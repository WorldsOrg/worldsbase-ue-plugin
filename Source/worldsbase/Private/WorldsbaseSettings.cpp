// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseSettings.h"

UWorldsbaseSettings::UWorldsbaseSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bExtendedLog = false;
	bUseChunkedParser = false;
}
