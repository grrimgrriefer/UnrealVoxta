// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "UserCharData.h"

struct ServerResponseWelcome : public IServerResponseBase
{
public:
	const FUserCharData m_user;

	explicit ServerResponseWelcome(const FUserCharData& userData) :
		m_user(userData)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::Welcome;
	}
};
