from qgis.core import (
    QgsProcessing,
    QgsProcessingAlgorithm,
    QgsProcessingParameterNumber,
)

a = QgsProcessing.TypeVectorLine
b = QgsProcessingParameterNumber.Double
c = QgsProcessingAlgorithm.FlagNoThreading
