# change the FW_LIMIT

based on the size of the BootLoader
change the defined value in jumper.h (line 28):

// for 32KB BL size:
#define FW_LIMIT 0x08008000

// for 64KB BL size:
#define FW_LIMIT 0x08010000

# change the BL_SIZE_LIMIT

based on the size of the BootLoader
change the defined value
file: Host_Tools\PostBuild_fwauth.bat, (line 180):

// for 32KB BL size:
SET BL_SIZE_LIMIT=32768

// for 64KB BL size:
SET BL_SIZE_LIMIT=65536

# generate a new ecc key pair

Generate ecc private key with NIST p256 curve:

openssl ecparam -name prime256v1 -genkey -out ecc.key

Generate ecc public key from private key:

openssl pkey -in ecc.key -pubout > ecc_pub.key

# generate ecc_pub_key.h

generate ecc_pub_key.h header file and copy to Inc folder

cd Host_Tools
.\PreBuild.bat .\ecc.key ..\L4_BLAuth\Core\Inc\

# config and build App

change the application base address to 0x8010200.
this change should be done based on the bootloader flash size
should be done in 2 places:

file: STM32L431RCTX_FLASH.ld

FLASH    (rx)    : ORIGIN = 0x8010200,   LENGTH = 192

file: system_stm32l4xx.c, uncomment the following line:

#define USER_VECT_TAB_ADDRESS

file: system_stm32l4xx.c after the "#define VECT_TAB_BASE_ADDRESS FLASH_BASE" line

#define VECT_TAB_OFFSET         0x00010200U     /*!< Vector Table base offset field.

build app and generate a binary file

# generate the full flash (BL + Meta + MetaHash + MetaHashSignature + App)

.\PostBuild_fwauth.bat ..\L4_BLAuth\Release\L4_BLAuth.bin path_to_app_binary_file.bin

# build, program and run BLAuth app on the STM32 chip.

verify that its working with serial port connection and print statements.

