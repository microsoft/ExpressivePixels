<img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/EPXGitHubLockup.png" style="float: left; margin-right: 10px;" />

# Working with Expressive Pixels Firmware

You can create your own versions of the Expressive Pixels firmware in C/C++ in a variety of ways, we recommend using Visual Studio Code as the build environment for Arduino based devices.


## Arduino and Visual Studio Code
VSCode provides great integration with the Arduino platform for a great development experience, follow the below sequence to get building the Expressive Pixels firmware on a variety of devices.

### 1. Install the Arduino IDE
Integration with Arduino has a pre-requisie of installing the Arduino IDE. You can download and install it from https://www.arduino.cc/en/main/software

### 2. Install VSCode
Visual Studio code can be downloaded and installed from https://code.visualstudio.com/

### 3. Install the VSCode Arduino Extension
The VSCode Arduino extension is installed using the following the steps
Click Extensions button and search for ‘Arduino’ for Visual Studio Code by Microsoft, then click on the Install button

<img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeInstallExtn.png" style="float: left; margin-right: 10px;" height="250" />
     
### 4. Install the appropriate Board Manager
Each Arduino device type requires its own respective board download, this is done using the VSCode Arduino Extension Board Manager. Additional boards can be specified by adding to Additional URLs.

- Select the Arduino Extension and press Ctrl+Shift+P to bring up the Extension options, selecting Arduino Board Manager. 

   <img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeBoardManager.png" style="float: left; margin-right: 10px;" height="250" />

- The board manager will open on the right hand side, select the Additional URLs link on the bottom right. 

   <img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeAdditionalUrls.png" style="float: left; margin-right: 10px;" height="400" />

- This will open up User Settings, select Extensions, Arduino Configuration, and in the Additional URLs section click on Edit in settings.json.  

   <img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeAdditionalUrlsSettings.png" style="float: left; margin-right: 10px;" height="250" />
   
   For Adafruit Boards specify the below URL. Then select File..Save.

   <img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeSettingsJSON.png" style="float: left; margin-right: 10px;" height="100" />

- Go back to the Arduino Board Manager tab and click 'Refresh Package Indexes', followed by selection and installation of the board you're targetting.


## Getting the source project ready
Depending on the Arduino device you are building for, the Expressive Pixels firmware leverages available features such as Bluetooth & Flash Storage.

### The following section explains how to build the firmware for the Adafruit Circuit PlayGround BlueFruit.
Install the following Arduino library packages from Adafruit’s GitHub site. Download the and extract the ZIP packages and extract to the Arduino Library location, this may be C:\Program Files (x86)\Arduino\libraries 

-	NeoPixel library https://github.com/adafruit/Adafruit_NeoPixel
-	SPIFlash library https://github.com/adafruit/Adafruit_SPIFlash
-	Adafruit nRF52 library https://github.com/adafruit/Adafruit_nRF52_Arduino 
-	SDFat Adafruit fork library https://github.com/adafruit/SdFat
-	Copy over the Microsoft ExpressivePixelsCore library
-	Copy the Microsoft Platform_CircuitPlaygroundBluefruit library
-	If you wish to deploy and debug using a Segger JLink also copy the Segger library
-	Copy the Microsoft Device_Arduino_VSCode folder to a local folder

Going back to the VSCode application
-	In VSCode select File..Select Folder and select the folder where you placed Device_Arduino_VSCode
-	Double click ExpressivePixels.ino in the Explorer panel
-	Click the Verify sketch button

   <img src="https://github.com/microsoft/ExpressivePixels/blob/master/images/VSCodeVerify.png" style="float: left; margin-right: 10px;" height="100" />



