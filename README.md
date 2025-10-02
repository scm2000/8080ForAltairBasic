# 8080 for Altair Basic
This code is based on the i8080 emulator, but has
a mainline with special implementations of in/out opcodes
to support Altair 8k basic. 
The code is targeted to run on a PicoCalc, and you can
use it to run any 8080 executable so long as you understand
it will behave oddly when io is done to the Altair casset io port

## Modifications
The code that handles io to the terminal port keeps track of the last character typed, then the code that handles io to the casset tape port uses that assuming the user just typed CLOAD &lt;letter&gt; or CSAVE &lt;letter&gt; in order to channel bytes to/from a file:
/Altair/tapes/tape_&lt;letter&gt;.dat

Because Altair 8K basic requires upper case keywords, the sense of the &lt;shift&gt; key is reversed by the terminal io port handling routine.   That is, alphabetic characters default to upper case, if you press shift while typing then you will get a lower case letter.

Altair 8K basic has a bug..   Upon start up, it will ask for how much memory to use.. If you type a number it will use that, but if you just hit enter it will go into it's automatic memory sizing routine, size the memory and use that.   Unfortunately at the time of writing the interpreter, it was assumed there would be at least some ROM in the sytem, and/or there would'nt be a full 64k of ram.   The sizing routine writes probe bytes to each location following basic's code itself... when it hits a location that does not read back what it wrote it assumes it hit rom or nonexsistent memory.   In our case we can have the full 64k of ram, and so basic will never find an end to memory, it will wrap around and overwrite itself to death.   To avoid that and allow almost all of the 64k of ram for use, I force the location 0xFFFF to look like rom.

Other than using CSAVE and CLOAD, you can store text files containing BASIC progam listings anywhere below /Altair.  Then once basic is running you can type &lt;ctrl&gt;i.  You will be prompted for the path to the basic listing, it will then source in the basic program.   This is useful if you want to create and edit a basic program on a laptop or desktop, as Altair 8K BASIC is pretty tedious when it comes to editing basic.   Also it allows you to source in BASIC programs found elsewhere into 8K BASIC.


## Build and Installation on a PicoCalc
### Prerequsites
You need to install pico-sdk on your system and set up PICO_SDK_PATH to point to it.  The repository can be found at: https://github.com/raspberrypi/pico-sdk.git

To make it easy on myself and to get up and running fast I use the drivers found in the picocalc-text-starter project found at https://github.com/BlairLeduc/picocalc-text-starter.git. 

I've included the drivers directory from that project for convenience, see its license file.

And of course you'll need a .bin file for the actual Altair 8K BASIC itself. I created mine from an original Altair 8K BASIC cassette tape.  Once obtained you need to call the file basicload.bin

If you have an Altair 8K BASIC tape, i've released in a separate repository: https://github.com/scm2000/decodeAltairTapes a python program taht will take a mono wav file of the tape and demodulate and unpack to a .bin file suitable for use here.   It probably works, with some changes, for other Altair format tapes.


You'll need a suitable compiler to cross compile to RP2040.

### Building
Go to the top level of this project and do

mkdir build

cd build

cmake ..

make

### Installing
The bootloader that ships on a PicoCalc (as of August 2025) seems to require a .bin file for executables.. That also means you need linker files to offset the load start of the executable.  This project is all set up for that.  Just copy i8080ForAltairBASIC.bin from the build directory over to your PicoCalc's SD card /firmware directory.   It will then show up in the boot menu upon power up... Some have said this is no longer nescessary and that you can build without the load offset and just provide the built .uf2 file to the boot loader. If you can figure out how to do that.. enjoy!


You need to create a directory /Altair at the root of your sd card, and the directory /Altair/tapes. Also you need to put your .bin for 8K basic at /Altair/basicload.bin

Once everything is on the PicoCalc, you should be able to boot it up from the boot menu.   Basic will prompt you for memory size, terminal width, and whether you want some trig functions configured in or out (to save memory configure them out)

Please note! The backspace key as we know it today did not exist on the teletypes in use at the time of writing Altair 8K BASIC, when typing a line if you want to delete the last character you need to type an underscore '_',   to delete multiple last characters you type multiple underscores.


----------------------
original i8080 readme
----------------------
# 8080

A complete emulation of the Intel 8080 processor written in C99. Goals:

- accuracy: it passes all test roms at my disposal
- readability
- portability: tested on debian 8 with gcc 5, macOS 10.12+ with clang and emscripten 1.39

You can see it in action in my [Space Invaders emulator](https://github.com/superzazu/invaders).

## Running tests

You can run the tests by running `make && ./i8080_tests`. The emulator passes the following tests:

- [x] TST8080.COM
- [x] CPUTEST.COM
- [x] 8080PRE.COM
- [x] 8080EXM.COM

The test roms (`cpu_tests` folder) are taken from [here](http://altairclone.com/downloads/cpu_tests/) and take approximately 30 seconds on my computer (MacBook Pro mid-2014) to run.

The standard output is as follows:

```
*** TEST: cpu_tests/TST8080.COM
MICROCOSM ASSOCIATES 8080/8085 CPU DIAGNOSTIC
 VERSION 1.0  (C) 1980

 CPU IS OPERATIONAL
*** 651 instructions executed on 4924 cycles (expected=4924, diff=0)

*** TEST: cpu_tests/CPUTEST.COM

DIAGNOSTICS II V1.2 - CPU TEST
COPYRIGHT (C) 1981 - SUPERSOFT ASSOCIATES

ABCDEFGHIJKLMNOPQRSTUVWXYZ
CPU IS 8080/8085
BEGIN TIMING TEST
END TIMING TEST
CPU TESTS OK

*** 33971311 instructions executed on 255653383 cycles (expected=255653383, diff=0)

*** TEST: cpu_tests/8080PRE.COM
8080 Preliminary tests complete
*** 1061 instructions executed on 7817 cycles (expected=7817, diff=0)

*** TEST: cpu_tests/8080EXM.COM
8080 instruction exerciser
dad <b,d,h,sp>................  PASS! crc is:14474ba6
aluop nn......................  PASS! crc is:9e922f9e
aluop <b,c,d,e,h,l,m,a>.......  PASS! crc is:cf762c86
<daa,cma,stc,cmc>.............  PASS! crc is:bb3f030c
<inr,dcr> a...................  PASS! crc is:adb6460e
<inr,dcr> b...................  PASS! crc is:83ed1345
<inx,dcx> b...................  PASS! crc is:f79287cd
<inr,dcr> c...................  PASS! crc is:e5f6721b
<inr,dcr> d...................  PASS! crc is:15b5579a
<inx,dcx> d...................  PASS! crc is:7f4e2501
<inr,dcr> e...................  PASS! crc is:cf2ab396
<inr,dcr> h...................  PASS! crc is:12b2952c
<inx,dcx> h...................  PASS! crc is:9f2b23c0
<inr,dcr> l...................  PASS! crc is:ff57d356
<inr,dcr> m...................  PASS! crc is:92e963bd
<inx,dcx> sp..................  PASS! crc is:d5702fab
lhld nnnn.....................  PASS! crc is:a9c3d5cb
shld nnnn.....................  PASS! crc is:e8864f26
lxi <b,d,h,sp>,nnnn...........  PASS! crc is:fcf46e12
ldax <b,d>....................  PASS! crc is:2b821d5f
mvi <b,c,d,e,h,l,m,a>,nn......  PASS! crc is:eaa72044
mov <bcdehla>,<bcdehla>.......  PASS! crc is:10b58cee
sta nnnn / lda nnnn...........  PASS! crc is:ed57af72
<rlc,rrc,ral,rar>.............  PASS! crc is:e0d89235
stax <b,d>....................  PASS! crc is:2b0471e9
Tests complete
*** 2919050698 instructions executed on 23803381171 cycles (expected=23803381171, diff=0)

```

## Resources used

- [CPU instructions](http://nemesis.lonestar.org/computers/tandy/software/apps/m4/qd/opcodes.html) and [this table](http://www.pastraiser.com/cpu/i8080/i8080_opcodes.html)
- [MAME i8085](https://github.com/mamedev/mame/blob/6c0fdfc5257ca20555fbc527203710d5af5401d1/src/devices/cpu/i8085/i8085.cpp)
- [thibaultimbert's Intel8080](https://github.com/thibaultimbert/Intel8080/blob/master/8080.js) and [begoon's i8080-js](https://github.com/begoon/i8080-js)
