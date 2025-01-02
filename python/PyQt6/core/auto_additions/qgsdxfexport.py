# The following has been generated automatically from src/core/dxf/qgsdxfexport.h
QgsDxfExport.FlagNoMText = QgsDxfExport.Flag.FlagNoMText
QgsDxfExport.FlagOnlySelectedFeatures = QgsDxfExport.Flag.FlagOnlySelectedFeatures
QgsDxfExport.FlagHairlineWidthExport = QgsDxfExport.Flag.FlagHairlineWidthExport
QgsDxfExport.Flags = lambda flags=0: QgsDxfExport.Flag(flags)
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
QgsDxfExport.Closed = QgsDxfExport.DxfPolylineFlag.Closed
QgsDxfExport.Curve = QgsDxfExport.DxfPolylineFlag.Curve
QgsDxfExport.Spline = QgsDxfExport.DxfPolylineFlag.Spline
QgsDxfExport.Is3DPolyline = QgsDxfExport.DxfPolylineFlag.Is3DPolyline
QgsDxfExport.Is3DPolygonMesh = QgsDxfExport.DxfPolylineFlag.Is3DPolygonMesh
QgsDxfExport.PolygonMesh = QgsDxfExport.DxfPolylineFlag.PolygonMesh
QgsDxfExport.PolyfaceMesh = QgsDxfExport.DxfPolylineFlag.PolyfaceMesh
QgsDxfExport.ContinuousPattern = QgsDxfExport.DxfPolylineFlag.ContinuousPattern
QgsDxfExport.DxfPolylineFlags = lambda flags=0: QgsDxfExport.DxfPolylineFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsDxfExport.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsDxfExport.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsDxfExport.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsDxfExport.Flag.__or__ = lambda flag1, flag2: QgsDxfExport.Flag(_force_int(flag1) | _force_int(flag2))
QgsDxfExport.DxfPolylineFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsDxfExport.DxfPolylineFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsDxfExport.DxfPolylineFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsDxfExport.DxfPolylineFlag.__or__ = lambda flag1, flag2: QgsDxfExport.DxfPolylineFlag(_force_int(flag1) | _force_int(flag2))
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
