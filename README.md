# An unofficial Voxta plugin for Unreal Engine

_Made in collaboration with [DZnnah](https://twitter.com/DZnnah)_  

> [!CAUTION]
> **This plugin is not affiliated, associated, endorsed by, or in any way officially connected to Voxta.ai**

> [!WARNING]  
> * This plugin currently supports Voxta **v1.0.0-beta.117**, other versions may / may not work.
> * This plugin currently only works in Unreal Engine 5.3

### Third-party code Licenses
* SignalR client (MIT license): [LICENSE](./Source/SignalR/License.txt)  
* RuntimeAudioImporter (MIT licensed): [LICENSE](./Source/VoxtaAudioUtility/Public/RuntimeAudioImporter/LICENSE.txt)  

### Optional dependencies
* OVR lipsync: [OVR lipsync for UE 5.3](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.3)
* A2F Omniverse lipsync: [direct link](https://install.launcher.omniverse.nvidia.com/installers/omniverse-launcher-win.exe) or download via the portal [https://www.nvidia.com/en-us/omniverse/](https://www.nvidia.com/en-us/omniverse/)

> [!TIP]
> Internal logic based on the native Windows C++ implementation: [TalkToMeCPP](https://github.com/grrimgrriefer/TalkToMeCPP). You can use this if you want the C++ code without all the Unreal Engine dependencies.

### Dev Progress
* Trello board:
https://trello.com/b/Biv7Si4l/unrealvoxta

## Voxta TOS
_This plugin has no additional Terms of Service, however Voxta's official Terms of Service still apply._

> [!IMPORTANT]  
> * Voxta has been developed as an experiment, and no guarantees whatsoever are offered.
> * The content you generate with this software must be legal in the United States and your country of residence.
> * The authors have no access to any of the data generated within this tool. However, keep in mind that remote services such as OpenAI may collect your data. Make sure to read their terms of services before using their software.
> * Furthermore, those use cases are prohibited:
>   * Depictions of minors are prohibited.
>   * Simulations of forced sexual interactions with non-consenting characters are prohibited.
>   * Torture and psychological violence.
>   * Impersonation of real people.
>   * Using the software to deceive someone, it is mandatory to inform users of the limitations and restrictions of the AI.
> * Additionally, if using Voxta Cloud, there are additional terms:
>   * You will not abuse the service and will not use it to harm others.
>   * You will not use the service to generate content that is illegal or harmful.
>   * You must immediately disclose any vulnerability to the Voxta team on Discord.
>   * You will not take action that can harm the service, such as using it to generate a large amount of content in a short period of time.

> [!NOTE]  
> * I have no power to enforce these rules, but they might revoke your access to future releases / cloud utility if you break them.
> * AI characters are not real. Remember that they can say make up things that can be hurtful, and simply false. Please seek help if you feel alone, or if you develop an unhealthy relationship with the AI.
> * In terms of the plugin itself, anything that doesn't break the [MIT license](./LICENSE) is fair game, have fun with it.

## Video guide
Full guide on how to install, how to use the templates, & info on how to make your own custom setup:  
[![YouTube UnrealVoxta tutorial video](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/YoutubeThumbnailGithub.PNG&resolveLfs=true&%24format=octetStream)](https://youtu.be/kWv_00FuTvg "YouTube UnrealVoxta tutorial video")  
https://youtu.be/kWv_00FuTvg

## Enable Voice Input
This plugin uses Unreal's fancy VoiceInput system, which makes the system far more reliable. However this has to be turned on for projects as it's disabled by default. You can keep it disabled if you only want to use text input, but if you want audio (microphone) input too,  
this is how you enable it:
1. Locate the Config folder in your project
2. Locate the DefaultEngine.ini
3. Add this below snippet to it.
```
[Voice]
bEnabled=true
```
![EnableVoiceInput image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/EnableVoiceInput.PNG&resolveLfs=true&%24format=octetStream "Manually enable VoiceInput for your project.")  

## Template setup
In the content folder of the plugin you will find a setup of default blueprints. This should allow you to have a fully working setup with most features enabled in only a handful of minutes. For a step-by-step guide, I refer you to the video mentioned above.  
![TemplateBlueprints image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/TemplateBlueprints.PNG&resolveLfs=true&%24format=octetStream "Template blueprints for example setup.")  

## Tests
The core functionality is covered by Integration test, more extensive testing is still WIP, but this should catch most critical issues that would break essential logic.  
![Tests image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Tests.PNG&resolveLfs=true&%24format=octetStream "Running Voxta tests.")  

## Blueprint nodes example usage
_Note, all of these snippets are from the above Template setup blueprints, feel free to explore & modify them how you see fit. For more in-depth explanation, I advise watching the first part of the youtube tutorial._

### Voxta category
Most of the utility is accessed through the VoxtaClient subsystem, which can be globally accessed via any blueprint as following:  
![GetClient image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/GetClient.PNG&resolveLfs=true&%24format=octetStream "Retrieve the VoxtaClient subsystem.")  

Everything related to Voxta is inside the 'Voxta' category. (also applies for other components and their utilities).  
Due to obvious reasons, the Events are still in the Events category but these should all be fairly obvious.  
![GetVoxtaFunctions image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/GetVoxtaFunctions.PNG&resolveLfs=true&%24format=octetStream "Bind the registration of the playback to the event of the characters being registered.")  

Every function and component has been extensively documented, so if something is unclear, just hover the mouse over it and the tooltip should explain the usage. If it's still unclear, just ping me, as I might ofc have missed something:  
![Documentation1 image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/Documentation1.PNG&resolveLfs=true&%24format=octetStream  "Example of node documentation.")  
![Documentation2 image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/Documentation2.PNG&resolveLfs=true&%24format=octetStream "Example of parameter documentation.")  

### Event binding pattern
In the template blueprints, Voxta Events are always bound to local events linked to functions, this helps make the eventgraph less cluttered, you'll see this pattern of hooking into events a lot:  
![BindingToEvents image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/BindingToEvents.PNG&resolveLfs=true&%24format=octetStream "Bind the registration of the playback to the event of the characters being registered.")  

### Establishing the connection
To start the connection, you will need to provide the host address (in ipv4 format) and port
![StartConnection image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/StartConnection.PNG&resolveLfs=true&%24format=octetStream "Start connection via UI")  
_Due to custom nature of this data, it is highly adviced to retrieve these from the user through UI in some way (such as in the example above_

### Character registration
Once the connection is established, the VoxtaClient will broadcast the characters that currently exist. This data can be used to populate a list of clickable buttons, which then can be presented to the user to give them a choice on which character they want to have an experience with.
![ListOfCharacters image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/ListOfCharacters.PNG&resolveLfs=true&%24format=octetStream "Loop over the list of characters and create clickable buttons for every characters.")

### Starting a conversation
You can start a conversation using the ID of any character that is available. (IDs are broadcasted as soon as the voxtaclient connects, and they can also be fetched afterwards via the VoxtaClient api)  
In this example, a UI widget button is injected with the characterID it is mapped to, and this allows it to start the conversation when being clicked.  
![StartConversation image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/StartConversation.PNG&resolveLfs=true&%24format=octetStream "Star conversation with a character.")

### Register the audio-playback for an AI Character
Audio is handled with a 'VoxtaAudioPlayback' actor component. This component will automatically download the audio, generate the lipsync data, and handle the playback, for any audio that is generated for the character with a matching ID.  
![VoxtaAudioPlaybackComponent image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/VoxtaAudioPlaybackComponent.PNG&resolveLfs=true&%24format=octetStream "Add the VoxtaAudioPlayback to an Actor")

It is important that once the conversation starts, you initialize the component with the ID of the character that you want it to represent. (i.e. especially for situations with multiple characters)  
_The example below uses index == 0, but you could also fetch the name etc. from the id, to map it to a specific model etc. Depending on what kind of application you're building._  
![RegisterCharacterAsPartOfConversation image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/RegisterCharacterAsPartOfConversation.PNG&resolveLfs=true&%24format=octetStream "Register this Actor with VoxtaAudioPlayback to Voxta")

Actors with this component automatically register themselves with the VoxtaClient subsystem. Right after they 'Initialize' themselves, the VoxtaClient will broadcast the notification that the character of that ID has been claimed by that audioPlaybackHandler.  
In the example below the UI stores the reference to it, so it can display an icon while the character is speaking.  
![RetrieveAudioPlaybackFromVoxtaClient image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/RetrieveAudioPlaybackFromVoxtaClient.PNG&resolveLfs=true&%24format=octetStream "Store the reference of the VoxtaAudioPlayback component after it has registered itself.")

### Sending text input from the user
The plugin supports sending user input via the microphone and via text. Above is an example of how such input is sent through via text.
![UserInput image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/UserInput.PNG&resolveLfs=true&%24format=octetStream "Send user input to Voxta.")

### Microphone audio input
Everything regarding the Microphone audio input is handled throught the Voice Input Handler, which is available via the VoxtaClient API.  

First the socket has to be initialized with the VoxtaServer. The default settings are ideal for what the Server wants, but if the user's hardware does not support these value, you might have to expose them via some kind of settings menu.  
![MicrophoneSocket image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/MicrophoneSocket.PNG&resolveLfs=true&%24format=octetStream "Send microphone user input to Voxta.")  

After the socket is initialized, you can turn on the streaming while the server is 'WaitingForUserResponse' and then disable it again once the audio playback starts. You can also leave it running all the time, but this method uses less network bandwith of course.
![StreamingMicInput image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/StreamingMicInput.PNG&resolveLfs=true&%24format=octetStream "Streaming microphone audio data to VoxtaServer.")  

You can hook into the Voice Input Handler to get the transcribed speech while the user is still speaking. This can then be shown in the UI, etc...
![StreamingMicInput image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/OngoingTranscription.PNG&resolveLfs=true&%24format=octetStream "Display the speech as it's being transcribed while the user is speaking.")

The Voice Input handler also provides general utility information, such as 'input decibels', 'current state', 'microphone device label', and more.
![VoiceInputExtraInfo image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/VoiceInputExtraInfo.PNG&resolveLfs=true&%24format=octetStream "Get info from the VoiceInputHandler.")  

### Character responses
When the VoxtaClient broadcasts that a message has been added for a character, the audio will play automatically* _(unless 'Custom lipsync is enabled')_  

Alternatively, you can add the messages to a UI chat history log. An example is shown below:  
![AddMessageToUI image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/AddMessageToUI.PNG&resolveLfs=true&%24format=octetStream "Add a message from a character to the chat history on the screen.")
This will spawn a custom 'text message widget' and initialize it with the data after it is added to the 'message scroll box' on the UI.

Note: It is important to support the removal of messages, as the server can remove them. This ensures your UI remains in sync. Example on how to remove them from a scroll box:  
![RemoveMessageFromUI image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/RemoveMessageFromUI.PNG&resolveLfs=true&%24format=octetStream "Remove a message from a character from the chat history on the screen.")

### Lipsync type
Both OVR and A2F are supported out of the box (A2F is still experimental). You can select which one to use in the inspector of the VoxtaAudioPlayback component.
![LipSyncSelection image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/LipSyncSelection.PNG&resolveLfs=true&%24format=octetStream "Select a type of lipsync you want to use.")

#### Lipsync animator
For MetaHumans, there is a template animator setup available that supports both Runtime OVR and A2F:  
![MetahumanLipsyncTemplateSetup image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/MetahumanLipsyncTemplateSetup.PNG&resolveLfs=true&%24format=octetStream "Use the template animation blueprint for metahumans.")

This setup fetches the LipSyncType from the VoxtaAudioPlayback component and caches it, allowing the animator blueprint to access it.
![GetLipsyncTypeFromParent image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/GetLipsyncTypeFromParent.PNG&resolveLfs=true&%24format=octetStream "Get the lipsynctype from the VoxtaAudioPLayback component")

#### 1. OVR lipsync
Keep in mind that for OVR lipsync, you will need to add the required plugin:  
**OVR lipsync: [OVR lipsync for UE 5.3](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.3)**  
![OVRLipSyncPlugin image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/OVRLipSyncPlugin.PNG&resolveLfs=true&%24format=octetStream "Add the OVR plugin to your project")

The component can be added by default or at runtime like this:  
![OVRAddComponent image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/OVRAddComponent.PNG&resolveLfs=true&%24format=octetStream "Add the OVR lipsync component")

Every frame, the current visemes are fetched and applied in the animgraph
![FetchOvrVisemes image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/FetchOvrVisemes.PNG&resolveLfs=true&%24format=octetStream "Fetch the visemes for the current frame")

Note: Extra smoothing is applied to prevent visible snapping, especially when starting and stopping speech.  
![OVRExtraSmoothing image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/OVRExtraSmoothing.PNG&resolveLfs=true&%24format=octetStream "Apply extra smoothing.")

Inside of the AnimGraph we then blend the Visemes according to their weight and apply them for the current frame.  
![OVRvisemeBlending image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/OVRvisemeBlending.PNG&resolveLfs=true&%24format=octetStream "Apply extra smoothing.")

#### 2. Audio2Face lipsync
Keep in mind that for A2F lipsync, you will need to add the required plugin:  
A2F Omniverse lipsync: [direct link](https://install.launcher.omniverse.nvidia.com/installers/omniverse-launcher-win.exe) or download via the portal [https://www.nvidia.com/en-us/omniverse/](https://www.nvidia.com/en-us/omniverse/)  
![A2FOmniverseDownload image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FOmniverseDownload.PNG&resolveLfs=true&%24format=octetStream "Download the Omniverse application")

Then, install A2F via Omniverse as follows:  
![A2FOmniverseInstall image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FOmniverseInstall.PNG&resolveLfs=true&%24format=octetStream "Install A2F via the Omniverse application")

After installation, locate your A2F installation and run the A2F_headless client:  
![A2FOmniverseLocate image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FOmniverseLocate.PNG&resolveLfs=true&%24format=octetStream "Locate your  A2F installation via the Omniverse application")  
![Audio2FaceHeadless image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/Audio2FaceHeadless.PNG&resolveLfs=true&%24format=octetStream "A2F headless ready")

Ensure your A2F_headless API is running and marked as "ready" for it to function correctly:
![A2FHeadless image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FHeadless.PNG&resolveLfs=true&%24format=octetStream "A2F headless ready")

Once playback in Unreal is started, the VoxtaClient will automatically attempt to connect to A2F if it is available.
![A2FCachePath image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FCachePath.PNG&resolveLfs=true&%24format=octetStream "Set the A2F cache path")

These are then applied in the animgraph using the custom node:  
![A2FCustomCurvesApplying image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FCustomCurvesApplying.PNG&resolveLfs=true&%24format=octetStream "Apply the fetched A2F curves for the current frame")

Additionally, ensure the A2F Pose mapping is correctly configured in your MetaHuman face animator blueprint.    
![A2FPoseMapping image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/A2FPoseMapping.PNG&resolveLfs=true&%24format=octetStream "Configure the A2F posemapping to the ARKit")

.  
.  
.  
.  
.  
.  
.  

# -------------------



# Voxta Utility Modules

## Overview

The Voxta plugin consists of several specialized modules that provide specific functionality for character animation and lip-sync capabilities in Unreal Engine. Three key modules are:

1. **VoxtaUtility_A2F** - Integration with NVIDIA's Audio2Face technology for high-quality facial animation.
2. **VoxtaUtility_OVR** - Integration with Oculus OVR lip-sync technology.
3. **UnrealVoxtaEditor** - Editor-specific functionality, primarily focusing on custom animation graph nodes.

These modules are designed following SOLID principles, keeping different functionalities separate for better maintainability and optional integration depending on project needs.


## Integration

These modules are designed to work together within the Voxta plugin framework, but they can also be used independently based on project requirements. The main integration points are:

1. **Common Interface** - Both lip-sync modules implement a common `ILipSyncBaseData` interface
2. **Animation Integration** - The animation node from VoxtaUtility_A2F is exposed in the editor via UnrealVoxtaEditor
3. **Modular Design** - Projects can choose to use either A2F, OVR, or both lip-sync technologies