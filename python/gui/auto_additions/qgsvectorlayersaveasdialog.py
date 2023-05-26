# The following has been generated automatically from src/gui/ogr/qgsvectorlayersaveasdialog.h
# monkey patching scoped based enum
QgsVectorLayerSaveAsDialog.Option.Symbology.__doc__ = "Show symbology options"
QgsVectorLayerSaveAsDialog.Option.DestinationCrs.__doc__ = "Show destination CRS (reprojection) option"
QgsVectorLayerSaveAsDialog.Option.Fields.__doc__ = "Show field customization group"
QgsVectorLayerSaveAsDialog.Option.AddToCanvas.__doc__ = "Show add to map option"
QgsVectorLayerSaveAsDialog.Option.SelectedOnly.__doc__ = "Show selected features only option"
QgsVectorLayerSaveAsDialog.Option.GeometryType.__doc__ = "Show geometry group"
QgsVectorLayerSaveAsDialog.Option.Extent.__doc__ = "Show extent group"
QgsVectorLayerSaveAsDialog.Option.Metadata.__doc__ = "Show metadata options"
QgsVectorLayerSaveAsDialog.Option.AllOptions.__doc__ = ""
QgsVectorLayerSaveAsDialog.Option.__doc__ = 'Available dialog options.\n\n' + '* ``Symbology``: ' + QgsVectorLayerSaveAsDialog.Option.Symbology.__doc__ + '\n' + '* ``DestinationCrs``: ' + QgsVectorLayerSaveAsDialog.Option.DestinationCrs.__doc__ + '\n' + '* ``Fields``: ' + QgsVectorLayerSaveAsDialog.Option.Fields.__doc__ + '\n' + '* ``AddToCanvas``: ' + QgsVectorLayerSaveAsDialog.Option.AddToCanvas.__doc__ + '\n' + '* ``SelectedOnly``: ' + QgsVectorLayerSaveAsDialog.Option.SelectedOnly.__doc__ + '\n' + '* ``GeometryType``: ' + QgsVectorLayerSaveAsDialog.Option.GeometryType.__doc__ + '\n' + '* ``Extent``: ' + QgsVectorLayerSaveAsDialog.Option.Extent.__doc__ + '\n' + '* ``Metadata``: ' + QgsVectorLayerSaveAsDialog.Option.Metadata.__doc__ + '\n' + '* ``AllOptions``: ' + QgsVectorLayerSaveAsDialog.Option.AllOptions.__doc__
# --
QgsVectorLayerSaveAsDialog.Option.baseClass = QgsVectorLayerSaveAsDialog
QgsVectorLayerSaveAsDialog.Options.baseClass = QgsVectorLayerSaveAsDialog
Options = QgsVectorLayerSaveAsDialog  # dirty hack since SIP seems to introduce the flags in module
