# The following has been generated automatically from src/core/qgssnappingconfig.h
QgsSnappingConfig.Vertex = QgsSnappingConfig.SnappingType.Vertex
QgsSnappingConfig.VertexAndSegment = QgsSnappingConfig.SnappingType.VertexAndSegment
QgsSnappingConfig.Segment = QgsSnappingConfig.SnappingType.Segment
QgsSnappingConfig.Disabled = QgsSnappingConfig.ScaleDependencyMode.Disabled
QgsSnappingConfig.Global = QgsSnappingConfig.ScaleDependencyMode.Global
QgsSnappingConfig.PerLayer = QgsSnappingConfig.ScaleDependencyMode.PerLayer
QgsSnappingConfig.ScaleDependencyMode.baseClass = QgsSnappingConfig
try:
    QgsSnappingConfig.snappingTypeToString = staticmethod(QgsSnappingConfig.snappingTypeToString)
    QgsSnappingConfig.snappingTypeFlagToString = staticmethod(QgsSnappingConfig.snappingTypeFlagToString)
    QgsSnappingConfig.snappingTypeToIcon = staticmethod(QgsSnappingConfig.snappingTypeToIcon)
    QgsSnappingConfig.snappingTypeFlagToIcon = staticmethod(QgsSnappingConfig.snappingTypeFlagToIcon)
except NameError:
    pass
