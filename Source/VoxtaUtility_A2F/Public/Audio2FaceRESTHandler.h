// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Json.h"
#include "JsonUtilities.h"

class VOXTAUTILITY_A2F_API Audio2FaceRESTHandler
{
public:
	void TryInitialize();
	void GetBlendshapes(FString wavFileName, FString shapesFilePath, FString shapesFileName, TFunction<void(FString shapesFile, bool success)> callback);
	bool IsBusy() const;

private:
	enum class currentState : uint8
	{
		notConnected,
		initializing,
		idle,
		busy
	};

	void GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const;
	void LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const;
	void SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const;
	//void GetInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	//void GetSettings(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	//void GetPlayerInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);
	//void GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback); // not needed?
	//void GetBlendshapeSolver(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback);

	void SetPlayerTrack(FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const;
	void GenerateBlendShapes(FString filePath, FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const;

	FString JsonToString(TSharedRef<FJsonObject> JsonObject) const;
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetBaseRequest(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;

	currentState m_currentState = currentState::notConnected;
};
