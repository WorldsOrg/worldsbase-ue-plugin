// Copyright 2014-2020 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "WorldsbaseJsonObject.h"
#include "WorldsbaseJsonValue.h"
#include "WorldsbaseRequestJSON.h"

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "HttpModule.h"
#include "HttpRequestAdapter.h"
#include "Interfaces/IHttpResponse.h"

#include "Delegates/Delegate.h"

#include "WorldsbaseSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FWorldsbaseCallDelegate, UWorldsbaseRequestJSON*, Request);
// Define a delegate with a FString parameter for the result
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHttpRequestCompleted, const FString&, Result);

/**
 * Data row struct that defines columns and values to be inserted into a table
 */
USTRUCT(BlueprintType)
struct FDataRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString ColumnName;

	UPROPERTY(BlueprintReadWrite)
	FString Value;
};

USTRUCT()
struct FWorldsbaseCallResponse
{
	GENERATED_BODY()

	UPROPERTY()
	UWorldsbaseRequestJSON* Request;

	UPROPERTY()
	FWorldsbaseCallDelegate Callback;

	FDelegateHandle CompleteDelegateHandle;
	FDelegateHandle FailDelegateHandle;

	FWorldsbaseCallResponse()
		: Request(nullptr)
	{
	}
};

UCLASS()
class WORLDSBASE_API UWorldsbaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UWorldsbaseSubsystem();

	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	//////////////////////////////////////////////////////////////////////////
	// Easy URL processing

public:
	/** Easy way to process http requests */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	void CallURL(const FString& URL, EWorldsbaseRequestVerb Verb, EWorldsbaseRequestContentType ContentType, UWorldsbaseJsonObject* WorldsbaseJson, const FWorldsbaseCallDelegate& Callback);

	/** Called when URL is processed (one for both success/unsuccess events)*/
	void OnCallComplete(UWorldsbaseRequestJSON* Request);

protected:
	UPROPERTY()
	TMap<UWorldsbaseRequestJSON*, FWorldsbaseCallResponse> RequestMap;

	//////////////////////////////////////////////////////////////////////////
	// Construction helpers

public:
	/** Creates new request (totally empty) */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Request (Empty)"), Category = "Worldsbase|Subsystem")
	UWorldsbaseRequestJSON* ConstructWorldsbaseRequest();

	/** Creates new request with defined verb and content type */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Request"), Category = "Worldsbase|Subsystem")
	UWorldsbaseRequestJSON* ConstructWorldsbaseRequestExt(EWorldsbaseRequestVerb Verb, EWorldsbaseRequestContentType ContentType);

	/** Create new Json object */
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Object"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonObject* ConstructWorldsbaseJsonObject();

	/** Create new Json object (static one for MakeJson node, hack for #293) */
	UFUNCTION()
	static UWorldsbaseJsonObject* StaticConstructWorldsbaseJsonObject();

	/** Create new Json Number value
	 * Attn.!! float used instead of double to make the function blueprintable! */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Number Value"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* ConstructJsonValueNumber(float Number);

	/** Create new Json String value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json String Value"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* ConstructJsonValueString(const FString& StringValue);

	/** Create new Json Bool value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Bool Value"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* ConstructJsonValueBool(bool InValue);

	/** Create new Json Array value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Array Value"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* ConstructJsonValueArray(const TArray<UWorldsbaseJsonValue*>& InArray);

	/** Create new Json Object value */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Construct Json Object Value"), Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* ConstructJsonValueObject(UWorldsbaseJsonObject* JsonObject);

	/** Create new Json value from FJsonValue (to be used from WorldsbaseJsonObject) */
	UWorldsbaseJsonValue* ConstructJsonValue(const TSharedPtr<FJsonValue>& InValue);

	//////////////////////////////////////////////////////////////////////////
	// Serialization

public:
	/** Construct Json value from string */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonValue* DecodeJsonValue(const FString& JsonString);

	/** Construct Json object from string */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Subsystem")
	UWorldsbaseJsonObject* DecodeJsonObject(const FString& JsonString);

	//////////////////////////////////////////////////////////////////////////
	// File system integration

public:
	/**
	 * Load JSON from formatted text file
	 * @param    bIsRelativeToContentDir    if set to 'false' path is treated as absolute
	 */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	UWorldsbaseJsonObject* LoadJsonFromFile(const FString& Path, const bool bIsRelativeToContentDir = true);

public:
	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void GetTable(const FString& TableName, const FWorldsbaseCallDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void InsertData(const FString& TableName, const TArray<FDataRow>& DataRows);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void UpdateData(const FString& TableName, const FString& ConditionColumn, const FString& ConditionValue, const TArray<FDataRow>& DataRows);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void UpdateFilteredData(const FString& TableName, const TArray<FDataRow>& Filters, const TArray<FDataRow>& DataRows);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void IncrementData(const FString& TableName, const FString& IncrementColumnName, const FString& ConditionColumn, const FString& ConditionValue, const int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void DecrementData(const FString& TableName, const FString& DecrementColumnName, const FString& ConditionColumn, const FString& ConditionValue, const int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void GetValue(const FString& TableName, const FString& ColumnName, const FString& ColumnValue, const FWorldsbaseCallDelegate& Callback);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void GetFilteredTableValues(const FString& TableName, const TMap<FString, FString>& Filters, const FWorldsbaseCallDelegate& Callback);

	// Delegate instance for Blueprint to bind to
	UPROPERTY(BlueprintAssignable, Category = "Worldsbase")
	FOnHttpRequestCompleted OnHttpRequestCompleted;

private:
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
