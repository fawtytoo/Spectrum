# Spectrum emulator

## About

This is a Sinclair ZX Spectrum emulator for the 48K and 128K models.
- 48K (issue 5)
- 128K (toast-rack)

The emulator is cycle based to provide the greatest accuracy.
However, there is an issue concerning contention that needs to be addressed.
The only game so far that noticeably exhibits the issue is "Aquaplane".

## Legal
Amstrad have kindly given their permission for the redistribution of their
copyrighted material but retain that copyright.

Please read Cliff Lawson's statement.

## Interface

Designed to be as simple to use as possible:
- Drag 'n drop interface for loading tapes
- No menus; a simple heads-up is provided
- Function keys F1-F12 for external interaction

## Joysticks

There is support for 5 joysticks:
- Sinclair interface 2 (both players)
- Cursor
- Kempston
- Fuller
- Programmable (2 players)

## AY-3-8192

This is permanently enabled and works for both 48K and 128K models.
Supports both stereo and mono configurations:
- ABC stereo
- ACB stereo
- Mono (default)

## Tape loading/saving
Tape loading can be either from an external source via the mic socket on your
computer or through the emulators drag 'n drop support.

Tape saving is accomplished similarly either through the headphone socket or via
drag 'n drop. If saving to a file, data is ALWAYS appended (corrupted files are
treated as read-only).

NOTE: It's highly recommended that tape files are marked as read-only in your
filing system to prevent accidental corruption.

## Future plans?

### Z80 CPU accuracy

There are no plans to support the X/Y flags.

### Other models/derivatives

This emulator is for the best of the Sinclair models only.
There will be no plans to add anything else.

### Tape formats

Currently, the only tape file format supported is .tap files.

There are some plans for a new format to accommodate both audio and data formats.
This will cleanly allow for both the standard ROM load/save format, and for a
compressed audio format as an alternative for .tzx/.pzx file formats.
If this goes ahead, CLI tools will be supplied for converting to the new format.

### Windows support

No plans.
