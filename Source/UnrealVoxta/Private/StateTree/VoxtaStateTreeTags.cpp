// Copyright(c) 2026 grrimgrriefer & DZnnah, see LICENSE for details.

#include "VoxtaStateTreeTags.h"

// Request meaning external asked and Subsystem invoked on statetree
UE_DEFINE_GAMEPLAY_TAG(TAG_Voxta_Request_Connection, "Voxta.Request.Connection");

// Notify meaning internal invoked and Subsystem forwarded to statetree
UE_DEFINE_GAMEPLAY_TAG(TAG_Voxta_Notify_SocketConnected, "Voxta.Notify.SocketConnected");

// Signal meaning invoked from a statetree task itself
UE_DEFINE_GAMEPLAY_TAG(TAG_Voxta_Signal_Authenticated, "Voxta.Signal.Authenticated");
