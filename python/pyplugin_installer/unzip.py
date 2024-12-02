"""
/***************************************************************************
                            Plugin Installer module
                            unzip function
                             -------------------
    Date                 : May 2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

import zipfile
import os


def unzip(file, targetDir, password=None):
    """Creates directory structure and extracts the zip contents to it.
    file (file object) - the zip file to extract
    targetDir (str) - target location
    password (str; optional) - password to decrypt the zip file (if encrypted)
    """

    # convert password to bytes
    if isinstance(password, str):
        password = bytes(password, "utf8")

    # create destination directory if doesn't exist
    if not targetDir.endswith(":") and not os.path.exists(targetDir):
        os.makedirs(targetDir)

    zf = zipfile.ZipFile(file)
    for name in zf.namelist():
        # Skip directories - they will be created when necessary by os.makedirs
        if name.endswith("/"):
            continue

        # Read the source file before creating any output,
        # so no directories are created if user doesn't know the password
        memberContent = zf.read(name, password)

        # create directory if doesn't exist
        localDir = os.path.split(name)[0]
        fullDir = os.path.normpath(os.path.join(targetDir, localDir))
        if not os.path.exists(fullDir):
            os.makedirs(fullDir)
        # extract file
        fullPath = os.path.normpath(os.path.join(targetDir, name))
        outfile = open(fullPath, "wb")
        outfile.write(memberContent)
        outfile.flush()
        outfile.close()

    zf.close()
