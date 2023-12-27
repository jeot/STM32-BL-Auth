# Simple bootloader for STM32L4 with firmware Integrity and Authenticity

Based on this YouTube video:

Security Part4 - STM32 security in practice - 14 Bootloader with authentication lab

in the following playlist:

MOOC - Security Part4 : STM32 security in practice

playlist link:

https://www.youtube.com/playlist?list=PLnMKNibPkDnF0wt-ZI74SflnsBV4yKzkO

# Steps to create the bootloader

## Download STM32 Ctypto library v4.x.x

Library download link:

https://www.st.com/en/embedded-software/x-cube-cryptolib.html

Extract the library zip and copy the folder "STM32_Cryptographic" into the project directory (in this case L4_BLAuth directory).
The "STM32_Cryptographic" folder is inside "STM32CubeExpansion_Crypto_V4.1.0\Middlewares\ST" folder.

## Change the FW_LIMIT

Based on the size of the bootloader change the defined value in jumper.h (line 28):

```
// for 32KB BL size:
#define FW_LIMIT 0x08008000

// for 64KB BL size:
#define FW_LIMIT 0x08010000
```

## Change the BL_SIZE_LIMIT

Based on the size of the bootloader Change the defined value

file: Host_Tools\PostBuild_fwauth.bat, (line 180):

```
// for 32KB BL size:
SET BL_SIZE_LIMIT=32768

// for 64KB BL size:
SET BL_SIZE_LIMIT=65536
```

## Generate a new ECC key pair

Generate ECC private key with NIST p256 curve:

```
openssl.exe ecparam -name prime256v1 -genkey -out ecc.key
```

Generate ecc public key from private key:

```
openssl.exe pkey -in ecc.key -pubout > ecc_pub.key
```

## Generate ecc_pub_key.h

Generate ecc_pub_key.h header file and copy to Inc folder:

```
cd Host_Tools
.\PreBuild.bat .\ecc.key ..\L4_BLAuth\Core\Inc\
```

## Config and build App

Change the application base address to 0x8010200.
This change should be done based on the bootloader flash size should be done in 2 places:

file: STM32L431RCTX_FLASH.ld

```
FLASH    (rx)    : ORIGIN = 0x8010200,   LENGTH = 192
```

file: system_stm32l4xx.c, uncomment the following line:

```
#define USER_VECT_TAB_ADDRESS
```

file: system_stm32l4xx.c after the "#define VECT_TAB_BASE_ADDRESS FLASH_BASE" line

```
#define VECT_TAB_OFFSET         0x00010200U     /*!< Vector Table base offset field.
```

Build app and generate a binary file.

## Generate the final binary

Generate the full flash binary (BL + Meta + MetaHash + MetaHashSignature + App):

```
.\PostBuild_fwauth.bat ..\L4_BLAuth\Release\L4_BLAuth.bin path_to_app_binary_file.bin
```

## Build, program and run BLAuth app on the STM32 chip

Verify that its working with serial port connection and print statements.

