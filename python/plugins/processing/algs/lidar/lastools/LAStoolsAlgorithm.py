# -*- coding: utf-8 -*-

"""
***************************************************************************
    LAStoolsAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : April 2014
    Copyright            : (C) 2014 by Martin Isenburg
    Email                : martin near rapidlasso point com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4 import QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm

from LAStoolsUtils import LAStoolsUtils

from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterSelection
from processing.core.outputs import OutputFile
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector

class LAStoolsAlgorithm(GeoAlgorithm):

    VERBOSE = "VERBOSE"
    CORES = "CORES"
    INPUT_LASLAZ = "INPUT_LASLAZ"
    INPUT_DIRECTORY = "INPUT_DIRECTORY"
    INPUT_WILDCARDS = "INPUT_WILDCARDS"
    OUTPUT_LASLAZ = "OUTPUT_LASLAZ"
    OUTPUT_DIRECTORY = "OUTPUT_DIRECTORY"
    OUTPUT_APPENDIX = "OUTPUT_APPENDIX"
    OUTPUT_POINT_FORMAT = "OUTPUT_POINT_FORMAT"
    OUTPUT_POINT_FORMATS = ["laz", "las"]
    OUTPUT_RASTER = "OUTPUT_RASTER"
    OUTPUT_RASTER_FORMAT = "OUTPUT_RASTER_FORMAT"
    OUTPUT_RASTER_FORMATS = ["tif", "bil", "img", "dtm", "asc", "xyz", "png", "jpg"]
    OUTPUT_VECTOR = "OUTPUT_VECTOR"
    OUTPUT_VECTOR_FORMAT = "OUTPUT_VECTOR_FORMAT"
    OUTPUT_VECTOR_FORMATS = ["shp", "wkt", "kml", "txt"]
    ADDITIONAL_OPTIONS = "ADDITIONAL_OPTIONS"
    TEMPORARY_DIRECTORY = "TEMPORARY_DIRECTORY"
    HORIZONTAL_FEET = "HORIZONTAL_FEET"
    VERTICAL_FEET = "VERTICAL_FEET"
    FILES_ARE_FLIGHTLINES = "FILES_ARE_FLIGHTLINES"
    STEP = "STEP"
    FILTER_RETURN_CLASS_FLAGS1 = "FILTER_RETURN_CLASS_FLAGS1"
    FILTER_RETURN_CLASS_FLAGS2 = "FILTER_RETURN_CLASS_FLAGS2"
    FILTER_RETURN_CLASS_FLAGS3 = "FILTER_RETURN_CLASS_FLAGS3"
    FILTERS_RETURN_CLASS_FLAGS = ["---", "keep_last", "keep_first", "keep_middle", "keep_single", "drop_single",
                                  "keep_double", "keep_class 2", "keep_class 2 8", "keep_class 8", "keep_class 6",
                                  "keep_class 9", "keep_class 3 4 5", "keep_class 2 6", "drop_class 7", "drop_withheld"]
    FILTER_COORDS_INTENSITY1 = "FILTER_COORDS_INTENSITY1"
    FILTER_COORDS_INTENSITY2 = "FILTER_COORDS_INTENSITY2"
    FILTER_COORDS_INTENSITY3 = "FILTER_COORDS_INTENSITY3"
    FILTER_COORDS_INTENSITY1_ARG = "FILTER_COORDS_INTENSITY1_ARG"
    FILTER_COORDS_INTENSITY2_ARG = "FILTER_COORDS_INTENSITY2_ARG"
    FILTER_COORDS_INTENSITY3_ARG = "FILTER_COORDS_INTENSITY3_ARG"
    FILTERS_COORDS_INTENSITY = ["---", "clip_x_above", "clip_x_below", "clip_y_above", "clip_y_below", "clip_z_above",
                                "clip_z_below", "drop_intensity_above", "drop_intensity_below", "drop_gps_time_above",
                                "drop_gps_time_below", "drop_scan_angle_above", "drop_scan_angle_below", "keep_point_source",
                                "drop_point_source", "drop_point_source_above", "drop_point_source_below", "keep_user_data",
                                "drop_user_data", "drop_user_data_above", "drop_user_data_below", "keep_every_nth",
                                "keep_random_fraction", "thin_with_grid" ]

    TRANSFORM_COORDINATE1 = "TRANSFORM_COORDINATE1"
    TRANSFORM_COORDINATE2 = "TRANSFORM_COORDINATE2"
    TRANSFORM_COORDINATE1_ARG = "TRANSFORM_COORDINATE1_ARG"
    TRANSFORM_COORDINATE2_ARG = "TRANSFORM_COORDINATE2_ARG"
    TRANSFORM_COORDINATES = ["---", "translate_x", "translate_y", "translate_z", "scale_x", "scale_y", "scale_z", "clamp_z_above", "clamp_z_below"]

    TRANSFORM_OTHER1 = "TRANSFORM_OTHER1"
    TRANSFORM_OTHER2 = "TRANSFORM_OTHER2"
    TRANSFORM_OTHER1_ARG = "TRANSFORM_OTHER1_ARG"
    TRANSFORM_OTHER2_ARG = "TRANSFORM_OTHER2_ARG"
    TRANSFORM_OTHERS = ["---", "scale_intensity", "translate_intensity",  "clamp_intensity_above", "clamp_intensity_below",
                        "scale_scan_angle", "translate_scan_angle", "translate_gps_time", "set_classification", "set_user_data",
                        "set_point_source", "scale_rgb_up", "scale_rgb_down", "repair_zero_returns" ]

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/../../../images/tool.png"
        return QtGui.QIcon(filepath)

    def checkBeforeOpeningParametersDialog(self):
        path = LAStoolsUtils.LAStoolsPath()
        if path == "":
            return "LAStools folder is not configured.\nPlease configure it before running LAStools algorithms."

    def addParametersVerboseGUI(self):
        self.addParameter(ParameterBoolean(LAStoolsAlgorithm.VERBOSE, "verbose", False))

    def addParametersVerboseCommands(self, commands):
        if self.getParameterValue(LAStoolsAlgorithm.VERBOSE):
            commands.append("-v")

    def addParametersCoresGUI(self):
        self.addParameter(ParameterNumber(LAStoolsAlgorithm.CORES, "number of cores", 1, 32, 4))

    def addParametersCoresCommands(self, commands):
        cores = self.getParameterValue(LAStoolsAlgorithm.CORES)
        if cores != 1:
            commands.append("-cores")
            commands.append(str(cores))

    def addParametersPointInputGUI(self):
        self.addParameter(ParameterFile(LAStoolsAlgorithm.INPUT_LASLAZ, "input LAS/LAZ file"))

    def addParametersPointInputCommands(self, commands):
        input = self.getParameterValue(LAStoolsAlgorithm.INPUT_LASLAZ)
        if input != None:
            commands.append("-i")
            commands.append(input)

    def addParametersPointInputFolderGUI(self):
        self.addParameter(ParameterFile(LAStoolsAlgorithm.INPUT_DIRECTORY, "input directory", True))
        self.addParameter(ParameterString(LAStoolsAlgorithm.INPUT_WILDCARDS, "input wildcard(s)", "*.laz"))

    def addParametersPointInputFolderCommands(self, commands):
        input = self.getParameterValue(LAStoolsAlgorithm.INPUT_DIRECTORY)
        wildcards = self.getParameterValue(LAStoolsAlgorithm.INPUT_WILDCARDS).split()
        for wildcard in wildcards:
            commands.append("-i")
            if input != None:
                commands.append('"' + input + "\\" + wildcard + '"')
            else:
                commands.append('"' + wildcard + '"')

    def addParametersHorizontalFeetGUI(self):
        self.addParameter(ParameterBoolean(LAStoolsAlgorithm.HORIZONTAL_FEET, "horizontal feet", False))

    def addParametersHorizontalFeetCommands(self, commands):
        if self.getParameterValue(LAStoolsAlgorithm.HORIZONTAL_FEET):
            commands.append("-feet")

    def addParametersVerticalFeetGUI(self):
        self.addParameter(ParameterBoolean(LAStoolsAlgorithm.VERTICAL_FEET, "vertical feet", False))

    def addParametersVerticalFeetCommands(self, commands):
        if self.getParameterValue(LAStoolsAlgorithm.VERTICAL_FEET):
            commands.append("-elevation_feet")

    def addParametersHorizontalAndVerticalFeetGUI(self):
        self.addParametersHorizontalFeetGUI()
        self.addParametersVerticalFeetGUI()

    def addParametersHorizontalAndVerticalFeetCommands(self, commands):
        self.addParametersHorizontalFeetCommands(commands)
        self.addParametersVerticalFeetCommands(commands)

    def addParametersFilesAreFlightlinesGUI(self):
        self.addParameter(ParameterBoolean(LAStoolsAlgorithm.FILES_ARE_FLIGHTLINES, "files are flightlines", True))

    def addParametersFilesAreFlightlinesCommands(self, commands):
        if self.getParameterValue(LAStoolsAlgorithm.FILES_ARE_FLIGHTLINES):
            commands.append("-files_are_flightlines")

    def addParametersStepGUI(self):
        self.addParameter(ParameterNumber(LAStoolsAlgorithm.STEP, "step size / pixel size", 0, None, 1.0))

    def addParametersStepCommands(self, commands):
        step = self.getParameterValue(LAStoolsAlgorithm.STEP)
        if step != 0.0:
            commands.append("-step")
            commands.append(str(step))

    def getParametersStepValue(self):
        step = self.getParameterValue(LAStoolsAlgorithm.STEP)
        return step

    def addParametersPointOutputGUI(self):
        self.addOutput(OutputFile(LAStoolsAlgorithm.OUTPUT_LASLAZ, "output LAS/LAZ file", "laz"))

    def addParametersPointOutputCommands(self, commands):
        output = self.getOutputValue(LAStoolsAlgorithm.OUTPUT_LASLAZ)
        if output != None:
            commands.append("-o")
            commands.append(output)

    def addParametersPointOutputFormatGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.OUTPUT_POINT_FORMAT, "output format", LAStoolsAlgorithm.OUTPUT_POINT_FORMATS, 0))

    def addParametersPointOutputFormatCommands(self, commands):
        format = self.getParameterValue(LAStoolsAlgorithm.OUTPUT_POINT_FORMAT)
        commands.append("-o" + LAStoolsAlgorithm.OUTPUT_POINT_FORMATS[format])

    def addParametersRasterOutputGUI(self):
        self.addOutput(OutputRaster(LAStoolsAlgorithm.OUTPUT_RASTER, "Output raster file"))

    def addParametersRasterOutputCommands(self, commands):
        commands.append("-o")
        commands.append(self.getOutputValue(LAStoolsAlgorithm.OUTPUT_RASTER))

    def addParametersRasterOutputFormatGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.OUTPUT_RASTER_FORMAT, "output format", LAStoolsAlgorithm.OUTPUT_RASTER_FORMATS, 0))

    def addParametersRasterOutputFormatCommands(self, commands):
        format = self.getParameterValue(LAStoolsAlgorithm.OUTPUT_RASTER_FORMAT)
        commands.append("-o" + LAStoolsAlgorithm.OUTPUT_RASTER_FORMATS[format])

    def addParametersVectorOutputGUI(self):
        self.addOutput(OutputVector(LAStoolsAlgorithm.OUTPUT_VECTOR, "Output vector file"))

    def addParametersVectorOutputCommands(self, commands):
        commands.append("-o")
        commands.append(self.getOutputValue(LAStoolsAlgorithm.OUTPUT_VECTOR))

    def addParametersVectorOutputFormatGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.OUTPUT_VECTOR_FORMAT, "output format", LAStoolsAlgorithm.OUTPUT_VECTOR_FORMATS, 0))

    def addParametersVectorOutputFormatCommands(self, commands):
        format = self.getParameterValue(LAStoolsAlgorithm.OUTPUT_VECTOR_FORMAT)
        commands.append("-o" + LAStoolsAlgorithm.OUTPUT_VECTOR_FORMATS[format])

    def addParametersOutputDirectoryGUI(self):
        self.addParameter(ParameterFile(LAStoolsAlgorithm.OUTPUT_DIRECTORY, "output directory", True))

    def addParametersOutputDirectoryCommands(self, commands):
        odir = self.getParameterValue(LAStoolsAlgorithm.OUTPUT_DIRECTORY)
        if odir != "":
            commands.append("-odir")
            commands.append(odir)

    def addParametersOutputAppendixGUI(self):
        self.addParameter(ParameterString(LAStoolsAlgorithm.OUTPUT_APPENDIX, "output appendix"))

    def addParametersOutputAppendixCommands(self, commands):
        odix = self.getParameterValue(LAStoolsAlgorithm.OUTPUT_APPENDIX)
        if odix != "":
            commands.append("-odix")
            commands.append(odix)

    def addParametersTemporaryDirectoryGUI(self):
        self.addParameter(ParameterFile(LAStoolsAlgorithm.TEMPORARY_DIRECTORY, "empty temporary directory", True))

    def addParametersTemporaryDirectoryAsOutputDirectoryCommands(self, commands):
        odir = self.getParameterValue(LAStoolsAlgorithm.TEMPORARY_DIRECTORY)
        if odir != "":
            commands.append("-odir")
            commands.append(odir)

    def addParametersTemporaryDirectoryAsInputFilesCommands(self, commands, files):
        idir = self.getParameterValue(LAStoolsAlgorithm.TEMPORARY_DIRECTORY)
        if idir != "":
            commands.append("-i")
            commands.append(idir+'\\'+files)

    def addParametersAdditionalGUI(self):
        self.addParameter(ParameterString(LAStoolsAlgorithm.ADDITIONAL_OPTIONS, "additional command line parameter(s)"))

    def addParametersAdditionalCommands(self, commands):
        additional_options = self.getParameterValue(LAStoolsAlgorithm.ADDITIONAL_OPTIONS).split()
        for option in additional_options:
            commands.append(option)

    def addParametersFilter1ReturnClassFlagsGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS1, "filter (by return, classification, flags)",
                                             LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS, 0))

    def addParametersFilter1ReturnClassFlagsCommands(self, commands):
        filter1 = self.getParameterValue(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS1)
        if filter1 != 0:
            commands.append("-" + LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS[filter1])

    def addParametersFilter2ReturnClassFlagsGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS2, "second filter (by return, classification, flags)",
                                             LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS, 0))

    def addParametersFilter2ReturnClassFlagsCommands(self, commands):
        filter2 = self.getParameterValue(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS2)
        if filter2 != 0:
            commands.append("-" + LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS[filter2])

    def addParametersFilter3ReturnClassFlagsGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS3, "third filter (by return, classification, flags)",
                                             LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS, 0))

    def addParametersFilter3ReturnClassFlagsCommands(self, commands):
        filter3 = self.getParameterValue(LAStoolsAlgorithm.FILTER_RETURN_CLASS_FLAGS3)
        if filter3 != 0:
            commands.append("-" + LAStoolsAlgorithm.FILTERS_RETURN_CLASS_FLAGS[filter3])

    def addParametersFilter1CoordsIntensityGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY1, "filter (by coordinate, intensity, GPS time, ...)",
                                             LAStoolsAlgorithm.FILTERS_COORDS_INTENSITY, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY1_ARG, "value for filter (by coordinate, intensity, GPS time, ...)"))

    def addParametersFilter1CoordsIntensityCommands(self, commands):
        filter1 = self.getParameterValue(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY1)
        filter1_arg = self.getParameterValue(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY1_ARG)
        if filter1 != 0 and filter1_arg != None:
            commands.append("-" + LAStoolsAlgorithm.FILTERS_COORDS_INTENSITY[filter1])
            commands.append(filter1_arg)

    def addParametersFilter2CoordsIntensityGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY2, "second filter (by coordinate, intensity, GPS time, ...)", LAStoolsAlgorithm.FILTERS_COORDS_INTENSITY, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY2_ARG, "value for second filter (by coordinate, intensity, GPS time, ...)"))

    def addParametersFilter2CoordsIntensityCommands(self, commands):
        filter2 = self.getParameterValue(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY2)
        filter2_arg = self.getParameterValue(LAStoolsAlgorithm.FILTER_COORDS_INTENSITY2_ARG)
        if filter2 != 0 and filter2_arg != None:
            commands.append("-" + LAStoolsAlgorithm.FILTERS_COORDS_INTENSITY[filter2])
            commands.append(filter2_arg)

    def addParametersTransform1CoordinateGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.TRANSFORM_COORDINATE1,
                                             "transform (coordinates)", LAStoolsAlgorithm.TRANSFORM_COORDINATES, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.TRANSFORM_COORDINATE1_ARG,
                                          "value for transform (coordinates)"))

    def addParametersTransform1CoordinateCommands(self, commands):
        transform1 = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_COORDINATE1)
        transform1_arg = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_COORDINATE1_ARG)
        if transform1 != 0 and transform1_arg != None:
            commands.append("-" + LAStoolsAlgorithm.TRANSFORM_COORDINATES[transform1])
            commands.append(transform1_arg)

    def addParametersTransform2CoordinateGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.TRANSFORM_COORDINATE2,
                                             "second transform (coordinates)", LAStoolsAlgorithm.TRANSFORM_COORDINATES, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.TRANSFORM_COORDINATE2_ARG,
                                          "value for second transform (coordinates)"))

    def addParametersTransform2CoordinateCommands(self, commands):
        transform2 = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_COORDINATE2)
        transform2_arg = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_COORDINATE2_ARG)
        if transform2 != 0 and transform2_arg != None:
            commands.append("-" + LAStoolsAlgorithm.TRANSFORM_COORDINATES[transform2])
            commands.append(transform2_arg)

    def addParametersTransform1OtherGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.TRANSFORM_OTHER1,
                                             "transform (intensities, scan angles, GPS times, ...)", LAStoolsAlgorithm.TRANSFORM_OTHERS, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.TRANSFORM_OTHER1_ARG,
                                          "value for transform (intensities, scan angles, GPS times, ...)"))

    def addParametersTransform1OtherCommands(self, commands):
        transform1 = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_OTHER1)
        transform1_arg = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_OTHER1_ARG)
        if transform1 != 0:
            commands.append("-" + LAStoolsAlgorithm.TRANSFORM_OTHERS[transform1])
            if transform1 < 11 and transform1_arg != None:
                commands.append(transform1_arg)

    def addParametersTransform2OtherGUI(self):
        self.addParameter(ParameterSelection(LAStoolsAlgorithm.TRANSFORM_OTHER2,
                                             "second transform (intensities, scan angles, GPS times, ...)", LAStoolsAlgorithm.TRANSFORM_OTHERS, 0))
        self.addParameter(ParameterString(LAStoolsAlgorithm.TRANSFORM_OTHER2_ARG,
                                          "value for second transform (intensities, scan angles, GPS times, ...)"))

    def addParametersTransform2OtherCommands(self, commands):
        transform2 = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_OTHER2)
        transform2_arg = self.getParameterValue(LAStoolsAlgorithm.TRANSFORM_OTHER2_ARG)
        if transform2 != 0:
            commands.append("-" + LAStoolsAlgorithm.TRANSFORM_OTHERS[transform2])
            if transform2 < 11 and transform2_arg != None:
                commands.append(transform2_arg)
