# The following has been generated automatically from src/core/plot/qgspiechartplot.h
# monkey patching scoped based enum
QgsPieChartPlot.LabelType.NoLabels.__doc__ = "Labels are not drawn"
QgsPieChartPlot.LabelType.CategoryLabels.__doc__ = "Category labels are drawn"
QgsPieChartPlot.LabelType.ValueLabels.__doc__ = "Value labels are drawn"
QgsPieChartPlot.LabelType.__doc__ = """The label types available to pie charts.

* ``NoLabels``: Labels are not drawn
* ``CategoryLabels``: Category labels are drawn
* ``ValueLabels``: Value labels are drawn

"""
# --
try:
    QgsPieChartPlot.create = staticmethod(QgsPieChartPlot.create)
    QgsPieChartPlot.createDataGatherer = staticmethod(QgsPieChartPlot.createDataGatherer)
    QgsPieChartPlot.__overridden_methods__ = ['type', 'renderContent', 'writeXml', 'readXml']
    QgsPieChartPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
