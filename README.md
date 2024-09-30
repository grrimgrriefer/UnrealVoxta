# An unofficial Unreal Engine client for Voxta

_Made in collaboration with [DZnnah](https://twitter.com/DZnnah)_  

_This client is not affiliated, associated, endorsed by, or in any way officially connected to Voxta.ai_  
Internal logic based on the native Windows C++ implementation, see [TalkToMeCPP](https://github.com/grrimgrriefer/TalkToMeCPP)

### Third party code Licenses
* SignalR client (MIT license): [LICENSE](./Source/SignalR/License.txt)
* RuntimeAudioImporter (MIT licensed): [LICENSE](link:Source/AudioUtility/Public/RuntimeAudioImporter/LICENSE.txt)

### Optional dependencies
* OVR lipsync: [OVR lipsync for UE 5.3](https://github.com/grrimgrriefer/OVRLipSync/releases/tag/UE-5.3)

### Dev Progress
* Trello board:
https://trello.com/b/Biv7Si4l/unrealvoxta

# How to use
[Embedded youtube video here](youtube video)  
Full guide on how to install, how to use the templates, & info on how to make your own custom setup.


## Blueprint example usage
_Note, these examples are all from the Template setup, if you want to quickly get the template setup working, I advice to watch the first part of the youtube tutorial._

### Voxta category
All components, utility and API calls regarding Voxta are in the seperate 'Voxta' category. Due to obvious reason the Events are still in the Events category but these should all be fairly obvious.  
<span style="color:red">TODO: Image on how to get the voxta subsystem </span>  

Every function and component has been extensively documented, so if something is unclear, just hover the mouse over it and the tooltip should explain the usage. If it's still unclear, just ping me, as I might ofc have missed something.  
<span style="color:red">TODO: Image of a tooltip </span>  

In the template blueprints, Voxta Events are usually used bound to local events mapped to functions, this helps make the eventgraph less cluttered, you'll see this type of hooking into event a lot:
![alt text](./Documentation/Blueprints/BindingToEvents.JPG "Bind the registration of the playback to the event of the characters being registered.")  

### Establishing the connection
To start the connection, you will need to provide the host address (in ipv4 format) and port.
![alt text](./Documentation/Blueprints/StartConnection.JPG "Start connection via UI")  
_Due to custom nature of this data, it is highly adviced to retrieve these from the user through UI in some way (such as in the example above_

### Character registration
Once the connection is established, the VoxtaClient will broadcast the characters that currently exist. This data can be used to populate a list of clickable buttons, which then can be presented to the user to give them a choice on which character they want to have an experience with.
![alt text](./Documentation/Blueprints/ListOfCharacters.JPG "Loop over the list of characters and create clickable buttons for every characters.")

### Starting a conversation
<span style="color:red">TODO: Figure out how the conversations are started lmao. Should be inside of the character button, maybe? idk?</span> 

### Register the audio-playback for an AI Character
<span style="color:red">TODO: Add an image of the VoxtaAudioPlayback component here.</span>  
Audio is handled with a 'VoxtaAudioPlayback' actor component. This component will automatically download the audio, generate the lipsync data, and handle the playback, for any audio that is generated for the character with a matching ID.

It is important that once the conversation starts, you initialize the component with the ID of the character that you want it to represent. (i.e. especially for situations with multiple characters)  
_The example below uses index == 0, but you could also fetch the name etc from the id, to map it to a specific model etc. Depending on what kind of application you're building._
![alt text](./Documentation/Blueprints/AudioPlaybackRegistering.JPG "Connect Actor with VoxtaAudioPlayback to Voxta")

### Sending text input from the user
The plugin supports sending user input via the microphone and via text. Above is an example of how such input is sent through via text.
![alt text](./Documentation/Blueprints/UserInput.JPG "Send user input to Voxta.")

### Microphone audio input
Everything regarding the Microphone audio input is handled throught the Voice Input Handler, which is available via the Voxta Client API.  

First the socket has to be initialized with the VoxtaServer. The default settings are ideal for what the Server wants, but if the user's hardware does not support these value, you might have to expose them via some kind of settings menu.  
![alt text](./Documentation/Blueprints/MicrophoneSocket.JPG "Send microphone user input to Voxta.")  

After the socket is initialized you can turn on the streaming while the server is 'WaitingForUserResponse' and then disable it again once the audio playback starts. You can also leave it running all the time, but this method uses less network bandwith of course.
![alt text](./Documentation/Blueprints/StreamingMicInput.JPG "Streaming microphone audio data to VoxtaServer.")  

You can hook into the Voice Input Handler to get the transcribed speech while the user is still speaking. This can then be shown in the UI, etc...
![alt text](./Documentation/Blueprints/OngoingTranscription.JPG "Display the speech as it's being transcribed while the user is speaking.")

The Voice Input handler also provides general utility info, such as 'input decibels', 'current state', 'microphone device label', etc...
![alt text](./Documentation/Blueprints/VoiceInputExtraInfo.JPG "Get info from the VoiceInputHandler.")  

### Character responses
When the VoxtaClient broadcasts that a message has been added for a character, the audio will be played automatically* _(unless 'Custom lipsync is enabled')_  

However, you can also decide to add the messages to a UI chat history log. An example of how to do this can be seen below:  
![alt text](./Documentation/Blueprints/AddMessageToUI.JPG "Add a message from a character to the chat history on the screen.")
This will spawn a custom 'text message widget' and initialize it with the data after it is added to the 'message scroll box' on the UI.

Note: It is important to also support removal of messages, as the Server can remove messages, so it's important if you want to keep it in sync. Example on how to remove them from a scroll box:  
![alt text](./Documentation/Blueprints/RemoveMessageFromUI.JPG "Remove a message from a character from the chat history on the screen.")


--
TODO: change UI blueprint so it fetches the VoxtaAudioPlayback from the VoxtaClient after it has already been registered instead of grabbing it via the tag in the world. As I really hate tags haha
--

* disclaimer
  * No official client, not connected to Voxta team in any way.
  * Preview version, things can & will break.
  * Don't use this in projects in production.
  * Tested & developed against v117
* template setup in empty project
  * Do it in a blueprint only project, (can be C++ but you can always change it later anyway)
  * The first thing to do when starting the engine, is to shut the bitch down.
  * Make download the release version of the plugin (unzip, if you use git, make sure to also pull the LFS files)
* quick overview on how to setup the template
  * Configure GameMode with the HUD template
  * Start up VoxtaServer
    * Make sure that TextGen, AudioGen & Microphone detection is enabled
  * Add metahuman with OVR lipsync
    * How to add a metahuman
    * How to add OVR lipsync
  * Add Audio2Face to the metahuman
    * How to install the A2F module
* set-by-step setup
  * Enable only text gen
  * Setup a basic hud with messages
  * Add audio playback (no lipsync)
    * Make an icon light up with the decibels
  * Add microphone input (with live transcription etc...)
  * Add a simple model with OVR lipsync (no metahuman)
    * Show again where the OVR module can be downloaded from & installed.
    * Show how to setup the animator manually, with the custom OVR viseme animations
    * Explain how the smoothing works & that it is optional, depends a bit on the style you're going with
  * Add a metahuman
    * Show how to use the crowd face animation
    * Show how to retarget a simple idle animation
* C++ guide
  * Make a derived type of the Actor that call the VoxtaServer, and initializes the whole stuff.
    * No visuals, but make sure to include microphone, and audio playback etc...


