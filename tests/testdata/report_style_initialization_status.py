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

from qgis.core import QgsProcessingAlgorithm, QgsProcessingOutputBoolean, QgsStyle


class CheckStyleInitializationStatus(QgsProcessingAlgorithm):
    IS_INITIALIZED = "IS_INITIALIZED"

    def createInstance(self):
        return CheckStyleInitializationStatus()

    def name(self):
        return "checkstyleinitstatus"

    def displayName(self):
        return "Check style initialization status"

    def shortDescription(self):
        return "Checks style initialization status"

    def initAlgorithm(self, config=None):
        self.addOutput(
            QgsProcessingOutputBoolean(self.IS_INITIALIZED, "Style is initialized")
        )

    def processAlgorithm(self, parameters, context, feedback):
        return {self.IS_INITIALIZED: QgsStyle.defaultStyle(False).isInitialized()}
