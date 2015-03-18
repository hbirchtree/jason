## How to use Jason

 * What is all this?
   + Jason consists of JSON structures representing different features in a launch process. The general jist of it is:
     - Take all these objects described by JSON structures
     - Look at them and see what the user wants to do with them, example "sys.exec": "gltron"
     - Run it, doing everything the user has described.
It is designed to be flexible and handy to manage a large amount of programs you may want to launch, while presenting a "simple" way of allowing them to launch with advanced options.

## Heres a much more in-depth explanation of each feature
 *   *.exec: an execution value. has to be prefixed with the selected system and is used to execute programs.
 *   workdir: where to execute.
 *   detachable-process: boolean value determining whether or not to expect a detached process. shows a dialog window that must be closed manually by the user in order to signal the end of the main process.
 *   desktop.file: contains entries used in the .desktop file generation process.
     + desktop.displayname: used in the Name= field as well as the GUI.
     + desktop.description: used in the Comment= field (may fill in the Description= field as well?)
     + desktop.wmclass: used in the StartupWMClass= field
     + desktop.icon: used in the Icon= field and possibly in the GUI in the future
     + desktop.action.*: an object containing a desktop action, in the form desktop.action.{action-id}
       - desktop.displayname: used in the Name= field as well as the GUI if the action is launched
       - *.exec: the execution value of the action, is not inserted into the .desktop file.
       - workdir: the working directory for the execution of the action, not inserted into the .desktop file.
     + Adding other entries into the .desktop file:
       - A "rawdesktop.*" entry containing a string value will be inserted into the desktop file directly, splitting off what is after the "." sign and until the next "." sign as the name of the value in the desktop file.

 *   global.*: a place where different values are thrown in order to be used by any system.
    + .jason-opts: an object containing several options related to Jason:
      - jason.hide-ui-on-run: boolean value, hides Jason's window when the main process starts.
      - jason.window-dimensions: string containing the desired window dimensions.

 *   launchtype: essential to selecting the main system. This is used to launch the main process only, one may use other systems to launch preruns and postruns and etc. if this is not defined somewhere, the program will likely report that Apples is stuck in a tree.

 *   shell.properties: an object containing information related to the shell.
    + shell: string containing the name of the shell binary, example "sh", "bash" or "zsh".
    + command.argument: the argument given to the shell in order for it to run commands. in the case of sh this is "-c". after this a string "--" is appended to stop all arguments to the shell, making it only run the command which is appended afterwards.
    + import-env-variables: a comma-separated list of environment variables to include in the substitutes dictionary.

 *   imports:
    an array containing objects with a value "file" containing a string.
    encourages sharing values and objects to conserve space.

 *   variables:
    an array containing objects with values "name" and "value", all containing strings.
    used for substitution of variables in format %name%. Variables are substituted in most variables.
    have different types that work in different ways:
    + config-input: Variable name is given by "name", value is given by "config-input". at the current time of writing this input value is "%CONFIG_PREFIX%.%CONFIG-INPUT%", and as such will carry the name of the main system. This will be changed in the future to be usable by any object, not just systems.
    + default: a value used when the input is empty.
	no type:
	    implies that it is a pure variable, simply added to the list of substitutes. substitution is performed on the variable, is in format "name" and "value".
	system: (future)
	    pulls in an environment variable to the list of substitutes, only has value "name" containing the name of the environment variable.

 *   subsystems:
     + is an array filled with objects that are subsystems.
     + type: specifies its behavior.
     + appearance: an object containing:
       - desktop.title
       - desktop.icon
       - Are displayed when the subsystem is activated, icon currently holds no purpose but may have a purpose in the future.
     + enabler: the name of the value, prepended with subsystems., which may contain different values used in the subsystem. the value provided by this is hereunder referred to as the input value.
     + environment: typical environment, enabled when the subsystem is enabled.
     + variables: typical, enabled when the subsystem is enabled.
     + Different types: (specified in the "type" key)
       - bool: toggled by a boolean value. is not processed if the boolean value is false. may contain variables, environments and executing with syntax similar to substitution subsystems.
       - substitution:
         * trigger: when to run, sys-prerun or sys-postrun
         * variable: the variable that the input value is assigned to, is local if there is a .exec value, global if there's an environment.
         * does-exec: true or false, determines if the program should search for keys related to execution.
         * *.exec: execute and substitute possible value inside the command with string from input variable.
         * substitutes the variable specified by "variable" with the one supplied by the enabler. may substitute parts of an environment or .exec statement.
       - option:
         * options: array of objects containing the options
         * id: identifier, used in input variable
         * environment: typical environment
         * input variable may be a comma-separated list
       - select:
         * subtype: specify the subtype
         * Different subtypes: (specified in "subtype" key)
           + key-value-set: (the values below are inserted at the same level as the subtype, which again is at the same level as the type, they are not nested.)
             - *.exec: two variables are substituted, a key and value, for use with operations that involve this.
             - trigger: can be sys-prerun or sys-postrun depending on when it is to be run.
             - sets: an array with objects specified by:
                * id: the string of this value is used to select it.
                * keysets specified by the keyname in the key and value in the value. example: "'Long Key With Spaces Here'": "'some value'"
                * keys and values are directly substituted into the *.exec command.
                * it does not follow the typical execution standard of Jason, but is simpler.

 *   environment:
     + Different types:
       - run-prefix: prefixed to the main execution command only, looks for  a "exec.prefix" key and adds this to the list of prefixes. no prioritization is supported.
       - run-suffix: suffixed to the main execution command only
       - same as run-prefix except it looks for "*.exec.suffix".
     + variable: normal variable, name and value, value has its variable resolved.

 *   systems:
     + in the global scope there may only be one *.exec and *.workdir value, as there is no prioritization between imported files. having more will be the same as dividing by zero.
     + type: a useless value which does nothing, but is nice to have around.
     + config-prefix: the prefix for every option related to the system, such as *.exec values. is required.
     + launch-prefix: prefixed to the commands run by the system.
     + inherit: imports environment from another system, may be extended to inherit more.
     + identifier: the key used in "launchtype". must not collide with other systems. is required.
     + may contain the standard variable array
     + may contain an environment array
     + *.exec value which is prefixed with the config-prefix, its command is prefixed with launch-prefix, and is run in directory specified by "workdir".
     + *.prerun and *.postrun must be the same type as the main system in order to run, they are otherwise not picked up. preruns and postruns are inherited.

 *   *.prerun/*.postrun: is an array of items that may be run before or after the main process
     + priority: an int, used to make an entry run before or after the others. 0 means standard procedure (append for prerun, prepend for postrun) while -1 means explicit appending and 1 explicit prepending.
     + display.title: used in the GUI to display what is going on.
     + *.exec: the program to run, may be of any kind of system. (its launch-prefix is determined from the prefix of the variable name.)
     + workdir: where to run the program.

 *   Execution spec:
     + Is used for preruns and postruns, the global executable commandline, actions and substituted subsystems. All options are available to these and work the exact same way for each of these.
     + Jason has a spec for every execution being performed, except for the "select" subsystem, it is as so:
        - bool lazy-exit-status : ignore exit status of program
        - bool detachable-process : wait for user to declare the program closed
        - bool start-detached : spawn the process and detach as soon as it is started.
        - private.process-environment : object with keys and values presenting a QProcessEnvironment
        - desktop.icon : the icon displayed when the program runs, currently not implemented
        - desktop.title : the title displayed when the item is activated or executed
        - *.exec : the config prefix determines whether this is to be run or not, and determines what is prefixed to the command before running.

## Variable substitution
 * Variables occur in the format %VARIABLE% and are substituted by the function resolveVariable(QString). The list of variables can be substituted internally by running resolveVariables(), which will loop through and resolve variables, making them pure strings. (None of this matters to the end-user, though.)
 * Because they result in strings, it is recommended to encapsulate strings that may contain spaces in *.exec values with single quotes. (Double quotes are used by JSON and may cause problems.)
 * Variables are a common cause of infinite loops, because the process of resolving variables loops as long as unresolved variables are present. Not doing this may cause variables to not be resolved if the complexity is high enough, however this may change in the future.

