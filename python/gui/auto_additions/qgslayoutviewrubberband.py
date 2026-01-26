# The following has been generated automatically from src/gui/layout/qgslayoutviewrubberband.h
try:
    QgsLayoutViewRubberBand.__attribute_docs__ = {'sizeChanged': 'Emitted when the size of the rubber band is changed. The ``size``\nargument gives a translated string describing the new rubber band size,\nwith a format which differs per subclass (e.g. rectangles may describe a\nsize using width and height, while circles may describe a size by\nradius).\n'}
    QgsLayoutViewRubberBand.__abstract_methods__ = ['create', 'start', 'update', 'finish']
    QgsLayoutViewRubberBand.__signal_arguments__ = {'sizeChanged': ['size: str']}
    QgsLayoutViewRubberBand.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutViewRectangularRubberBand.__overridden_methods__ = ['create', 'start', 'update', 'finish']
    QgsLayoutViewRectangularRubberBand.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutViewEllipticalRubberBand.__overridden_methods__ = ['create', 'start', 'update', 'finish']
    QgsLayoutViewEllipticalRubberBand.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutViewTriangleRubberBand.__overridden_methods__ = ['create', 'start', 'update', 'finish']
    QgsLayoutViewTriangleRubberBand.__group__ = ['layout']
except (NameError, AttributeError):
    pass
