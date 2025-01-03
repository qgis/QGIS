# The following has been generated automatically from src/core/symbology/qgsgraduatedsymbolrenderer.h
QgsGraduatedSymbolRenderer.EqualInterval = QgsGraduatedSymbolRenderer.Mode.EqualInterval
QgsGraduatedSymbolRenderer.Quantile = QgsGraduatedSymbolRenderer.Mode.Quantile
QgsGraduatedSymbolRenderer.Jenks = QgsGraduatedSymbolRenderer.Mode.Jenks
QgsGraduatedSymbolRenderer.StdDev = QgsGraduatedSymbolRenderer.Mode.StdDev
QgsGraduatedSymbolRenderer.Pretty = QgsGraduatedSymbolRenderer.Mode.Pretty
QgsGraduatedSymbolRenderer.Custom = QgsGraduatedSymbolRenderer.Mode.Custom
try:
    QgsGraduatedSymbolRenderer.makeBreaksSymmetric = staticmethod(QgsGraduatedSymbolRenderer.makeBreaksSymmetric)
    QgsGraduatedSymbolRenderer.calcEqualIntervalBreaks = staticmethod(QgsGraduatedSymbolRenderer.calcEqualIntervalBreaks)
    QgsGraduatedSymbolRenderer.createRenderer = staticmethod(QgsGraduatedSymbolRenderer.createRenderer)
    QgsGraduatedSymbolRenderer.create = staticmethod(QgsGraduatedSymbolRenderer.create)
    QgsGraduatedSymbolRenderer.convertFromRenderer = staticmethod(QgsGraduatedSymbolRenderer.convertFromRenderer)
    QgsGraduatedSymbolRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
