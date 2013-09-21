# EfU Launcher

This is a launcher for the Neverwinter Nights persistent world [Escape from the Underdark][efupw]. It allows for automatic downloading of haks and directly connecting to the EfU server. The launcher will automatically check for updates.

## Installation

1. Download the latest release [here][releases].
2. Extract the archive with [7-zip][7zip].
3. Place the files anywhere, in the same directory. EfU Launcher will attempt to locate your Neverwinter Nights installation automatically, from

    1. the `-nwn` argument, if present;
    2. the launcher's current directory;
    3. Neverwinter Night's default installation directory, `C:\NeverwinterNights\NWN`; or
    4. the Windows registry.

4. Play!

Even if the launcher can't find your Neverwinter Nights installation and you didn't specify the `-nwn` argument the launcher can still download the necessary files. Just tell it to continue when asked. However, you will have to move the files manually afterwards so this is not recommended.

## Dependencies

EfU Launcher depends on

* libcurl (static)
* OpenSSL (dynamic)

All files needed to run the launcher are included in the archives. You will never need to keep files from one version of the launcher for another version.

## Command line arguments

Command line arguments start with `-` and the command and value are separated by `=`: `-argument=value`. If there are errors or unrecognised arguments the launcher will pause after processing all arguments so you can see the problems. Press any key to continue.

* **-nwn** Specify the path to the Neverwinter Nights 
  root directory. If present, the launcher will first search this path for the Neverwinter Nights executable. Remember to enclose this value in quotation marks if it contains spaces. If the launcher doesn't find the executable it will try its defaults, as per above.
* **-dmpass** Specify a DM server password and tell the launcher to start the DM client.

You can make a shortcut to the launcher and put any arguments in the Target field of the shortcut's properties, so it looks like this:

    C:\efulauncher\EfULauncher.exe -nwn="C:\Neverwinter Nights\NWN" -dmpass=beastlypoodle

# Licence

MIT/Expat

[efupw]: http://www.efupw.com
[releases]: https://github.com/commonquail/efulauncher/releases
[7zip]: http://7-zip.com/