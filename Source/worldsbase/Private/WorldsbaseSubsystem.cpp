#include "WorldsbaseSubsystem.h"

#include "WorldsbaseJsonObject.h"
#include "WorldsbaseHttpTypes.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Serialization/JsonSerializer.h"


/**
 * Reads data from a specified table using Worldsbase.
 *
 * @param TableName The name of the table where the data will be read
 */
void UWorldsbaseSubsystem::GetTable(const FString& TableName)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	FString baseUrl = "https://wgs-node-production.up.railway.app/table/getTable/";
	FString fullUrl = FString::Printf(TEXT("%s%s"), *baseUrl, *TableName);
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("GET");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *fullUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteString);
	HttpRequest->ProcessRequest();
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

	FString baseUrl = "https://wgs-node-production.up.railway.app/table/insertData";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteString);
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
void UWorldsbaseSubsystem::UpdateData(const FString& TableName, const FString& ConditionColumn,
                                      const FString& ConditionValue, const TArray<FDataRow>& DataRows)
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

	FString baseUrl = "https://wgs-node-production.up.railway.app/table/updateData/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("PUT");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteString);
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
void UWorldsbaseSubsystem::IncrementData(const FString& TableName, const FString& IncrementColumnName,
                                         const FString& ConditionColumn, const FString& ConditionValue,
                                         const int32 Value)
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

	FString baseUrl = "https://wgs-node-production.up.railway.app/table/incrementData/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteString);
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
void UWorldsbaseSubsystem::DecrementData(const FString& TableName, const FString& DecrementColumnName,
                                         const FString& ConditionColumn, const FString& ConditionValue,
                                         const int32 Value)
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

	FString baseUrl = "https://wgs-node-production.up.railway.app/table/decrementData/";
	FString ApiKey = "323f7dfb-6ba3-4ba0-99cb-c493a3a712d7";

	HttpRequest->SetVerb("POST");
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetContentAsString(OutputString);
	HttpRequest->SetURL(*FString::Printf(TEXT("%s"), *baseUrl));
	HttpRequest->SetHeader("x-api-key", *FString::Printf(TEXT("%s"), *ApiKey));
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteString);
	HttpRequest->ProcessRequest();
}

void UWorldsbaseSubsystem::MakeHttpRequest(
	const FString& URL,
	const ERequestVerb Verb,
	UJsonObject* Body = NewObject<UJsonObject>(),
	const ERequestContentType ContentType = ERequestContentType::json,
	const TMap<FString, FString>& RequestHeaders = {},
	const TArray<uint8>& Bytes = TArray<uint8>{},
	const FString& StringContent = ""
)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb(GetRequestVerb(Verb));
	HttpRequest->SetHeader(TEXT("Content-Type"), GetRequestContentType(ContentType));

	FString UrlParams = "";
	switch (ContentType)
	{
	case ERequestContentType::x_www_form_urlencoded:
		{
			uint16 ParamIdx = 0;
			for (auto RequestIt = Body->GetRootObject()->Values.CreateIterator(); RequestIt; ++RequestIt)
			{
				FString Key = RequestIt.Key();
				FString Value = RequestIt.Value().Get()->AsString();
				if (!Key.IsEmpty() && !Value.IsEmpty())
				{
					UrlParams += ParamIdx == 0 ? "?" : "&";
					UrlParams += FGenericPlatformHttp::UrlEncode(Key) + "=" + FGenericPlatformHttp::UrlEncode(Value);
				}
				ParamIdx++;
			}

			if (!StringContent.IsEmpty())
			{
				HttpRequest->SetContentAsString(StringContent);
			}

			break;
		}
	case ERequestContentType::binary:
		{
			HttpRequest->SetContent(Bytes);
			break;
		}
	case ERequestContentType::json:
		{
			if (Verb == ERequestVerb::GET)
			{
				break;
			}
			FString OutputString;
			const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(Body->GetRootObject(), Writer);
			HttpRequest->SetContentAsString(OutputString);
			break;
		}
	default:
		break;
	}

	HttpRequest->SetURL(URL + UrlParams);

	for (TMap<FString, FString>::TConstIterator It(RequestHeaders); It; ++It)
	{
		HttpRequest->SetHeader(It.Key(), It.Value());
	}

	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseSubsystem::OnProcessRequestCompleteJson);
	HttpRequest->ProcessRequest();
}

UJsonObject* UWorldsbaseSubsystem::ConstructJsonObject()
{
	return NewObject<UJsonObject>(this);
}


/**
 * Function that gets called when http request processes are complete
 *
 * @param Request Pointer to Request object
 * @param Response Pointer to Response object
 * @param bWasSuccessful Indicates if request was successful
 */
void UWorldsbaseSubsystem::OnProcessRequestCompleteString(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                          bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(JsonReader, JsonObject);
		FString ContentAsString = Response->GetContentAsString();

		// Trigger the delegate with the result. This allows blueprint node to do
		// something with the result.
		OnHttpRequestCompletedString.Broadcast(ContentAsString);

		// print to debugger display
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, ContentAsString);
	}
	else
	{
		// Handle error here
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Error"));
	}
}

void UWorldsbaseSubsystem::OnProcessRequestCompleteJson(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                        bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		TSharedPtr<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
		FJsonSerializer::Deserialize(JsonReader, JsonObject);

		UJsonObject* ResponseBody = NewObject<UJsonObject>();
		ResponseBody->SetRootObject(JsonObject);

		OnHttpRequestCompletedJson.Broadcast(
			static_cast<EHttpStatusCode>(Response.Get()->GetResponseCode()),
			ResponseBody
		);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Error"));
	}
}
