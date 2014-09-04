jason
=====

A quite advanced launcher using JSON to store information on how to start the program. It is the opposite of KISS.
It is written using Qt and its QJson libraries along with QProcess to actually launch the program(s). Though it is aimed for Linux, it may work a little bit on Mac OS X and would most likely be equal to double-clicking a .exe file when run under Windows, although it now only parses and prints information found in the files.

Planned features:
 - Lots of configurability, will definitely overlap with functionality found in shellscript. You should use shellscript instead of this. Really.
 - Some GUI features like showing messages and etc, perhaps interacting with Jason through it. No GUI will be made to create Jason-compatible files, although I might do it if I find it manageable. (So far it is not.)
