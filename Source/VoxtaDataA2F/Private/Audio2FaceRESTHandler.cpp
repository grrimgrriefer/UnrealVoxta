// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"
#include <Logging/StructuredLog.h>

void Audio2FaceRESTHandler::TryInitialize()
{
	AsyncTask(ENamedThreads::AnyThread, [&] ()
	{
		GetStatus([&] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			if (success)
			{
				UE_LOGFMT(LogTemp, Warning, "{0}", resp->GetContentAsString());
				success = success && resp->GetContentAsString() == TEXT("\"OK\"");

				LoadUsdFile([&] (FHttpRequestPtr req2, FHttpResponsePtr resp2, bool success2)
				{
					UE_LOGFMT(LogTemp, Warning, "{0}", resp2->GetContentAsString());
					if (success2)
					{
						SetPlayerRootPath([&] (FHttpRequestPtr req3, FHttpResponsePtr resp3, bool success3)
						{
							UE_LOGFMT(LogTemp, Warning, "{0}", resp3->GetContentAsString());
						});
					}
				});
			}
		});
	});
}

void Audio2FaceRESTHandler::GetBlendshapes(FString wavFileName, FString shapesFilePath, FString shapesFileName,
	TFunction<void(FString shapesFile, bool success)> callback) const
{
	AsyncTask(ENamedThreads::AnyThread, [&, WavFileName0 = wavFileName, ShapesFilePath0 = shapesFilePath,
		ShapesFileName0 = shapesFileName, Callback0 = callback] ()
		{
			SetPlayerRootPath(
				[&, ShapesFilePath1 = ShapesFilePath0, ShapesFileName1 = ShapesFileName0, Callback1 = Callback0, WavFileName1 = WavFileName0] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
			{
				SetPlayerTrack(WavFileName1,
					[&, ShapesFilePath2 = ShapesFilePath1, ShapesFileName2 = ShapesFileName1, Callback2 = Callback1] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
				{
					bool totalSuccess = success;
					if (success)
					{
						TSharedPtr<FJsonObject> JsonObject;
						TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
						if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
						{
							totalSuccess = totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
						}
						else
						{
							totalSuccess = false;
						}
					}
					UE_LOGFMT(LogTemp, Warning, "{0}", resp->GetContentAsString());

					if (!totalSuccess)
					{
						UE_LOG(LogTemp, Warning, TEXT("SetPlayerTrack failed, aborting GetBlendshapes"));
						AsyncTask(ENamedThreads::GameThread,
							[totalSuccess, Callback3 = Callback2] ()
						{
							Callback3(FString(), totalSuccess);
						});
						return;
					}

					UE_LOG(LogTemp, Log, TEXT("SetPlayerTrack success"));

					GenerateBlendShapes(ShapesFilePath2, ShapesFileName2,
						[&totalSuccess, ShapesFilePath3 = ShapesFilePath2, ShapesFileName3 = ShapesFileName2, Callback3 = Callback2] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
					{
						totalSuccess = totalSuccess && success;
						if (success)
						{
							TSharedPtr<FJsonObject> JsonObject;
							TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
							if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
							{
								totalSuccess = totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
							}
							else
							{
								totalSuccess = false;
							}
						}
						UE_LOGFMT(LogTemp, Warning, "{0}", resp->GetContentAsString());

						AsyncTask(ENamedThreads::GameThread,
							[totalSuccess, ShapesFilePath4 = ShapesFilePath3, ShapesFileName4 = ShapesFileName3, Callback4 = Callback3] ()
						{
							if (totalSuccess)
							{
								UE_LOG(LogTemp, Log, TEXT("GenerateBlendShapes success"));
								Callback4(FPaths::Combine(ShapesFilePath4, ShapesFileName4), totalSuccess);
							}
							if (!totalSuccess)
							{
								UE_LOG(LogTemp, Warning, TEXT("GenerateBlendShapes failed, aborting GetBlendshapes"));
								Callback4(FString(), totalSuccess);
								return;
							}
						});
					});
				});
		});
	});
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
	JsonObject->SetStringField(TEXT("file_name"), TEXT("D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content\\claire_solved_arkit.usd"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

/*
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
	Request->SetURL("http://localhost:8011/A2F/GetSettings");
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
*/

void Audio2FaceRESTHandler::SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/SetRootPath");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));
	JsonObject->SetStringField(TEXT("dir_path"), TEXT("D:\\Documents\\Unreal Projects\\VoxtaTestProject\\Plugins\\UnrealVoxta\\Content"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

/*
void Audio2FaceRESTHandler::GetPlayerTracks(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/GetTracks");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));

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
*/

void Audio2FaceRESTHandler::SetPlayerTrack(FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/SetTrack");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));
	JsonObject->SetStringField(TEXT("file_name"), fileName);
	TArray<TSharedPtr<FJsonValue>> TimeRangeArray;
	TimeRangeArray.Add(MakeShareable(new FJsonValueNumber(0)));
	TimeRangeArray.Add(MakeShareable(new FJsonValueNumber(-1)));
	JsonObject->SetArrayField(TEXT("time_range"), TimeRangeArray);

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::GenerateBlendShapes(FString filePath, FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Exporter/ExportBlendshapes");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("solver_node"), TEXT("/World/audio2face/BlendshapeSolve"));
	JsonObject->SetStringField(TEXT("export_directory"), filePath);
	JsonObject->SetStringField(TEXT("file_name"), fileName);
	JsonObject->SetStringField(TEXT("format"), TEXT("json"));
	JsonObject->SetStringField(TEXT("batch"), TEXT("false"));
	JsonObject->SetStringField(TEXT("fps"), TEXT("30"));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

FString Audio2FaceRESTHandler::JsonToString(TSharedRef<FJsonObject> JsonObject) const
{
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return RequestBody;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Audio2FaceRESTHandler::GetBaseRequest(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda([Callback = callback] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			Callback(Request, Response, bWasSuccessful);
		});
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("accept", "application/json");
	return Request;
}