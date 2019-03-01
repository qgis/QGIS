# -*- coding: utf-8 -*-

"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Rashad Kanavath'
__date__ = 'January 2019'
__copyright__ = '(C) CNES 2019'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


class OtbSettings(object):
    """
    OtbSetting's key names
    """
    # Checkbox to enable/disable otb provider (bool).
    ACTIVATE = "OTB_ACTIVATE"

    # Path to otb installation folder (string, directory).
    FOLDER = "OTB_FOLDER"

    # Path to otb application folder. multiple paths are supported (string, directory).
    APP_FOLDER = "OTB_APP_FOLDER"

    # A string to hold current version number. Useful for bug reporting.
    VERSION = "OTB_VERSION"

    # Default directory were DEM tiles are stored. It should only contain ```.hgt`` or or georeferenced ``.tif`` files. Empty if not set (no directory set).
    SRTM_FOLDER = "OTB_SRTM_FOLDER"

    # Default path to the geoid file that will be used to retrieve height of DEM above ellipsoid. Empty if not set (no geoid set).
    GEOID_FILE = "OTB_GEOID_FILE"

    # Default maximum memory that OTB should use for processing, in MB. If not set, default value is 128 MB.
    # This is set through environment variable ``OTB_MAX_RAM_HINT``
    MAX_RAM_HINT = 'OTB_MAX_RAM_HINT'

    # ``OTB_LOGGER_LEVEL``: Default level of logging for OTB. Should be one of ``DEBUG``, ``INFO``, ``WARNING``, ``CRITICAL`` or ``FATAL``, by increasing order of priority. Only messages with a higher priority than the level of logging will be displayed. If not set, default level is ``INFO``.
    LOGGER_LEVEL = 'OTB_LOGGER_LEVEL'

    @staticmethod
    def keys():
        return [
            OtbSettings.ACTIVATE,
            OtbSettings.FOLDER,
            OtbSettings.SRTM_FOLDER,
            OtbSettings.GEOID_FILE,
            OtbSettings.LOGGER_LEVEL,
            OtbSettings.MAX_RAM_HINT
        ]
