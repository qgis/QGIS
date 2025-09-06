# The following has been generated automatically from src/gui/plot/qgsplotwidget.h
try:
    QgsBarChartPlotWidget.create = staticmethod(QgsBarChartPlotWidget.create)
    QgsBarChartPlotWidget.__overridden_methods__ = ['setPlot', 'createPlot']
    QgsBarChartPlotWidget.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsLineChartPlotWidget.create = staticmethod(QgsLineChartPlotWidget.create)
    QgsLineChartPlotWidget.__overridden_methods__ = ['setPlot', 'createPlot']
    QgsLineChartPlotWidget.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPieChartPlotWidget.create = staticmethod(QgsPieChartPlotWidget.create)
    QgsPieChartPlotWidget.__overridden_methods__ = ['setPlot', 'createPlot']
    QgsPieChartPlotWidget.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlotWidget.__abstract_methods__ = ['setPlot', 'createPlot']
    QgsPlotWidget.__overridden_methods__ = ['createExpressionContext']
    QgsPlotWidget.__group__ = ['plot']
except (NameError, AttributeError):
    pass
