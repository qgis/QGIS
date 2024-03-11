# The following has been generated automatically from src/core/dxf/qgsdxfexport.h
QgsDxfExport.FlagNoMText = QgsDxfExport.Flag.FlagNoMText
QgsDxfExport.FlagOnlySelectedFeatures = QgsDxfExport.Flag.FlagOnlySelectedFeatures
QgsDxfExport.Flags = lambda flags=0: QgsDxfExport.Flag(flags)
# monkey patching scoped based enum
QgsDxfExport.ExportResult.Success.__doc__ = "Successful export"
QgsDxfExport.ExportResult.InvalidDeviceError.__doc__ = "Invalid device error"
QgsDxfExport.ExportResult.DeviceNotWritableError.__doc__ = "Device not writable error"
QgsDxfExport.ExportResult.EmptyExtentError.__doc__ = "Empty extent, no extent given and no extent could be derived from layers"
QgsDxfExport.ExportResult.__doc__ = "The result of an export as dxf operation\n\n.. versionadded:: 3.10.1\n\n" + '* ``Success``: ' + QgsDxfExport.ExportResult.Success.__doc__ + '\n' + '* ``InvalidDeviceError``: ' + QgsDxfExport.ExportResult.InvalidDeviceError.__doc__ + '\n' + '* ``DeviceNotWritableError``: ' + QgsDxfExport.ExportResult.DeviceNotWritableError.__doc__ + '\n' + '* ``EmptyExtentError``: ' + QgsDxfExport.ExportResult.EmptyExtentError.__doc__
# --
# monkey patching scoped based enum
QgsDxfExport.VAlign.VBaseLine.__doc__ = "Top (0)"
QgsDxfExport.VAlign.VBottom.__doc__ = "Bottom (1)"
QgsDxfExport.VAlign.VMiddle.__doc__ = "Middle (2)"
QgsDxfExport.VAlign.VTop.__doc__ = "Top (3)"
QgsDxfExport.VAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.VAlign.__doc__ = "Vertical alignments.\n\n" + '* ``VBaseLine``: ' + QgsDxfExport.VAlign.VBaseLine.__doc__ + '\n' + '* ``VBottom``: ' + QgsDxfExport.VAlign.VBottom.__doc__ + '\n' + '* ``VMiddle``: ' + QgsDxfExport.VAlign.VMiddle.__doc__ + '\n' + '* ``VTop``: ' + QgsDxfExport.VAlign.VTop.__doc__ + '\n' + '* ``Undefined``: ' + QgsDxfExport.VAlign.Undefined.__doc__
# --
# monkey patching scoped based enum
QgsDxfExport.HAlign.HLeft.__doc__ = "Left (0)"
QgsDxfExport.HAlign.HCenter.__doc__ = "Centered (1)"
QgsDxfExport.HAlign.HRight.__doc__ = "Right (2)"
QgsDxfExport.HAlign.HAligned.__doc__ = "Aligned = (3) (if VAlign==0)"
QgsDxfExport.HAlign.HMiddle.__doc__ = "Middle = (4) (if VAlign==0)"
QgsDxfExport.HAlign.HFit.__doc__ = "Fit into point = (5) (if VAlign==0)"
QgsDxfExport.HAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.HAlign.__doc__ = "Horizontal alignments.\n\n" + '* ``HLeft``: ' + QgsDxfExport.HAlign.HLeft.__doc__ + '\n' + '* ``HCenter``: ' + QgsDxfExport.HAlign.HCenter.__doc__ + '\n' + '* ``HRight``: ' + QgsDxfExport.HAlign.HRight.__doc__ + '\n' + '* ``HAligned``: ' + QgsDxfExport.HAlign.HAligned.__doc__ + '\n' + '* ``HMiddle``: ' + QgsDxfExport.HAlign.HMiddle.__doc__ + '\n' + '* ``HFit``: ' + QgsDxfExport.HAlign.HFit.__doc__ + '\n' + '* ``Undefined``: ' + QgsDxfExport.HAlign.Undefined.__doc__
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
