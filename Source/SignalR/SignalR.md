# SignalR Module

## Overview

The SignalR module provides a trimmed-down implementation of the SignalR protocol for Unreal Engine, integrated into the Voxta plugin. It provides real-time bidirectional communication between the client and server using WebSockets. 

Do keep in mind that this is a modified version of the Module made by Frozen Storm Interactive, Yoann Potinet, licensed under MIT, with various changes and tweaks that are Voxta-specific. Any existing features not required for Voxta communication are considered untested.

## Module Structure

### Public API

- `IHubConnection` : Defines the hub connection interface
- `SignalRModule` : The main module entry point
- `SignalRSubsystem` : Engine subsystem for managing connections
- `SignalRValue` : Type system for handling various data types

### Internal Components

- `CallbackManager` : Manages callbacks for async operations
- `Connection` : Handles the underlying WebSocket connection
- `HandshakeProtocol` : Implements the SignalR handshake protocol
- `HubConnection` : Core implementation of the hub connection
- `IHubProtocol` : Protocol interface for message formatting
- `JsonHubProtocol` : JSON implementation of the hub protocol
- `MessageType` - Defines message type enumerations
- `NegotiationResponse` : Data structures for connection negotiation
- `StringUtils` : String manipulation utilities

## Sequence diagram

![SequenceDiagramSignalR image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/SequenceDiagramSignalR.PNG&resolveLfs=true&%24format=octetStream "SequenceDiagramSignalR image.")  

## Features

### Real-time Communication

The primary functionality of SignalR is providing real-time communication:

- Bidirectional messaging between client and server
- Automatic reconnection handling when server allows it
- Support for different message types (invocation, completion, ping, close)
- Thread-safe callback management for asynchronous operations

### Value System

The module includes a flexible type system (`FSignalRValue`) that supports:

- Primitive types (numbers, strings, booleans)
- Complex types (objects, arrays)
- Binary data (as byte arrays)
- Null values
- Automatic type conversion and validation

## Usage

### Hub Connections

The module provides a hub-based connection model:

```cpp
// Create a hub connection using the engine subsystem
if (USignalRSubsystem* signalR = GEngine->GetEngineSubsystem<USignalRSubsystem>())
{
    TSharedPtr<IHubConnection> HubConnection = signalR->CreateHubConnection(
         FString::Format(*FString(TEXT("http://{0}:{1}/hub")), {
            hostAddress,
            hostPort
         }),
        TMap<FString, FString>() // Optional headers
    );

    // Start the connection
    HubConnection->Start();
}
```

### Remote Method Invocation

SignalR supports invoking methods on the server and handling server-initiated calls:

```cpp
// Register a handler for a server-initiated method call
hubConnection->On("ReceiveMessage").BindUObject(this, &YourClass::OnReceivedMessage);

void YourClass::OnReceivedMessage(const TArray<FSignalRValue>& arguments)
{
    // NOTE: this is executed on the networking thread, use FTSTicker if you want to interact with UObjects.
    // ...
}

// Invoke a server method with complex arguments
FSignalRValue message =  FSignalRValue(TMap<FString, FSignalRValue> {
        // { ..., ... }, properties here
        // ...
    });

hubConnection->Invoke("SendMessage", message).BindUObject(this, &YourClass::OnMessageSent);

void YourClass::OnMessageSent(const FSignalRInvokeResult& deliveryReceipt)
{
    if (deliveryReceipt.HasError())
    {
        // NOTE: this is executed on the networking thread, use FTSTicker if you want to interact with UObjects.
        // ... deliveryReceipt.GetErrorMessage()
    }
}
```

### Cleanup

```cpp
// Stop the connection when done
if (hubConnection.IsValid())
{
    hubConnection->Stop();
}
```

## Dependencies

- UnrealEngine 
  - `Core`
  - `CoreUObject`
  - `Engine`
  - `Json`
  - `HTTP` 
  - `WebSockets`

## Licensing

Original code:

MIT license - copyright (c) 2020-2022 Frozen Storm Interactive, Yoann Potinet. See /SignalR/License for details.
