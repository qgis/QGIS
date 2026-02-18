from qgis.core import (
    QgsProcessing,
    QgsProcessingAlgorithm,
    QgsProcessingParameterNumber,
)

a = QgsProcessing.SourceType.TypeVectorLine
b = QgsProcessingParameterNumber.Type.Double
c = QgsProcessingAlgorithm.Flag.FlagNoThreading
