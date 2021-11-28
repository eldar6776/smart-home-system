ReadMe.txt for the ST STM32F746 start project.

This project was built for Segger Embedded Studio V2.20.
It has been tested with the following versions:
- V2.20

Supported hardware:
===================
The sample project for ST STM32F746NG is prepared
to run on a ST STM32F746G-DISCO, but may be used on other 
target hardware as well.

Using different target hardware may require modifications.

Configurations:
===============
- Debug
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS debug and profiling library is used.

- Release
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS release library is used.
  
- Release_SystemView
  This configuration is prepared for download into
  internal Flash using J-Link.
  An embOS stack-check and profiling library is used.

Notes:
======
  To use SEGGER SystemView with Debug or Release_SystemView 
  configuration, configure SystemViewer for STM32F746NG as 
  target device and SWD at 2000 kHz as target interface.

Included middleware components (trial versions):
================================================
- embOS V4.24
  - (1.23.01.28) embOS-Cortex-M-SES (embOS designed for ARM Cortex-M and SEGGER Embedded Studio)

- emFile V4.04a
  - (2.00.01) emFile FAT (FAT File system for embedded applications, supporting FAT12, FAT16 and FAT32)
  - (2.00.05) emFile Journaling (Journaling Add-on for emFile)
  - (2.03.00) embOS layer for emFile (abstraction layer for emFile providing OS specific functions)
  - (2.05.00) emFile Encryption (Encryption Add-on for emFile)
  - (2.10.10) emFile FAT LFN Module (Support for Long File Name)
  - Device drivers:
    - (2.10.03) emFile device driver for SD/SDHC/MultiMedia (Device driver for SD / SDHC & Multimedia card)

- embOS/IP V3.08
  - (7.60.02) embOS/IP BASE IPv4/IPv6 Dual Stack (TCP/IP Protocol Stack including: IPv4, IPv6, ARP, ICMP, UDP, TCP, DHCPc, DNSc, ACD, Multicast, AutoIP, VLAN and BSD 4.4 Socket Interface, RAW sockets, TFTPc, TFTPs, Loopback device)
  - (7.02.00) embOS/IP FTP Server (File Transfer Protocol (Server))
  - (7.02.10) embOS/IP FTP Client (File Transfer Protocol (Client))
  - (7.03.01) embOS/IP NetBIOS Name Service (NetBIOS Name Service protocol)
  - (7.05.00) embOS/IP Web Server (Hyper Text Transfer Protocol (Server))
  - (7.16.00) embOS/IP SMTP Client (Simple Mail Transfer Protocol (Client))
  - (7.30.00) embOS layer for embOS/IP (abstraction layer for embOS/IP providing OS specific functions)
  - Drivers:
    - (7.01.28) embOS/IP Synopsys (embOS/IP driver for CPUs with integrated Ethernet controller using Synopsys Ethernet IP) 

- emUSB-Device V3.00d
  - (9.00.00) emUSB-Device BASE (USB core + HID component)
  - (9.00.01) emUSB-Device Bulk Component (Bulk component + Windows driver (binary))
  - (9.00.03) emUSB-Device MSD Component (MSD component)
  - (9.00.04) emUSB-Device CDC Component (CDC component)
  - (9.00.05) emUSB-Device MSD-CDROM Component (MSD-CDROM component)
  - (9.00.07) emUSB-Device Printer Component (Printer Class component)
  - (9.00.10) emUSB-Device MTP Component (Media Transfer Protocol Component)
  - (9.01.05) embOS layer for emUSB-Device (Abstraction layer for emUSB-Device providing OS specific functions)
  - Drivers:
    - (9.10.56) emUSB-Device target driver ST STM32F4xx (Target driver for ST STM32F4xx)
- emWin V5.34
  - (3.00.01) emWin BASE color (Complete graphic library, ANSI "C"-source code for 8/16/32 bit CPUs)
  - (3.00.04) emWin sim (emWin simulation)
  - (3.01.00) emWin WM/Widgets (Windows manager and GUIBuilder for emWin/GSC and Widgets)
  - (3.01.02) emWin Memory Devices (Memory devices for flicker-free animation)
  - (3.01.03) emWin Antialiasing (Antialiasing smoothes curves and diagonal lines)
  - (3.01.04) emWin VNC server (VNC server)
  - (3.02.01) emWin Bitmap Converter (Bitmap Converter for emWin)
  - (3.04.00) emWin Font Converter (Font Converter for emWin)
  - (3.xx.xx) embOS layer for emWin (Abstraction layer for emWin providing OS specific functions)
  - Drivers:
    - (3.10.23.02) GUIDRV_Lin
