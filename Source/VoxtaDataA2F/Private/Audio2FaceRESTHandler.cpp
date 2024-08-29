// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"

void Audio2FaceRESTHandler::Initialize()
{
	AsyncTask(ENamedThreads::AnyThread, [&] ()
	{
		bool waiting = true;
		bool totalSuccess = true;

		GetStatus([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			totalSuccess = totalSuccess && resp->GetContentAsString() == TEXT("OK");
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetStatus failed, aborting Initializing"));
			return;
		}

		LoadUsdFile([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("message")) == TEXT("Succeeded");
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadUsdFile failed, aborting Initializing"));
			return;
		}

		GetInstance([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("message")) == TEXT("Succeeded");
					if (totalSuccess)
					{
						TArray<FString> stringArray;
						totalSuccess == totalSuccess && JsonObject->GetObjectField(TEXT("result"))->TryGetStringArrayField(TEXT("fullface_instances"), stringArray);
						totalSuccess == totalSuccess && stringArray.Num() == 1 && stringArray[0] == TEXT("/World/audio2face/CoreFullface");
					}
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetInstance failed, aborting Initializing"));
			return;
		}

		GetSettings([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
					if (totalSuccess)
					{
						// make sure streaming is off, it conflicts with exporting
						totalSuccess == totalSuccess && (false == JsonObject->GetObjectField(TEXT("result"))->GetBoolField(TEXT("a2e_streaming_live_mode")));
					}
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetSettings failed, aborting Initializing"));
			return;
		}

		GetPlayerInstance([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("message")) == TEXT("Suceeded to retrieve Player instances");
					if (totalSuccess)
					{
						TArray<FString> stringArray;
						totalSuccess == totalSuccess && JsonObject->GetObjectField(TEXT("result"))->TryGetStringArrayField(TEXT("regular"), stringArray);
						totalSuccess == totalSuccess && stringArray.Num() == 1 && stringArray[0] == TEXT("/World/audio2face/Player");
					}
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetPlayerInstance failed, aborting Initializing"));
			return;
		}

		SetPlayerRootPath([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetPlayerRootPath failed, aborting Initializing"));
			return;
		}

		GetBlendshapeSolver([&waiting, &totalSuccess] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
		{
			totalSuccess = totalSuccess && success;
			if (success)
			{
				TSharedPtr<FJsonObject> JsonObject;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(resp->GetContentAsString());
				if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
				{
					totalSuccess == totalSuccess && JsonObject->GetStringField(TEXT("status")) == TEXT("OK");
					if (totalSuccess)
					{
						TArray<FString> stringArray;
						totalSuccess == totalSuccess && JsonObject->TryGetStringArrayField(TEXT("result"), stringArray);
						totalSuccess == totalSuccess && stringArray.Num() == 1 && stringArray[0] == TEXT("/World/audio2face/BlendshapeSolve");
					}
				}
				else
				{
					totalSuccess = false;
				}
			}
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetPlayerTracks failed, aborting Initializing"));
			return;
		}
	});
}

void Audio2FaceRESTHandler::GetBlendshapes(FString wavFileName, FString shapesFilePath, FString shapesFileName,
	TFunction<void(FString shapesFile, bool success)> callback)
{
	AsyncTask(ENamedThreads::AnyThread, [&] ()
	{
		bool waiting = true;
		bool totalSuccess = false;
		SetPlayerTrack(wavFileName, [&totalSuccess, &waiting] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
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
			waiting = false;
		});
		while (waiting) {};
		if (!totalSuccess)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetPlayerTrack failed, aborting GetBlendshapes"));
			callback(FString(), totalSuccess);
			return;
		}

		GenerateBlendShapes(shapesFilePath, shapesFileName,
			[&totalSuccess, &waiting, &shapesFilePath, &shapesFileName, callback]
			(FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
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
			if (totalSuccess)
			{
				callback(FPaths::Combine(shapesFilePath, shapesFileName), totalSuccess);
			}
			if (!totalSuccess)
			{
				UE_LOG(LogTemp, Warning, TEXT("GenerateBlendShapes failed, aborting GetBlendshapes"));
				callback(FString(), totalSuccess);
				return;
			}
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

void Audio2FaceRESTHandler::SetPlayerTrack(FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
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

void Audio2FaceRESTHandler::GenerateBlendShapes(FString filePath, FString fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback)
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