# NXP Application Code Hub
[<img src="https://mcuxpresso.nxp.com/static/icon/nxp-logo-color.svg" width="100"/>](https://www.nxp.com)

## lvgl_cluster_rt1170_evkb_rpi
This is a cluster demo with LVGL on Raspberry Pi 7" touch panel. This demo shows the interface of the car/e-bike when driving, including the interface of speedometer gear switching and user menu switching.

#### Boards: MIMXRT1170-EVKB
#### Categories: Graphics
#### Peripherals: DISPLAY
#### Toolchains: MCUXpresso IDE

## Table of Contents
1. [Software](#step1)
2. [Hardware](#step2)
3. [Setup](#step3)
4. [Results](#step4)
5. [FAQs](#step5) 
6. [Support](#step6)
7. [Release Notes](#step7)

## 1. Software<a name="step1"></a>
* This project based on SDK 2.13.0.
* LVGL version 8.3.2.

## 2. Hardware<a name="step2"></a>
1. i.MX RT1170-EVKB(This project works on REV B board).
2. [Raspberry Pi 7" touch display.](https://www.raspberrypi.com/products/raspberry-pi-touch-display/)
   ![Display](images/Display.png)
3. Power adapter.

## 3. Setup<a name="step3"></a>

1. Connect the Raspberry Pi 7" touch display to i.MX RT1170-EVKB board.
   Assemble the Raspberry Pi Display.
   ![Raspberry Pi Display](images/RPI_back.png)

   Connect MIPI DSI cable to J84 on MIMXRT1170-EVKB.
   Connect J1 power of Raspberry Pi to J85 of MIMXRT1170-EVKB.
   ![Connection](images/Connect_back.png) 
2. Import the project from MCUXpresso IDE.
3. Download the built project to the board and run the example.

## 4. Results<a name="step4"></a>
When the demo runs correctly, we will see the following interfaces.
   ![Result](images/result1.jpg)    
   ![Result](images/result2.jpg) 
   ![Result](images/result3.jpg) 
## 5. FAQs<a name="step5"></a>

## 6. Support<a name="step6"></a>
#### Project Metadata
<!----- Boards ----->
[![Board badge](https://img.shields.io/badge/Board-MIMXRT1170&ndash;EVKB-blue)](https://github.com/search?q=org%3Anxp-appcodehub+MIMXRT1170-EVKB+in%3Areadme&type=Repositories)

<!----- Categories ----->
[![Category badge](https://img.shields.io/badge/Category-GRAPHICS-yellowgreen)](https://github.com/search?q=org%3Anxp-appcodehub+graphics+in%3Areadme&type=Repositories)

<!----- Peripherals ----->
[![Peripheral badge](https://img.shields.io/badge/Peripheral-DISPLAY-yellow)](https://github.com/search?q=org%3Anxp-appcodehub+display+in%3Areadme&type=Repositories)

<!----- Toolchains ----->
[![Toolchain badge](https://img.shields.io/badge/Toolchain-MCUXPRESSO%20IDE-orange)](https://github.com/search?q=org%3Anxp-appcodehub+mcux+in%3Areadme&type=Repositories)

Questions regarding the content/correctness of this example can be entered as Issues within this GitHub repository.

>**Warning**: For more general technical questions regarding NXP Microcontrollers and the difference in expected funcionality, enter your questions on the [NXP Community Forum](https://community.nxp.com/)

[![Follow us on Youtube](https://img.shields.io/badge/Youtube-Follow%20us%20on%20Youtube-red.svg)](https://www.youtube.com/@NXP_Semiconductors)
[![Follow us on LinkedIn](https://img.shields.io/badge/LinkedIn-Follow%20us%20on%20LinkedIn-blue.svg)](https://www.linkedin.com/company/nxp-semiconductors)
[![Follow us on Facebook](https://img.shields.io/badge/Facebook-Follow%20us%20on%20Facebook-blue.svg)](https://www.facebook.com/nxpsemi/)
[![Follow us on Twitter](https://img.shields.io/badge/Twitter-Follow%20us%20on%20Twitter-white.svg)](https://twitter.com/NXP)

## 7. Release Notes<a name="step7"></a>
| Version | Description / Update                           | Date                        |
|:-------:|------------------------------------------------|----------------------------:|
| 1.0     | Initial release on Application Code HUb        | 六月 27<sup>th</sup> 2023 |

