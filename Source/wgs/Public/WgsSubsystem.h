#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "HttpRequestAdapter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

#include "Delegates/Delegate.h"

#include "WgsSubsystem.generated.h"


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

/**
 *
 */
UCLASS()
class WGS_API UWgsSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "WGS")
	void GetTable(const FString& TableName);

	UFUNCTION(BlueprintCallable, Category = "WGS")
	void InsertData(const FString& TableName, const TArray<FDataRow>& DataRows);

	// Delegate instance for Blueprint to bind to
	UPROPERTY(BlueprintAssignable, Category = "WGS")
	FOnHttpRequestCompleted OnHttpRequestCompleted;

private:

	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
