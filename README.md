# An Unreal Engine plugin for Voxta

_By GrrimGrriefer, DZnnah and Barty._  
_Development progress board: https://trello.com/b/Biv7Si4l/unrealvoxta_

**⚠️ AI characters are not real, always use common sense! ⚠️**  

> [!CAUTION]  
> This plugin is not affiliated, associated, endorsed by, or in any way officially connected to Voxta.ai  
>  
> This plugin currently supports [Voxta](https://voxta.ai/) version **v1.0.0-beta.132** (API **2024-11**), other versions are unlikely to work.  
> This plugin currently supports Unreal Engine **5.5**, other versions are unlikely to work (out of the box).  
>  
> **If you connect to VoxtaCloud services, their [Terms of Service](https://doc.voxta.ai/cloud/terms/) apply.**  
> 
> Please do not use this plugin to create anything illegal or malicious.

**Third party licenses:**  
* SignalR client [MIT licensed](./Source/SignalR/License.txt)  
* RuntimeAudioImporter [MIT licensed](./Source/VoxtaAudioUtility/Public/RuntimeAudioImporter/LICENSE.txt)  

## Latest patch overview

Quick 2min overview on the main changes in the most recent patch (**version 0.1.1**)  
[![youtubeThumbnailPatch011 image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/youtubeThumbnailPatch011.PNG&resolveLfs=true&%24format=octetStream)](https://youtu.be/LINK_HERE "youtubeThumbnailPatch011 image")  
https://youtu.be/LINK_HERE

## Optional prerequisites

### Lipsync
ℹ️ _If you do not plan on using Lipsync, you can ignore this section._  

* [OVR lipsync for UE 5.5](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.5)  
Unzip this modified OVR lipsync in your plugins folder.
* [A2F Omniverse lipsync](https://www.nvidia.com/en-us/omniverse/) : [installer DIRECT LINK](https://install.launcher.omniverse.nvidia.com/installers/omniverse-launcher-win.exe)  
Experimental support, see [installation details](#installing-audio2face-lipsync) below.

### Microphone input
ℹ️ _If you do not plan on using Voice input, you can ignore this section._  

Audio input for this plugin relies on Unreal's VoiceInput system, which has to be turned on.  
1. Locate the Config folder in your project
2. Locate the DefaultEngine.ini
3. Add this below snippet to it.
```
[Voice]
bEnabled=true
```
![EnableVoiceInput image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.0/Images/EnableVoiceInput.PNG&resolveLfs=true&%24format=octetStream "EnableVoiceInput image.")  

## Tests
[Basic test coverage](./Tests/VoxtaClientTests.cpp) (85 tests atm) of the main VoxtaClient public C++ API. Additional test coverage for the blueprint API and for audio input & output are scheduled for upcoming releases.  

**Note:**  
All tests are integration tests and require a valid instance of VoxtaServer to be running (configured on localhost but that can be easily modified).  

⚠️⚠️ **Be mindful running tests when using VoxtaCloud services, especially cloud TTS audio. It is highly adviced to only execute relevant tests during development to avoid draining cloud credits.** ⚠️⚠️

![Tests image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Tests.PNG&resolveLfs=true&%24format=octetStream "Tests image.")

## Getting started [C++]

### Doxygen

All header files are fully documented and easily navigatable via the [online doxygen](https://grrimgrriefer.github.io/UnrealVoxtaDocs/hierarchy.html)  
For a rough overview of what is included:

#### Sequence diagram for each module, displaying how its internal classes interact with each other.

![docsSequenceDiagrams image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/docsSequenceDiagrams.PNG&resolveLfs=true&%24format=octetStream "docsSequenceDiagrams image.")

#### High level bullet-point list of each modules purpose, features and dependencies.

![docsModuleExplanation image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/docsModuleExplanation.PNG&resolveLfs=true&%24format=octetStream "docsModuleExplanation image.")

#### Boilerplate snippets that can be used to start writing your own custom code.

![docsBoilerplateCodeSnippets image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/docsBoilerplateCodeSnippets.PNG&resolveLfs=true&%24format=octetStream "docsBoilerplateCodeSnippets image.")

#### Per-class/struct pages, including inheritance diagrams, summaries for each public function & property.

![docsInheritanceDiagrams image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/docsInheritanceDiagrams.PNG&resolveLfs=true&%24format=octetStream "docsInheritanceDiagrams image.")

#### Easily navigatable overview of all classes and structs included in the plugin

![docsClassHierarchy image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/docsClassHierarchy.PNG&resolveLfs=true&%24format=octetStream "docsClassHierarchy image.")

### Markdown links

The high-level overviews are located in markdowns as part of the repository. These can be viewed without going to the doxygen link if needed.  

Links are below:

* [UnrealVoxta](./Source/UnrealVoxta/UnrealVoxta.md): Main api to interact with the plugin.
* [VoxtaData](./Source/VoxtaData/VoxtaData.md): Contains data structures.  
(native c++ structs, classes, enums; along with UObject data wrappers)
* [VoxtaAudioUtility](./Source/VoxtaAudioUtility/VoxtaAudioUtility.md): Logic specific to audio input & output.
* [LogUtility](./Source/LogUtility/LogUtility.md): Logging-macro overrides to exclude sensitive info from logs & crashreports.
* [SignalR](./Source/SignalR/SignalR.md): Modified 3rd party library for websocket communication via the SignalR protocol.
* [UnrealVoxtaEditor](./Source/UnrealVoxtaEditor/UnrealVoxtaEditor.md): Custom animation node required for in-editor preview of non-streaming A2F.
* [VoxtaUtility_OVR](./Source/VoxtaUtility_OVR/VoxtaUtility_OVR.md): Custom wrapper to allow automatic seamless connection to the UnrealVoxta api.
* [VoxtaUtility_A2F](./Source/VoxtaUtility_A2F/VoxtaUtility_A2F.md): Custom logic to upload audio to local A2F, then download and parse the blendshapes and apply them at runtime.

## Getting started [Blueprints]

Due to the alpha nature of the plugin, only C++ documentation is provide. However, each blueprint node maps 1:1 to a UFUNCTION and will have the same discription visible when hovered.  

![summaryOnHover image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/summaryOnHover.PNG&resolveLfs=true&%24format=octetStream "summaryOnHover image.") 

A full in-depth breakdown on each node will be made in video-format once the plugin reaches beta (v0.2)  

For alpha usage, it is adviced to use the new Modular Template UI as reference.

## Template screens

**Note**: All source files for the template are included (.psd) to allow for easy reskinning. Do keep in mind that for the Beta all UI will be reskinned (again) by us too. 
![Template_filesNfolders image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_filesNfolders.PNG&resolveLfs=true&%24format=octetStream "Template_filesNfolders image.")  

### Activate the Template gamemode

Either select the **BP_ExampleGameMode**, or manually select the example hud in your own GameMode if you have one already.  

![Template_Gamemode image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_Gamemode.PNG&resolveLfs=true&%24format=octetStream "Template_Gamemode image.")  

### Template status icons  
These icons are visible in the top right of the template HUD, and display Voice input, Audio output, VoxtaServer status, connection, and settings.  
In general:
- Grey = Inactive
- Cyan = Idle
- Green = Active
- Orange = Busy (server only)

![Template_StatusIcons image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_StatusIcons.PNG&resolveLfs=true&%24format=octetStream "Template_StatusIcons image.")  

### Template settings menu
This menu contains all global meta-configuration elements. Host IPv4, ServerAPI version, configurable microphone gain & noisegate, LogCensoring, global audio (2d)...  
![Template_Settings image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_Settings.PNG&resolveLfs=true&%24format=octetStream "Template_Settings image.")  

After connection:  
![Template_Connected image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_Connected.PNG&resolveLfs=true&%24format=octetStream "Template_Connected image.")  

### Template start chat menu  
Screen with a dropdown to select the character, and an option to change the chat context. A button is available to edit the character in the system-browser.  
**Note:** Thumbnails will be resized if too large, but will preserve their aspect ratio.
![Template_StartChat image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_StartChat.PNG&resolveLfs=true&%24format=octetStream "Template_StartChat image.")  

### Template chat handle
Screen displaying the current chat, minimal UI to avoid blocking the rest of the screen.  
![Template_ChatWindow image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_ChatWindow.PNG&resolveLfs=true&%24format=octetStream "Template_ChatWindow image.")  

### Template chat settings screen
Can be triggered with the cogwheel, shows the current chat and context can be modified. Also displays VoxtaServer services status.  
![Template_ChatSettings image](https://dev.azure.com/grrimgrriefer/b22f0465-b773-42a3-9f3e-cd0bfb60dd2f/_apis/git/repositories/c5225fce-9f91-406e-9a06-07514397eb7d/items?path=/Documentation/0.1.1/Images/Template_ChatSettings.PNG&resolveLfs=true&%24format=octetStream "Template_ChatSettings image.")  

## Installing Audio2Face lipsync

⚠️⚠️ **Be mindful that Audio2Face is experimental and will receive a full overhaul before going to beta. Do NOT build anything you need to support medium/long term with this.** ⚠️⚠️

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
