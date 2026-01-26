"""
***************************************************************************
    g_extension_list.py
    ------------------
    Date                 : May 2023
    Copyright            : (C) 2023 by Alister Hood
    Email                : alister.hood at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alister Hood"
__date__ = "May 2023"
__copyright__ = "(C) 2023, Alister Hood"

from qgis.core import QgsProcessingParameterBoolean


def processInputs(alg, parameters, context, feedback):
    # get the option we want to run
    # we would need to update the if statements below if the order in the description file is changed
    selectedOption = alg.parameterAsInt(parameters, "list", context)
    # Locally installed extensions
    if selectedOption == 0:
        optionString = "-a"
    # Extensions available in the official GRASS GIS Addons repository
    elif selectedOption == 1:
        optionString = "-l"
    # Extensions available in the official GRASS GIS Addons repository including module description
    else:
        optionString = "-c"
    param = QgsProcessingParameterBoolean(optionString, "", True)
    alg.addParameter(param)
    # Don't forget to remove the old input parameter
    alg.removeParameter("list")


def convertToHtml(alg, fileName, outputs):
    # don't copy this for something that will have lots of data like r.stats
    # it will be very slow
    with open(fileName) as f:
        outputs["GRASS_ADDONS"] = f.read()
    # this will read the file a second time, but calling it saves us duplicating the code here
    alg.convertToHtml(fileName)
