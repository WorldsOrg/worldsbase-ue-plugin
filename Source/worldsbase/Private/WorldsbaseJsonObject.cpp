#include "WorldsbaseJsonObject.h"
#include "WorldsbaseJsonValue.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

using FCondensedJsonStringWriterFactory = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;
using FCondensedJsonStringWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;

UJsonObject::UJsonObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , JsonObj(MakeShared<FJsonObject>())
{
}

void UJsonObject::Reset()
{
	JsonObj = MakeShared<FJsonObject>();
}

TSharedRef<FJsonObject>& UJsonObject::GetRootObject()
{
	return JsonObj;
}

void UJsonObject::SetRootObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (JsonObject.IsValid())
	{
		JsonObj = JsonObject.ToSharedRef();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to set invalid json object as root one. Reset now."));
		Reset();
	}
}

FString UJsonObject::EncodeJson() const
{
	FString OutputString;
	const auto Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);

	return OutputString;
}

FString UJsonObject::EncodeJsonToSingleString() const
{
	FString OutputString;
	const auto Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);

	return OutputString;
}

bool UJsonObject::DecodeJson(const FString& JsonString)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*JsonString);
	TSharedPtr<FJsonObject> OutJsonObj;
	if (FJsonSerializer::Deserialize(Reader, OutJsonObj))
	{
		JsonObj = OutJsonObj.ToSharedRef();
		return true;
	}

	Reset();

	UE_LOG(LogTemp, Error, TEXT("Json decoding failed for: %s"), *JsonString);

	return false;
}

FString UJsonObject::GetFieldTypeString(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Null>(FieldName))
	{
		return TEXT("Null");
	}
	if (!JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		return TEXT("String");
	}
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		return TEXT("Number");
	}
	if (!JsonObj->HasTypedField<EJson::Boolean>(FieldName))
	{
		return TEXT("Boolean");
	}
	if (!JsonObj->HasTypedField<EJson::Object>(FieldName))
	{
		return TEXT("Object");
	}
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		return TEXT("Array");
	}

	UE_LOG(LogTemp, Warning, TEXT("Field with name %s type unknown"), *FieldName);
	return "";
}

TArray<FString> UJsonObject::GetFieldNames() const
{
	TArray<FString> Result;
	JsonObj->Values.GetKeys(Result);

	return Result;
}

bool UJsonObject::HasField(const FString& FieldName) const
{
	if (FieldName.IsEmpty())
	{
		return false;
	}

	return JsonObj->HasField(FieldName);
}

void UJsonObject::RemoveField(const FString& FieldName)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->RemoveField(FieldName);
}

UJsonValue* UJsonObject::GetField(const FString& FieldName) const
{
	if (FieldName.IsEmpty())
	{
		return nullptr;
	}

	TSharedPtr<FJsonValue> NewVal = JsonObj->TryGetField(FieldName);
	if (NewVal.IsValid())
	{
		UJsonValue* NewValue = NewObject<UJsonValue>();
		NewValue->SetRootValue(NewVal);

		return NewValue;
	}

	return nullptr;
}

void UJsonObject::SetField(const FString& FieldName, UJsonValue* JsonValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetField(FieldName, JsonValue->GetRootValue());
}

float UJsonObject::GetNumberField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0.0f;
	}

	return JsonObj->GetNumberField(FieldName);
}

void UJsonObject::SetNumberField(const FString& FieldName, float Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

void UJsonObject::SetNumberFieldDouble(const FString& FieldName, double Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

int32 UJsonObject::GetIntegerField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0;
	}

	return JsonObj->GetIntegerField(FieldName);
}

void UJsonObject::SetIntegerField(const FString& FieldName, int32 Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

int64 UJsonObject::GetInt64Field(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0;
	}

	return static_cast<int64>(JsonObj->GetNumberField(FieldName));
}

void UJsonObject::SetInt64Field(const FString& FieldName, int64 Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

FString UJsonObject::GetStringField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type String"), *FieldName);
		return TEXT("");
	}

	return JsonObj->GetStringField(FieldName);
}

void UJsonObject::SetStringField(const FString& FieldName, const FString& StringValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetStringField(FieldName, StringValue);
}

bool UJsonObject::GetBoolField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Boolean>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Boolean"), *FieldName);
		return false;
	}

	return JsonObj->GetBoolField(FieldName);
}

void UJsonObject::SetBoolField(const FString& FieldName, bool InValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetBoolField(FieldName, InValue);
}

TArray<UJsonValue*> UJsonObject::GetArrayField(const FString& FieldName) const
{
	TArray<UJsonValue*> OutArray;
	if (FieldName.IsEmpty())
	{
		return OutArray;
	}

	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Array"), *FieldName);
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonObj->GetArrayField(FieldName);
	for (auto Value : ValArray)
	{
		UJsonValue* NewValue = NewObject<UJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

void UJsonObject::SetArrayField(const FString& FieldName, const TArray<UJsonValue*>& InArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray;

	for (auto InVal : InArray)
	{
		if (InVal == nullptr)
		{
			continue;
		}

		const TSharedPtr<FJsonValue> JsonVal = InVal->GetRootValue();

		switch (InVal->GetType())
		{
		case EJsonValue::None:
			break;

		case EJsonValue::Null:
			ValArray.Add(MakeShareable(new FJsonValueNull()));
			break;

		case EJsonValue::String:
			ValArray.Add(MakeShareable(new FJsonValueString(JsonVal->AsString())));
			break;

		case EJsonValue::Number:
			ValArray.Add(MakeShareable(new FJsonValueNumber(JsonVal->AsNumber())));
			break;

		case EJsonValue::Boolean:
			ValArray.Add(MakeShareable(new FJsonValueBoolean(JsonVal->AsBool())));
			break;

		case EJsonValue::Array:
			ValArray.Add(MakeShareable(new FJsonValueArray(JsonVal->AsArray())));
			break;

		case EJsonValue::Object:
			ValArray.Add(MakeShareable(new FJsonValueObject(JsonVal->AsObject())));
			break;

		default:
			break;
		}
	}

	JsonObj->SetArrayField(FieldName, ValArray);
}

void UJsonObject::MergeJsonObject(UJsonObject* InJsonObject, bool Overwrite)
{
	if (!InJsonObject || !InJsonObject->IsValidLowLevel())
	{
		return;
	}

	TArray<FString> Keys = InJsonObject->GetFieldNames();

	for (auto Key : Keys)
	{
		if (Overwrite == false && HasField(Key))
		{
			continue;
		}

		SetField(Key, InJsonObject->GetField(Key));
	}
}

UJsonObject* UJsonObject::GetObjectField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Object>(FieldName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Object"), *FieldName);
		return nullptr;
	}

	const TSharedPtr<FJsonObject> JsonObjField = JsonObj->GetObjectField(FieldName);

	UJsonObject* OutRestJsonObj = NewObject<UJsonObject>();
	OutRestJsonObj->SetRootObject(JsonObjField);

	return OutRestJsonObj;
}

void UJsonObject::SetObjectField(const FString& FieldName, UJsonObject* JsonObject)
{
	if (FieldName.IsEmpty() || !JsonObject || !JsonObject->IsValidLowLevel())
	{
		return;
	}

	JsonObj->SetObjectField(FieldName, JsonObject->GetRootObject());
}

void UJsonObject::SetMapFields_string(const TMap<FString, FString>& Fields)
{
	for (auto& field : Fields)
	{
		SetStringField(field.Key, field.Value);
	}
}

void UJsonObject::SetMapFields_uint8(const TMap<FString, uint8>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UJsonObject::SetMapFields_int32(const TMap<FString, int32>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UJsonObject::SetMapFields_int64(const TMap<FString, int64>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UJsonObject::SetMapFields_bool(const TMap<FString, bool>& Fields)
{
	SetMapFields_Impl(Fields);
}

TArray<float> UJsonObject::GetNumberArrayField(const FString& FieldName) const
{
	return GetTypeArrayField<float>(FieldName);
}

TArray<int32> UJsonObject::GetIntegerArrayField(const FString& FieldName) const
{
	return GetTypeArrayField<int32>(FieldName);
}

void UJsonObject::SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;

	for (auto Number : NumberArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueNumber(Number)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

void UJsonObject::SetNumberArrayFieldDouble(const FString& FieldName, const TArray<double>& NumberArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;

	for (auto Number : NumberArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueNumber(Number)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<FString> UJsonObject::GetStringArrayField(const FString& FieldName) const
{
	TArray<FString> StringArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Array"), *FieldName);
		return StringArray;
	}

	const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
	{
		const auto Value = It->Get();
		if (Value->Type != EJson::String)
		{
			UE_LOG(LogTemp, Error, TEXT("Not String element in array with field name %s"), *FieldName);
		}

		StringArray.Add((*It)->AsString());
	}

	return StringArray;
}

void UJsonObject::SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (auto String : StringArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueString(String)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<bool> UJsonObject::GetBoolArrayField(const FString& FieldName) const
{
	TArray<bool> BoolArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Array"), *FieldName);
		return BoolArray;
	}

	const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
	{
		const auto Value = It->Get();
		if (Value->Type != EJson::Boolean)
		{
			UE_LOG(LogTemp, Error, TEXT("Not Boolean element in array with field name %s"), *FieldName);
		}

		BoolArray.Add((*It)->AsBool());
	}

	return BoolArray;
}

void UJsonObject::SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (auto Boolean : BoolArray)
	{
		EntriesArray.Add(MakeShareable(new FJsonValueBoolean(Boolean)));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

TArray<UJsonObject*> UJsonObject::GetObjectArrayField(const FString& FieldName) const
{
	TArray<UJsonObject*> OutArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No field with name %s of type Array"), *FieldName);
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonObj->GetArrayField(FieldName);
	for (const auto& Value : ValArray)
	{
		if (Value->Type != EJson::Object)
		{
			UE_LOG(LogTemp, Error, TEXT("Not Object element in array with field name %s"), *FieldName);
		}

		TSharedPtr<FJsonObject> NewObj = Value->AsObject();

		UJsonObject* NewJson = NewObject<UJsonObject>();
		NewJson->SetRootObject(NewObj);

		OutArray.Add(NewJson);
	}

	return OutArray;
}

void UJsonObject::SetObjectArrayField(const FString& FieldName, const TArray<UJsonObject*>& ObjectArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (auto Value : ObjectArray)
	{
		if (Value == nullptr)
		{
			continue;
		}

		EntriesArray.Add(MakeShareable(new FJsonValueObject(Value->GetRootObject())));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}
