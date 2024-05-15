// Copyright 2014-2019 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Engine/LatentActionManager.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "LatentActions.h"

#include "WorldsbaseTypes.h"

#include "WorldsbaseRequestJSON.generated.h"

class UWorldsbaseJsonValue;
class UWorldsbaseJsonObject;
class UWorldsbaseSettings;

/**
 * @author Original latent action class by https://github.com/unktomi
 */
template <class T>
class FWorldsbaseLatentAction : public FPendingLatentAction
{
public:
	virtual void Call(const T& Value)
	{
		Result = Value;
		Called = true;
	}

	void operator()(const T& Value)
	{
		Call(Value);
	}

	void Cancel();

	FWorldsbaseLatentAction(FWeakObjectPtr RequestObj, T& ResultParam, const FLatentActionInfo& LatentInfo)
		: Called(false)
		  , Request(RequestObj)
		  , ExecutionFunction(LatentInfo.ExecutionFunction)
		  , OutputLink(LatentInfo.Linkage)
		  , CallbackTarget(LatentInfo.CallbackTarget)
		  , Result(ResultParam)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Response.FinishAndTriggerIf(Called, ExecutionFunction, OutputLink, CallbackTarget);
	}

	virtual void NotifyObjectDestroyed() override
	{
		Cancel();
	}

	virtual void NotifyActionAborted() override
	{
		Cancel();
	}

private:
	bool Called;
	FWeakObjectPtr Request;

public:
	const FName ExecutionFunction;
	const int32 OutputLink;
	const FWeakObjectPtr CallbackTarget;
	T& Result;
};

/** Generate a delegates for callback events */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestComplete, class UWorldsbaseRequestJSON*, Request);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestFail, class UWorldsbaseRequestJSON*, Request);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaticRequestComplete, class UWorldsbaseRequestJSON*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStaticRequestFail, class UWorldsbaseRequestJSON*);

/**
 * General helper class http requests via blueprints
 */
UCLASS(BlueprintType, Blueprintable)
class WORLDSBASE_API UWorldsbaseRequestJSON : public UObject
{
	GENERATED_UCLASS_BODY()
	//////////////////////////////////////////////////////////////////////////
	// Construction

	/** Set verb to the request */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetVerb(EWorldsbaseRequestVerb Verb);

	/** Set custom verb to the request */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetCustomVerb(FString Verb);

	/** Set content type to the request. If you're using the x-www-form-urlencoded,
	 * params/constaints should be defined as key=ValueString pairs from Json data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetContentType(EWorldsbaseRequestContentType ContentType);

	/** Set content type of the request for binary post data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetBinaryContentType(const FString& ContentType);

	/** Set content of the request for binary post data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetBinaryRequestContent(const TArray<uint8>& Content);

	/** Set content of the request as a plain string */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetStringRequestContent(const FString& Content);

	/** Sets optional header info */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetHeader(const FString& HeaderName, const FString& HeaderValue);

	/** Sets API Key header info */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetApiKey(const FString& ApiKey);

	/** Sets authorization header info */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetAuthorizationToken(const FString& AuthToken);

	/** Sets basic authorization header info */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetBasicAuthorization(const FString& Username, const FString& Password);

	//////////////////////////////////////////////////////////////////////////
	// Destruction and reset

	/** Reset all internal saved data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	void ResetData();

	/** Reset saved request data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void ResetRequestData();

	/** Reset saved response data */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	void ResetResponseData();

	/** Cancel latent response waiting  */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	void Cancel();

	//////////////////////////////////////////////////////////////////////////
	// JSON data accessors

	/** Get the Request Json object */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	UWorldsbaseJsonObject* GetRequestObject() const;

	/** Set the Request Json object */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetRequestObject(UWorldsbaseJsonObject* JsonObject);

	/** Get the Response Json object */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	UWorldsbaseJsonObject* GetResponseObject() const;

	/** Set the Response Json object */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	void SetResponseObject(UWorldsbaseJsonObject* JsonObject);

	/** Get the Response Json value */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	UWorldsbaseJsonValue* GetResponseValue() const;

	///////////////////////////////////////////////////////////////////////////
	// Request/response data access

	/** Get url of http request */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Request")
	FString GetURL() const;

	/** Get verb to the request */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Request")
	EWorldsbaseRequestVerb GetVerb() const;

	/** Get status of http request */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Request")
	EWorldsbaseRequestStatus GetStatus() const;

	/** Get the response code of the last query */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Response")
	int32 GetResponseCode() const;

	/** Get value of desired response header */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Response")
	FString GetResponseHeader(const FString& HeaderName);

	/** Get list of all response headers */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Response")
	TArray<FString> GetAllResponseHeaders() const;

	/**
	 * Shortcut to get the Content-Length header value. Will not always return non-zero.
	 * If you want the real length of the payload, get the payload and check it's length.
	 *
	 * @return the content length (if available)
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Response")
	int32 GetResponseContentLength() const;

	/**
	 * Get the content payload of the request or response.
	 *
	 * @param Content - array that will be filled with the content.
	 */
	UFUNCTION(BlueprintPure, Category = "Worldsbase|Response")
	const TArray<uint8>& GetResponseContent() const;

	//////////////////////////////////////////////////////////////////////////
	// URL processing

	/** Setting request URL */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	void SetURL(const FString& Url = TEXT("http://alyamkin.com"));

	/** Open URL with current setup */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	virtual void ProcessURL(const FString& Url = TEXT("http://alyamkin.com"));

	/** Open URL in latent mode */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request", meta = (Latent, LatentInfo = "LatentInfo", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	virtual void ApplyURL(const FString& Url, UWorldsbaseJsonObject*& Result, UObject* WorldContextObject, struct FLatentActionInfo LatentInfo);

	/** Check URL and execute process request */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Request")
	virtual void ExecuteProcessRequest();

protected:
	/** Apply current internal setup to request and process it */
	void ProcessRequest();

	//////////////////////////////////////////////////////////////////////////
	// Request callbacks

private:
	/** Internal bind function for the IHTTPRequest::OnProcessRequestCompleted() event */
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	/** Event occured when the request has been completed */
	UPROPERTY(BlueprintAssignable, Category = "Worldsbase|Event")
	FOnRequestComplete OnRequestComplete;

	/** Event occured when the request wasn't successfull */
	UPROPERTY(BlueprintAssignable, Category = "Worldsbase|Event")
	FOnRequestFail OnRequestFail;

	/** Event occured when the request has been completed */
	FOnStaticRequestComplete OnStaticRequestComplete;

	/** Event occured when the request wasn't successfull */
	FOnStaticRequestFail OnStaticRequestFail;

	//////////////////////////////////////////////////////////////////////////
	// Tags

	/** Add tag to this request */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	void AddTag(FName Tag);

	/**
	 * Remove tag from this request
	 *
	 * @return Number of removed elements
	 */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	int32 RemoveTag(FName Tag);

	/** See if this request contains the supplied tag */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Utility")
	bool HasTag(FName Tag) const;

protected:
	/** Array of tags that can be used for grouping and categorizing */
	TArray<FName> Tags;

	//////////////////////////////////////////////////////////////////////////
	// Data

public:
	/**
	 * Get request response stored as a string
	 * @param bCacheResponseContent - Set true if you plan to use it few times to prevent deserialization each time
	 */
	UFUNCTION(BlueprintCallable, Category = "Worldsbase|Response")
	FString GetResponseContentAsString(bool bCacheResponseContent = true);

	/** Response size */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Worldsbase|Response")
	int32 ResponseSize;

	/** DEPRECATED: Please use GetResponseContentAsString() instead */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Worldsbase|Response")
	FString ResponseContent;

	/** Is the response valid JSON? */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Worldsbase|Response")
	bool bIsValidJsonResponse;

protected:
	/** Default value for deprecated ResponseContent variable */
	static FString DeprecatedResponseString;

	/** Latent action helper */
	FWorldsbaseLatentAction<UWorldsbaseJsonObject*>* ContinueAction;

	/** Internal request data stored as JSON */
	UPROPERTY()
	UWorldsbaseJsonObject* RequestJsonObj;

	TArray<uint8> RequestBytes;
	FString BinaryContentType;

	/** Raw response storage */
	TArray<uint8> ResponseBytes;
	int32 ResponseContentLength;

	/** Used for special cases when used wants to have plain string data in request.
	 * Attn.! Content-type x-www-form-urlencoded only. */
	FString StringRequestContent;

	/** Response data stored as JSON */
	UPROPERTY()
	UWorldsbaseJsonObject* ResponseJsonObj;

	/** Response data stored as JSON value */
	UPROPERTY()
	UWorldsbaseJsonValue* ResponseJsonValue;

	/** Verb for making request (GET,POST,etc) */
	EWorldsbaseRequestVerb RequestVerb;

	/** Content type to be applied for request */
	EWorldsbaseRequestContentType RequestContentType;

	/** Mapping of header section to values. Used to generate final header string for request */
	TMap<FString, FString> RequestHeaders;

	/** Cached key/value header pairs. Parsed once request completes */
	TMap<FString, FString> ResponseHeaders;

	/** Http Response code */
	int32 ResponseCode;

	/** Custom verb that will be used with RequestContentType == CUSTOM */
	FString CustomVerb;

	/** Request we're currently processing */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

public:
	/** Returns reference to internal request object */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetHttpRequest() const { return HttpRequest; };
};