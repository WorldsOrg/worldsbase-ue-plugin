#include "WorldsbaseJsonValue.h"
#include "WorldsbaseJsonObject.h"

UJsonValue::UJsonValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UJsonValue::Reset()
{
	JsonVal = nullptr;
}

TSharedPtr<FJsonValue>& UJsonValue::GetRootValue()
{
	return JsonVal;
}

void UJsonValue::SetRootValue(TSharedPtr<FJsonValue>& JsonValue)
{
	JsonVal = JsonValue;
}


EJsonValue UJsonValue::GetType() const
{
	if (!JsonVal.IsValid())
	{
		return EJsonValue::None;
	}
	switch (JsonVal->Type)
	{
	case EJson::None:
		return EJsonValue::None;
	case EJson::Null:
		return EJsonValue::Null;
	case EJson::String:
		return EJsonValue::String;
	case EJson::Number:
		return EJsonValue::Number;
	case EJson::Boolean:
		return EJsonValue::Boolean;
	case EJson::Array:
		return EJsonValue::Array;
	case EJson::Object:
		return EJsonValue::Object;
	default:
		return EJsonValue::None;
	}
}

FString UJsonValue::GetTypeString() const
{
	if (!JsonVal.IsValid())
	{
		return "None";
	}
	switch (JsonVal->Type)
	{
	case EJson::None:
		return TEXT("None");
	case EJson::Null:
		return TEXT("Null");
	case EJson::String:
		return TEXT("String");
	case EJson::Number:
		return TEXT("Number");
	case EJson::Boolean:
		return TEXT("Boolean");
	case EJson::Array:
		return TEXT("Array");
	case EJson::Object:
		return TEXT("Object");
	default:
		return TEXT("None");
	}
}

bool UJsonValue::IsNull() const
{
	if (!JsonVal.IsValid())
	{
		return true;
	}
	return JsonVal->IsNull();
}

float UJsonValue::AsNumber() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}
	return static_cast<float>(JsonVal->AsNumber());
}

int32 UJsonValue::AsInt32() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}
	return static_cast<int32>(JsonVal->AsNumber());
}

int64 UJsonValue::AsInt64() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}
	return static_cast<int64>(JsonVal->AsNumber());
}

FString UJsonValue::AsString() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("String"));
		return FString();
	}
	return JsonVal->AsString();
}

bool UJsonValue::AsBool() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Boolean"));
		return false;
	}
	return JsonVal->AsBool();
}

TArray<UJsonValue*> UJsonValue::AsArray() const
{
	TArray<UJsonValue*> OutArray;

	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Array"));
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonVal->AsArray();
	for (auto Value : ValArray)
	{
		UJsonValue* NewValue = NewObject<UJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

UJsonObject* UJsonValue::AsObject()
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Object"));
		return nullptr;
	}

	const TSharedPtr<FJsonObject> NewObj = JsonVal->AsObject();

	UJsonObject* JsonObj = NewObject<UJsonObject>();
	JsonObj->SetRootObject(NewObj);

	return JsonObj;
}


void UJsonValue::ErrorMessage(const FString& InType) const
{
	UE_LOG(LogTemp, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetTypeString(), *InType);
}
