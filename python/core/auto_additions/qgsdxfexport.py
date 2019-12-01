# The following has been generated automatically from src/core/dxf/qgsdxfexport.h
# monkey patching scoped based enum
QgsDxfExport.ExportResult.Success.__doc__ = "Successful export"
QgsDxfExport.ExportResult.InvalidDeviceError.__doc__ = "Invalid device error"
QgsDxfExport.ExportResult.DeviceNotWritableError.__doc__ = "Device not writable error"
QgsDxfExport.ExportResult.EmptyExtentError.__doc__ = "Empty extent, no extent given and no extent could be derived from layers"
QgsDxfExport.ExportResult.__doc__ = 'The result of an export as dxf operation\n\n.. versionadded:: 3.10.1\n\n' + '* ``Success``: ' + QgsDxfExport.ExportResult.Success.__doc__ + '\n' + '* ``InvalidDeviceError``: ' + QgsDxfExport.ExportResult.InvalidDeviceError.__doc__ + '\n' + '* ``DeviceNotWritableError``: ' + QgsDxfExport.ExportResult.DeviceNotWritableError.__doc__ + '\n' + '* ``EmptyExtentError``: ' + QgsDxfExport.ExportResult.EmptyExtentError.__doc__
# --
# monkey patching scoped based enum
QgsDxfExport.VAlign.VBaseLine.__doc__ = "Top (0)"
QgsDxfExport.VAlign.VBottom.__doc__ = "Bottom (1)"
QgsDxfExport.VAlign.VMiddle.__doc__ = "Middle (2)"
QgsDxfExport.VAlign.VTop.__doc__ = "Top (3)"
QgsDxfExport.VAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.VAlign.__doc__ = 'Vertical alignments.\n\n' + '* ``VBaseLine``: ' + QgsDxfExport.VAlign.VBaseLine.__doc__ + '\n' + '* ``VBottom``: ' + QgsDxfExport.VAlign.VBottom.__doc__ + '\n' + '* ``VMiddle``: ' + QgsDxfExport.VAlign.VMiddle.__doc__ + '\n' + '* ``VTop``: ' + QgsDxfExport.VAlign.VTop.__doc__ + '\n' + '* ``Undefined``: ' + QgsDxfExport.VAlign.Undefined.__doc__
# --
# monkey patching scoped based enum
QgsDxfExport.HAlign.HLeft.__doc__ = "Left (0)"
QgsDxfExport.HAlign.HCenter.__doc__ = "Centered (1)"
QgsDxfExport.HAlign.HRight.__doc__ = "Right (2)"
QgsDxfExport.HAlign.HAligned.__doc__ = "Aligned = (3) (if VAlign==0)"
QgsDxfExport.HAlign.HMiddle.__doc__ = "Middle = (4) (if VAlign==0)"
QgsDxfExport.HAlign.HFit.__doc__ = "Fit into point = (5) (if VAlign==0)"
QgsDxfExport.HAlign.Undefined.__doc__ = "Undefined"
QgsDxfExport.HAlign.__doc__ = 'Horizontal alignments.\n\n' + '* ``HLeft``: ' + QgsDxfExport.HAlign.HLeft.__doc__ + '\n' + '* ``HCenter``: ' + QgsDxfExport.HAlign.HCenter.__doc__ + '\n' + '* ``HRight``: ' + QgsDxfExport.HAlign.HRight.__doc__ + '\n' + '* ``HAligned``: ' + QgsDxfExport.HAlign.HAligned.__doc__ + '\n' + '* ``HMiddle``: ' + QgsDxfExport.HAlign.HMiddle.__doc__ + '\n' + '* ``HFit``: ' + QgsDxfExport.HAlign.HFit.__doc__ + '\n' + '* ``Undefined``: ' + QgsDxfExport.HAlign.Undefined.__doc__
# --
# monkey patching scoped based enum
QgsDxfExport.DxfPolylineFlag.Closed.__doc__ = "This is a closed polyline (or a polygon mesh closed in the M direction)"
QgsDxfExport.DxfPolylineFlag.Curve.__doc__ = "Curve-fit vertices have been added"
QgsDxfExport.DxfPolylineFlag.Spline.__doc__ = ""
QgsDxfExport.DxfPolylineFlag.Is3DPolyline.__doc__ = "This is a 3D polyline"
QgsDxfExport.DxfPolylineFlag.Is3DPolygonMesh.__doc__ = "This is a 3D polygon mesh"
QgsDxfExport.DxfPolylineFlag.PolygonMesh.__doc__ = "The polygon mesh is closed in the N direction"
QgsDxfExport.DxfPolylineFlag.PolyfaceMesh.__doc__ = "The polyline is a polyface mesh"
QgsDxfExport.DxfPolylineFlag.ContinuousPattern.__doc__ = "The linetype pattern is generated continuously around the vertices of this polyline"
QgsDxfExport.DxfPolylineFlag.__doc__ = 'Flags for polylines\n\n.. versionadded:: 3.12\n\n' + '* ``Closed``: ' + QgsDxfExport.DxfPolylineFlag.Closed.__doc__ + '\n' + '* ``Curve``: ' + QgsDxfExport.DxfPolylineFlag.Curve.__doc__ + '\n' + '* ``Spline``: ' + QgsDxfExport.DxfPolylineFlag.Spline.__doc__ + '\n' + '* ``Is3DPolyline``: ' + QgsDxfExport.DxfPolylineFlag.Is3DPolyline.__doc__ + '\n' + '* ``Is3DPolygonMesh``: ' + QgsDxfExport.DxfPolylineFlag.Is3DPolygonMesh.__doc__ + '\n' + '* ``PolygonMesh``: ' + QgsDxfExport.DxfPolylineFlag.PolygonMesh.__doc__ + '\n' + '* ``PolyfaceMesh``: ' + QgsDxfExport.DxfPolylineFlag.PolyfaceMesh.__doc__ + '\n' + '* ``ContinuousPattern``: ' + QgsDxfExport.DxfPolylineFlag.ContinuousPattern.__doc__
# --
