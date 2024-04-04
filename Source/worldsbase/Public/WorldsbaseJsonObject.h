#pragma once

#include "WorldsbaseJsonObject.generated.h"

class UJsonValue;

UCLASS(BlueprintType, Blueprintable)
class WORLDSBASE_API UJsonObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void Reset();

	TSharedRef<FJsonObject>& GetRootObject();

	void SetRootObject(const TSharedPtr<FJsonObject>& JsonObject);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	FString EncodeJson() const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	FString EncodeJsonToSingleString() const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	bool DecodeJson(const FString& JsonString);

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	FString GetFieldTypeString(const FString& FieldName) const;

	UFUNCTION(BlueprintPure, Category = "Worldsbase")
	TArray<FString> GetFieldNames() const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	bool HasField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void RemoveField(const FString& FieldName);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	UJsonValue* GetField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetField(const FString& FieldName, UJsonValue* JsonValue);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<UJsonValue*> GetArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetArrayField(const FString& FieldName, const TArray<UJsonValue*>& InArray);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void MergeJsonObject(UJsonObject* InJsonObject, bool Overwrite);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	float GetNumberField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetNumberField(const FString& FieldName, float Number);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetNumberFieldDouble(const FString& FieldName, double Number);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	int32 GetIntegerField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetIntegerField(const FString& FieldName, int32 Number);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	int64 GetInt64Field(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetInt64Field(const FString& FieldName, int64 Number);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	FString GetStringField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetStringField(const FString& FieldName, const FString& StringValue);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	bool GetBoolField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetBoolField(const FString& FieldName, bool InValue);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	UJsonObject* GetObjectField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetObjectField(const FString& FieldName, UJsonObject* JsonObject);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetMapFields_string(const TMap<FString, FString>& Fields);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetMapFields_uint8(const TMap<FString, uint8>& Fields);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetMapFields_int32(const TMap<FString, int32>& Fields);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetMapFields_int64(const TMap<FString, int64>& Fields);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetMapFields_bool(const TMap<FString, bool>& Fields);

private:
	template <typename T>
	void SetMapFields_Impl(const TMap<FString, T>& Fields)
	{
		for (auto& field : Fields)
		{
			if (std::is_same_v<T, uint8> || std::is_same_v<T, int32> || std::is_same_v<T, int64> || std::is_same_v<
				T, float>)
			{
				SetNumberField(field.Key, field.Value);
			}
			else if (std::is_same_v<T, bool>)
			{
				SetBoolField(field.Key, static_cast<bool>(field.Value));
			}
		}
	}

	template <typename T>
	TArray<T> GetTypeArrayField(const FString& FieldName) const
	{
		TArray<T> NumberArray;
		if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Array"), *FieldName);
			return NumberArray;
		}

		const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
		for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
		{
			const auto Value = It->Get();
			if (Value->Type != EJson::Number)
			{
				UE_LOG(LogTemp, Error, TEXT("Not Number element in array with field name %s"), *FieldName);
			}

			NumberArray.Add((*It)->AsNumber());
		}

		return NumberArray;
	}

public:
	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<float> GetNumberArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<int32> GetIntegerArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetNumberArrayFieldDouble(const FString& FieldName, const TArray<double>& NumberArray);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<FString> GetStringArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<bool> GetBoolArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray);

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	TArray<UJsonObject*> GetObjectArrayField(const FString& FieldName) const;

	UFUNCTION(BlueprintCallable, Category = "Worldsbase")
	void SetObjectArrayField(const FString& FieldName, const TArray<UJsonObject*>& ObjectArray);

private:
	TSharedRef<FJsonObject> JsonObj;
};
