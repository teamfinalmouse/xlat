# XLAT Mouse Latency Measurement Tool

## Introduction
The XLAT Mouse Latency Measurement Tool by Finalmouse is engineered to accurately measure the latency of wired and wireless mice. This manual provides a step-by-step guide on how to set up and use this tool.

## Required hardware
Ensure you have at least the following hardware available:
1. Finalmouse XLAT or STM32F746G-DISCO devkit
2. Micro-USB to USB-A Adapter
3. Mini-USB Cable
4. The USB mouse you want to test

## Setup Procedure
### 1. Power Supply:
   - Connect the mini-USB cable to the XLAT device and a power source to supply power to the XLAT device.

### 2. USB Connection: 
   - For wired mice: Connect the USB plug of the mouse directly to the USB OTG port of the XLAT tool using the supplied Micro-USB to USB-A adapter.
   - For wireless mice: Connect the USB plug of the wireless receiver dongle to the USB OTG port of the XLAT tool using the supplied Micro-USB to USB-A adapter.

### 3. Special Cable Connection:
   - Create a custom cable with two connectors: one for GROUND (black cable) and one for the left button click (red cable).
   - Modify the mouse to have connectors for the GROUND and left button click.
   - Connect the GROUND (black cable) and left button click (red cable) of the mouse to the respective ports on the XLAT tool.

### 4. Device Recognition:
   - Upon successful connection of the mouse via USB, the XLAT tool should display the device name along with the VID:PID (Vendor ID: Product ID) in the top right corner of the screen.

## Accuracy of XLAT
XLAT boasts a high degree of accuracy in its measurements, due to the firmware running on an onboard microcontroller. This low-level, lightweight design ensures highly accurate and reliable measurements.
The firmware is open source, and it's contained in the git repository you're looking at right now.

## How XLAT Measures Click Latency
XLAT measures click latency by accurately measuring the time between the mouse button click (measured electrically) and the corresponding USB packet coming in, sent by the mouse, which contains the button click data. This measurement is reported in microseconds (µs).

## User Interface
- **CLEAR Button**: Clears the measurement results and allows you to start over.
- **REBOOT Button**: Reboots the device and re-initializes the connected USB device.
- **SETTINGS Button**: Takes you to the settings page where you can configure the tool.
- **TRIGGER Button**: Activates the "auto-trigger" functionality where XLAT will attempt to automatically click the mouse for you, eliminating the need for manual clicks.

## Measurement Procedure
### 1. Initiate Measurement:
   - Press the "TRIGGER" button if your mouse supports the auto-trigger functionality, or manually click the mouse to begin measuring click latency.
   - (Provide any additional steps on how to initiate the measurement, e.g., software to launch, etc.)

### 2. Analyze Results:
   - For every click registered, results will be displayed on screen immediately. You'll find the latest latency measurements, as well as the average and the amount of registered clicks.

### 3. Resetting and Re-testing:
   - Use the "CLEAR" button to clear previous measurements and start over, or the "REBOOT" button if a re-initialization of the connected USB device is needed.

## Troubleshooting
- **LCD Issues**:
    - If the LCD doesn't properly initialize or contains artifacts, reboot the device by pressing the "REBOOT" button or power cycling the XLAT device.

## Support
For further assistance or inquiries, contact Finalmouse support via email at support@finalmouse.com or on Discord.

## License
The software is licensed under the GPLv3 license. See `COPYING` for more information.
The libraries used are licensed under their respective licenses. See `LICENSES` / `LICENSE.txt` or similar for more information.

## Conclusion
The XLAT Mouse Latency Measurement Tool provides an accurate method for measuring mouse latency, aiding in achieving optimal performance for a competitive edge.
