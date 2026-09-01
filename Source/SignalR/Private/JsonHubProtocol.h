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
#include "IHubProtocol.h"

/**
 * Implementation of the SignalR JSON hub protocol.
 * Handles serialization and deserialization of SignalR messages in JSON format.
 */
class SIGNALR_API FJsonHubProtocol : public IHubProtocol
{
public:
    /**
     * Character used to separate records in the SignalR protocol stream.
     */
    static constexpr TCHAR RecordSeparator = TEXT('\x1e');

    /**
     * Virtual destructor for the JSON hub protocol.
     */
    virtual ~FJsonHubProtocol() override = default;

    /**
     * Gets the name of this hub protocol.
     * 
     * @return The name of the protocol.
     */
    virtual FName Name() const override;

    /**
     * Gets the version of this hub protocol.
     * 
     * @return The protocol version.
     */
    virtual int Version() const override;

    /**
     * Serializes a hub message to a JSON string.
     * 
     * @param InMessage The message to serialize.
     * 
     * @return The serialized message string.
     */
    virtual FString SerializeMessage(const FHubMessage* InMessage) const override;

    /**
     * Parses a string containing one or more JSON messages into hub message objects.
     * 
     * @param InMessage The string to parse.
     * 
     * @return Array of parsed hub messages.
     */
    virtual TArray<TSharedPtr<FHubMessage>> ParseMessages(const FString& InMessage) const override;

private:
    TSharedPtr<FHubMessage> ParseMessage(const FString& InMessage) const;
};
