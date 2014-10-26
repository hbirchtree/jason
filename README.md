jason
=====

A quite advanced launcher using JSON to store information on how to start the program. It is the opposite of KISS.
It is written using Qt and its QJson libraries along with QProcess to actually launch the program(s). Though it is aimed for Linux, it may work a little bit on Mac OS X and would most likely be equal to double-clicking a .exe file when run under Windows.

Remember: You are personally responsible for whatever happens when using this program.

Far off in the distance:
 - A GUI for making Jason-compatible JSON files?
 - Making it a backend library for different GUIs to access?

Planned features or things to do:
 - Adding a function in the GUI to make use of the icon label. (It's there, but nothing is in place to set the icon.)


Finished features:
 - It is really bloated, files take up a lot of space when complexity increases. (Please don't use this if you have ~1GB of RAM.)
 - Launches your programs according to your JSON files. Read the MANUAL.md on how to write them.
 - Supports hiding itself in the background when it is not needed
 - Aims to be small, but consumes some amount of memory. (And I am too lazy to hunt those memory-hogging bugs)
 - Is generally fast at launching
 - Supports generating desktop files
 - If you know what to do, it's just as good as cobbling together shellscripts, only this one works and is designed for sharing data between instances.
 - You are free to introduce your own changes to the code and, if you want to, set up a pull request to contribute back.
