# -*- coding:utf-8 -*-
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

def unzip(file, targetDir):
    """ Creates directory structure and extracts the zip contents to it.
        file - the zip file to extract
        targetDir - target location
    """

    # create destination directory if doesn't exist
    if not targetDir.endswith(':') and not os.path.exists(targetDir):
        os.makedirs(targetDir)

    zf = zipfile.ZipFile(file)
    for name in zf.namelist():
        # create directory if doesn't exist
        localDir = os.path.split(name)[0]
        fullDir = os.path.normpath( os.path.join(targetDir, localDir) )
        if not os.path.exists(fullDir):
            os.makedirs(fullDir)
        # extract file
        if not name.endswith('/'):
            fullPath = os.path.normpath( os.path.join(targetDir, name) )
            outfile = open(fullPath, 'wb')
            outfile.write(zf.read(name))
            outfile.flush()
            outfile.close()
