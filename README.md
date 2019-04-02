# EmbeddedSystemSoftware
Embedded System Software

## Device info.
* Freescale i.MX 6 Quadcore (Cortex-A9 based, 1.2GHz)
* Linux 3.10.17
* Android 4.3 JellyBean

## Cross-Compile (to ARM)
```
    arm-none-linux-gnueabi-gcc –static –o hello hello.c
```
## Adb
How to transfer files from HOST to TARGET
```
    adb push [file name] /data/local/tmp 
```
/data/local/tmp is non-volatile
