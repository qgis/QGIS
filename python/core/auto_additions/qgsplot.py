# The following has been generated automatically from src/core/plot/qgsplot.h
try:
    QgsPlotDefaultSettings.axisLabelNumericFormat = staticmethod(QgsPlotDefaultSettings.axisLabelNumericFormat)
    QgsPlotDefaultSettings.axisGridMajorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMajorSymbol)
    QgsPlotDefaultSettings.axisGridMinorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMinorSymbol)
    QgsPlotDefaultSettings.chartBackgroundSymbol = staticmethod(QgsPlotDefaultSettings.chartBackgroundSymbol)
    QgsPlotDefaultSettings.chartBorderSymbol = staticmethod(QgsPlotDefaultSettings.chartBorderSymbol)
    QgsPlotDefaultSettings.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlot.__virtual_methods__ = ['writeXml', 'readXml']
    QgsPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractPlotSeries.__virtual_methods__ = ['categories']
    QgsAbstractPlotSeries.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    Qgs2DPlot.__virtual_methods__ = ['render', 'renderContent', 'interiorPlotArea']
    Qgs2DPlot.__overridden_methods__ = ['writeXml', 'readXml']
    Qgs2DPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsXyPlotSeries.__overridden_methods__ = ['categories']
    QgsXyPlotSeries.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    Qgs2DXyPlot.__overridden_methods__ = ['writeXml', 'readXml', 'render', 'interiorPlotArea']
    Qgs2DXyPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlotData.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlotAxis.__group__ = ['plot']
except (NameError, AttributeError):
    pass
