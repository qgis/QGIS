"""
/***************************************************************************
                        Plugin Installer module
                        Plugin version comparison functions
                             -------------------
    Date                 : 2008-11-24
    Copyright            : (C) 2008 by Borys Jurgiel
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

Here is Python function for comparing version numbers. It's case insensitive
and recognizes all major notations, prefixes (ver. and version), delimiters
(. - and _) and suffixes (alpha, beta, rc, preview and trunk).

Usage: compareVersions(version1, version2)

The function accepts arguments of any type convertible to Unicode string
and returns integer value:
0 - the versions are equal
1 - version 1 is higher
2 - version 2 is higher

-----------------------------------------------------------------------------
HOW DOES IT WORK...
It relies on packaging.version and PEP440 for version comparisons
"""

import re

from packaging.version import InvalidVersion, Version
from qgis.core import Qgis

# ------------------------------------------------------------------------ #


def normalizeVersion(s):
    """remove possible prefix from given string and convert to uppercase"""
    try:
        return Version(s).public
    except InvalidVersion:
        # handles bit more things than packaging:
        #   like "ver. 1.0-201609011405-2690BD9"
        pattern = r"^(?P<epoch>\D+)?(?P<version>(?P<release>\d+(?:\.\d+)*)(?P<pre>(?:a|alpha|b|beta|rc)\d+?)?((?P<post>\.post\d+))?(?P<dev>\.dev\d+)?)(?P<build>\+|-\S+)?$"
        m = re.match(pattern, s.lower())
        if not m:
            return ""
        return Version(
            f"{m.group('release') or ''}{m.group('pre') or ''}{m.group('post') or ''}{m.group('dev') or ''}"
        ).public


# ------------------------------------------------------------------------ #
def compareVersions(a, b):
    """Compare two version numbers. Return 0 if a==b or error, 1 if a>b and 2 if b>a"""
    try:
        # PEP 440 comparison
        va, vb = Version(a), Version(b)
        if va < vb:
            return 2
        elif va > vb:
            return 1
        else:
            return 0
    except InvalidVersion:
        # blunt str comparison
        a = normalizeVersion(a)
        b = normalizeVersion(b)
        if a < b:
            return 2
        elif a > b:
            return 1
        else:
            return 0


def isCompatible(curVer, minVer, maxVer):
    """Check if minVer <= curVer <= maxVer"""

    return compareVersions(curVer, minVer) < 2 and compareVersions(maxVer, curVer) < 2


def pyQgisVersion() -> str:
    """Return current QGIS version number."""
    return Version(Qgis.QGIS_VERSION.split("-", 1)[0]).public
