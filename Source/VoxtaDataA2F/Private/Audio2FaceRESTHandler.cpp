// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"

void Audio2FaceRESTHandler::Initialize()
{
	/*
	GetStatus
"OK"

	LoadUsdFile
{
  "status": "OK",
  "message": "Succeeded"
}

	GetInstance
{
  "status": "OK",
  "result": {
	"fullface_instances": [
	  "/World/audio2face/CoreFullface"
	]
  },
  "message": "Succeeded"
}

	GetSettings
{
  "status": "OK",
  "result": {
	"a2e_streaming_live_mode": false
}

	GetPlayerInstance
{
  "status": "OK",
  "result": {
	"regular": [
	  "/World/audio2face/Player"
	],
	"streaming": []
  },
  "message": "Suceeded to retrieve Player instances"
}

	SetPlayerRootPath
{
  "status": "OK",
  "message": "Set the audio file root to D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"
}

	GetPlayerTracks
{
  "status": "OK",
  "result": [
	"speak.wav"
  ],
  "message": "Suceeded"
}

	GetBlendshapeSolver
{
  "status": "OK",
  "result": [
	"/World/audio2face/BlendshapeSolve"
  ]
}
	*/
}

void Audio2FaceRESTHandler::GetBlendshapes()
{
	/*

	SetPlayerTrack
{
  "status": "OK",
  "message": "Set track to speak.wav"
}

	GetBlendshapes
{
  "status": "OK"
}
	*/
}

void Audio2FaceRESTHandler::GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/status");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/USD/Load");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("file_name"), TEXT("D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content\\test.usd"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GetInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/GetInstances");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GetSettings(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/USD/Load");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_instance"), TEXT("/World/audio2face/CoreFullface"));
	TArray<TSharedPtr<FJsonValue>> SettingsArray;
	SettingsArray.Add(MakeShareable(new FJsonValueString(TEXT("a2e_streaming_live_mode"))));
	JsonObject->SetArrayField(TEXT("settings"), SettingsArray);

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GetPlayerInstance(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/GetInstances");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/SetRootPath");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/GetTracks");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));
	JsonObject->SetStringField(TEXT("dir_path"), TEXT("D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GetBlendshapeSolver(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Exporter/GetBlendShapeSolvers");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::SetPlayerTrack(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/SetTrack");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));
	JsonObject->SetStringField(TEXT("file_name"), TEXT("speak.wav"));
	TArray<TSharedPtr<FJsonValue>> TimeRangeArray;
	TimeRangeArray.Add(MakeShareable(new FJsonValueNumber(0)));
	TimeRangeArray.Add(MakeShareable(new FJsonValueNumber(-1)));
	JsonObject->SetArrayField(TEXT("time_range"), TimeRangeArray);

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GenerateBlendShapes(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Exporter/ExportBlendshapes");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("solver_node"), TEXT("/World/audio2face/BlendshapeSolve"));
	JsonObject->SetStringField(TEXT("export_directory"), TEXT("D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"));
	JsonObject->SetStringField(TEXT("file_name"), TEXT("blendshapes1"));
	JsonObject->SetStringField(TEXT("format"), TEXT("json"));
	JsonObject->SetStringField(TEXT("batch"), TEXT("false"));
	JsonObject->SetStringField(TEXT("fps"), TEXT("30"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

FString Audio2FaceRESTHandler::JsonToString(TSharedRef<FJsonObject> JsonObject)
{
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject, Writer);
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Audio2FaceRESTHandler::GetBaseRequest(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([Callback] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			Callback(Request, Response, bWasSuccessful);
		});
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("accept", "application/json");
	return Request;
}