// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "WorldsbaseTypes.h"

#include "WorldsbaseLibrary.generated.h"

class UWorldsbaseSettings;

/**
 * Useful tools for REST communications
 */
UCLASS()
class WORLDSBASE_API UWorldsbaseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//////////////////////////////////////////////////////////////////////////
	// Data Accessors
public:
	/** Direct access to the plugin settings */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Common")
	static UWorldsbaseSettings* GetWorldsbaseSettings();

	//////////////////////////////////////////////////////////////////////////
	// Helpers

public:
	/** Applies percent-encoding to text */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility")
	static FString PercentEncode(const FString& Source);

	/**
	 * Encodes a FString into a Base64 string
	 *
	 * @param Source	The string data to convert
	 * @return			A string that encodes the binary data in a way that can be safely transmitted via various Internet protocols
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "Base64 Encode"))
	static FString Base64Encode(const FString& Source);

	/**
	 * Decodes a Base64 string into a FString
	 *
	 * @param Source	The stringified data to convert
	 * @param Dest		The out buffer that will be filled with the decoded data
	 * @return			True if the buffer was decoded, false if it failed to decode
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "Base64 Decode"))
	static bool Base64Decode(const FString& Source, FString& Dest);

	/**
	 * Encodes a byte array into a Base64 string
	 *
	 * @param Dara		The data to convert
	 * @return			A string that encodes the binary data in a way that can be safely transmitted via various Internet protocols
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "Base64 Encode Data"))
	static bool Base64EncodeData(const TArray<uint8>& Data, FString& Dest);

	/**
	 * Decodes a Base64 string into a byte array
	 *
	 * @param Source	The stringified data to convert
	 * @param Dest		The out buffer that will be filled with the decoded data
	 * @return			True if the buffer was decoded, false if it failed to decode
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "Base64 Decode Data"))
	static bool Base64DecodeData(const FString& Source, TArray<uint8>& Dest);

	/**
	 * Helper to perform the very common case of hashing an ASCII string into a hex representation.
	 *
	 * @param String	Hex representation of the hash (32 lower-case hex digits)
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "String to MD5"))
	static FString StringToMd5(const FString& StringToHash);

	/**
	 * Helper to perform the SHA1 hash operation on string.
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "String to SHA1"))
	static FString StringToSha1(const FString& StringToHash);

	/**
	 * Helper method to convert a status code from HTTP to an enum for easier readability
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "HTTP Status Int To Enum"))
	static FORCEINLINE EWorldsbaseHttpStatusCode::Type HTTPStatusIntToEnum(int32 StatusCode) { return (EWorldsbaseHttpStatusCode::Type)StatusCode; }

	/**
	 * Get the plugin's version
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (DisplayName = "Get Worldsbase Version"))
	static FString GetWorldsbaseVersion();

	//////////////////////////////////////////////////////////////////////////
	// Common Network Helpers

public:
	/**
	 * Get the URL that was used when loading this World
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Utility", meta = (WorldContext = "WorldContextObject"))
	static FWorldsbaseURL GetWorldURL(UObject* WorldContextObject);
};
