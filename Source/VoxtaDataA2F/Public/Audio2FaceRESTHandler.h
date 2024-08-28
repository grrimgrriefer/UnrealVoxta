// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

class VOXTADATAA2F_API Audio2FaceRESTHandler
{
public:

	void Initialize();
	void GetBlendshapes();

private:
	void GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GetInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GetSettings(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GetPlayerInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GetBlendshapeSolver(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);

	void SetPlayerTrack(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	void GenerateBlendShapes(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);

	FString JsonToString(TSharedRef<FJsonObject> JsonObject);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetBaseRequest(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
};
