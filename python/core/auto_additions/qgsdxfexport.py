# The following has been generated automatically from src/core/dxf/qgsdxfexport.h
# monkey patching scoped based enum
QgsDxfExport.ExportResult.Success.__doc__ = "Successful export"
QgsDxfExport.ExportResult.InvalidDeviceError.__doc__ = "Invalid device error"
QgsDxfExport.ExportResult.DeviceNotWritableError.__doc__ = "Device not writable error"
QgsDxfExport.ExportResult.EmptyExtentError.__doc__ = "Empty extent, no extent given and no extent could be derived from layers"
QgsDxfExport.ExportResult.__doc__ = """The result of an export as dxf operation

.. versionadded:: 3.10.1

* ``Success``: Successful export
* ``InvalidDeviceError``: Invalid device error
* ``DeviceNotWritableError``: Device not writable error
* ``EmptyExtentError``: Empty extent, no extent given and no extent could be derived from layers

"""
# --
# monkey patching scoped based enum
QgsDxfExport.VAlign.VBaseLine.__doc__ = "Top (0)"
QgsDxfExport.VAlign.VBottom.__doc__ = "Bottom (1)"
QgsDxfExport.VAlign.VMiddle.__doc__ = "Middle (2)"
QgsDxfExport.VAlign.VTop.__doc__ = "Top (3)"
QgsDxfExport.VAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.VAlign.__doc__ = """Vertical alignments.

* ``VBaseLine``: Top (0)
* ``VBottom``: Bottom (1)
* ``VMiddle``: Middle (2)
* ``VTop``: Top (3)
* ``Undefined``: Undefined

"""
# --
# monkey patching scoped based enum
QgsDxfExport.HAlign.HLeft.__doc__ = "Left (0)"
QgsDxfExport.HAlign.HCenter.__doc__ = "Centered (1)"
QgsDxfExport.HAlign.HRight.__doc__ = "Right (2)"
QgsDxfExport.HAlign.HAligned.__doc__ = "Aligned = (3) (if VAlign==0)"
QgsDxfExport.HAlign.HMiddle.__doc__ = "Middle = (4) (if VAlign==0)"
QgsDxfExport.HAlign.HFit.__doc__ = "Fit into point = (5) (if VAlign==0)"
QgsDxfExport.HAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.HAlign.__doc__ = """Horizontal alignments.

* ``HLeft``: Left (0)
* ``HCenter``: Centered (1)
* ``HRight``: Right (2)
* ``HAligned``: Aligned = (3) (if VAlign==0)
* ``HMiddle``: Middle = (4) (if VAlign==0)
* ``HFit``: Fit into point = (5) (if VAlign==0)
* ``Undefined``: Undefined

"""
# --
try:
    QgsDxfExport.closestColorMatch = staticmethod(QgsDxfExport.closestColorMatch)
    QgsDxfExport.mapUnitScaleFactor = staticmethod(QgsDxfExport.mapUnitScaleFactor)
    QgsDxfExport.dxfLayerName = staticmethod(QgsDxfExport.dxfLayerName)
    QgsDxfExport.dxfEncoding = staticmethod(QgsDxfExport.dxfEncoding)
    QgsDxfExport.encodings = staticmethod(QgsDxfExport.encodings)
    QgsDxfExport.__group__ = ['dxf']
except (NameError, AttributeError):
    pass
try:
    QgsDxfExport.DxfLayer.__doc__ = """Layers and optional attribute index to split
into multiple layers using attribute value as layer name."""
    QgsDxfExport.DxfLayer.__group__ = ['dxf']
except (NameError, AttributeError):
    pass
