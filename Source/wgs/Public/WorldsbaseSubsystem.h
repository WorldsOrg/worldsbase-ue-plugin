#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "HttpRequestAdapter.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

#include "Delegates/Delegate.h"

#include "WorldsbaseSubsystem.generated.h"


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
class WORLDSBASE_API UWorldsbaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void GetTable(const FString& TableName);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void InsertData(const FString& TableName, const TArray<FDataRow>& DataRows);

	// Delegate instance for Blueprint to bind to
	UPROPERTY(BlueprintAssignable, Category = "Worldsbase")
	FOnHttpRequestCompleted OnHttpRequestCompleted;

private:

	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
