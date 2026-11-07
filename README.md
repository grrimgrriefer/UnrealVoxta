## Elements
* UCharacterSubsystem: The public-facing 'high level' API of the plugin. Provides utility to control the behavior of APawns in the level and trigger them to do general things that utilize Voxta integration. (e.g. start conversations, decide where in the level to move towards based on 'mood' and 'points of interest' in the level, etc...)
* IVoxtaClient: The public-facing 'lower level' API of the plugin. Physically lives on the UVoxtaStateTreeSubsystem but hides derived functionality from the subsystem itself.
* UVoxtaStateTreeSubsystem: Main 'container' that persists across the entire gameinstance and effectively holds all (sub)components either directly or indirectly. Important: Takes zero action by itself, only forwards Interface requests to it's statetree.
  * Holds the StateTree component and thus indirectly owns all its tasks.
  * Holds the UserConfigData compoment and thus, which itself contains the user provided ipv4 and port for the VoxtaServer host.
  * Holds the RuntimeData compoment, which has the serverside info (username, version etc.) as well as the available characters and their metadata (including avatar image, etc.)
* Internal StateTree: The UnrealStateTree object that dictates the actions taken by the plugin. States are influenced by Events comfing from the UVoxtaStateTreeSubsystem itself (forwarded from the public interface) as well as events comfing from the ApiHandler (e.g. disconnects etc.).
  * Holds all States and their Tasks (as well as conditions, evaluators, contextData, etc.)
* StateTree Tasks: Placed on individual states, and responsible for 'doing things' (i.e. turning events into Serverside API calls and handling the reponses). Must always be self-contained (open-closed), rely on dependency-inversion, and must never touch raw SignalR messages i.e. only deal with strongly typed objects.
* UVoxtaApiHandler: Low level translation layer that serializes and deserializes information between the plugin's objects and raw SignalR messages. This should be the only object that handles the raw SignalR messages, all other systems will use strongly typed objects to avoid and ease future version upgrades. NOTE: maybe also responsible for caching responses if that becomes needed, but not sure yet.
* UVoxtaSocketHandler: Lowest level layer that owns the socket connection directly and passes messages through.

## High level states
```mermaid
---
config:
  theme: dark
---
stateDiagram-v2
	state StateTree {
		state Disconnected {
			[*] --> AttemptingToConnect : Open socket connection
		}
		
		state Connected {
			[*] --> AttemptingToAuthenticate : Start handshake

			state Authenticated {
				Idle --> StartingChat : User or NPC initiated chat

				state Chatting {
					GeneratingReply --> AudioPlayback : Audio generated and imported into UE
					AudioPlayback --> WaitingForUserResponse : Audio finished & Mic is activated
					GeneratingReply --> WaitingForUserResponse : Reply was text-only
					WaitingForUserResponse --> GeneratingReply : Speech started & completed events triggered
				}
				
				StartingChat --> Chatting : Sever is ready for chat
				Chatting --> Idle : User or NPC ended chat
			}

			AttemptingToAuthenticate --> Authenticated : Handshake successful
		}    

		Disconnected --> Connected : Socket creating successful
		Connected --> Disconnected : Socket closed / connection loss
	}
```

## Auth flow
```mermaid
---
config:
  theme: dark
---
sequenceDiagram
	Note over UCharacterSubsystem,IVoxtaClient: Public facing API
    participant UVoxtaStateTreeSubsystem

    Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status

	UCharacterSubsystem->>+IVoxtaClient: Ensure connection
	UVoxtaStateTreeSubsystem->>+UVoxtaStateTreeSubsystem: Check current status

	alt IF NOT AUTHORIZED AND NOT ALREADY TRYING TO CONNECT
		UVoxtaStateTreeSubsystem->>+Internal StateTree: Request connection
			Note over UVoxtaStateTreeSubsystem,Internal StateTree: Event payload includes <br /> ipv4&port from user config
		Internal StateTree->>+Internal StateTree: Activate AttemptConnect sub-state
            Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status

		Internal StateTree->>+UVoxtaApiHandler: EstablishConnection
			Note over Internal StateTree,UVoxtaApiHandler: Forwarding ipv4&port payload

		UVoxtaApiHandler->>+UVoxtaSocketHandler: Trigger raw SignalR message
		UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
			Note over UVoxtaApiHandler: Broadcast connected event

		UVoxtaStateTreeSubsystem->>+Internal StateTree: Notify that VoxtaSocket is connected

		Internal StateTree->>+Internal StateTree: Deactivate both Disconnected and AttemptConnect substate<br /> and activate Connected state transition
		Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status

		Internal StateTree->>+Internal StateTree: Auto-activate AttemptAuth sub-state
            Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status

		Internal StateTree->>+UVoxtaApiHandler: SendMessage
			Note over Internal StateTree,UVoxtaApiHandler: Authentication handshake request

		UVoxtaApiHandler->>+UVoxtaSocketHandler: Trigger raw SignalR message
		UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
			Note over UVoxtaApiHandler: Broadcast message: welcomeResponse

		par [Receive configuration]
				UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
					Note over UVoxtaApiHandler: Broadcast message: configurationResponse
		and [Request available characters]
				Internal StateTree->>+UVoxtaApiHandler: SendMessage
					Note over Internal StateTree,UVoxtaApiHandler: Load Characters request

				UVoxtaApiHandler->>+UVoxtaSocketHandler: Trigger raw SignalR message
				UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
					Note over UVoxtaApiHandler: Broadcast message: characterList response
		end		

		Internal StateTree->>+UVoxtaStateTreeSubsystem: Update runtime info
			Note over UVoxtaStateTreeSubsystem: Save username & available characters

		Internal StateTree->>+Internal StateTree: Deactivate AttemptAuth substate<br /> and activate Authenticated state transition
        Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status

		Internal StateTree->>+Internal StateTree: Auto-activate Idle sub-state transition
		    Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status
		Note over IVoxtaClient:  Broadcast status update: Ready
	end

	rect rgb(28, 33, 44)
		Note over UVoxtaSocketHandler,UVoxtaStateTreeSubsystem: Continuous / Asynchronous Lifecycle Monitoring
		loop At any time during session (Service starts, preloads, crashes, or stops)
			UVoxtaSocketHandler->>+UVoxtaApiHandler: Push message: moduleRuntimeInstances
			UVoxtaApiHandler->>+UVoxtaStateTreeSubsystem: Broadcast & update service status (Running vs Cold)
		end
	end
```

## Conversation flow
```mermaid
---
config:
  theme: dark
---
sequenceDiagram
	Note over UCharacterSubsystem,IVoxtaClient: Public facing API

	UCharacterSubsystem->>+IVoxtaClient: Check if services are running
	IVoxtaClient-->>+UCharacterSubsystem: Return service status (Running or not)

	UCharacterSubsystem->>+IVoxtaClient: Request start conversation
		Note over UCharacterSubsystem,IVoxtaClient: Event payload includes Character ID
	UVoxtaStateTreeSubsystem->>+Internal StateTree: Forward Request event
		Note over UVoxtaStateTreeSubsystem,Internal StateTree: Forwarding includes Character ID payload

	Internal StateTree->>+Internal StateTree: Activate StartingChat sub-state
	Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status (AttemptingToStartChat)

	Internal StateTree->>+UVoxtaApiHandler: SendMessage
		Note over Internal StateTree,UVoxtaApiHandler: Start chat request (Character ID, Context)

	UVoxtaApiHandler->>+UVoxtaSocketHandler: Trigger raw SignalR message
		Note over UVoxtaApiHandler,UVoxtaSocketHandler: Action: startChat

	UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
		Note over UVoxtaApiHandler: Broadcast message: chatStarting
	Internal StateTree->>+UVoxtaStateTreeSubsystem: Store current chat sessionId 

	rect rgb(28, 33, 44)
		Note over UVoxtaSocketHandler,UCharacterSubsystem: Asynchronous Chat Loading Progress
		loop While waiting on VoxtaServer services to get started up (0% to 100%)
			UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
				Note over UVoxtaApiHandler: Broadcast message: chatLoadingMessage
			Internal StateTree->>+UVoxtaStateTreeSubsystem: NotifyChatLoadingProgress(sessionId, text, progress)
			IVoxtaClient->>+UCharacterSubsystem: Broadcast chat loading
				Note over UCharacterSubsystem: Forward broadcast to external listeners (includes progress percentage and text)
		end
	end

	UVoxtaSocketHandler->>+UVoxtaApiHandler: Return raw SignalR response
		Note over UVoxtaApiHandler: Broadcast message: chatStarted
	Internal StateTree->>+UVoxtaStateTreeSubsystem: NotifyChatStarted(sessionId, chatId)


	IVoxtaClient->>+UCharacterSubsystem: Broadcast chat started
		Note over UCharacterSubsystem: Forward broadcast to external listeners
	UVoxtaStateTreeSubsystem->>+Internal StateTree: Send event to trigger Chatting state
	Internal StateTree->>+Internal StateTree: Deactivate StartingChat sub-state<br /> and activate Chatting state transition
	Internal StateTree->>+UVoxtaStateTreeSubsystem: Update status (Chatting)
```
