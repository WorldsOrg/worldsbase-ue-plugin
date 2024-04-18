// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseJsonValue.h"

#include "WorldsbaseDefines.h"
#include "WorldsbaseJsonObject.h"

UWorldsbaseJsonValue::UWorldsbaseJsonValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWorldsbaseJsonValue::Reset()
{
	JsonVal = nullptr;
}

TSharedPtr<FJsonValue>& UWorldsbaseJsonValue::GetRootValue()
{
	return JsonVal;
}

void UWorldsbaseJsonValue::SetRootValue(TSharedPtr<FJsonValue>& JsonValue)
{
	JsonVal = JsonValue;
}

//////////////////////////////////////////////////////////////////////////
// FJsonValue API

EWBJson UWorldsbaseJsonValue::GetType() const
{
	if (!JsonVal.IsValid())
	{
		return EWBJson::None;
	}

	switch (JsonVal->Type)
	{
	case EJson::None:
		return EWBJson::None;

	case EJson::Null:
		return EWBJson::Null;

	case EJson::String:
		return EWBJson::String;

	case EJson::Number:
		return EWBJson::Number;

	case EJson::Boolean:
		return EWBJson::Boolean;

	case EJson::Array:
		return EWBJson::Array;

	case EJson::Object:
		return EWBJson::Object;

	default:
		return EWBJson::None;
	}
}

FString UWorldsbaseJsonValue::GetTypeString() const
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

bool UWorldsbaseJsonValue::IsNull() const
{
	if (!JsonVal.IsValid())
	{
		return true;
	}

	return JsonVal->IsNull();
}

float UWorldsbaseJsonValue::AsNumber() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}

	return static_cast<float>(JsonVal->AsNumber());
}

int32 UWorldsbaseJsonValue::AsInt32() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}

	return static_cast<int32>(JsonVal->AsNumber());
}

int64 UWorldsbaseJsonValue::AsInt64() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Number"));
		return 0.f;
	}

	return static_cast<int64>(JsonVal->AsNumber());
}

FString UWorldsbaseJsonValue::AsString() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("String"));
		return FString();
	}

	return JsonVal->AsString();
}

bool UWorldsbaseJsonValue::AsBool() const
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Boolean"));
		return false;
	}

	return JsonVal->AsBool();
}

TArray<UWorldsbaseJsonValue*> UWorldsbaseJsonValue::AsArray() const
{
	TArray<UWorldsbaseJsonValue*> OutArray;

	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Array"));
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonVal->AsArray();
	for (auto Value : ValArray)
	{
		UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

UWorldsbaseJsonObject* UWorldsbaseJsonValue::AsObject()
{
	if (!JsonVal.IsValid())
	{
		ErrorMessage(TEXT("Object"));
		return nullptr;
	}

	const TSharedPtr<FJsonObject> NewObj = JsonVal->AsObject();

	UWorldsbaseJsonObject* JsonObj = NewObject<UWorldsbaseJsonObject>();
	JsonObj->SetRootObject(NewObj);

	return JsonObj;
}

//////////////////////////////////////////////////////////////////////////
// Helpers

void UWorldsbaseJsonValue::ErrorMessage(const FString& InType) const
{
	UE_LOG(LogWorldsbase, Error, TEXT("Json Value of type '%s' used as a '%s'."), *GetTypeString(), *InType);
}
