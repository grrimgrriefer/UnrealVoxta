// Copyright(c) 2024 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "CoreMinimal.h"
#include "ServerResponseBase.h"
#include "CharData.h"

struct ServerResponseWelcome : public ServerResponseBase
{
	const CharData m_user;

	explicit ServerResponseWelcome(const CharData& userData) :
		m_user(userData)
	{
	}

	ServerResponseType GetType() final
	{
		return ServerResponseType::Welcome;
	}
};
