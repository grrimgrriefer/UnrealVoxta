// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

/**
 * Audio2FaceRESTHandler
 * Manages the HTTP REST API for A2F_headless mode.
 * Handles initialization, blendshape generation, and state tracking for Audio2Face.
 * Not intended for parallel generation; manages single-threaded access.
 */
class VOXTAUTILITY_A2F_API Audio2FaceRESTHandler : public TSharedFromThis<Audio2FaceRESTHandler>
{
#pragma region public API
public:
	/**
	 * Check if A2F is running and attempt to initialize it.
	 * Loads the USD file for Metahuman ARKit and sets the player root path.
	 */
	void TryInitialize();

	/**
	 * Use the A2F export REST API to generate a JSON file containing all blendshape curve values for ARKit mapping.
	 * Note: The wave file must be on the same machine as the A2F instance.
	 *
	 * @param wavFileName The name of the audio file to use for lipsync data generation.
	 * @param shapesFilePath The path to write the JSON shapes file to.
	 * @param shapesFileName The name of the JSON shapes file.
	 * @param callback Callback with the path of the JSON file and success status.
	 */
	void GetBlendshapes(const FString& wavFileName, const FString& shapesFilePath, const FString& shapesFileName,
		TFunction<void(const FString&, bool /*success*/)> callback);

	/** @return True if A2F is currently initializing. */
	bool IsInitializing() const;

	/** @return True if A2F is ready to generate new lipsync data. */
	bool IsAvailable() const;
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
	std::atomic<CurrentA2FState> m_currentState = CurrentA2FState::NotConnected;
#pragma endregion

#pragma region private API
	/** Send a request to the http://localhost:8011/status REST call (GET) */
	void GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/USD/Load" REST call (POST) */
	void LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Player/SetRootPath" REST call (POST) */
	void SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Player/SetTrack" REST call (POST) */
	void SetPlayerTrack(const FString& fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
	/** Send a request to the http://localhost:8011/A2F/Exporter/ExportBlendshapes" REST call (POST) */
	void GenerateBlendShapes(const FString& filePath, const FString& fileName,
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;

	// TODO: implement these, maybe? idk yet
	//void GetInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetSettings(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetPlayerInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);
	//void GetBlendshapeSolver(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback);

	/**
	 * Serialize a JsonObject into an FString.
	 *
	 * @param jsonObject The object to convert.
	 * 
	 * @return The serialized JSON as an FString.
	 */
	FString JsonToString(TSharedRef<FJsonObject> jsonObject) const;

	/**
	 * Create a generic threadsafe HTTP request for JSON data.
	 * Does not send the request; only prepares it and binds the callback.
	 *
	 * @param callback The response callback.
	 * 
	 * @return The prepared HTTP request.
	 */
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetBaseRequest(
		TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const;
#pragma endregion
};
