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
