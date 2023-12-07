# The following has been generated automatically from src/gui/ogr/qgsvectorlayersaveasdialog.h
# monkey patching scoped based enum
QgsVectorLayerSaveAsDialog.Option.Symbology.__doc__ = "Show symbology options"
QgsVectorLayerSaveAsDialog.Symbology = QgsVectorLayerSaveAsDialog.Option.Symbology
QgsVectorLayerSaveAsDialog.Option.DestinationCrs.__doc__ = "Show destination CRS (reprojection) option"
QgsVectorLayerSaveAsDialog.DestinationCrs = QgsVectorLayerSaveAsDialog.Option.DestinationCrs
QgsVectorLayerSaveAsDialog.Option.Fields.__doc__ = "Show field customization group"
QgsVectorLayerSaveAsDialog.Fields = QgsVectorLayerSaveAsDialog.Option.Fields
QgsVectorLayerSaveAsDialog.Option.AddToCanvas.__doc__ = "Show add to map option"
QgsVectorLayerSaveAsDialog.AddToCanvas = QgsVectorLayerSaveAsDialog.Option.AddToCanvas
QgsVectorLayerSaveAsDialog.Option.SelectedOnly.__doc__ = "Show selected features only option"
QgsVectorLayerSaveAsDialog.SelectedOnly = QgsVectorLayerSaveAsDialog.Option.SelectedOnly
QgsVectorLayerSaveAsDialog.Option.GeometryType.__doc__ = "Show geometry group"
QgsVectorLayerSaveAsDialog.GeometryType = QgsVectorLayerSaveAsDialog.Option.GeometryType
QgsVectorLayerSaveAsDialog.Option.Extent.__doc__ = "Show extent group"
QgsVectorLayerSaveAsDialog.Extent = QgsVectorLayerSaveAsDialog.Option.Extent
QgsVectorLayerSaveAsDialog.Option.Metadata.__doc__ = "Show metadata options"
QgsVectorLayerSaveAsDialog.Metadata = QgsVectorLayerSaveAsDialog.Option.Metadata
QgsVectorLayerSaveAsDialog.Option.AllOptions.__doc__ = ""
QgsVectorLayerSaveAsDialog.AllOptions = QgsVectorLayerSaveAsDialog.Option.AllOptions
QgsVectorLayerSaveAsDialog.Option.__doc__ = "Available dialog options.\n\n" + '* ``Symbology``: ' + QgsVectorLayerSaveAsDialog.Option.Symbology.__doc__ + '\n' + '* ``DestinationCrs``: ' + QgsVectorLayerSaveAsDialog.Option.DestinationCrs.__doc__ + '\n' + '* ``Fields``: ' + QgsVectorLayerSaveAsDialog.Option.Fields.__doc__ + '\n' + '* ``AddToCanvas``: ' + QgsVectorLayerSaveAsDialog.Option.AddToCanvas.__doc__ + '\n' + '* ``SelectedOnly``: ' + QgsVectorLayerSaveAsDialog.Option.SelectedOnly.__doc__ + '\n' + '* ``GeometryType``: ' + QgsVectorLayerSaveAsDialog.Option.GeometryType.__doc__ + '\n' + '* ``Extent``: ' + QgsVectorLayerSaveAsDialog.Option.Extent.__doc__ + '\n' + '* ``Metadata``: ' + QgsVectorLayerSaveAsDialog.Option.Metadata.__doc__ + '\n' + '* ``AllOptions``: ' + QgsVectorLayerSaveAsDialog.Option.AllOptions.__doc__
# --
QgsVectorLayerSaveAsDialog.Option.baseClass = QgsVectorLayerSaveAsDialog
QgsVectorLayerSaveAsDialog.Options = lambda flags=0: QgsVectorLayerSaveAsDialog.Option(flags)
QgsVectorLayerSaveAsDialog.Options.baseClass = QgsVectorLayerSaveAsDialog
Options = QgsVectorLayerSaveAsDialog  # dirty hack since SIP seems to introduce the flags in module
