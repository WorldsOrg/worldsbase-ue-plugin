#pragma once

#include "WorldsbaseJsonValue.generated.h"

class UJsonObject;
class FJsonValue;

UENUM(BlueprintType)
enum class EJsonValue : uint8
{
	None,
	Null,
	String,
	Number,
	Boolean,
	Array,
	Object,
};

UCLASS(BlueprintType, Blueprintable)
class WORLDSBASE_API UJsonValue : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void Reset();

	TSharedPtr<FJsonValue>& GetRootValue();

	void SetRootValue(TSharedPtr<FJsonValue>& JsonValue);

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	EJsonValue GetType() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	FString GetTypeString() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	bool IsNull() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	float AsNumber() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	int32 AsInt32() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	int64 AsInt64() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	FString AsString() const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	bool AsBool() const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<UJsonValue*> AsArray() const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	UJsonObject* AsObject();

private:
	TSharedPtr<FJsonValue> JsonVal;

protected:
	void ErrorMessage(const FString& InType) const;
};
