jason
=====

A quite advanced launcher using JSON to store information on how to start the program. It is the opposite of KISS.
It is written using Qt and its QJson libraries along with QProcess to actually launch the program(s). Though it is aimed for Linux, it may work a little bit on Mac OS X and would most likely be equal to double-clicking a .exe file when run under Windows.

Far off in the distance:
 - A GUI for making Jason-compatible JSON files?

Planned features or things to do:
 - Desktop file generation (Easy)
 -  A window for showing error messages if there are any (Easy, too)
 -  Making a manual on how it all works. (Currently, one has to read the code in order to comprehend how it works.)

Finished features:
 - The GUI! It is quite simple, but it works in a satisfactory way, not taking up more space than it needs to.
 - A decent enough structure for JSON, still needs *some* more features as I have been writing the parser to cope with the example files which do not bring up every possible scenario, but the ground work is there.
 - Tons of configurability for launching. Really needs a manual on how it works, but until then I (the developer) would say it works decently for a wide enough amount of features, and it is pretty fast even though there are some bad programming choices inside it. (Hash tables. Too many of them.)
 - Some ground-work for desktop actions, which already are parsed fully and launchable, but it needs a function that prints a .desktop file containing it all.
