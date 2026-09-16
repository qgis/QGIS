"""
***************************************************************************
    GdalUtils.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"


try:
    from osgeo import gdal, ogr

    gdal.UseExceptions()
    ogr.UseExceptions()

    gdalAvailable = True
except:
    gdalAvailable = False


class GdalUtils:
    supportedRasters = None
    supportedOutputRasters = None

    @staticmethod
    def getSupportedRasters():
        if not gdalAvailable:
            return {}

        if GdalUtils.supportedRasters is not None:
            return GdalUtils.supportedRasters

        if gdal.GetDriverCount() == 0:
            gdal.AllRegister()

        GdalUtils.supportedRasters = {}
        GdalUtils.supportedOutputRasters = {}
        GdalUtils.supportedRasters["GTiff"] = ["tif", "tiff"]
        GdalUtils.supportedOutputRasters["GTiff"] = ["tif", "tiff"]

        for i in range(gdal.GetDriverCount()):
            driver = gdal.GetDriver(i)
            if driver is None:
                continue
            shortName = driver.ShortName
            metadata = driver.GetMetadata()
            if gdal.DCAP_RASTER not in metadata or metadata[gdal.DCAP_RASTER] != "YES":
                continue

            if gdal.DMD_EXTENSIONS in metadata:
                extensions = metadata[gdal.DMD_EXTENSIONS].split(" ")
                if extensions:
                    GdalUtils.supportedRasters[shortName] = extensions
                    # Only creatable rasters can be referenced in output rasters
                    if (
                        gdal.DCAP_CREATE in metadata
                        and metadata[gdal.DCAP_CREATE] == "YES"
                    ) or (
                        gdal.DCAP_CREATECOPY in metadata
                        and metadata[gdal.DCAP_CREATECOPY] == "YES"
                    ):
                        GdalUtils.supportedOutputRasters[shortName] = extensions

        return GdalUtils.supportedRasters

    @staticmethod
    def getFormatShortNameFromFilename(filename):
        ext = filename[filename.rfind(".") + 1 :]
        supported = GdalUtils.getSupportedRasters()
        for name in list(supported.keys()):
            exts = supported[name]
            if ext in exts:
                return name
        return "GTiff"
