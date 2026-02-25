<h1 align="center"> Finalmouse XLAT Mouse Latency Measurement Tool </h1>

<div align="center">

![XLAT logo](https://github.com/teamfinalmouse/xlat/blob/main/img/xlat_logo_black_33.png?raw=true)

</div>

![Build Status](https://github.com/teamfinalmouse/xlat/actions/workflows/build.yml/badge.svg)
![License](https://img.shields.io/badge/license-GPLv3-blue.svg)


# Table of Contents

- [Introduction](#introduction)
- [Reason for creating XLAT](#reason-for-creating-xlat)
- [Required hardware](#required-hardware)
- [Setup Procedure](#setup-procedure)
   * [1. Power Supply:](#1-power-supply)
   * [2. USB Connection: ](#2-usb-connection)
   * [3. Special Cable Connection:](#3-special-cable-connection)
   * [4. Device Recognition:](#4-device-recognition)
- [Accuracy of XLAT](#accuracy-of-xlat)
- [How XLAT Measures Click Latency](#how-xlat-measures-click-latency)
- [User Interface](#user-interface)
- [Measurement Procedure](#measurement-procedure)
   * [1. Initiate Measurement:](#1-initiate-measurement)
   * [2. Analyze Results:](#2-analyze-results)
   * [3. Resetting and Re-testing:](#3-resetting-and-re-testing)
- [Troubleshooting](#troubleshooting)
- [Support](#support)
- [License](#license)
- [Conclusion](#conclusion)

## Introduction
The XLAT Mouse Latency Measurement Tool by Finalmouse is engineered to accurately measure the latency of wired and wireless mice. This manual provides a step-by-step guide on how to set up and use this tool.

XLAT overview video:<br/>
[![XLAT overview video](http://img.youtube.com/vi/RyWXZj3j_xU/0.jpg)](http://www.youtube.com/watch?v=RyWXZj3j_xU "XLAT overview")


## Reason for creating XLAT
The inception of the XLAT Mouse Latency Measurement Tool stemmed from a need to address the shortcomings of existing latency measurement methodologies, particularly concerning mouse latency. Here are the key motivators behind the creation of XLAT:

1. **Specific Focus on Mouse Latency**:
  - Previous methods were either rudimentary or did not focus specifically on mouse latency, making it difficult to obtain precise measurements for this critical aspect of system performance.

2. **System-wide Latency Measurement**:
  - Other techniques measured complete system latency, encompassing OS, GPU, and screen delay, which could obfuscate the true latency of the mouse itself.

3. **Limitations of Audio-Based Methods**:
  - Some methods relied on audio, which is too slow to measure the minuscule latency numbers associated with mouse clicks accurately.

4. **Lack of Accessible, High-Quality Alternatives**:
  - While accurate measurements could be obtained using high-end equipment like expensive logic analyzers, USB analyzers, or oscilloscopes, these tools are not only costly but also complex to use. There was a distinct lack of easy-to-use, high-quality, and affordable alternatives in the market.

5. **Single Source of Truth**:
  - XLAT is intended as a "single source of truth" for mouse latency measurement. It's repeatable, affordable, easy to use, and can be independently verified for accuracy since it's open source. This level of reliability and transparency positions XLAT as an invaluable tool for those seeking precise and trustworthy mouse latency measurements. Additionally, check out our [latency database](https://github.com/teamfinalmouse/xlat/wiki/Latency-database)

XLAT was developed to fill these gaps, providing a specialized, accessible, and cost-effective solution for accurately measuring mouse latency, making it an invaluable tool for both enthusiasts and professionals alike.


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

##  User Interface
- **CLEAR Button**: Clears the measurement results and allows you to start over.
- **REBOOT Button**: Reboots the device and re-initializes the connected USB device.
- **SETTINGS Button**: Takes you to the settings page where you can configure the tool.
- **TRIGGER Button**: Activates the "auto-trigger" functionality where XLAT will attempt to automatically click the mouse for you, eliminating the need for manual clicks.

## Measurement Procedure
### 1. Initiate Measurement:
   - Press the "TRIGGER" button if your mouse supports the auto-trigger functionality, or manually click the mouse to begin measuring click latency.
   - (Provide any additional steps on how to initiate the measurement, e.g., software to launch, etc.)

### 2. Analyze Results:
   - For every click registered, results will be displayed on screen immediately. You'll find the latest latency measurements, as well as both the average and standard deviation, along with the number of registered clicks. In addition, the data may be exported over a virtual COM port.

### 3. Resetting and Re-testing:
   - Use the "CLEAR" button to clear previous measurements and start over, or the "REBOOT" button if a re-initialization of the connected USB device is needed.

## Troubleshooting
- **LCD Issues**:
    - If the LCD doesn't properly initialize or contains artifacts, reboot the device by pressing the "REBOOT" button or power cycling the XLAT device.

- **Mouse / USB device not detected**:

    1. **Try the latest firmware**: Build and flash the latest version from the `main` branch. Support for new devices is added regularly.

    2. **Collect USB descriptors**: If updating doesn't help, we'll need the USB and USB-HID descriptors for your device to add support.

       **Linux**:
       - Plug in the device and run `lsusb` to find the VID:PID (e.g. `1234:5678`).
       - Get the full USB descriptor: `lsusb -v -d 1234:5678`
       - Get the HID report descriptor (replace the device path as needed):
         ```
         sudo cat /sys/bus/usb/devices/<bus>-<port>:1.0/0003:1234:5678.*/report_descriptor | xxd
         ```
         Alternatively, if you have `usbhid-dump` installed:
         ```
         sudo usbhid-dump -d 1234:5678
         ```

       **Windows**:
       - Download and run [USBTreeView](https://www.uwe-sieber.de/usbtreeview_e.html) or [USBDeview](https://www.nirsoft.net/utils/usb_devices_view.html).
       - Find your device in the tree, right-click it, and copy the device descriptor information.
       - For HID report descriptors, use [Wireshark](https://www.wireshark.org/) with USBPcap, or use [usbhid-dump](https://github.com/DIGImend/usbhid-dump) under WSL.

    3. **Open a GitHub issue**: File an issue at the [XLAT GitHub repository](https://github.com/teamfinalmouse/xlat/issues) and include:
       - Your device name and model
       - The full USB descriptor output
       - The HID report descriptor output

## Support
For further assistance or inquiries, contact Finalmouse support via email at support@finalmouse.com or on Discord.

## License
The software is licensed under the GPLv3 license. See `COPYING` for more information.
The libraries used are licensed under their respective licenses. See `LICENSES` / `LICENSE.txt` or similar for more information.

## Conclusion
The XLAT Mouse Latency Measurement Tool provides an accurate method for measuring mouse latency, aiding in achieving optimal performance for a competitive edge.
