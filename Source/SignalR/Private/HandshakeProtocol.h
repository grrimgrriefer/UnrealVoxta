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

class IHubProtocol;
class FJsonObject;

class FHandshakeProtocol
{
public:
    /**
     * Creates a handshake message for the specified hub protocol.
     * 
     * @param InProtocol The protocol to create a handshake message for
     * 
     * @return A string containing the serialized handshake message
     */
    static FString CreateHandshakeMessage(TSharedPtr<IHubProtocol> InProtocol);

    /**
     * Parses a handshake response from the server.
     * 
     * @param Response The raw response string from the server
     * 
     * @return A tuple containing the parsed handshake message as a JSON object and any remaining response data
     */
    static TTuple<TSharedPtr<FJsonObject>, FString> ParseHandshakeResponse(const FString& Response);
};
