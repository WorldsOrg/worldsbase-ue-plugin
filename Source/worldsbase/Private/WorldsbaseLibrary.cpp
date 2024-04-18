// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseLibrary.h"

#include "Worldsbase.h"
#include "WorldsbaseDefines.h"
#include "WorldsbaseRequestJSON.h"

#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Base64.h"

UWorldsbaseSettings* UWorldsbaseLibrary::GetWorldsbaseSettings()
{
	return FWorldsbaseModule::Get().GetSettings();
}

FString UWorldsbaseLibrary::PercentEncode(const FString& Source)
{
	return FGenericPlatformHttp::UrlEncode(Source);
}

FString UWorldsbaseLibrary::Base64Encode(const FString& Source)
{
	TArray<uint8> ByteArray;
	const FTCHARToUTF8 StringSrc = FTCHARToUTF8(Source.GetCharArray().GetData());
	ByteArray.Append((uint8*)StringSrc.Get(), StringSrc.Length());

	return FBase64::Encode(ByteArray);
}

bool UWorldsbaseLibrary::Base64Decode(const FString& Source, FString& Dest)
{
	TArray<uint8> ByteArray;
	const bool Success = FBase64::Decode(Source, ByteArray);

	const FUTF8ToTCHAR StringSrc = FUTF8ToTCHAR((const ANSICHAR*)ByteArray.GetData(), ByteArray.Num());
	Dest = FString();
	Dest.AppendChars(StringSrc.Get(), StringSrc.Length());

	return Success;
}

bool UWorldsbaseLibrary::Base64EncodeData(const TArray<uint8>& Data, FString& Dest)
{
	if (Data.Num() > 0)
	{
		Dest = FBase64::Encode(Data);
		return true;
	}

	Dest = FString();
	return false;
}

bool UWorldsbaseLibrary::Base64DecodeData(const FString& Source, TArray<uint8>& Dest)
{
	return FBase64::Decode(Source, Dest);
}

FString UWorldsbaseLibrary::StringToMd5(const FString& StringToHash)
{
	return FMD5::HashAnsiString(*StringToHash);
}

FString UWorldsbaseLibrary::StringToSha1(const FString& StringToHash)
{
	FSHA1 Sha1Gen;

	Sha1Gen.Update((unsigned char*)TCHAR_TO_ANSI(*StringToHash), FCString::Strlen(*StringToHash));
	Sha1Gen.Final();

	FString Sha1String;
	for (int32 i = 0; i < 20; i++)
	{
		Sha1String += FString::Printf(TEXT("%02x"), Sha1Gen.m_digest[i]);
	}

	return Sha1String;
}

FString UWorldsbaseLibrary::GetWorldsbaseVersion()
{
	const auto PluginRef = IPluginManager::Get().FindPlugin("Worldsbase");

	return !PluginRef.IsValid() ? FString("invalid") : PluginRef->GetDescriptor().VersionName;
}

FWorldsbaseURL UWorldsbaseLibrary::GetWorldURL(UObject* WorldContextObject)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			return FWorldsbaseURL(World->URL);
		}
	}

	return FWorldsbaseURL();
}
