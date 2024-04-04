#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "HttpRequestAdapter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

#include "Delegates/Delegate.h"

#include "WorldsbaseSubsystem.generated.h"

enum class ERequestContentType : uint8;
enum class ERequestVerb : uint8;
enum EHttpStatusCode;
class UJsonObject;

// Define a delegate with a FString parameter for the result
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHttpRequestCompletedString, const FString&, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnHttpRequestCompletedJson, EHttpStatusCode, Status,
	UJsonObject*, Result
);

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

/**
 *
 */
UCLASS()
class WORLDSBASE_API UWorldsbaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void GetTable(const FString& TableName);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void InsertData(const FString& TableName, const TArray<FDataRow>& DataRows);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void UpdateData(const FString& TableName, const FString& ConditionColumn, const FString& ConditionValue,
	                const TArray<FDataRow>& DataRows);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void IncrementData(const FString& TableName, const FString& IncrementColumnName, const FString& ConditionColumn,
	                   const FString& ConditionValue, const int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void DecrementData(const FString& TableName, const FString& DecrementColumnName, const FString& ConditionColumn,
	                   const FString& ConditionValue, const int32 Value);

	// Delegate instance for Blueprint to bind to
	UPROPERTY(BlueprintAssignable, Category = "Worldsbase")
	FOnHttpRequestCompletedString OnHttpRequestCompletedString;

	UFUNCTION(BlueprintCallable, Category="Worldsbase")
	void MakeHttpRequest(
		UPARAM() const FString& URL,
		UPARAM() const ERequestVerb Verb,
		UPARAM() UJsonObject* Body,
		UPARAM() const ERequestContentType ContentType,
		UPARAM() const TMap<FString, FString>& RequestHeaders,
		UPARAM() const TArray<uint8>& Bytes,
		UPARAM() const FString& StringContent
	);

	UPROPERTY(BlueprintAssignable, Category = "Worldsbase")
	FOnHttpRequestCompletedJson OnHttpRequestCompletedJson;

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Construct Json Object"), Category = "VaRest|Subsystem")
	UJsonObject* ConstructJsonObject();

private:
	void OnProcessRequestCompleteString(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void OnProcessRequestCompleteJson(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
