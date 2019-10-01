# The following has been generated automatically from src/core/qgis.h
# monkey patching scoped based enum
Qgis.Info = Qgis.MessageLevel.Info
Qgis.MessageLevel.Info.__doc__ = ""
Qgis.Warning = Qgis.MessageLevel.Warning
Qgis.MessageLevel.Warning.__doc__ = ""
Qgis.Critical = Qgis.MessageLevel.Critical
Qgis.MessageLevel.Critical.__doc__ = ""
Qgis.Success = Qgis.MessageLevel.Success
Qgis.MessageLevel.Success.__doc__ = ""
Qgis.Undefined = Qgis.MessageLevel.Undefined
Qgis.MessageLevel.Undefined.__doc__ = ""
Qgis.MessageLevel.__doc__ = 'Level for messages\nThis will be used both for message log and message bar in application.\n\n' + '* ``Info``: ' + Qgis.MessageLevel.Info.__doc__ + '\n' + '* ``Warning``: ' + Qgis.MessageLevel.Warning.__doc__ + '\n' + '* ``Critical``: ' + Qgis.MessageLevel.Critical.__doc__ + '\n' + '* ``Success``: ' + Qgis.MessageLevel.Success.__doc__ + '\n' + '* ``Undefined``: ' + Qgis.MessageLevel.Undefined.__doc__
# --
Qgis.MessageLevel.baseClass = Qgis
# monkey patching scoped based enum
Qgis.UnknownDataType = Qgis.DataType.UnknownDataType
Qgis.DataType.UnknownDataType.__doc__ = "Unknown or unspecified type"
Qgis.Byte = Qgis.DataType.Byte
Qgis.DataType.Byte.__doc__ = "Eight bit unsigned integer (quint8)"
Qgis.UInt16 = Qgis.DataType.UInt16
Qgis.DataType.UInt16.__doc__ = "Sixteen bit unsigned integer (quint16)"
Qgis.Int16 = Qgis.DataType.Int16
Qgis.DataType.Int16.__doc__ = "Sixteen bit signed integer (qint16)"
Qgis.UInt32 = Qgis.DataType.UInt32
Qgis.DataType.UInt32.__doc__ = "Thirty two bit unsigned integer (quint32)"
Qgis.Int32 = Qgis.DataType.Int32
Qgis.DataType.Int32.__doc__ = "Thirty two bit signed integer (qint32)"
Qgis.Float32 = Qgis.DataType.Float32
Qgis.DataType.Float32.__doc__ = "Thirty two bit floating point (float)"
Qgis.Float64 = Qgis.DataType.Float64
Qgis.DataType.Float64.__doc__ = "Sixty four bit floating point (double)"
Qgis.CInt16 = Qgis.DataType.CInt16
Qgis.DataType.CInt16.__doc__ = "Complex Int16"
Qgis.CInt32 = Qgis.DataType.CInt32
Qgis.DataType.CInt32.__doc__ = "Complex Int32"
Qgis.CFloat32 = Qgis.DataType.CFloat32
Qgis.DataType.CFloat32.__doc__ = "Complex Float32"
Qgis.CFloat64 = Qgis.DataType.CFloat64
Qgis.DataType.CFloat64.__doc__ = "Complex Float64"
Qgis.ARGB32 = Qgis.DataType.ARGB32
Qgis.DataType.ARGB32.__doc__ = "Color, alpha, red, green, blue, 4 bytes the same as QImage::Format_ARGB32"
Qgis.ARGB32_Premultiplied = Qgis.DataType.ARGB32_Premultiplied
Qgis.DataType.ARGB32_Premultiplied.__doc__ = "Color, alpha, red, green, blue, 4 bytes  the same as QImage::Format_ARGB32_Premultiplied"
Qgis.DataType.__doc__ = 'Raster data types.\nThis is modified and extended copy of GDALDataType.\n\n' + '* ``UnknownDataType``: ' + Qgis.DataType.UnknownDataType.__doc__ + '\n' + '* ``Byte``: ' + Qgis.DataType.Byte.__doc__ + '\n' + '* ``UInt16``: ' + Qgis.DataType.UInt16.__doc__ + '\n' + '* ``Int16``: ' + Qgis.DataType.Int16.__doc__ + '\n' + '* ``UInt32``: ' + Qgis.DataType.UInt32.__doc__ + '\n' + '* ``Int32``: ' + Qgis.DataType.Int32.__doc__ + '\n' + '* ``Float32``: ' + Qgis.DataType.Float32.__doc__ + '\n' + '* ``Float64``: ' + Qgis.DataType.Float64.__doc__ + '\n' + '* ``CInt16``: ' + Qgis.DataType.CInt16.__doc__ + '\n' + '* ``CInt32``: ' + Qgis.DataType.CInt32.__doc__ + '\n' + '* ``CFloat32``: ' + Qgis.DataType.CFloat32.__doc__ + '\n' + '* ``CFloat64``: ' + Qgis.DataType.CFloat64.__doc__ + '\n' + '* ``ARGB32``: ' + Qgis.DataType.ARGB32.__doc__ + '\n' + '* ``ARGB32_Premultiplied``: ' + Qgis.DataType.ARGB32_Premultiplied.__doc__
# --
# monkey patching scoped based enum
Qgis.PythonMacroMode.Never.__doc__ = "Macros are never run"
Qgis.PythonMacroMode.Ask.__doc__ = "User is prompt before running"
Qgis.PythonMacroMode.SessionOnly.__doc__ = "Only during this session"
Qgis.PythonMacroMode.Always.__doc__ = "Macros are always run"
Qgis.PythonMacroMode.NotForThisSession.__doc__ = "Macros will not be run for this session"
Qgis.PythonMacroMode.__doc__ = 'Authorisation to run Python Macros\n\n.. versionadded:: 3.10\n\n' + '* ``Never``: ' + Qgis.PythonMacroMode.Never.__doc__ + '\n' + '* ``Ask``: ' + Qgis.PythonMacroMode.Ask.__doc__ + '\n' + '* ``SessionOnly``: ' + Qgis.PythonMacroMode.SessionOnly.__doc__ + '\n' + '* ``Always``: ' + Qgis.PythonMacroMode.Always.__doc__ + '\n' + '* ``NotForThisSession``: ' + Qgis.PythonMacroMode.NotForThisSession.__doc__
# --
Qgis.PythonMacroMode.baseClass = Qgis
