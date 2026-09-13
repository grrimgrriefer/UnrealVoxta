// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once
#include "Delegates/DelegateSignatureImpl.inl"

struct IVoxtaResponseDispatcher
{
	virtual ~IVoxtaResponseDispatcher() = default;
};

template <typename T>
struct TVoxtaResponseDispatcher : IVoxtaResponseDispatcher
{
	TMulticastDelegate<void(const T&)> m_Delegate;
};

struct FVoxtaResponseRoute
{
	TFunction<void(const FString&)> m_DeserializeAndDispatch;
};
