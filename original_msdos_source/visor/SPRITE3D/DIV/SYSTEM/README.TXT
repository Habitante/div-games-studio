

======================================================================
    Microsoft Product Support Services Application Note (Text File)
            HD0534: MOUSE DRIVER 8.20A, DRIVER ONLY UPDATE
======================================================================
                                                   Revision Date: 6/92
                                                         Disk Included

The following information applies to Microsoft Mouse Version 8.20a.

 --------------------------------------------------------------------
| INFORMATION PROVIDED IN THIS DOCUMENT AND ANY SOFTWARE THAT MAY    |
| ACCOMPANY THIS DOCUMENT (collectively referred to as an            |
| Application Note) IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      |
| KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO    |
| THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A     |
| PARTICULAR PURPOSE. The user assumes the entire risk as to the     |
| accuracy and the use of this Application Note. This Application    |
| Note may be copied and distributed subject to the following        |
| conditions: 1) All text must be copied without modification and    |
| all pages must be included; 2) If software is included, all files  |
| on the disk(s) must be copied without modification [the MS-DOS(R)  |
| utility DISKCOPY is appropriate for this purpose]; 3) All          |
| components of this Application Note must be distributed together;  |
| and 4) This Application Note may not be distributed for profit.    |
|                                                                    |
| Copyright 1992 Microsoft Corporation. All Rights Reserved.         |
| Microsoft and MS-DOS are registered trademarks and Windows         |
| is a trademark of Corporation Microsoft.                           |
 --------------------------------------------------------------------

  NOTE: The files included on the enclosed HD0534 disk are intended
  for use with the Microsoft Mouse and Microsoft BallPoint mouse only.

The HD0534 disk contains the following files:

   MOUSE.COM
   MOUSE.DRV
   MOUSE.SYS

This update contains only the basic mouse driver files for the MS-DOS
and Windows operating systems. To order the full mouse driver update
version 8.20a, with all added features and user's guide, call
Microsoft End User Sales at (800) 426-9400. The cost of the full
package is $25.00 plus shipping and handling.

  NOTE: This document is divided into two sections. If you are not an
  experienced MS-DOS user, proceed to the section titled "Detailed
  Manual Installation." Experienced MS-DOS/Windows users can perform
  the steps listed below for "Manual Installation."

Manual Installation (Experienced Users)
---------------------------------------

1. Locate the directory you are currently loading your MS-DOS mouse
   driver from. If it is loading from your Windows directory, we
   recommend that you create a directory called \MOUSE for your mouse
   driver files off of your root directory on drive C (that is,
   C:\MOUSE).

2. Copy any other MS-DOS mouse drivers that are on your hard disk to a
   floppy disk, then delete them from your hard disk.

3. Copy the three mouse driver files from the enclosed HD0534 disk to
   the directory you created in step 1.

4. If you load the MS-DOS mouse driver automatically, edit your
   AUTOEXEC.BAT or CONFIG.SYS file to reflect any path changes to the
   mouse directory where you just copied the enclosed files.

5. If you use Windows 3.0 or 3.1, use a text editor such as Notepad to
   open the SYSTEM.INI file (located in your Windows directory) and
   change the line "mouse.drv=" in the [boot] section to show the
   correct path to the directory where you copied the MOUSE.DRV file.
   A typical example is the following:

      mouse.drv=c:\mouse\mouse.drv

6. Restart your computer.

Detailed Manual Installation (Novice Users)
-------------------------------------------

 1. Before installing the enclosed mouse driver files, we recommend
    that you delete any existing MS-DOS mouse driver files from your
    hard disk. Change to the root directory of drive C. To do this, at
    the command prompt type "c:" (without the quotation marks) and
    press ENTER and then type "cd\ " (without the quotation marks) and
    press ENTER again.

 2. If you are running MS-DOS version 5.0, use the following command
    to search all the subdirectories on drive C. Write down the name
    of the directory listed above any MOUSE.COM or MOUSE.SYS files
    that are found.

       dir mouse.* /s/p

       NOTE: If you are using a previous version of MS-DOS, you will
       have to manually maneuver around into any subdirectories and
       look for mouse driver files. See your MS-DOS user's guide for
       instructions on how to do this.

 3. If your hard disk has more than one partition, such as drive D,
    repeat steps 1 and 2, substituting D: for C:. Do this for all
    drive letters you have.

 4. Copy all the MOUSE.COM and MOUSE.SYS files that you find to a
    formatted floppy disk. Label the disk "Old Mouse Files." To copy
    the files, at the command prompt, type the entire directory path
    you wrote down in step 1, ending in the filename, followed by the
    drive you placed your floppy disk in. For example, type

       copy c:\dos\mouse.com a:

    where "c:\dos" is the directory you wrote down, "mouse.com" is the
    name of the file found, and the floppy disk is in drive A.

 5. After copying the mouse drivers to a floppy disk, you should then
    delete those files from your hard disk. Following the example
    above, you would type the following at the command prompt

       del c:\dos\mouse.com

    to delete the MOUSE.COM file from the C:\DOS directory. Do this
    for each file you found.

 6. Change back to the root directory on drive C (see step 1). Check
    for a directory named \MOUSE. To do this, type the following at
    the MS-DOS prompt:

       dir *.

 7. If a \MOUSE directory is not listed, you should create one. If a
    \MOUSE directory is listed, then skip to step 8. To create the
    \MOUSE directory, type:

       md mouse

 8. Place the enclosed HD0534 disk in your floppy disk drive and copy
    the files to the C:\MOUSE directory. For example, if your floppy
    disk is in drive A, you would type:

       copy a:\*.* c:\mouse

 9. If you would like your mouse driver software to load
    automatically, change to the root directory on drive C (see step
    1). Prepare to edit the AUTOEXEC.BAT file.

       NOTE: Prior to editing the AUTOEXEC.BAT file, you should make a
       backup copy of it first. Use the following command to make a
       backup copy of your AUTOEXEC.BAT file:

       copy c:\autoexec.bat c:\autoexec.old

10. If you are running MS-DOS 5.0, type the following:

       edit autoexec.bat

       NOTE: If you are using a previous version of MS-DOS, see your
       MS-DOS user's guide for instructions on editing this file.

11. Add the following line to the end of your AUTOEXEC.BAT file. To
    move the cursor around, press the arrow keys:

       c:\mouse\mouse /y

12. Remove any other lines in the AUTOEXEC.BAT file that contain the
    word "mouse," except for the PATH statement. For the PATH
    statement, remove only the <section> of that line that contains
    the word "mouse." Save the file and exit the editor.

13. If you use Windows 3.0 or 3.1, start Windows (if it is not already
    running). If you don't use Windows 3.0 or 3.1, skip to step 16.
    Prepare to edit the SYSTEM.INI file by making a backup of it.

       NOTE: To make a backup copy of your SYSTEM.INI file, start File
       Manager and open the Windows directory (by default, this is
       C:\WINDOWS). Select the SYSTEM.INI file and choose Copy from
       the File menu. In the To box, type the following:

       system.old

14. Open the SYSTEM.INI file in a text editor such as Notepad. Find
    the following line in the [boot] section of the SYSTEM.INI file

       mouse.drv=***

    where "***" could be anything.
       
15. Change the line found in step 14 to read as follows, then save the
    SYSTEM.INI file by choosing Save from the File menu in Notepad.

    mouse.drv=c:\mouse\mouse.drv

16. Quit Windows (if it is running) or MS-DOS Editor and restart your
    computer.

Optional Steps
--------------

17. Depending on how much free space you have on your hard disk, you
    may want to delete any files found in other mouse directories
    (such as C:\MOUSE1) in steps 2 and 3. If you have a CPANEL.COM
    file, you may want to copy it to your C:\MOUSE directory, but you
    do not have to do this. For more information on this file, skip to
    step 18.

       CAUTION: Do not delete any files from C:\DOS or C:\WINDOWS
       other than MOUSE.COM or MOUSE.SYS.

    If you had a directory similar to the one used as an example
    (C:\MOUSE1), you can delete all the files in it, and remove the
    directory. For information on the commands necessary to do this,
    see your MS-DOS user's guide.

       NOTE: We recommend that you copy any files you plan to delete
       from your hard disk to the Old Mouse Files floppy disk used in
       step 4. This will give you a backup copy of those files in case
       you have problems.

18. Depending on how your mouse was previously installed, you may or
    may not have a CPANEL.COM file. You do not need this file, but if
    you have one, it should be copied to the C:\MOUSE directory. This
    file contains the mouse Control Panel, which allows you to change
    the speed of the mouse cursor in MS-DOS applications. This file
    will also be helpful for troubleshooting should you ever need to
    contact Microsoft Product Support Services.

    If you are running MS-DOS 5.0, change back to the root directory
    of drive C (see step 1). At the MS-DOS command prompt, type the
    following command, and write down any directories listed above any
    CPANEL.COM files that are found:

       dir cpanel.com /s/p

       NOTE: If you are using a previous version of MS-DOS, you will
       have to manually maneuver around into any subdirectories and
       look for CPANEL.COM. See your MS-DOS manual for the
       instructions on how to do this.

19. Type the following command at the MS-DOS command prompt to copy
    any CPANEL.COM files that you find to your C:\MOUSE directory. If
    more than one file is found, copy only the file with the most
    recent file date. In this example, the CPANEL.COM file was found
    in a directory called C:\MOUSE1:

       copy c:\mouse1\cpanel.com c:\mouse\*.*