// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#include "Audio2FaceRESTHandler.h"
#include "VoxtaDefines.h"
#include "Logging/StructuredLog.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IPluginManager.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Paths.h"

void Audio2FaceRESTHandler::TryInitialize()
{
	if (m_currentState != CurrentA2FState::NotConnected)
	{
		return;
	}

	m_currentState = CurrentA2FState::Initializing;
	AsyncTask(ENamedThreads::AnyThread, [Self = TWeakPtr<Audio2FaceRESTHandler>(AsShared())] ()
	{
		if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf = Self.Pin())
		{
			SharedSelf->GetStatus(
				[Self2 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf->AsShared())]
				(FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
				{
					if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf2 = Self2.Pin())
					{
						if (success)
						{
							UE_LOGFMT(VoxtaLog, Log, "{0}", resp->GetContentAsString());
							success = success && resp->GetContentAsString() == TEXT("\"OK\"");

							SharedSelf2->LoadUsdFile(
								[Self3 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf2->AsShared())]
								(FHttpRequestPtr req2, FHttpResponsePtr resp2, bool success2)
								{
									if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf3 = Self3.Pin())
									{
										UE_LOGFMT(VoxtaLog, Log, "{0}", resp2->GetContentAsString());
										if (success2)
										{
											SharedSelf3->SetPlayerRootPath(
												[Self4 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf3->AsShared())]
												(FHttpRequestPtr req3, FHttpResponsePtr resp3, bool success3)
												{
													if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf4 = Self4.Pin())
													{
														UE_LOGFMT(VoxtaLog, Log, "{0}", resp3->GetContentAsString());
														if (success3)
														{
															SharedSelf4->m_currentState = CurrentA2FState::Idle;
														}
														else
														{
															UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler SetPlayerRootPath request was not completed successfully, "
																"this should never happen, A2F will not be able to find our audio files, Aborting initialization...");
															SharedSelf4->m_currentState = CurrentA2FState::NotConnected;
														}
													}
													else
													{
														UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the response of the "
															"SetPlayerRootPath request was able to be handled. Aborting initialization...");
													}
												});
										}
										else
										{
											UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler loadUSD request was not completed successfully, "
												"this should never happen, but either way, we can use A2F without this, Aborting initialization...");
											SharedSelf3->m_currentState = CurrentA2FState::NotConnected;
										}
									}
									else
									{
										UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the response of the "
											"LoadUsdFile request was able to be handled. Aborting initialization...");
									}
								});
						}
						else
						{
							UE_LOGFMT(VoxtaLog, Warning, "Audio2FaceRESTHandler status request was not completed successfully, "
								"assuming we don't want A2F, Aborting initialization...");
							SharedSelf2->m_currentState = CurrentA2FState::NotConnected;
						}
					}
					else
					{
						UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the response of the "
							"GetStatus request was able to be handled. Aborting initialization...");
					}
				});
		}
		else
		{
			UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the background thread was able "
					"to start. Aborting initialization...");
		}
	});
}

void Audio2FaceRESTHandler::GetBlendshapes(const FString& wavFileName, const FString& shapesFilePath, const FString& shapesFileName,
	TFunction<void(const FString&, bool /*success*/)> callback)
{
	if (m_currentState != CurrentA2FState::Idle)
	{
		UE_LOGFMT(VoxtaLog, Error, "RESTHandler busy, GetBlendshapes request rejected; "
			"always check if the RESTHandler is busy before trying to request A2F blendshape generation, skipping request");
		AsyncTask(ENamedThreads::GameThread, [Callback = callback] () { Callback(FString(), false); });
		return;
	}
	m_currentState = CurrentA2FState::Busy;

	AsyncTask(ENamedThreads::AnyThread,
		[Self = TWeakPtr<Audio2FaceRESTHandler>(AsShared()), WavFileName0 = wavFileName, ShapesFilePath0 = shapesFilePath,
		ShapesFileName0 = shapesFileName, Callback0 = callback]()
		{
			if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf = Self.Pin())
			{
				SharedSelf->SetPlayerRootPath(
					[Self2 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf->AsShared()), ShapesFilePath1 = ShapesFilePath0, ShapesFileName1 = ShapesFileName0, Callback1 = Callback0, WavFileName1 = WavFileName0] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
					{
						if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf2 = Self2.Pin())
						{
							if (!success)
							{
								UE_LOGFMT(VoxtaLog, Warning, "SetPlayerRootPath failed, aborting GetBlendshapes");
								SharedSelf2->m_currentState = CurrentA2FState::Idle;
								AsyncTask(ENamedThreads::GameThread, [Callback1] () { Callback1(FString(), false); });
								return;
							}
							SharedSelf2->SetPlayerTrack(WavFileName1,
								[Self3 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf2->AsShared()), ShapesFilePath2 = ShapesFilePath1, ShapesFileName2 = ShapesFileName1, Callback2 = Callback1] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
								{
									if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf3 = Self3.Pin())
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
										UE_LOGFMT(VoxtaLog, Log, "{0}", resp->GetContentAsString());

										if (!totalSuccess)
										{
											UE_LOGFMT(VoxtaLog, Warning, "SetPlayerTrack failed, aborting GetBlendshapes");
											SharedSelf3->m_currentState = CurrentA2FState::Idle;
											AsyncTask(ENamedThreads::GameThread,
												[totalSuccess, Callback3 = Callback2] ()
												{
													Callback3(FString(), totalSuccess);
												});
											return;
										}

										UE_LOGFMT(VoxtaLog, Log, "SetPlayerTrack success");

										SharedSelf3->GenerateBlendShapes(ShapesFilePath2, ShapesFileName2,
											[Self4 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf3->AsShared()), TotalSuccess = totalSuccess, ShapesFilePath3 = ShapesFilePath2, ShapesFileName3 = ShapesFileName2, Callback3 = Callback2] (FHttpRequestPtr req, FHttpResponsePtr resp, bool success)
											{
												if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf4 = Self4.Pin())
												{
													bool totalSuccess = TotalSuccess && success;
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
													UE_LOGFMT(VoxtaLog, Log, "{0}", resp->GetContentAsString());

													AsyncTask(ENamedThreads::GameThread,
														[Self5 = TWeakPtr<Audio2FaceRESTHandler>(SharedSelf4->AsShared()), TotalSuccess2 = totalSuccess, ShapesFilePath4 = ShapesFilePath3, ShapesFileName4 = ShapesFileName3, Callback4 = Callback3] ()
														{
															if (TSharedPtr<Audio2FaceRESTHandler> SharedSelf5 = Self5.Pin())
															{
																if (TotalSuccess2)
																{
																	UE_LOGFMT(VoxtaLog, Log, "GenerateBlendShapes success");
																	SharedSelf5->m_currentState = CurrentA2FState::Idle;
																	Callback4(FPaths::Combine(ShapesFilePath4, ShapesFileName4), TotalSuccess2);
																}
																else
																{
																	UE_LOGFMT(VoxtaLog, Warning, "GenerateBlendShapes failed, aborting GetBlendshapes");
																	SharedSelf5->m_currentState = CurrentA2FState::Idle;
																	Callback4(FString(), TotalSuccess2);
																	return;
																}
															}
															else
															{
																UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the we could send the response of the "
																	" blendshapes back to the gamethread. Aborting blendshape generation...");
															}
														});
												}
												else
												{
													UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the GenerateBlendShapes response could be handled, "
															"Aborting blendshape generation...");
												}
											});
									}
									else
									{
										UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the SetPlayerTrack response could be handled, "
												"Aborting blendshape generation...");
									}
								});
						}
						else
						{
							UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the SetPlayerRootPath response could be handled, "
									"Aborting blendshape generation...");
						}
					});
			}
			else
			{
				UE_LOGFMT(VoxtaLog, Error, "Audio2FaceRESTHandler was destroyed before the background thread was able "
						"to start. Aborting blendshape generation...");
			}
		});
}

bool Audio2FaceRESTHandler::IsInitializing() const
{
	return m_currentState == CurrentA2FState::Initializing;
}

bool Audio2FaceRESTHandler::IsAvailable() const
{
	return m_currentState == CurrentA2FState::Idle;
}

void Audio2FaceRESTHandler::GetStatus(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/status");
	Request->SetVerb("GET");
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::LoadUsdFile(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/USD/Load");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("file_name"), FString::Format(TEXT("{0}\\claire_solved_arkit.usd"),
		{ IPluginManager::Get().FindPlugin("UnrealVoxta")->GetContentDir() }));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::SetPlayerRootPath(TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = GetBaseRequest(Callback);
	Request->SetURL("http://localhost:8011/A2F/Player/SetRootPath");
	Request->SetVerb("POST");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("a2f_player"), TEXT("/World/audio2face/Player"));
	JsonObject->SetStringField(TEXT("dir_path"), FString::Format(TEXT("{0}\\A2FCache"),
		{ IPluginManager::Get().FindPlugin("UnrealVoxta")->GetContentDir() }));

	Request->SetContentAsString(JsonToString(JsonObject.ToSharedRef()));
	Request->ProcessRequest();
}

void Audio2FaceRESTHandler::SetPlayerTrack(const FString& fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
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

void Audio2FaceRESTHandler::GenerateBlendShapes(const FString& filePath, const FString& fileName, TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> Callback) const
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

FString Audio2FaceRESTHandler::JsonToString(TSharedRef<FJsonObject> jsonObject) const
{
	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(jsonObject, Writer);
	return RequestBody;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Audio2FaceRESTHandler::GetBaseRequest(
	TFunction<void(FHttpRequestPtr, FHttpResponsePtr, bool)> callback) const
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindLambda(
		[Callback = callback] (FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			Callback(Request, Response, bWasSuccessful);
		});
	Request->SetHeader("Content-Type", "application/json");
	Request->SetHeader("accept", "application/json");
	Request->SetTimeout(10.0f); // 10 seconds timeout
	return Request;
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