# LED_Strip_Visualizer

## Hardware

* WS2812B Individually Addressable LED Strip
* ESP32
* 5V 10A Power Supply
* PC with Python



## Setup

* Install latest Python version and ensure Add to PATH is selected
* In CMD, run "pip install numpy sounddevice pyserial"
* In Python, run
    * "import sounddevice as sd"
    * "print(sd.query_devices())"
* Set audio_grabber.py to use proper device for Stereo Mix
* Update audio_grabber.py to use whatever COM port the ESP32 is connected to
* Move audio_grabber.py and run_visualizer.bat to a good location where they won't be deleted
* Update the run_visualizer.bat file with the file path for audio_grabber.py
* Create a shortcut for run_visualizer.bat in C:\Users\<USERNAME>\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
* Enjoy


## Troubleshooting

For systems that do not use the default Realtek(R) Speakers as an audio output, the stereo mix will not pick up desktop audio. My aux output Realtek 2nd Audio Output did not work, so I used Voicemeeter Banana to give virtual audio cables. Set Playback to Voicemeeter Input, Recording to Voicemeeter Out B1, and then configure accordingly in Voicemeeter to route the audio. Since Voicemeeter takes full control of the audio, the Windows audio volume adjustement doesnt work, so [this release](https://github.com/Frosthaven/voicemeeter-windows-volume/releases/) works to bind those together. 
