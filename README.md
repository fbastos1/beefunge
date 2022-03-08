# beefunge - The sweetest multiplatform Funge-93 Graphical IDE

beefunge is a Befunge-93 graphical IDE whose purpose in existence is to make your life easier. It's still under development, but feel free to try it out (and even contribute, if you're so inclined).

## Binaries
soon(tm)

## Build instructions
Install Qt Creator and open the Beefunge.pro file as a Qt Creator project.
Hit Ctrl-B to start the build, then Ctrl-R to run the project.

Qt Creator is available in most repositories.

Arch: `pacman -S qtcreator`
Debian/Ubuntu and derivatives: `apt install qtcreator`
Fedora: `dnf install qtcreator`

Most distros have the same package name. Usually, it will automatically install all the required Qt dependencies, but you may need to separately install the qt5-devel/qt6-devel packages for example. You will also need gcc, binutils, glibc, etc. Most distros have a metapackage to install all the essential development tools. On Ubuntu and derivatives, `apt install build-essential' will get the important packages.

## Why did you write another Funge-93 IDE?
Because I've had bad experiences with the few others I could get to work. So I decided I'd write my own, from scratch, because that sounded like a fun project.

## Project status
* (nearly) fully compliant with reference Befunge-93 (see: todo list)
* Basic debugging capabilities (i.e., stack display, output display, stepping, crawling, etc.)
* File i/o functionalities
* "Syntax highlighting" (character highlighting?)
* Stable enough for good work to be done (but save often - it's a beta)

## To-dos
* Befunge-98 compliance (hence the 93 noncompliance - it's a few quirks with whitespace and wrapping around)
* Better debugging capabilities (highlighting crawling, etc.)
* Auto saving & file recovery
* Customizable themes and colors (which comes with a Preferences menu)
* Command line interpreter mode (output only)
* Art for the program
* Modify code editor display with `p` instruction
* Make crawling speed user-settable

