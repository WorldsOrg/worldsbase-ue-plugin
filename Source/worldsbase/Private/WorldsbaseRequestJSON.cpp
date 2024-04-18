// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#include "WorldsbaseRequestJSON.h"

#include "WorldsbaseDefines.h"
#include "WorldsbaseJsonObject.h"
#include "WorldsbaseJsonValue.h"
#include "WorldsbaseLibrary.h"
#include "WorldsbaseSettings.h"

#include "Engine/Engine.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

FString UWorldsbaseRequestJSON::DeprecatedResponseString(TEXT("DEPRECATED: Please use GetResponseContentAsString() instead"));

template <class T>
void FWorldsbaseLatentAction<T>::Cancel()
{
	UObject* Obj = Request.Get();
	if (Obj != nullptr)
	{
		((UWorldsbaseRequestJSON*)Obj)->Cancel();
	}
}

UWorldsbaseRequestJSON::UWorldsbaseRequestJSON(const class FObjectInitializer& PCIP)
	: Super(PCIP)
	, BinaryContentType(TEXT("application/octet-stream"))
{
	ContinueAction = nullptr;

	RequestVerb = EWorldsbaseRequestVerb::GET;
	RequestContentType = EWorldsbaseRequestContentType::x_www_form_urlencoded_url;

	ResetData();
}

void UWorldsbaseRequestJSON::SetVerb(EWorldsbaseRequestVerb Verb)
{
	RequestVerb = Verb;
}

void UWorldsbaseRequestJSON::SetCustomVerb(FString Verb)
{
	CustomVerb = Verb;
}

void UWorldsbaseRequestJSON::SetContentType(EWorldsbaseRequestContentType ContentType)
{
	RequestContentType = ContentType;
}

void UWorldsbaseRequestJSON::SetBinaryContentType(const FString& ContentType)
{
	BinaryContentType = ContentType;
}

void UWorldsbaseRequestJSON::SetBinaryRequestContent(const TArray<uint8>& Bytes)
{
	RequestBytes = Bytes;
}

void UWorldsbaseRequestJSON::SetStringRequestContent(const FString& Content)
{
	StringRequestContent = Content;
}

void UWorldsbaseRequestJSON::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
	RequestHeaders.Add(HeaderName, HeaderValue);
}

//////////////////////////////////////////////////////////////////////////
// Destruction and reset

void UWorldsbaseRequestJSON::ResetData()
{
	ResetRequestData();
	ResetResponseData();
}

void UWorldsbaseRequestJSON::ResetRequestData()
{
	if (RequestJsonObj != nullptr)
	{
		RequestJsonObj->Reset();
	}
	else
	{
		RequestJsonObj = NewObject<UWorldsbaseJsonObject>();
	}

	// See issue #90
	// HttpRequest = FHttpModule::Get().CreateRequest();

	RequestBytes.Empty();
	StringRequestContent.Empty();
}

void UWorldsbaseRequestJSON::ResetResponseData()
{
	if (ResponseJsonObj != nullptr)
	{
		ResponseJsonObj->Reset();
	}
	else
	{
		ResponseJsonObj = NewObject<UWorldsbaseJsonObject>();
	}

	if (ResponseJsonValue != nullptr)
	{
		ResponseJsonValue->Reset();
	}
	else
	{
		ResponseJsonValue = NewObject<UWorldsbaseJsonValue>();
	}

	ResponseHeaders.Empty();
	ResponseCode = -1;
	ResponseSize = 0;

	bIsValidJsonResponse = false;

	// #127 Reset string to deprecated state
	ResponseContent = DeprecatedResponseString;

	ResponseBytes.Empty();
	ResponseContentLength = 0;
}

void UWorldsbaseRequestJSON::Cancel()
{
	ContinueAction = nullptr;

	ResetResponseData();
}

//////////////////////////////////////////////////////////////////////////
// JSON data accessors

UWorldsbaseJsonObject* UWorldsbaseRequestJSON::GetRequestObject() const
{
	check(RequestJsonObj);
	return RequestJsonObj;
}

void UWorldsbaseRequestJSON::SetRequestObject(UWorldsbaseJsonObject* JsonObject)
{
	if (JsonObject == nullptr)
	{
		UE_LOG(LogWorldsbase, Error, TEXT("%s: Provided JsonObject is nullptr"), *VA_FUNC_LINE);
		return;
	}

	RequestJsonObj = JsonObject;
}

UWorldsbaseJsonObject* UWorldsbaseRequestJSON::GetResponseObject() const
{
	check(ResponseJsonObj);
	return ResponseJsonObj;
}

void UWorldsbaseRequestJSON::SetResponseObject(UWorldsbaseJsonObject* JsonObject)
{
	if (JsonObject == nullptr)
	{
		UE_LOG(LogWorldsbase, Error, TEXT("%s: Provided JsonObject is nullptr"), *VA_FUNC_LINE);
		return;
	}

	ResponseJsonObj = JsonObject;
}

UWorldsbaseJsonValue* UWorldsbaseRequestJSON::GetResponseValue() const
{
	check(ResponseJsonValue);
	return ResponseJsonValue;
}

///////////////////////////////////////////////////////////////////////////
// Response data access

FString UWorldsbaseRequestJSON::GetURL() const
{
	return HttpRequest->GetURL();
}

EWorldsbaseRequestVerb UWorldsbaseRequestJSON::GetVerb() const
{
	return RequestVerb;
}

EWorldsbaseRequestStatus UWorldsbaseRequestJSON::GetStatus() const
{
	return EWorldsbaseRequestStatus((uint8)HttpRequest->GetStatus());
}

int32 UWorldsbaseRequestJSON::GetResponseCode() const
{
	return ResponseCode;
}

FString UWorldsbaseRequestJSON::GetResponseHeader(const FString& HeaderName)
{
	FString Result;

	FString* Header = ResponseHeaders.Find(HeaderName);
	if (Header != nullptr)
	{
		Result = *Header;
	}

	return Result;
}

TArray<FString> UWorldsbaseRequestJSON::GetAllResponseHeaders() const
{
	TArray<FString> Result;
	for (TMap<FString, FString>::TConstIterator It(ResponseHeaders); It; ++It)
	{
		Result.Add(It.Key() + TEXT(": ") + It.Value());
	}
	return Result;
}

int32 UWorldsbaseRequestJSON::GetResponseContentLength() const
{
	return ResponseContentLength;
}

const TArray<uint8>& UWorldsbaseRequestJSON::GetResponseContent() const
{
	return ResponseBytes;
}

//////////////////////////////////////////////////////////////////////////
// URL processing

void UWorldsbaseRequestJSON::SetURL(const FString& Url)
{
	// Be sure to trim URL because it can break links on iOS
	FString TrimmedUrl = Url;

	TrimmedUrl.TrimStartInline();
	TrimmedUrl.TrimEndInline();

	HttpRequest->SetURL(TrimmedUrl);
}

void UWorldsbaseRequestJSON::ProcessURL(const FString& Url)
{
	SetURL(Url);
	ProcessRequest();
}

void UWorldsbaseRequestJSON::ApplyURL(const FString& Url, UWorldsbaseJsonObject*& Result, UObject* WorldContextObject, FLatentActionInfo LatentInfo)
{
	// Be sure to trim URL because it can break links on iOS
	FString TrimmedUrl = Url;

	TrimmedUrl.TrimStartInline();
	TrimmedUrl.TrimEndInline();

	HttpRequest->SetURL(TrimmedUrl);

	// Prepare latent action
	if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FWorldsbaseLatentAction<UWorldsbaseJsonObject*>* Kont = LatentActionManager.FindExistingAction<FWorldsbaseLatentAction<UWorldsbaseJsonObject*>>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Kont != nullptr)
		{
			Kont->Cancel();
			LatentActionManager.RemoveActionsForObject(LatentInfo.CallbackTarget);
		}

		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ContinueAction = new FWorldsbaseLatentAction<UWorldsbaseJsonObject*>(this, Result, LatentInfo));
	}

	ProcessRequest();
}

void UWorldsbaseRequestJSON::ExecuteProcessRequest()
{
	if (HttpRequest->GetURL().Len() == 0)
	{
		UE_LOG(LogWorldsbase, Error, TEXT("Request execution attempt with empty URL"));
		return;
	}

	ProcessRequest();
}

void UWorldsbaseRequestJSON::ProcessRequest()
{
	// Set verb
	switch (RequestVerb)
	{
	case EWorldsbaseRequestVerb::GET:
		HttpRequest->SetVerb(TEXT("GET"));
		break;

	case EWorldsbaseRequestVerb::POST:
		HttpRequest->SetVerb(TEXT("POST"));
		break;

	case EWorldsbaseRequestVerb::PUT:
		HttpRequest->SetVerb(TEXT("PUT"));
		break;

	case EWorldsbaseRequestVerb::DEL:
		HttpRequest->SetVerb(TEXT("DELETE"));
		break;

	case EWorldsbaseRequestVerb::CUSTOM:
		HttpRequest->SetVerb(CustomVerb);
		break;

	default:
		break;
	}

	// Set content-type
	switch (RequestContentType)
	{
	case EWorldsbaseRequestContentType::x_www_form_urlencoded_url:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

		FString UrlParams = "";
		uint16 ParamIdx = 0;

		// Loop through all the values and prepare additional url part
		for (auto RequestIt = RequestJsonObj->GetRootObject()->Values.CreateIterator(); RequestIt; ++RequestIt)
		{
			FString Key = RequestIt.Key();
			FString Value = RequestIt.Value().Get()->AsString();

			if (!Key.IsEmpty() && !Value.IsEmpty())
			{
				UrlParams += ParamIdx == 0 ? "?" : "&";
				UrlParams += UWorldsbaseLibrary::PercentEncode(Key) + "=" + UWorldsbaseLibrary::PercentEncode(Value);
			}

			ParamIdx++;
		}

		// Apply params
		HttpRequest->SetURL(HttpRequest->GetURL() + UrlParams);

		// Add optional string content
		if (!StringRequestContent.IsEmpty())
		{
			HttpRequest->SetContentAsString(StringRequestContent);
		}

		// Check extended log to avoid security vulnerability (#133)
		if (UWorldsbaseLibrary::GetWorldsbaseSettings()->bExtendedLog)
		{
			UE_LOG(LogWorldsbase, Log, TEXT("%s: Request (urlencoded): %s %s %s %s"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL(), *UrlParams, *StringRequestContent);
		}
		else
		{
			UE_LOG(LogWorldsbase, Log, TEXT("%s: Request (urlencoded): %s %s (check bExtendedLog for additional data)"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}
	case EWorldsbaseRequestContentType::x_www_form_urlencoded_body:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));

		FString UrlParams = "";
		uint16 ParamIdx = 0;

		// Add optional string content
		if (!StringRequestContent.IsEmpty())
		{
			UrlParams = StringRequestContent;
		}
		else
		{
			// Loop through all the values and prepare additional url part
			for (auto RequestIt = RequestJsonObj->GetRootObject()->Values.CreateIterator(); RequestIt; ++RequestIt)
			{
				FString Key = RequestIt.Key();
				FString Value = RequestIt.Value().Get()->AsString();

				if (!Key.IsEmpty() && !Value.IsEmpty())
				{
					UrlParams += ParamIdx == 0 ? "" : "&";
					UrlParams += UWorldsbaseLibrary::PercentEncode(Key) + "=" + UWorldsbaseLibrary::PercentEncode(Value);
				}

				ParamIdx++;
			}
		}

		// Apply params
		HttpRequest->SetContentAsString(UrlParams);

		// Check extended log to avoid security vulnerability (#133)
		if (UWorldsbaseLibrary::GetWorldsbaseSettings()->bExtendedLog)
		{
			UE_LOG(LogWorldsbase, Log, TEXT("%s: Request (url body): %s %s %s"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL(), *UrlParams);
		}
		else
		{
			UE_LOG(LogWorldsbase, Log, TEXT("%s: Request (url body): %s %s (check bExtendedLog for additional data)"), *VA_FUNC_LINE, *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}
	case EWorldsbaseRequestContentType::binary:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), BinaryContentType);
		HttpRequest->SetContent(RequestBytes);

		UE_LOG(LogWorldsbase, Log, TEXT("Request (binary): %s %s"), *HttpRequest->GetVerb(), *HttpRequest->GetURL());

		break;
	}
	case EWorldsbaseRequestContentType::json:
	{
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		// Body is ignored for get requests, so we shouldn't place json into it even if it's empty
		if (RequestVerb == EWorldsbaseRequestVerb::GET)
		{
			break;
		}

		// Serialize data to json string
		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RequestJsonObj->GetRootObject(), Writer);

		// Set Json content
		HttpRequest->SetContentAsString(OutputString);

		if (UWorldsbaseLibrary::GetWorldsbaseSettings()->bExtendedLog)
		{
			UE_LOG(LogWorldsbase, Log, TEXT("Request (json): %s %s %sJSON(%s%s%s)JSON"), *HttpRequest->GetVerb(), *HttpRequest->GetURL(), LINE_TERMINATOR, LINE_TERMINATOR, *OutputString, LINE_TERMINATOR);
		}
		else
		{
			UE_LOG(LogWorldsbase, Log, TEXT("Request (json): %s %s (check bExtendedLog for additional data)"), *HttpRequest->GetVerb(), *HttpRequest->GetURL());
		}

		break;
	}

	default:
		break;
	}

	// Apply additional headers
	for (TMap<FString, FString>::TConstIterator It(RequestHeaders); It; ++It)
	{
		HttpRequest->SetHeader(It.Key(), It.Value());
	}

	// Bind event
	HttpRequest->OnProcessRequestComplete().BindUObject(this, &UWorldsbaseRequestJSON::OnProcessRequestComplete);

	// Execute the request
	HttpRequest->ProcessRequest();
}

//////////////////////////////////////////////////////////////////////////
// Request callbacks

void UWorldsbaseRequestJSON::OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Be sure that we have no data from previous response
	ResetResponseData();

	// Check we have a response and save response code as int32
	if (Response.IsValid())
	{
		ResponseCode = Response->GetResponseCode();
	}

	// Check we have result to process futher
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogWorldsbase, Error, TEXT("Request failed (%d): %s"), ResponseCode, *Request->GetURL());

		// Broadcast the result event
		OnRequestFail.Broadcast(this);
		OnStaticRequestFail.Broadcast(this);

		return;
	}

#if PLATFORM_DESKTOP
	// Log response state
	UE_LOG(LogWorldsbase, Log, TEXT("Response (%d): %sJSON(%s%s%s)JSON"), ResponseCode, LINE_TERMINATOR, LINE_TERMINATOR, *Response->GetContentAsString(), LINE_TERMINATOR);
#endif

	// Process response headers
	TArray<FString> Headers = Response->GetAllHeaders();
	for (FString Header : Headers)
	{
		FString Key;
		FString Value;
		if (Header.Split(TEXT(": "), &Key, &Value))
		{
			ResponseHeaders.Add(Key, Value);
		}
	}

	if (UWorldsbaseLibrary::GetWorldsbaseSettings()->bUseChunkedParser)
	{
		// Try to deserialize data to JSON
		const TArray<uint8>& Bytes = Response->GetContent();
		ResponseSize = ResponseJsonObj->DeserializeFromUTF8Bytes((const ANSICHAR*)Bytes.GetData(), Bytes.Num());

		// Log errors
		if (ResponseSize == 0)
		{
			// As we assume it's recommended way to use current class, but not the only one,
			// it will be the warning instead of error
			UE_LOG(LogWorldsbase, Warning, TEXT("JSON could not be decoded!"));
		}
	}
	else
	{
		FString Content = *Response->GetContentAsString();
		FString ResultString;

		for (const TCHAR& Char : Content)
		{
			// Skip '[' and ']' characters
			if (Char != '[' && Char != ']')
			{
				ResultString += Char;
			}
		}
		
		// Use default unreal one
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResultString);
		TSharedPtr<FJsonValue> OutJsonValue;
		if (FJsonSerializer::Deserialize(Reader, OutJsonValue))
		{
			ResponseJsonValue->SetRootValue(OutJsonValue);

			EWBJson Type = ResponseJsonValue->GetType();

			if (ResponseJsonValue->GetType() == EWBJson::Object)
			{
				ResponseJsonObj->SetRootObject(ResponseJsonValue->GetRootValue()->AsObject());
				ResponseSize = Response->GetContentLength();
			}
		}
	}

	// Decide whether the request was successful
	bIsValidJsonResponse = bWasSuccessful && (ResponseSize > 0);

	if (!bIsValidJsonResponse)
	{
		// Save response data as a string
		ResponseContent = Response->GetContentAsString();
		ResponseSize = ResponseContent.GetAllocatedSize();

		ResponseBytes = Response->GetContent();
		ResponseContentLength = Response->GetContentLength();
	}

	// Broadcast the result events on next tick
	OnRequestComplete.Broadcast(this);
	OnStaticRequestComplete.Broadcast(this);

	// Finish the latent action
	if (ContinueAction)
	{
		FWorldsbaseLatentAction<UWorldsbaseJsonObject*>* K = ContinueAction;
		ContinueAction = nullptr;

		K->Call(ResponseJsonObj);
	}
}

//////////////////////////////////////////////////////////////////////////
// Tags

void UWorldsbaseRequestJSON::AddTag(FName Tag)
{
	if (Tag != NAME_None)
	{
		Tags.AddUnique(Tag);
	}
}

int32 UWorldsbaseRequestJSON::RemoveTag(FName Tag)
{
	return Tags.Remove(Tag);
}

bool UWorldsbaseRequestJSON::HasTag(FName Tag) const
{
	return (Tag != NAME_None) && Tags.Contains(Tag);
}

//////////////////////////////////////////////////////////////////////////
// Data

FString UWorldsbaseRequestJSON::GetResponseContentAsString(bool bCacheResponseContent)
{
	// Check we have valid json response
	if (!bIsValidJsonResponse)
	{
		// We've cached response content in OnProcessRequestComplete()
		return ResponseContent;
	}

	// Check we have valid response object
	if (!ResponseJsonObj || !ResponseJsonObj->IsValidLowLevel())
	{
		// Discard previous cached string if we had one
		ResponseContent = DeprecatedResponseString;

		return TEXT("Invalid response");
	}

	// Check if we should re-genetate it in runtime
	if (!bCacheResponseContent)
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: Use of uncashed getter could be slow"), *VA_FUNC_LINE);
		return ResponseJsonObj->EncodeJson();
	}

	// Check that we haven't cached content yet
	if (ResponseContent == DeprecatedResponseString)
	{
		UE_LOG(LogWorldsbase, Warning, TEXT("%s: Response content string is cached"), *VA_FUNC_LINE);
		ResponseContent = ResponseJsonObj->EncodeJson();
	}

	// Return previously cached content now
	return ResponseContent;
}
