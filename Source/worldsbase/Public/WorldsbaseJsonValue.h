// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "WorldsbaseJsonValue.generated.h"

class UWorldsbaseJsonObject;
class FJsonValue;

/**
 * Represents all the types a Json Value can be.
 */
UENUM(BlueprintType)
enum class EWBJson : uint8
{
	None,
	Null,
	String,
	Number,
	Boolean,
	Array,
	Object,
};

/**
 * Blueprintable FJsonValue wrapper
 */
UCLASS(BlueprintType, Blueprintable)
class WORLDSBASE_API UWorldsbaseJsonValue : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** Reset all internal data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Json")
	void Reset();

	/** Get the root Json value */
	TSharedPtr<FJsonValue>& GetRootValue();

	/** Set the root Json value */
	void SetRootValue(TSharedPtr<FJsonValue>& JsonValue);

	//////////////////////////////////////////////////////////////////////////
	// FJsonValue API

	/** Get type of Json value (Enum) */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	EWBJson GetType() const;

	/** Get type of Json value (String) */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	FString GetTypeString() const;

	/** Returns true if this value is a 'null' */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	bool IsNull() const;

	/** Returns this value as a double, throwing an error if this is not an Json Number
	 * Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	float AsNumber() const;

	/** Returns this value as a int32, throwing an error if this is not an Json Number */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	int32 AsInt32() const;

	/** Returns this value as a int64, throwing an error if this is not an Json Number */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	int64 AsInt64() const;

	/** Returns this value as a string, throwing an error if this is not an Json String */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	FString AsString() const;

	/** Returns this value as a boolean, throwing an error if this is not an Json Bool */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Json")
	bool AsBool() const;

	/** Returns this value as an array, throwing an error if this is not an Json Array */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Json")
	TArray<UWorldsbaseJsonValue*> AsArray() const;

	/** Returns this value as an object, throwing an error if this is not an Json Object */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Json")
	UWorldsbaseJsonObject* AsObject();

	//////////////////////////////////////////////////////////////////////////
	// Data

private:
	/** Internal JSON data */
	TSharedPtr<FJsonValue> JsonVal;

	//////////////////////////////////////////////////////////////////////////
	// Helpers

protected:
	/** Simple error logger */
	void ErrorMessage(const FString& InType) const;
};
