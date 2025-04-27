/*
 * MIT License
 *
 * Copyright (c) 2020-2022 Frozen Storm Interactive, Yoann Potinet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"
#include "IHubConnection.h"

/**
 * Manages callbacks for asynchronous method invocations in SignalR connections.
 * This class provides functionality to register, invoke, and remove callbacks for hub methods.
 */
class FCallbackManager
{
public:
	FCallbackManager();
	~FCallbackManager();

	/**
	 * Registers a new callback and returns its ID along with a reference to the callback.
	 * 
	 * @return A tuple containing the callback ID and a reference to the callback.
	 */
	TTuple<FName, IHubConnection::FOnMethodCompletion&> RegisterCallback();

	/**
	 * Invokes a callback with the specified arguments.
	 * 
	 * @param InCallbackId The ID of the callback to invoke.
	 * @param InArguments The arguments to pass to the callback.
	 * @param InRemoveCallback Whether to remove the callback after invoking it.
	 * 
	 * @return True if the callback was found and invoked, false otherwise.
	 */
	bool InvokeCallback(FName InCallbackId, const FSignalRValue& InArguments, bool InRemoveCallback);

	 /**
	 * Removes a callback with the specified ID.
	 * 
	 * @param InCallbackId The ID of the callback to remove.
	 * 
	 * @return True if the callback was found and removed, false otherwise.
	 */
	bool RemoveCallback(FName InCallbackId);

	/**
	 * Removes all callbacks and invokes them with the specified error message.
	 * 
	 * @param ErrorMessage The error message to pass to the callbacks.
	 */
	void Clear(const FString& ErrorMessage);

private:
	FName GenerateCallbackId();

	TMap<FName, IHubConnection::FOnMethodCompletion> Callbacks;
	FCriticalSection CallbacksLock;

	FThreadSafeCounter CurrentId;
};
