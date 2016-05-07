FlashROM 99
===========

The *TI 99/4A Flash ROM Cartridge*, or FlashROM 99 for short, is a cartridge
for the TI 99/4A home computer that allows for running ROM cartridge images
stored on an SD card.

![The FlashROM 99 Cartridge](/doc/flashrom99.jpg)

The FlashROM 99 supports ROM-only images of up to 32K that use the
write-to-ROM bank switching scheme.  It will not work with programs using
GROMs or CRU-based bank switching.

FlashROM 99 is released as Open Source Hardware under the
[CERN OHL license][4] and the [GNU GPL license][5].  Both hardware design
files and firmware sources are available on [GitHub][2].


Quick Start
-----------

Using the FlashROM 99 is simple and doesn't require and special hardware or
software.

To begin, copy up to 171 cartridge image files onto an SD card.  Switch off
the TI 99 and plug in the FlashROM 99, then insert the SD card into the
FlashROM 99.  Switch on the TI 99 and wait until the activity indicator on
the FlashROM 99 is no longer lit.

Press any key to bring up the TI menu screen.  You should see the list of
images found on the SD card.  If the SD card contains more than 8 images, a
`FLASHROM 99` entry is shown instead.  Selecting this entry starts an image
browser where you can page through the list of available images with `,`,
`.`, `SPACE`, and number keys.

Select the image you want to run.  The screen will show a loader animation
while the image loads.  Once the image has been loaded, it will run
automatically.  The SD card is now no longer required and may be removed.

If only one image is found on the SD card, it is loaded immediatly without
the need to select it first.

If you want to run a different program from the SD card, reset the TI 99 by
pressing `FCTN-=` and then reset the FlashROM 99 by pushing the reset
button.  If you do not reset the FlashROM 99, the TI menu will show only the
previously selected image.  Alternatively, you can power cycle the console,
which will reset both TI 99 and FlashROM 99.


How to Use the FlashROM 99
--------------------------

The following sections provide a detailed look at the FlashROM 99 and
explain all of its features.


### Images

A cartridge image is a binary file containing raw machine code that is
loaded verbatim into the TI 99 cartridge memory.  Many programs are
distributed as image files, and typically have a `.BIN` extension.

An additional source for images are cartridge files for emulators.  The MESS
emulator, for example, uses `.rpk` files, which are ordinary ZIP archives
with an unconventional extension.  Ignoring `layout.xml` and `meta-inf.xml`,
if you see one or two files that end in `C.BIN` and `D.BIN`, you're good to
go.  If you also see a `G.BIN`, however, then this cartridge contains a GROM
and cannot be used.

Finally, many assemblers such as the [`xas99` cross-assembler][3] can
generate images.  Note that files generated for Editor/Assembler Option 5
*cannot* be used!

Images are single files of up to 32K that must be "non-inverted", which is
the default until noted otherwise by the creator.  If you have an inverted
image, you can use the Python script `invert.py` in the `lib/` folder: An
inverted inverted image is a non-inverted image.

Unfortunately, there's no way to determine if an image is inverted or not --
you'll have to try it.  If an image crashes the TI 99, invert it and see if
that works instead.

Some images are split into multiple files of 8K each and named sequentially,
with the first file ending in `C`:

    HELLOC.BIN
    HELLOD.BIN

You can use multi-file images by concatenating the files into a single file:

	$ cat HELLO?.BIN > HELLO.BIN                        # Linux and Mac
	C:> copy /b HELLOC.BIN + /b HELLOD.BIN HELLO.BIN    # Windows

Note that the FlashROM 99 can also be built with multi-file support, but as
this deteriorates the startup time this option is not enabled by default.

Images may have more than one program entry to select.  The FlashROM 99
lists all entries for all images.


### SD Cards

The FlashROM 99 supports both SD and SDHC cards of class 4 or higher.
Unrated cards may also work, just try and see.

Cards must be formatted with the FAT16 or FAT32 filesystem.  FAT16 is the
default for new SD cards of up to 2 GB, and FAT32 is the default for new
SDHC cards of 4 GB and up.  Windows let you choose which filesystem to use
when formating a new card, so it's best to go with the default.

Note that some cards seem to have a difficult personality.  If you
absolutely cannot get a particular SD/SDHC card to work, just move on to
another one.

Image files can be dragged and dropped as-is into the root folder of the SD
card.  Filenames must end in `.BIN`, but case is ignored.  Files with other
extensions may be present and are skipped.  For multi-file images, filenames
must have at most 8 characters w/o extension due to filesystem limitations.

The FlashROM 99 handles up to 171 image entries per SD card.  Additional
images will be ignored.


### Operation

The FlashROM 99 is always in one of two modes: _menu mode_ or _image mode_.

When powering up, or after pushing the reset button, the FlashROM 99 is in
menu mode.  In this mode, the TI 99 menu screen shows the list of available
images or the image browser.  Selecting an entry will load the selected
image from the SD card and run it.  Once the image is running, the FlashROM
99 switches to image mode.

In image mode, the FlashROM 99 acts like a ROM cartridge containing the
selected image only.  In this mode, the TI 99 menu screen shows the entries
of the selected image.  Pressing `FCTN-=` will warm reset the console
without affecting the currently stored image.  The only way to return to
menu mode is to push the FlashROM 99 reset button, to power cycle the
console, or to remove and re-plug the FlashROM 99 cartridge.

Note that the SD card is read only once in menu mode.  If you swap SD cards
you need to push the reset button to re-read the SD card.  Once in image
mode, the SD card is no longer needed and may be removed until you want to
run a different image.

If only one image is found on the SD card, then it is loaded immediately.
In this case the FlashROM 99 skips menu mode and enters image mode directly.

The activity indicator LED lights up whenever the FlashROM 99 is busy
reading the SD card.  During this time, the entire cartridge is "offline" so
that the TI 99 cannot detect that a cartridge is plugged in.  You may still
operate the console, e.g., work with TI BASIC, while the LED is on, but
depending on your actions you may crash the TI 99.


### TL;DR

Above description may sound confusing at first, but operation is actually
straightforward in practice.  Some simple rules of thumb will help you to
use the FlashROM 99 successfully:

- After inserting an SD card, push the FlashROM 99 reset button or power
  cycle the console.
- Only push the reset button when the TI 99 title screen is shown.
- Leave both TI 99 and SD card alone whenever the LED is lit and not
  blinking.

It is not possible to damage the FlashROM 99 by incorrect operation,
assuming that shorting contacts or smashing components is way beyond
incorrect operation.


How the FlashROM 99 Works
-------------------------

The FlashROM 99 uses a 32K SRAM chip to store a cartridge image of up to
that size.  The TI 99 addresses the SRAM just like it would address a
cartridge ROM chip.

The 377 register maps 8K banks of the 32K SRAM into the cartridge space
`>6000` through `>7FFF`.  The bank switching design used here was originally
developed by Jon Guidry (acadiel) for the [16K multi-cart][8] and has since
been used in many homebrew projects.

The ATmega 8515 microcontroller reads the SD card and writes first the menu
and browser code and then the selected image into the SRAM chip.  The
microcontroller code uses a modified version of the [Petit FatFs][9] library
to make sense of the FAT16/FAT32 filesystems used by the SD cards.

Whenever the 8515 is active, the TI 99 must not access the SRAM chip, or the
TI 99 will crash and either one might get damaged.  Thus, three 541 line
drivers isolate the FlashROM 99 from the cartridge bus whenever the ATmega
is writing to the SRAM.

The 377 also buffers the serial one-way communication from the TI 99 to the
FlashROM 99 that relays the image selection.  The `>6800` address line
transmits the bit data and the `>7000` address line transmits the clock.
Currently, 10 bytes of data are transferred, containing the filename and
some internal data of the selected image.


Where to Get the FlashROM 99
----------------------------

The [GitHub repository][2] contains all hardware design files and software
sources required to build the FlashROM 99 yourself.

Your can also purchase a professionally made FlashROM 99 PCB board for €10
or a fully assembled FlashROM 99 cartridge for €45 plus shipping and
handling.  Please contact the developer at <r@0x01.de> for further
information.


About the Project
-----------------

The TI 99/4A Flash ROM Cartridge is Open Source Hardware released under the
[CERN OHL license][4], in the hope that TI 99 enthusiasts may find it useful.
Software components are released under the [GNU GPL license][5].

Thanks go to Jon Guidry (acadiel) of `hexbus.com` for designing the original
bank switching circuitry for the first TI multi-carts.

Contributions to both hardware and software are welcome.  Please email
feedback, support questions, inquiries for parts, and bug reports to the
developer at <r@0x01.de>.


[1]: https://endlos99.github.io/flashrom99
[2]: https://github.com/endlos99/flashrom99
[3]: https://endlos99.github.io/xdt99
[4]: http://www.ohwr.org/projects/cernohl/wiki
[5]: http://www.gnu.org/licenses/gpl.html
