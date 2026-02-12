# The following has been generated automatically from src/core/layout/qgslayoutitempage.h
QgsLayoutItemPage.Portrait = QgsLayoutItemPage.Orientation.Portrait
QgsLayoutItemPage.Landscape = QgsLayoutItemPage.Orientation.Landscape
QgsLayoutItemPage.UndoPageSymbol = QgsLayoutItemPage.UndoCommand.UndoPageSymbol
try:
    QgsLayoutItemPage.create = staticmethod(QgsLayoutItemPage.create)
    QgsLayoutItemPage.decodePageOrientation = staticmethod(QgsLayoutItemPage.decodePageOrientation)
    QgsLayoutItemPage.__overridden_methods__ = ['type', 'displayName', 'boundingRect', 'attemptResize', 'createCommand', 'exportLayerBehavior', 'accept', 'redraw', 'draw', 'drawFrame', 'drawBackground', 'writePropertiesToElement', 'readPropertiesFromElement']
    QgsLayoutItemPage.__group__ = ['layout']
except (NameError, AttributeError):
    pass
