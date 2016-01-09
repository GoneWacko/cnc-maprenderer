# cnc-maprenderer
cnc-maprenderer is a small tool written in C that uses the data files (.MIX files) of the first Command & Conquer game, "Tiberian Dawn", and renders its maps to PNG files. It is licensed under the terms of the MIT license.

The tool isn't completely finished, and not currently actively being worked on, but can render most of the maps and levels of the game in its current form.

## Usage:
After compilation (using `make`), the tool can be executed using `cnc-maprenderer <mapname>`.

`<mapname>` is the game's internal name of a level or multiplayer map of the form `sc[gbjm][0-9][0-9][ew][a-c]`.

Thus, `cnc-maprenderer scg03ea` will create a file `scg03ea.png` containing the rendered version of the third level of the GDI campaign.

You can look at the provided `all-maps.sh` file for a list of all the maps of the game, in the form of a shell script that renders them all to individual files.

## Dependencies
cnc-maprenderer uses two libraries:
* [GLib2](https://developer.gnome.org/glib/) (`apt-get install libglib2.0-dev`)
* [pnglite](https://github.com/dankar/pnglite) (`apt-get install libpnglite-dev`)

Compilation was last tested with GCC 4.9.2 on a Debian GNU/Linux 8.2 "Jessie".

## Data files
The data files for C&C are not provided with this tool, and must be provided manually. Command & Conquer has been released free of charge by its copyright holders and can be downloaded legally from various websites around the internet.

The data files required by cnc-maprenderer are:
* `CONQUER.MIX`
* `DESERT.MIX`
* `TEMPERAT.MIX`
* `WINTER.MIX`
* `GENERAL.MIX` from the GDI disc, renamed to `GENERAL_GDI.MIX`
* `GENERAL.MIX` from the NOD disc, renamed to `GENERAL_NOD.MIX`

These files should be placed inside the `data` directory, alongside the `tilemap.dat` file.

## (Major) missing features
* Infantry are not currently rendered.
* Units and turrets are not rotated according to the map's description.
* Turrets for units such as tanks are not being rendered.
* The sandy foundations for buildings are not rendered.
* Shadows are not rendered.
* PNGs are not clipped to the actual boundaries of each map; instead, the entire 64x64 maps are shown, including bogus tile information that is part of the game's descriptions of the maps but outside their logical boundaries.
* The code makes a few assumptions about the machine it's executed on, such as its endianness.

## Acknowledgements
Some of the code in this tool was inspired by the `freecnc` code, which is released under GPL. In particular: the VFS and SHP-handling code; though all code was written from scratch.

Command & Conquer was made by Westwood Studios and is now property of EA Games.

## Author
cnc-maprenderer was originally written by Stijn Gijsen in 2013, based on an older, unreleased, Python script by the same author. During development, the tool's code name was 'Mhenlo', a name which is still used in some of the `#define`s.
