- How to use Jason

 - What is all this?
 Jason consists of JSON structures representing different features in a launch process. The general jist of it is:
 - Take all these objects described by JSON structures
 - Look at them and see what the user wants to do with them, example "sys.exec": "gltron"
 - Run it, doing everything the user has described.
It is designed to be flexible and handy to manage a large amount of programs you may want to launch, while presenting a "simple" way of allowing them to launch with advanced options.

 - Heres a much more in-depth explanation of each feature
*.exec: an execution value. has to be prefixed with a system and is used to execute programs.

*.workdir: where to execute. Is paired with a *.exec value, will not be used unless there is a *.exec value.

desktop.file: contains entries used in the .desktop file generation process.
    desktop.displayname: used in the Name= field as well as the GUI.
    desktop.description: used in the Comment= field (may fill in the Description= field as well?)
    desktop.wmclass: used in the StartupWMClass= field
    desktop.icon: used in the Icon= field and possibly in the GUI in the future
    desktop.action.*: an object containing a desktop action.
	action-id: should match the suffix of the action, desktop.action.launcher for instance.
	desktop.displayname: used in the Name= field as well as the GUI if the action is launched
	*.exec: the execution value of the action, is not inserted into the .desktop file.
	*.workdir: the working directory for the execution of the action, not inserted into the .desktop file.

global.*: a place where different values are thrown in order to be used by any system.
    .detachable-process: boolean value determining whether or not to expect a detached process. shows a dialog window that must be closed manually by the user in order to signal the end of the main process.
    .jason-opts: an object containing several options related to Jason:
        jason.hide-ui-on-run: boolean value, hides Jason's progress window when the main process starts.

launchtype: essential to selecting the main system. This is used to launch the main process only, one may use other systems to launch preruns and postruns and etc. if this is not defined somewhere, the program will likely report that Apples is stuck in a tree.

shell.properties: an object containing information related to the shell.
    shell: string containing the name of the shell binary, example "sh", "bash" or "zsh".
    command.argument: the argument given to the shell in order for it to run commands. in the case of sh this is "-c". after this a string "--" is appended to stop all arguments to the shell, making it only run the command which is appended afterwards.
    import-env-variables: a comma-separated list of environment variables to include in the substitutes dictionary.

imports:
    an array containing objects with a value "file" containing a string.
    encourages sharing values and objects to conserve space.

variables:
    an array containing objects with values "name" and "value", all containing strings.
    used for substitution of variables in format %name%. Variables are substituted in most variables.
    have different types that work in different ways:
	config-input:
	    variable name is given by "name", input value is given by "input". at the current time of writing this input value is "%CONFIG_PREFIX%.input", and as such will carry the name of the main system. This will be changed in the future.
	    has a value "default" used when no input is provided.
	no type:
	    implies that it is a pure variable, simply added to the list of substitutes. substitution is performed on the variable, is in format "name" and "value".
	system: (future)
	    pulls in an environment variable to the list of substitutes, only has value "name" containing the name of the environment variable.

subsystems:
	is an array filled with objects that are subsystems.
    have a variable "type" that specifies its behavior.
    have an object "appearance" which containing strings "desktop.title" and "desktop.icon" which determine the title of the operation displayed the GUI as well as an icon (if the icon is ever given a purpose)
    enabler: the name of the value, prepended with subsystems., which may contain different values used in the subsystem. the value provided by this is hereunder referred to as the input value. constants do not need this.
    types: (specified in "type" key)
		constant: all its values are applied on every run, regardless of any options.
		bool: toggled by a boolean value. is not processed if the boolean value is false.
			environment: typical environment.
		substitution:
			trigger: when to run, sys-prerun or sys-postrun
			variable: the variable that the input value is assigned to, is local if there is a .exec value, global if there's an environment.
			*.exec: execute and substitute possible value inside the command with string from input variable.
			substitutes the variable specified by "variable" with the one supplied by the enabler. may substitute parts of an environment or .exec statement.
		option:
			options: array of objects containing the options
				id: identifier, used in input variable
				environment: typical environment
			input variable may be a comma-separated list
                select:
                        subtypes: (specified in "subtype" key)
                            key-value-set:
                                *.exec: two variables are substituted, a key and value, for use with operations that involve this.
                                trigger: can be sys-prerun or sys-postrun depending on when it is to be run.
                                sets: an array with objects.
                                    id: the string of this value is used to select it.
                                    keysets: an array containing keys beginning with key.* and value.*. given "key.KEYHERE": "STRING", KEYHERE is the variable it replaces (in format %KEYHERE%) and STRING is the substituted variable.

environment:
	types:
		run-prefix: prefixed to the main execution command only
			looks for "*.exec.prefix" and adds this to the list of prefixes. no prioritization is supported. no launch prefixes are applied.
		run-suffix: suffixed to the main execution command only
			same as run-prefix except it looks for "*.exec.suffix".
		variable: normal variable, name and value, value has its variable resolved.

systems:
    have a useless value "type" which does nothing, but is nice to have around.
    config-prefix: the prefix for every option related to the system, such as *.exec values. is required.
    launch-prefix: prefixed to the commands run by the system.
    inherit: imports environment from another system, may be extended to inherit more.
    identifier: the key used in "launchtype". must not collide with other systems. is required.
    may contain variables and environment
    externally:
    	has a *.exec value which is prefixed with the config-prefix, its command is prefixed with launch-prefix, and is run in a *.workdir of matching system.
    	in the global scope there may only be one *.exec and *.workdir value, as there is no prioritization between imported files. having more will likely cause an error or unwanted behavior.
    	*.prerun and *.postrun must be the same type as the main system in order to run, they are otherwise not picked up.

*.prerun/*.postrun: is an array of items that may be run before or after the main process
	priority: an int, used to make an entry run before or after the others, 0 makes it run before the others in case of prerun, 1 makes it run after the others in case of postrun.
	display.title: used in the GUI to display what is going on.
	*.exec: the program to run, may be of any kind of system. (its launch-prefix is determined from the prefix of the variable name.)
	*.workdir: where to run the program.

 - Variable substitution
 Variables occur in the format %VARIABLE% and are substituted by the function resolveVariable(QString). The list of variables can be substituted internally by running resolveVariables(), which will loop through and resolve variables, making them pure strings.
Because they result in strings, it is recommended to encapsulate strings that may contain spaces in *.exec values with single quotes. (Double quotes are used by JSON and may cause problems.)
