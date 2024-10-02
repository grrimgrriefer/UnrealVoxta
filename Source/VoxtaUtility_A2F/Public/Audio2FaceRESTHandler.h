// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * Audio2FaceRESTHandler
 * Manages the HTTP REST api for A2F_headless mode.
 * Still very much a work in progress, but we might move things over to the docker API, so I don't want to spend
 * too much time on this.
 */
class VOXTAUTILITY_A2F_API Audio2FaceRESTHandler : public TSharedFromThis<Audio2FaceRESTHandler>
{
#pragma region public API
public:
	/**
	 * Check if A2F is running at all. If so, it will attempt to load the usd file that matches the Metahuman ARKit,
	 * and also set the player root path to our folder so it can read the audio files that we will use as cache.
	 */
	void TryInitialize();

	/**
	 * Use that A2F export REST api to generate JSON file containing all the blendshape curve values for the
	 * ARKit mapping.
	 * Note: Due to obvious reasons, the wavefile needs to be on the same machine as the A2F instance.
	 *
	 * @param wavFileName The name of the audio file, that should be used to generate lipsync data.
	 * @param shapesFilePath The desired path to write the JSON shapesfile to.
	 * @param shapesFileName The desired name of the JSON shapesfile.
	 * @param callback The response after A2F is finished, containing the path of the JSON shapes file and if A2F
	 * reported the generation of it to be successful or not.
	 */
	void GetBlendshapes(FString wavFileName, FString shapesFilePath, FString shapesFileName,
		TFunction<void(FString shapesFilePath, bool success)> callback);

	/** @return True if A2F is currently trying to initialize itself. */
	bool IsInitializing() const;

	/**
	 * A2F headless cannot generate in parallel, which is why we need this.
	 *
	 * @return True if A2F is still busy generating lipsync data for an earlier call.
	 */
	bool IsBusy() const;
#pragma endregion

#pragma region private helper classes
private:
	/** Internal helper class, easier to keep track of what's going on, as well as user-friendly logging. */
	enum class CurrentA2FState : uint8
	{
		NotConnected,
		Initializing,
		Idle,
		Busy
	};
#pragma endregion

#pragma region data
private:
	CurrentA2FState m_currentState = CurrentA2FState::NotConnected;
#pragma endregion

#pragma region private API
	/** Send a request to the http://localhost:8011/status REST call (GET) */
	void GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/USD/Load" REST call (POST) */
	void LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Player/SetRootPath" REST call (POST) */
	void SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Player/SetTrack" REST call (POST) */
	void SetPlayerTrack(FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Exporter/ExportBlendshapes" REST call (POST) */
	void GenerateBlendShapes(FString filePath, FString fileName,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;

	// TODO: implement these, maybe? idk yet
	//void GetInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetSettings(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetPlayerInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetBlendshapeSolver(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);

	/**
	 * Helper utility to serialize a JsonObject into the FString representation.
	 *
	 * @param JsonObject The object to be converted.
	 *
	 * @return The serialized version of the provided object, in FString format.
	 */
	FString JsonToString(TSharedRef<FJsonObject> jsonObject) const;

	/**
	 * Helper utility to create a generic threadsafe HTTP request that is setup for JSON data.
	 * Note: This does NOT send the request, it just makes it and binds the callback to the eventual response.
	 *
	 * @param callback The response of the endpoint for the request.
	 *
	 * @return A generic HTTPrequest that is setup to send and receive JSON data.
	 */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetBaseRequest(
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
#pragma endregion
};
