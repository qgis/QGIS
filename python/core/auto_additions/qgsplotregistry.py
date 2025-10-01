# The following has been generated automatically from src/core/plot/qgsplotregistry.h
try:
    QgsPlotRegistry.__attribute_docs__ = {'plotAdded': 'Emitted whenever a new plot type is added to the registry, with the\nspecified ``type`` and visible ``name``.\n', 'plotAboutToBeRemoved': 'Emitted whenever a plot type is about to be remove from the registry,\nwith the specified ``type`` and visible ``name``.\n'}
    QgsPlotRegistry.__signal_arguments__ = {'plotAdded': ['type: str', 'name: str'], 'plotAboutToBeRemoved': ['type: str']}
    QgsPlotRegistry.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlotAbstractMetadata.__abstract_methods__ = ['createPlot', 'createPlotDataGatherer', 'createPlotWidget']
    QgsPlotAbstractMetadata.__group__ = ['plot']
except (NameError, AttributeError):
    pass
