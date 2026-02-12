# The following has been generated automatically from src/core/plot/qgsplot.h
QgsPlot.Property = QgsPlot.DataDefinedProperty
# monkey patching scoped based enum
QgsPlot.MarginLeft = QgsPlot.DataDefinedProperty.MarginLeft
QgsPlot.MarginLeft.is_monkey_patched = True
QgsPlot.MarginLeft.__doc__ = "Left margin"
QgsPlot.MarginTop = QgsPlot.DataDefinedProperty.MarginTop
QgsPlot.MarginTop.is_monkey_patched = True
QgsPlot.MarginTop.__doc__ = "Top margin"
QgsPlot.MarginRight = QgsPlot.DataDefinedProperty.MarginRight
QgsPlot.MarginRight.is_monkey_patched = True
QgsPlot.MarginRight.__doc__ = "Right margin"
QgsPlot.MarginBottom = QgsPlot.DataDefinedProperty.MarginBottom
QgsPlot.MarginBottom.is_monkey_patched = True
QgsPlot.MarginBottom.__doc__ = "Bottom margin"
QgsPlot.XAxisMajorInterval = QgsPlot.DataDefinedProperty.XAxisMajorInterval
QgsPlot.XAxisMajorInterval.is_monkey_patched = True
QgsPlot.XAxisMajorInterval.__doc__ = "Major grid line interval for X axis"
QgsPlot.XAxisMinorInterval = QgsPlot.DataDefinedProperty.XAxisMinorInterval
QgsPlot.XAxisMinorInterval.is_monkey_patched = True
QgsPlot.XAxisMinorInterval.__doc__ = "Minor grid line interval for X axis"
QgsPlot.XAxisLabelInterval = QgsPlot.DataDefinedProperty.XAxisLabelInterval
QgsPlot.XAxisLabelInterval.is_monkey_patched = True
QgsPlot.XAxisLabelInterval.__doc__ = "Label interval for X axis"
QgsPlot.YAxisMajorInterval = QgsPlot.DataDefinedProperty.YAxisMajorInterval
QgsPlot.YAxisMajorInterval.is_monkey_patched = True
QgsPlot.YAxisMajorInterval.__doc__ = "Major grid line interval for Y axis"
QgsPlot.YAxisMinorInterval = QgsPlot.DataDefinedProperty.YAxisMinorInterval
QgsPlot.YAxisMinorInterval.is_monkey_patched = True
QgsPlot.YAxisMinorInterval.__doc__ = "Minor grid line interval for Y axis"
QgsPlot.YAxisLabelInterval = QgsPlot.DataDefinedProperty.YAxisLabelInterval
QgsPlot.YAxisLabelInterval.is_monkey_patched = True
QgsPlot.YAxisLabelInterval.__doc__ = "Label interval for Y axis"
QgsPlot.XAxisMinimum = QgsPlot.DataDefinedProperty.XAxisMinimum
QgsPlot.XAxisMinimum.is_monkey_patched = True
QgsPlot.XAxisMinimum.__doc__ = "Minimum X axis value"
QgsPlot.XAxisMaximum = QgsPlot.DataDefinedProperty.XAxisMaximum
QgsPlot.XAxisMaximum.is_monkey_patched = True
QgsPlot.XAxisMaximum.__doc__ = "Maximum X axis value"
QgsPlot.YAxisMinimum = QgsPlot.DataDefinedProperty.YAxisMinimum
QgsPlot.YAxisMinimum.is_monkey_patched = True
QgsPlot.YAxisMinimum.__doc__ = "Minimum Y axis value"
QgsPlot.YAxisMaximum = QgsPlot.DataDefinedProperty.YAxisMaximum
QgsPlot.YAxisMaximum.is_monkey_patched = True
QgsPlot.YAxisMaximum.__doc__ = "Maximum Y axis value"
QgsPlot.DataDefinedProperty.__doc__ = """Data defined properties for different plot types

.. versionadded:: 4.0

* ``MarginLeft``: Left margin
* ``MarginTop``: Top margin
* ``MarginRight``: Right margin
* ``MarginBottom``: Bottom margin
* ``XAxisMajorInterval``: Major grid line interval for X axis
* ``XAxisMinorInterval``: Minor grid line interval for X axis
* ``XAxisLabelInterval``: Label interval for X axis
* ``YAxisMajorInterval``: Major grid line interval for Y axis
* ``YAxisMinorInterval``: Minor grid line interval for Y axis
* ``YAxisLabelInterval``: Label interval for Y axis
* ``XAxisMinimum``: Minimum X axis value
* ``XAxisMaximum``: Maximum X axis value
* ``YAxisMinimum``: Minimum Y axis value
* ``YAxisMaximum``: Maximum Y axis value

"""
# --
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
