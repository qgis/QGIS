# The following has been generated automatically from src/core/qgsmaptopixelgeometrysimplifier.h
QgsMapToPixelSimplifier.NoFlags = QgsMapToPixelSimplifier.SimplifyFlag.NoFlags
QgsMapToPixelSimplifier.SimplifyGeometry = QgsMapToPixelSimplifier.SimplifyFlag.SimplifyGeometry
QgsMapToPixelSimplifier.SimplifyEnvelope = QgsMapToPixelSimplifier.SimplifyFlag.SimplifyEnvelope
try:
    QgsMapToPixelSimplifier.calculateLengthSquared2D = staticmethod(QgsMapToPixelSimplifier.calculateLengthSquared2D)
    QgsMapToPixelSimplifier.equalSnapToGrid = staticmethod(QgsMapToPixelSimplifier.equalSnapToGrid)
    QgsMapToPixelSimplifier.__overridden_methods__ = ['simplify']
except (NameError, AttributeError):
    pass
