// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "UserCharData.h"

/// <summary>
/// Read-only data struct containing the relevant data of the 'welcome' response
/// from the VoxtaServer.
/// </summary>
struct ServerResponseWelcome : public IServerResponseBase
{
public:
	const FUserCharData m_user;

	explicit ServerResponseWelcome(const FUserCharData& userData) :
		m_user(userData)
	{
	}

	/// <summary>
	/// Identifies the response type as Welcome.
	/// </summary>
	/// <returns>Returns MessageType::Welcome.</returns>
	ServerResponseType GetType() const final
	{
		return ServerResponseType::Welcome;
	}
};
