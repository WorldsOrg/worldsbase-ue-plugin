// Copyright 2014-2020 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseSubsystem.h"

#include "WorldsbaseDefines.h"
#include "WorldsbaseJsonObject.h"
#include "WorldsbaseJsonValue.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

UWorldsbaseSubsystem::UWorldsbaseSubsystem()
	: UEngineSubsystem()
{
}

void UWorldsbaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogWorldsbase, Log, TEXT("%s: Worldsbase subsystem initialized"), *VA_FUNC_LINE);
}

void UWorldsbaseSubsystem::Deinitialize()
{
	// Do nothing for now
	Super::Deinitialize();
}

void UWorldsbaseSubsystem::CallURL(const FString& URL, EWorldsbaseRequestVerb Verb, EWorldsbaseRequestContentType ContentType, UWorldsbaseJsonObject* WorldsbaseJson, const FWorldsbaseCallDelegate& Callback)
{
	// Check we have valid data json
	if (WorldsbaseJson == nullptr)
	{
		WorldsbaseJson = ConstructWorldsbaseJsonObject();
	}

	UWorldsbaseRequestJSON* Request = ConstructWorldsbaseRequest();

	Request->SetVerb(Verb);
	Request->SetContentType(ContentType);
	Request->SetRequestObject(WorldsbaseJson);

	FWorldsbaseCallResponse Response;
	Response.Request = Request;
	Response.Callback = Callback;

	Response.CompleteDelegateHandle = Request->OnStaticRequestComplete.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);
	Response.FailDelegateHandle = Request->OnStaticRequestFail.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);

	RequestMap.Add(Request, Response);

	Request->ResetResponseData();
	Request->ProcessURL(URL);
}

void UWorldsbaseSubsystem::OnCallComplete(UWorldsbaseRequestJSON* Request)
{
	if (!RequestMap.Contains(Request))
	{
		return;
	}

	const auto Response = RequestMap.Find(Request);
	Request->OnStaticRequestComplete.Remove(Response->CompleteDelegateHandle);
	Request->OnStaticRequestFail.Remove(Response->FailDelegateHandle);

	Response->Callback.ExecuteIfBound(Request);
	Response->Request = nullptr;
	RequestMap.Remove(Request);
}

UWorldsbaseRequestJSON* UWorldsbaseSubsystem::ConstructWorldsbaseRequest()
{
	return NewObject<UWorldsbaseRequestJSON>(this);
}

UWorldsbaseRequestJSON* UWorldsbaseSubsystem::ConstructWorldsbaseRequestExt(EWorldsbaseRequestVerb Verb, EWorldsbaseRequestContentType ContentType)
{
	UWorldsbaseRequestJSON* Request = ConstructWorldsbaseRequest();

	Request->SetVerb(Verb);
	Request->SetContentType(ContentType);

	return Request;
}

UWorldsbaseJsonObject* UWorldsbaseSubsystem::ConstructWorldsbaseJsonObject()
{
	return NewObject<UWorldsbaseJsonObject>(this);
}

UWorldsbaseJsonObject* UWorldsbaseSubsystem::StaticConstructWorldsbaseJsonObject()
{
	auto SelfSystem = CastChecked<UWorldsbaseSubsystem>(USubsystemBlueprintLibrary::GetEngineSubsystem(UWorldsbaseSubsystem::StaticClass()), ECastCheckedType::NullChecked);
	return SelfSystem->ConstructWorldsbaseJsonObject();
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValueNumber(float Number)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueNumber(Number));

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValueString(const FString& StringValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueString(StringValue));

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValueBool(bool InValue)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueBoolean(InValue));

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValueArray(const TArray<UWorldsbaseJsonValue*>& InArray)
{
	// Prepare data array to create new value
	TArray<TSharedPtr<FJsonValue>> ValueArray;
	for (auto InVal : InArray)
	{
		ValueArray.Add(InVal->GetRootValue());
	}

	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueArray(ValueArray));

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValueObject(UWorldsbaseJsonObject* JsonObject)
{
	TSharedPtr<FJsonValue> NewVal = MakeShareable(new FJsonValueObject(JsonObject->GetRootObject()));

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::ConstructJsonValue(const TSharedPtr<FJsonValue>& InValue)
{
	TSharedPtr<FJsonValue> NewVal = InValue;

	UWorldsbaseJsonValue* NewValue = NewObject<UWorldsbaseJsonValue>(this);
	NewValue->SetRootValue(NewVal);

	return NewValue;
}

UWorldsbaseJsonValue* UWorldsbaseSubsystem::DecodeJsonValue(const FString& JsonString)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*JsonString);
	TSharedPtr<FJsonValue> OutJsonValue;
	if (FJsonSerializer::Deserialize(Reader, OutJsonValue))
	{
		return ConstructJsonValue(OutJsonValue);
	}

	return nullptr;
}

UWorldsbaseJsonObject* UWorldsbaseSubsystem::DecodeJsonObject(const FString& JsonString)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*JsonString);
	TSharedPtr<FJsonObject> OutJsonObj;
	if (FJsonSerializer::Deserialize(Reader, OutJsonObj))
	{
		auto NewJsonObj = NewObject<UWorldsbaseJsonObject>(this);
		NewJsonObj->SetRootObject(OutJsonObj);
		return NewJsonObj;
	}

	return nullptr;
}

class UWorldsbaseJsonObject* UWorldsbaseSubsystem::LoadJsonFromFile(const FString& Path, const bool bIsRelativeToContentDir)
{
	auto* Json = ConstructWorldsbaseJsonObject();

	FString JSONString;
	if (FFileHelper::LoadFileToString(JSONString, *(bIsRelativeToContentDir ? FPaths::ProjectContentDir() / Path : Path)))
	{
		if (Json->DecodeJson(JSONString))
		{
			return Json;
		}
		else
		{
			UE_LOG(LogWorldsbase, Error, TEXT("%s: Can't decode json from file %s"), *VA_FUNC_LINE, *Path);
		}
	}
	else
	{
		UE_LOG(LogWorldsbase, Error, TEXT("%s: Can't open file %s"), *VA_FUNC_LINE, *Path);
	}

	return nullptr;
}

/**
 * Reads data from a specified table using Worldsbase.
 *
 * @param TableName The name of the table where the data will be read
 */
void UWorldsbaseSubsystem::GetTable(const FString& TableName, const FWorldsbaseCallDelegate& Callback)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	FString baseUrl = "https://chain-production.up.railway.app/table/gettable/";
	FString fullUrl = FString::Printf(TEXT("%s%s"), *baseUrl, *TableName);
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	// Check we have valid data json
	UWorldsbaseJsonObject* WorldsbaseJson = ConstructWorldsbaseJsonObject();

	UWorldsbaseRequestJSON* Request = ConstructWorldsbaseRequest();

	Request->SetVerb(EWorldsbaseRequestVerb::GET);
	Request->SetContentType(EWorldsbaseRequestContentType::json);
	Request->SetHeader("x-api-key", *ApiKey);
	Request->SetRequestObject(WorldsbaseJson);

	FWorldsbaseCallResponse Response;
	Response.Request = Request;
	Response.Callback = Callback;

	Response.CompleteDelegateHandle = Request->OnStaticRequestComplete.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);
	Response.FailDelegateHandle = Request->OnStaticRequestFail.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);

	RequestMap.Add(Request, Response);

	Request->ProcessURL(*fullUrl);
}

/**
 * Reads data from a specified table using Worldsbase, filtering by a specified column.
 *
 * @param TableName The name of the table where the data will be read
 * @param ColumnName The name of the column to filter by
 * @param ColumnValue The value to filter by
 */
void UWorldsbaseSubsystem::GetValue(const FString& TableName, const FString& ColumnName, const FString& ColumnValue, const FWorldsbaseCallDelegate& Callback)
{
	FString baseUrl = "https://chain-production.up.railway.app/table/gettablevalue/";
	FString fullUrl = FString::Printf(TEXT("%s%s/%s/%s"), *baseUrl, *TableName, *ColumnName, *ColumnValue);
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	// Check we have valid data json
	UWorldsbaseJsonObject* WorldsbaseJson = ConstructWorldsbaseJsonObject();

	UWorldsbaseRequestJSON* Request = ConstructWorldsbaseRequest();

	Request->SetVerb(EWorldsbaseRequestVerb::GET);
	Request->SetContentType(EWorldsbaseRequestContentType::json);
	Request->SetHeader("x-api-key", *ApiKey);
	Request->SetRequestObject(WorldsbaseJson);

	FWorldsbaseCallResponse Response;
	Response.Request = Request;
	Response.Callback = Callback;

	Response.CompleteDelegateHandle = Request->OnStaticRequestComplete.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);
	Response.FailDelegateHandle = Request->OnStaticRequestFail.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);

	RequestMap.Add(Request, Response);

	Request->ProcessURL(*fullUrl);
}

/**
 * Reads data from a specified table in Worldsbase, filtering by multiple column values.
 *
 * @param TableName The name of the table where the data will be read
 * @param Filters A map of column names to their respective values for filtering
 */
void UWorldsbaseSubsystem::GetFilteredTableValues(const FString& TableName, const TMap<FString, FString>& Filters, const FWorldsbaseCallDelegate& Callback)
{
	FString baseUrl = "https://chain-production.up.railway.app/table/gettablevalues/";
	FString filterParams = TEXT("?filters=");
	bool isFirst = true;

	for (const TPair<FString, FString>& Filter : Filters)
	{
		if (!isFirst)
		{
			filterParams += ",";
		}
		filterParams += FString::Printf(TEXT("%s=%s"), *Filter.Key, *Filter.Value);
		isFirst = false;
	}

	// Check we have valid data json
	UWorldsbaseJsonObject* WorldsbaseJson = ConstructWorldsbaseJsonObject();

	FString fullUrl = baseUrl + TableName + filterParams;
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	UWorldsbaseRequestJSON* Request = ConstructWorldsbaseRequest();

	Request->SetVerb(EWorldsbaseRequestVerb::GET);
	Request->SetContentType(EWorldsbaseRequestContentType::json);
	Request->SetHeader("x-api-key", *ApiKey);
	Request->SetRequestObject(WorldsbaseJson);

	FWorldsbaseCallResponse Response;
	Response.Request = Request;
	Response.Callback = Callback;

	Response.CompleteDelegateHandle = Request->OnStaticRequestComplete.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);
	Response.FailDelegateHandle = Request->OnStaticRequestFail.AddUObject(this, &UWorldsbaseSubsystem::OnCallComplete);

	RequestMap.Add(Request, Response);

	Request->ProcessURL(*fullUrl);
}

/**
 * Inserts data into a specified table using Worldsbase. This function constructs
 * a JSON object from the provided data rows, and then sends a POST request
 * to a predefined URL to insert the data into the specified table.
 *
 * @param TableName The name of the table where the data will be inserted.
 * @param DataRows An array of FDataRow structures, each containing the column name and the value to be inserted for that column.
 */
void UWorldsbaseSubsystem::InsertData(const FString& TableName, const TArray<FDataRow>& DataRows)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	// construct json object from data array
	TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	for (const FDataRow& DataRow : DataRows)
	{
		DataJsonObject->SetStringField(DataRow.ColumnName, DataRow.Value);
	}

	// create json object containing data json. This is the final form for Worldsbase
	// insertData endpoint
	TSharedPtr<FJsonObject> FinalJsonObject = MakeShareable(new FJsonObject);
	FinalJsonObject->SetObjectField(TEXT("data"), DataJsonObject);
	FinalJsonObject->SetStringField(TEXT("tableName"), TableName);

	// convert json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FinalJsonObject.ToSharedRef(), Writer);

	FString baseUrl = "https://chain-production.up.railway.app/table/insertdata";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestComplete);
	HttpRequest->ProcessRequest();
}

/**
 * Updates data in a specified column matching a condition in a specified table.
 *
 * @param TableName The name of the table where the data will be incremented
 * @param ConditionColumn The name of the column used to identify the row to be updated
 * @param ConditionValue Value for column that is used to identify row to be updated
 * @param DataRows An array of FDataRow structures, each containing the column name and the value to be updated for that column.
 */
void UWorldsbaseSubsystem::UpdateData(const FString& TableName, const FString& ConditionColumn, const FString& ConditionValue, const TArray<FDataRow>& DataRows)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	// construct sql condition to select upon
	FString Condition = FString::Printf(TEXT("%s='%s'"), *ConditionColumn, *ConditionValue);

	// construct json object from data array
	TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	for (const FDataRow& DataRow : DataRows)
	{
		DataJsonObject->SetStringField(DataRow.ColumnName, DataRow.Value);
	}

	// create json object for post request
	TSharedPtr<FJsonObject> FinalJsonObject = MakeShareable(new FJsonObject);
	FinalJsonObject->SetStringField(TEXT("tableName"), TableName);
	FinalJsonObject->SetStringField(TEXT("condition"), Condition);
	FinalJsonObject->SetObjectField(TEXT("data"), DataJsonObject);

	// convert json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FinalJsonObject.ToSharedRef(), Writer);

	FString baseUrl = "https://chain-production.up.railway.app/table/updatedata/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("PUT");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestComplete);
	HttpRequest->ProcessRequest();
}

/**
 * Updates data in a specified column matching a condition in a specified table.
 *
 * @param TableName The name of the table where the data will be incremented
 * @param ConditionColumn The name of the column used to identify the row to be updated
 * @param ConditionValue Value for column that is used to identify row to be updated
 * @param DataRows An array of FDataRow structures, each containing the column name and the value to be updated for that column.
 */
void UWorldsbaseSubsystem::UpdateFilteredData(const FString& TableName, const TArray<FDataRow>& Filters, const TArray<FDataRow>& DataRows)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	// construct sql condition to select upon
	//FString Condition = FString::Printf(TEXT("%s='%s'"), *ConditionColumn, *ConditionValue);
	//FString Conditions = "{' player ' : ' player_2 ', ' name ':' aar ' }";

	// construct json object from data array
	TSharedPtr<FJsonObject> FiltersJsonObject = MakeShareable(new FJsonObject);
	for (const FDataRow& DataRow : Filters)
	{
		FiltersJsonObject->SetStringField(DataRow.ColumnName, DataRow.Value);
	}

	// construct json object from data array
	TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	for (const FDataRow& DataRow : DataRows)
	{
		DataJsonObject->SetStringField(DataRow.ColumnName, DataRow.Value);
	}

	// create json object for post request
	TSharedPtr<FJsonObject> FinalJsonObject = MakeShareable(new FJsonObject);
	FinalJsonObject->SetStringField(TEXT("tableName"), TableName);
	FinalJsonObject->SetObjectField(TEXT("filters"), FiltersJsonObject);
	FinalJsonObject->SetObjectField(TEXT("data"), DataJsonObject);

	// convert json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FinalJsonObject.ToSharedRef(), Writer);

	FString baseUrl = "https://chain-production.up.railway.app/table/updatefiltereddata";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	FString OutputStringTest = TEXT("{\"tableName\":\"wtf_player_inventory\",\"data\":{\"upgrades\":\"{ 'skin' : 'skin aaaa11', 'handguard' : 'handguard 444111' }\"},\"filters\":{\"player\":\"player_2\",\"name\":\"aar\"}}");

	HttpRequest->SetVerb("PUT");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestComplete);
	HttpRequest->ProcessRequest();
}

/**
 * Increments data to a specified column matching a condition in a specified table.
 *
 * @param TableName The name of the table where the data will be incremented
 * @param IncrementColumnName The name of the column where the data will be incremented
 * @param ConditionColumn The name of the column used to identify the row to be incremented
 * @param ConditionValue Value for column that is used to identify row to be incremented
 * @param Value Value to increment by
 */
void UWorldsbaseSubsystem::IncrementData(const FString& TableName, const FString& IncrementColumnName, const FString& ConditionColumn, const FString& ConditionValue, const int32 Value)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	// construct sql condition to select upon
	FString Condition = FString::Printf(TEXT("%s='%s'"), *ConditionColumn, *ConditionValue);

	// create json object for post request
	TSharedPtr<FJsonObject> FinalJsonObject = MakeShareable(new FJsonObject);
	FinalJsonObject->SetStringField(TEXT("tableName"), TableName);
	FinalJsonObject->SetStringField(TEXT("columnName"), IncrementColumnName);
	FinalJsonObject->SetStringField(TEXT("condition"), Condition);
	FinalJsonObject->SetStringField(TEXT("value"), FString::FromInt(Value));

	// convert json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FinalJsonObject.ToSharedRef(), Writer);

	FString baseUrl = "https://chain-production.up.railway.app/table/incrementdata/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestComplete);
	HttpRequest->ProcessRequest();
}

/**
 * Decrements data to a specified column matching a condition in a specified table.
 *
 * @param TableName The name of the table where the data will be decremented
 * @param DecrementColumnName The name of the column where the data will be decremented
 * @param ConditionColumn The name of the column used to identify the row to be decremented
 * @param ConditionValue Value for column that is used to identify row to be decremented
 * @param Value Value to decrement by
 */
void UWorldsbaseSubsystem::DecrementData(const FString& TableName, const FString& DecrementColumnName, const FString& ConditionColumn, const FString& ConditionValue, const int32 Value)
{

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	// construct sql condition to select upon
	FString Condition = FString::Printf(TEXT("%s='%s'"), *ConditionColumn, *ConditionValue);

	// create json object for post request
	TSharedPtr<FJsonObject> FinalJsonObject = MakeShareable(new FJsonObject);
	FinalJsonObject->SetStringField(TEXT("tableName"), TableName);
	FinalJsonObject->SetStringField(TEXT("columnName"), DecrementColumnName);
	FinalJsonObject->SetStringField(TEXT("condition"), Condition);
	FinalJsonObject->SetStringField(TEXT("value"), FString::FromInt(Value));

	// convert json to string
	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(FinalJsonObject.ToSharedRef(), Writer);

	FString baseUrl = "https://chain-production.up.railway.app/table/decrementdata/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestComplete);
	HttpRequest->ProcessRequest();
}

/**
 * Function that gets called when http request processes are complete
 *
 * @param Request Pointer to Request object
 * @param Response Pointer to Response object
 * @param bWasSuccessful Indicates if request was successful
 */
void UWorldsbaseSubsystem::OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(JsonReader, JsonObject);
		FString ContentAsString = Response->GetContentAsString();

		// Trigger the delegate with the result. This allows blueprint node to do
		// something with the result.
		OnHttpRequestCompleted.Broadcast(ContentAsString);

		// print to debugger display
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, ContentAsString);
	}
	else
	{
		// Handle error here
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Error"));
	}
}