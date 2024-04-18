// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseJsonObject.h"

#include "WorldsbaseDefines.h"
#include "WorldsbaseJsonParser.h"
#include "WorldsbaseJsonValue.h"

#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

typedef TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> FCondensedJsonStringWriterFactory;
typedef TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>> FCondensedJsonStringWriter;

UWorldsbaseJsonObject::UWorldsbaseJsonObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, JsonObj(MakeShared<FJsonObject>())
{
}

void UWorldsbaseJsonObject::Reset()
{
	JsonObj = MakeShared<FJsonObject>();
}

TSharedRef<FJsonObject>& UWorldsbaseJsonObject::GetRootObject()
{
	return JsonObj;
}

void UWorldsbaseJsonObject::SetRootObject(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (JsonObject.IsValid())
	{
		JsonObj = JsonObject.ToSharedRef();
	}
	else
	{
		UE_LOG(LogWorldsbase, Error, TEXT("%s: Trying to set invalid json object as root one. Reset now."), *VA_FUNC_LINE);
		Reset();
	}
}

//////////////////////////////////////////////////////////////////////////
// Serialization

FString UWorldsbaseJsonObject::EncodeJson() const
{
	FString OutputString;
	const auto Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);

	return OutputString;
}

FString UWorldsbaseJsonObject::EncodeJsonToSingleString() const
{
	FString OutputString;
	const auto Writer = FCondensedJsonStringWriterFactory::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObj, Writer);

	return OutputString;
}

bool UWorldsbaseJsonObject::DecodeJson(const FString& JsonString, bool bUseIncrementalParser)
{
	if (bUseIncrementalParser)
	{
		const int32 BytesRead = DeserializeFromTCHARBytes(JsonString.GetCharArray().GetData(), JsonString.Len());

		// JsonObj is always valid, but read bytes is zero when something went wrong
		if (BytesRead > 0)
		{
			return true;
		}
	}
	else
	{
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*JsonString);
		TSharedPtr<FJsonObject> OutJsonObj;
		if (FJsonSerializer::Deserialize(Reader, OutJsonObj))
		{
			JsonObj = OutJsonObj.ToSharedRef();
			return true;
		}
	}

	// If we've failed to deserialize the string, we should clear our internal data
	Reset();

	UE_LOG(LogWorldsbase, Error, TEXT("Json decoding failed for: %s"), *JsonString);

	return false;
}

//////////////////////////////////////////////////////////////////////////
// FJsonObject API

FString UWorldsbaseJsonObject::GetFieldTypeString(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Null>(FieldName))
	{
		return TEXT("Null");
	}
	else if (!JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		return TEXT("String");
	}
	else if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		return TEXT("Number");
	}
	else if (!JsonObj->HasTypedField<EJson::Boolean>(FieldName))
	{
		return TEXT("Boolean");
	}
	else if (!JsonObj->HasTypedField<EJson::Object>(FieldName))
	{
		return TEXT("Object");
	}
	else if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		return TEXT("Array");
	}

	UE_LOG(LogWorldsbase, Warning, TEXT("Field with name %s type unknown"), *FieldName);
	return "";
}

TArray<FString> UWorldsbaseJsonObject::GetFieldNames() const
{
	TArray<FString> Result;
	JsonObj->Values.GetKeys(Result);

	return Result;
}

bool UWorldsbaseJsonObject::HasField(const FString& FieldName) const
{
	if (FieldName.IsEmpty())
	{
		return false;
	}

	return JsonObj->HasField(FieldName);
}

void UWorldsbaseJsonObject::RemoveField(const FString& FieldName)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->RemoveField(FieldName);
}

UWorldsbaseJsonValue* UWorldsbaseJsonObject::GetField(const FString& FieldName) const
{
	if (FieldName.IsEmpty())
	{
		return nullptr;
	}

	TSharedPtr<FJsonValue> NewVal = JsonObj->TryGetField(FieldName);
	if (NewVal.IsValid())
	{
		UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>();
		NewValue->SetRootValue(NewVal);

		return NewValue;
	}

	return nullptr;
}

void UWorldsbaseJsonObject::SetField(const FString& FieldName, UWorldsbaseJsonValue* JsonValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetField(FieldName, JsonValue->GetRootValue());
}

//////////////////////////////////////////////////////////////////////////
// FJsonObject API Helpers (easy to use with simple Json objects)

float UWorldsbaseJsonObject::GetNumberField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0.0f;
	}

	return JsonObj->GetNumberField(FieldName);
}

void UWorldsbaseJsonObject::SetNumberField(const FString& FieldName, float Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

int32 UWorldsbaseJsonObject::GetIntegerField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0;
	}

	return JsonObj->GetIntegerField(FieldName);
}

void UWorldsbaseJsonObject::SetIntegerField(const FString& FieldName, int32 Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

int64 UWorldsbaseJsonObject::GetInt64Field(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Number>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("No field with name %s of type Number"), *FieldName);
		return 0;
	}

	return static_cast<int64>(JsonObj->GetNumberField(FieldName));
}

void UWorldsbaseJsonObject::SetInt64Field(const FString& FieldName, int64 Number)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetNumberField(FieldName, Number);
}

FString UWorldsbaseJsonObject::GetStringField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::String>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("No field with name %s of type String"), *FieldName);
		return TEXT("");
	}

	return JsonObj->GetStringField(FieldName);
}

void UWorldsbaseJsonObject::SetStringField(const FString& FieldName, const FString& StringValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetStringField(FieldName, StringValue);
}

bool UWorldsbaseJsonObject::GetBoolField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Boolean>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("No field with name %s of type Boolean"), *FieldName);
		return false;
	}

	return JsonObj->GetBoolField(FieldName);
}

void UWorldsbaseJsonObject::SetBoolField(const FString& FieldName, bool InValue)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	JsonObj->SetBoolField(FieldName, InValue);
}

TArray<UWorldsbaseJsonValue*> UWorldsbaseJsonObject::GetArrayField(const FString& FieldName) const
{
	TArray<UWorldsbaseJsonValue*> OutArray;
	if (FieldName.IsEmpty())
	{
		return OutArray;
	}

	if (!JsonObj->HasTypedField<EJson::Array>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: No field with name %s of type Array"), *VA_FUNC_LINE, *FieldName);
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonObj->GetArrayField(FieldName);
	for (auto Value : ValArray)
	{
		UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>();
		NewValue->SetRootValue(Value);

		OutArray.Add(NewValue);
	}

	return OutArray;
}

void UWorldsbaseJsonObject::SetArrayField(const FString& FieldName, const TArray<UWorldsbaseJsonValue*>& InArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray;

	// Process input array and COPY original values
	for (auto InVal : InArray)
	{
		if (InVal == nullptr)
			continue;

		const TSharedPtr<FJsonValue> JsonVal = InVal->GetRootValue();

		switch (InVal->GetType())
		{
		case EWBJson::None:
			break;

		case EWBJson::Null:
			ValArray.Add(MakeShareable(new FJsonValueNull()));
			break;

		case EWBJson::String:
			ValArray.Add(MakeShareable(new FJsonValueString(JsonVal->AsString())));
			break;

		case EWBJson::Number:
			ValArray.Add(MakeShareable(new FJsonValueNumber(JsonVal->AsNumber())));
			break;

		case EWBJson::Boolean:
			ValArray.Add(MakeShareable(new FJsonValueBoolean(JsonVal->AsBool())));
			break;

		case EWBJson::Array:
			ValArray.Add(MakeShareable(new FJsonValueArray(JsonVal->AsArray())));
			break;

		case EWBJson::Object:
			ValArray.Add(MakeShareable(new FJsonValueObject(JsonVal->AsObject())));
			break;

		default:
			break;
		}
	}

	JsonObj->SetArrayField(FieldName, ValArray);
}

void UWorldsbaseJsonObject::MergeJsonObject(UWorldsbaseJsonObject* InJsonObject, bool Overwrite)
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

UWorldsbaseJsonObject* UWorldsbaseJsonObject::GetObjectField(const FString& FieldName) const
{
	if (!JsonObj->HasTypedField<EJson::Object>(FieldName))
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: No field with name %s of type Object"), *VA_FUNC_LINE, *FieldName);
		return nullptr;
	}

	const TSharedPtr<FJsonObject> JsonObjField = JsonObj->GetObjectField(FieldName);

	UWorldsbaseJsonObject* OutRestJsonObj = NewObject<UWorldsbaseJsonObject>();
	OutRestJsonObj->SetRootObject(JsonObjField);

	return OutRestJsonObj;
}

void UWorldsbaseJsonObject::SetObjectField(const FString& FieldName, UWorldsbaseJsonObject* JsonObject)
{
	if (FieldName.IsEmpty() || !JsonObject || !JsonObject->IsValidLowLevel())
	{
		return;
	}

	JsonObj->SetObjectField(FieldName, JsonObject->GetRootObject());
}

void UWorldsbaseJsonObject::SetMapFields_string(const TMap<FString, FString>& Fields)
{
	for (auto& field : Fields)
	{
		SetStringField(field.Key, field.Value);
	}
}

void UWorldsbaseJsonObject::SetMapFields_uint8(const TMap<FString, uint8>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UWorldsbaseJsonObject::SetMapFields_int32(const TMap<FString, int32>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UWorldsbaseJsonObject::SetMapFields_int64(const TMap<FString, int64>& Fields)
{
	SetMapFields_Impl(Fields);
}

void UWorldsbaseJsonObject::SetMapFields_bool(const TMap<FString, bool>& Fields)
{
	SetMapFields_Impl(Fields);
}

//////////////////////////////////////////////////////////////////////////
// Array fields helpers (uniform arrays)

TArray<float> UWorldsbaseJsonObject::GetNumberArrayField(const FString& FieldName) const
{
	return GetTypeArrayField<float>(FieldName);
}

TArray<int32> UWorldsbaseJsonObject::GetIntegerArrayField(const FString& FieldName) const
{
	return GetTypeArrayField<int32>(FieldName);
}

void UWorldsbaseJsonObject::SetNumberArrayField(const FString& FieldName, const TArray<float>& NumberArray)
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

TArray<FString> UWorldsbaseJsonObject::GetStringArrayField(const FString& FieldName) const
{
	TArray<FString> StringArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: No field with name %s of type Array"), *VA_FUNC_LINE, *FieldName);
		return StringArray;
	}

	const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
	{
		const auto Value = (*It).Get();
		if (Value->Type != EJson::String)
		{
			UE_LOG(LogWorldsbase, Error, TEXT("Not String element in array with field name %s"), *FieldName);
		}

		StringArray.Add((*It)->AsString());
	}

	return StringArray;
}

void UWorldsbaseJsonObject::SetStringArrayField(const FString& FieldName, const TArray<FString>& StringArray)
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

TArray<bool> UWorldsbaseJsonObject::GetBoolArrayField(const FString& FieldName) const
{
	TArray<bool> BoolArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: No field with name %s of type Array"), *VA_FUNC_LINE, *FieldName);
		return BoolArray;
	}

	const TArray<TSharedPtr<FJsonValue>> JsonArrayValues = JsonObj->GetArrayField(FieldName);
	for (TArray<TSharedPtr<FJsonValue>>::TConstIterator It(JsonArrayValues); It; ++It)
	{
		const auto Value = (*It).Get();
		if (Value->Type != EJson::Boolean)
		{
			UE_LOG(LogWorldsbase, Error, TEXT("Not Boolean element in array with field name %s"), *FieldName);
		}

		BoolArray.Add((*It)->AsBool());
	}

	return BoolArray;
}

void UWorldsbaseJsonObject::SetBoolArrayField(const FString& FieldName, const TArray<bool>& BoolArray)
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

TArray<UWorldsbaseJsonObject*> UWorldsbaseJsonObject::GetObjectArrayField(const FString& FieldName) const
{
	TArray<UWorldsbaseJsonObject*> OutArray;
	if (!JsonObj->HasTypedField<EJson::Array>(FieldName) || FieldName.IsEmpty())
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: No field with name %s of type Array"), *VA_FUNC_LINE, *FieldName);
		return OutArray;
	}

	TArray<TSharedPtr<FJsonValue>> ValArray = JsonObj->GetArrayField(FieldName);
	for (const auto& Value : ValArray)
	{
		if (Value->Type != EJson::Object)
		{
			UE_LOG(LogWorldsbase, Error, TEXT("Not Object element in array with field name %s"), *FieldName);
		}

		TSharedPtr<FJsonObject> NewObj = Value->AsObject();

		UWorldsbaseJsonObject* NewJson = NewObject<UWorldsbaseJsonObject>();
		NewJson->SetRootObject(NewObj);

		OutArray.Add(NewJson);
	}

	return OutArray;
}

void UWorldsbaseJsonObject::SetObjectArrayField(const FString& FieldName, const TArray<UWorldsbaseJsonObject*>& ObjectArray)
{
	if (FieldName.IsEmpty())
	{
		return;
	}

	TArray<TSharedPtr<FJsonValue>> EntriesArray;
	for (auto Value : ObjectArray)
	{
		if (Value == nullptr)
			continue;

		EntriesArray.Add(MakeShareable(new FJsonValueObject(Value->GetRootObject())));
	}

	JsonObj->SetArrayField(FieldName, EntriesArray);
}

//////////////////////////////////////////////////////////////////////////
// Deserialize

int32 UWorldsbaseJsonObject::DeserializeFromUTF8Bytes(const ANSICHAR* Bytes, int32 Size)
{
	FJSONReader Reader;

	const ANSICHAR* EndByte = Bytes + Size;
	while (Bytes < EndByte)
	{
		TCHAR Char = FUtf8Helper::CodepointFromUtf8(Bytes, EndByte - Bytes);
		if (Char > 0xFFFF)
		{
			Char = UNICODE_BOGUS_CHAR_CODEPOINT;
		}

		if (!Reader.Read(Char))
		{
			break;
		}
	}

	SetRootObject(Reader.State.Root);
	return Reader.State.Size;
}

int32 UWorldsbaseJsonObject::DeserializeFromTCHARBytes(const TCHAR* Bytes, int32 Size)
{
	FJSONReader Reader;

	int32 i = 0;
	while (i < Size)
	{
		if (!Reader.Read(Bytes[i++]))
		{
			break;
		}
	}

	SetRootObject(Reader.State.Root);
	return Reader.State.Size;
}

void UWorldsbaseJsonObject::DecodeFromArchive(TUniquePtr<FArchive>& Reader)
{
	FArchive& Ar = (*Reader.Get());
	uint8 SymbolBytes[2];

	// Read first two bytes
	Ar << SymbolBytes[0];
	Ar << SymbolBytes[1];

	bool bIsIntelByteOrder = true;

	if (SymbolBytes[0] == 0xff && SymbolBytes[1] == 0xfe)
	{
		// Unicode Intel byte order. Less 1 for the FFFE header, additional 1 for null terminator.
		bIsIntelByteOrder = true;
	}
	else if (SymbolBytes[0] == 0xfe && SymbolBytes[1] == 0xff)
	{
		// Unicode non-Intel byte order. Less 1 for the FFFE header, additional 1 for null terminator.
		bIsIntelByteOrder = false;
	}

	FJSONReader JsonReader;
	TCHAR Char;

	while (!Ar.AtEnd())
	{
		Ar << SymbolBytes[0];

		if (Ar.AtEnd())
		{
			break;
		}

		Ar << SymbolBytes[1];

		if (bIsIntelByteOrder)
		{
			Char = CharCast<TCHAR>(static_cast<UCS2CHAR>(static_cast<uint16>(SymbolBytes[0]) + static_cast<uint16>(SymbolBytes[1]) * 256));
		}
		else
		{
			Char = CharCast<TCHAR>(static_cast<UCS2CHAR>(static_cast<uint16>(SymbolBytes[1]) + static_cast<uint16>(SymbolBytes[0]) * 256));
		}

		if (!JsonReader.Read(Char))
		{
			break;
		}
	}

	SetRootObject(JsonReader.State.Root);

	if (!Ar.Close())
	{
		UE_LOG(LogWorldsbase, Error, TEXT("UWorldsbaseJsonObject::DecodeFromArchive: Error! Can't close file!"));
	}
}

//////////////////////////////////////////////////////////////////////////
// Serialize

bool UWorldsbaseJsonObject::WriteToFile(const FString& Path) const
{
	TUniquePtr<FArchive> FileWriter(IFileManager::Get().CreateFileWriter(*Path));
	if (!FileWriter)
	{
		return false;
	}

	FArchive& Ar = *FileWriter.Get();

	UCS2CHAR BOM = UNICODE_BOM;
	Ar.Serialize(&BOM, sizeof(UCS2CHAR));

	FString Str = FString(TEXT("{"));
	WriteStringToArchive(Ar, *Str, Str.Len());

	int32 ElementCount = 0;
	FJSONWriter JsonWriter;
	for (auto JsonObjectValuePair : JsonObj->Values)
	{
		Str = FString(TEXT("\""));
		WriteStringToArchive(Ar, *Str, Str.Len());

		const TCHAR* BufferPtr = *JsonObjectValuePair.Key;
		for (int i = 0; i < JsonObjectValuePair.Key.Len(); ++i)
		{
			Str = FString(1, &BufferPtr[i]);
			WriteStringToArchive(Ar, *Str, Str.Len());
		}

		Str = FString(TEXT("\""));
		WriteStringToArchive(Ar, *Str, Str.Len());

		Str = FString(TEXT(":"));
		WriteStringToArchive(Ar, *Str, Str.Len());

		++ElementCount;

		JsonWriter.Write(JsonObjectValuePair.Value, FileWriter.Get(), ElementCount >= JsonObj->Values.Num());
	}

	Str = FString(TEXT("}"));
	WriteStringToArchive(Ar, *Str, Str.Len());

	FileWriter->Close();

	return true;
}

bool UWorldsbaseJsonObject::WriteToFilePath(const FString& Path, const bool bIsRelativeToProjectDir)
{
	return WriteToFile(bIsRelativeToProjectDir ? FPaths::ProjectDir() / Path : Path);
}

bool UWorldsbaseJsonObject::WriteStringToArchive(FArchive& Ar, const TCHAR* StrPtr, int64 Len)
{
	const auto Src = StringCast<UCS2CHAR>(StrPtr, Len);
	Ar.Serialize(const_cast<UCS2CHAR*>(Src.Get()), Src.Length() * sizeof(UCS2CHAR));

	return true;
}
