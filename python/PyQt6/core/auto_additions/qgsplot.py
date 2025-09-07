# The following has been generated automatically from src/core/plot/qgsplot.h
try:
    QgsPlotDefaultSettings.axisLabelNumericFormat = staticmethod(QgsPlotDefaultSettings.axisLabelNumericFormat)
    QgsPlotDefaultSettings.axisGridMajorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMajorSymbol)
    QgsPlotDefaultSettings.axisGridMinorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMinorSymbol)
    QgsPlotDefaultSettings.chartBackgroundSymbol = staticmethod(QgsPlotDefaultSettings.chartBackgroundSymbol)
    QgsPlotDefaultSettings.chartBorderSymbol = staticmethod(QgsPlotDefaultSettings.chartBorderSymbol)
    QgsPlotDefaultSettings.lineChartMarkerSymbol = staticmethod(QgsPlotDefaultSettings.lineChartMarkerSymbol)
    QgsPlotDefaultSettings.lineChartLineSymbol = staticmethod(QgsPlotDefaultSettings.lineChartLineSymbol)
    QgsPlotDefaultSettings.barChartFillSymbol = staticmethod(QgsPlotDefaultSettings.barChartFillSymbol)
    QgsPlotDefaultSettings.pieChartFillSymbol = staticmethod(QgsPlotDefaultSettings.pieChartFillSymbol)
    QgsPlotDefaultSettings.pieChartColorRamp = staticmethod(QgsPlotDefaultSettings.pieChartColorRamp)
    QgsPlotDefaultSettings.pieChartNumericFormat = staticmethod(QgsPlotDefaultSettings.pieChartNumericFormat)
    QgsPlotDefaultSettings.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlot.__virtual_methods__ = ['type', 'writeXml', 'readXml']
    QgsPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    Qgs2DPlot.__virtual_methods__ = ['render', 'renderContent', 'interiorPlotArea']
    Qgs2DPlot.__overridden_methods__ = ['writeXml', 'readXml']
    Qgs2DPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractPlotSeries.__abstract_methods__ = ['clone']
    QgsAbstractPlotSeries.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsXyPlotSeries.__overridden_methods__ = ['clone']
    QgsXyPlotSeries.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    Qgs2DXyPlot.__overridden_methods__ = ['writeXml', 'readXml', 'render', 'interiorPlotArea']
    Qgs2DXyPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlotRenderContext.__group__ = ['plot']
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
