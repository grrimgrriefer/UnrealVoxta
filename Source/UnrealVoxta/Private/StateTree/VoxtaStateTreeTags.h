// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#pragma once

#include "NativeGameplayTags.h"

// Request meaning external asked and Subsystem invoked on statetree
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Voxta_Request_Connection);

// Notify meaning internal invoked and Subsystem forwarded to statetree
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Voxta_Notify_SocketConnected);

// Signal meaning invoked from a statetree task itself
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Voxta_Signal_Authenticated);
