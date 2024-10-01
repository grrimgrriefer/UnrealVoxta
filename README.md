# An unofficial Unreal Engine client for Voxta

_Made in collaboration with [DZnnah](https://twitter.com/DZnnah)_  

### Notes
* **This client is not affiliated, associated, endorsed by, or in any way officially connected to Voxta.ai**
* This client currently suppports Voxta **v1.0.0-beta.117**, other version may / may not work.
* Internal logic based on the native Windows C++ implementation: [TalkToMeCPP](https://github.com/grrimgrriefer/TalkToMeCPP)

### Third party code Licenses
* SignalR client (MIT license): [LICENSE](./Source/SignalR/License.txt)
* RuntimeAudioImporter (MIT licensed): [LICENSE](link:Source/AudioUtility/Public/RuntimeAudioImporter/LICENSE.txt)

### Optional dependencies
* OVR lipsync: [OVR lipsync for UE 5.3](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.3)
* A2F Omniverse lipsync: [direct link](https://install.launcher.omniverse.nvidia.com/installers/omniverse-launcher-win.exe) or download via the portal [https://www.nvidia.com/en-us/omniverse/](https://www.nvidia.com/en-us/omniverse/)

### Dev Progress
* Trello board:
https://trello.com/b/Biv7Si4l/unrealvoxta

# Video guide
Full guide on how to install, how to use the templates, & info on how to make your own custom setup:  
[![YouTube UnrealVoxta tutorial video](https://thumbnailLinkHere.png)](https://www.youtubelinkhere.com/watch?v=hmmmm "YouTube UnrealVoxta tutorial video")  

# Blueprint template setup
In the content folder of the plugin you will find a setup of default blueprints. This should allow you to have a fully working setup with most features enabled in only a handful of minutes. For a step-by-step guide, I refer you to the video mentioned above.  
![TemplateBlueprints image](./Documentation/Blueprints/TemplateBlueprints.PNG "Template blueprints for example setup.")  

# Blueprint example usage
_Note, all of these snippets are from the above Template setup blueprints, feel free to explore & modify them how you see fit. For more in-depth explanation, I advice to watch the first part of the youtube tutorial._

## Voxta category
Most of the utility is accessed through the VoxtaClient subsystem, which can be globally accessed via any blueprint as following:  
![GetClient image](./Documentation/Blueprints/GetClient.PNG "Retrieve the VoxtaClient subsystem.")  

Everything related to Voxta is inside the 'Voxta' category. (also applies for other components and their utilities).  
Due to obvious reason the Events are still in the Events category but these should all be fairly obvious.  
![GetVoxtaFunctions image](./Documentation/Blueprints/GetVoxtaFunctions.PNG "Bind the registration of the playback to the event of the characters being registered.")  

Every function and component has been extensively documented, so if something is unclear, just hover the mouse over it and the tooltip should explain the usage. If it's still unclear, just ping me, as I might ofc have missed something:  
![Documentation1 image](./Documentation/Blueprints/Documentation1.PNG "Example of node documentation.")  
![Documentation2 image](./Documentation/Blueprints/Documentation2.PNG "Example of parameter documentation.")  

## Event binding pattern
In the template blueprints, Voxta Events are always bound to local events linked to functions, this helps make the eventgraph less cluttered, you'll see this pattern of hooking into events a lot:  
![BindingToEvents image](./Documentation/Blueprints/BindingToEvents.PNG "Bind the registration of the playback to the event of the characters being registered.")  

## Establishing the connection
To start the connection, you will need to provide the host address (in ipv4 format) and port
![StartConnection image](./Documentation/Blueprints/StartConnection.PNG "Start connection via UI")  
_Due to custom nature of this data, it is highly adviced to retrieve these from the user through UI in some way (such as in the example above_

## Character registration
Once the connection is established, the VoxtaClient will broadcast the characters that currently exist. This data can be used to populate a list of clickable buttons, which then can be presented to the user to give them a choice on which character they want to have an experience with.
![ListOfCharacters image](./Documentation/Blueprints/ListOfCharacters.PNG "Loop over the list of characters and create clickable buttons for every characters.")

## Starting a conversation
You can start a conversation using the ID of any character that is available. (IDs are broadcasted as soon as the voxtaclient connects, and they cal also be fetched afterwards via the VotaClient api)  
In this example, a UI widget button is injected with the characterID it is mapped to, and this allows it to start the conversation when being clicked.  
![StartConversation image](./Documentation/Blueprints/StartConversation.PNG "Star conversation with a character.")

## Register the audio-playback for an AI Character
Audio is handled with a 'VoxtaAudioPlayback' actor component. This component will automatically download the audio, generate the lipsync data, and handle the playback, for any audio that is generated for the character with a matching ID.  
![VoxtaAudioPlaybackComponent image](./Documentation/Blueprints/VoxtaAudioPlaybackComponent.PNG "Add the VoxtaAudioPlayback to an Actor")

It is important that once the conversation starts, you initialize the component with the ID of the character that you want it to represent. (i.e. especially for situations with multiple characters)  
_The example below uses index == 0, but you could also fetch the name etc from the id, to map it to a specific model etc. Depending on what kind of application you're building._  
![RegisterCharacterAsPartOfConversation image](./Documentation/Blueprints/RegisterCharacterAsPartOfConversation.PNG "Register this Actor with VoxtaAudioPlayback to Voxta")

Actors with this component automatically register themselves with the VoxtaClient subsystem and an immutable pointer to them can be easily retrieved from it using the CharacterID that was used to register it:  
![FetchAudioPlaybackFromCharacter image](./Documentation/Blueprints/FetchAudioPlaybackFromCharacter.PNG "Fetch this Actor with VoxtaAudioPlayback from Voxta")

## Sending text input from the user
The plugin supports sending user input via the microphone and via text. Above is an example of how such input is sent through via text.
![UserInput image](./Documentation/Blueprints/UserInput.PNG "Send user input to Voxta.")

## Microphone audio input
Everything regarding the Microphone audio input is handled throught the Voice Input Handler, which is available via the VoxtaClient API.  

First the socket has to be initialized with the VoxtaServer. The default settings are ideal for what the Server wants, but if the user's hardware does not support these value, you might have to expose them via some kind of settings menu.  
![MicrophoneSocket image](./Documentation/Blueprints/MicrophoneSocket.PNG "Send microphone user input to Voxta.")  

After the socket is initialized you can turn on the streaming while the server is 'WaitingForUserResponse' and then disable it again once the audio playback starts. You can also leave it running all the time, but this method uses less network bandwith of course.
![StreamingMicInput image](./Documentation/Blueprints/StreamingMicInput.PNG "Streaming microphone audio data to VoxtaServer.")  

You can hook into the Voice Input Handler to get the transcribed speech while the user is still speaking. This can then be shown in the UI, etc...
![StreamingMicInput image](./Documentation/Blueprints/OngoingTranscription.PNG "Display the speech as it's being transcribed while the user is speaking.")

The Voice Input handler also provides general utility information, such as 'input decibels', 'current state', 'microphone device label', and more.
![VoiceInputExtraInfo image](./Documentation/Blueprints/VoiceInputExtraInfo.PNG "Get info from the VoiceInputHandler.")  

## Character responses
When the VoxtaClient broadcasts that a message has been added for a character, the audio will play automatically* _(unless 'Custom lipsync is enabled')_  

Alternatively, you can add the messages to a UI chat history log. An example is shown below:  
![AddMessageToUI image](./Documentation/Blueprints/AddMessageToUI.PNG "Add a message from a character to the chat history on the screen.")
This will spawn a custom 'text message widget' and initialize it with the data after it is added to the 'message scroll box' on the UI.

Note: It is important to support the removal of messages, as the server can remove them. This ensures your UI remains in sync. Example on how to remove them from a scroll box:  
![RemoveMessageFromUI image](./Documentation/Blueprints/RemoveMessageFromUI.PNG "Remove a message from a character from the chat history on the screen.")

## Lipsync type
Both OVR and A2F are supported out of the box (A2F is still experimental). You can select which one to use in the inspector of the VoxtaAudioPlayback component.
![LipSyncSelection image](./Documentation/Blueprints/LipSyncSelection.PNG "Select a type of lipsync you want to use.")

### Lipsync animator
For MetaHumans, there is a template animator setup available that supports both Runtime OVR and A2F:  
![MetahumanLipsyncTemplateSetup image](./Documentation/Blueprints/MetahumanLipsyncTemplateSetup.PNG "Use the template animation blueprint for metahumans.")

This setup fetches the LipSyncType from the VoxtaAudioPlayback component and caches it, allowing the animator blueprint to access it.
![GetLipsyncTypeFromParent image](./Documentation/Blueprints/GetLipsyncTypeFromParent.PNG "Get the lipsynctype from the VoxtaAudioPLayback component")

### 1. OVR lipsync
Keep in mind that for OVR lipsync, you will need to add the required plugin:  
**OVR lipsync: [OVR lipsync for UE 5.3](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.3)**  
![OVRLipSyncPlugin image](./Documentation/Blueprints/OVRLipSyncPlugin.PNG "Add the OVR plugin to your project")

The component can be added by default or at runtime like this:  
![OVRAddComponent image](./Documentation/Blueprints/OVRAddComponent.PNG "Add the OVR lipsync component")

Every frame, the current visemes are fetched and applied in the animgraph
![FetchOvrVisemes image](./Documentation/Blueprints/FetchOvrVisemes.PNG "Fetch the visemes for the current frame")

Note: Extra smoothing is applied to prevent visible snapping, especially when starting and stopping speech.  
![OVRExtraSmoothing image](./Documentation/Blueprints/OVRExtraSmoothing.PNG "Apply extra smoothing.")

Inside of the AnimGraph we then blend the Visemes according to their weight and apply them for the current frame.  
![OVRvisemeBlending image](./Documentation/Blueprints/OVRvisemeBlending.PNG "Apply extra smoothing.")

### 2. Audio2Face lipsync
Keep in mind that for A2F lipsync, you will need to add the required plugin:  
A2F Omniverse lipsync: [direct link](https://install.launcher.omniverse.nvidia.com/installers/omniverse-launcher-win.exe) or download via the portal [https://www.nvidia.com/en-us/omniverse/](https://www.nvidia.com/en-us/omniverse/)  
![A2FOmniverseDownload image](./Documentation/Blueprints/A2FOmniverseDownload.PNG "Download the Omniverse application")

Then, install A2F via Omniverse as follows:  
![A2FOmniverseInstall image](./Documentation/Blueprints/A2FOmniverseInstall.PNG "Install A2F via the Omniverse application")

After installation, locate your A2F installation and run the A2F_headless client:  
![A2FOmniverseLocate image](./Documentation/Blueprints/A2FOmniverseLocate.PNG "Locate your  A2F installation via the Omniverse application")  
![Audio2FaceHeadless image](./Documentation/Blueprints/Audio2FaceHeadless.PNG "A2F headless ready")

Ensure your A2F_headless API is running and marked as "ready" for it to function correctly:
![A2FHeadless image](./Documentation/Blueprints/A2FHeadless.PNG "A2F headless ready")

Once playback in Unreal is started, the VoxtaClient will automatically attempt to connect to A2F if it is available.
![A2FCachePath image](./Documentation/Blueprints/A2FCachePath.PNG "Set the A2F cache path")

These are then applied in the animgraph using the custom node:  
![A2FCustomCurvesApplying image](./Documentation/Blueprints/A2FCustomCurvesApplying.PNG "Apply the fetched A2F curves for the current frame")

Additionally, ensure the A2F Pose mapping is correctly configured in your MetaHuman face animator blueprint.    
![A2FPoseMapping image](./Documentation/Blueprints/A2FPoseMapping.PNG "Configure the A2F posemapping to the ARKit")