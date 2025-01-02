# The following has been generated automatically from src/gui/editorwidgets/core/qgswidgetwrapper.h
# monkey patching scoped based enum
QgsWidgetWrapper.RootPath = QgsWidgetWrapper.Property.RootPath
QgsWidgetWrapper.RootPath.is_monkey_patched = True
QgsWidgetWrapper.RootPath.__doc__ = "Root path for external resource"
QgsWidgetWrapper.DocumentViewerContent = QgsWidgetWrapper.Property.DocumentViewerContent
QgsWidgetWrapper.DocumentViewerContent.is_monkey_patched = True
QgsWidgetWrapper.DocumentViewerContent.__doc__ = "Document type for external resource"
QgsWidgetWrapper.StorageUrl = QgsWidgetWrapper.Property.StorageUrl
QgsWidgetWrapper.StorageUrl.is_monkey_patched = True
QgsWidgetWrapper.StorageUrl.__doc__ = "Storage URL for external resource"
QgsWidgetWrapper.Property.__doc__ = """Data defined properties for different editor widgets.

* ``RootPath``: Root path for external resource
* ``DocumentViewerContent``: Document type for external resource
* ``StorageUrl``: Storage URL for external resource

"""
# --
try:
    QgsWidgetWrapper.__attribute_docs__ = {'contextChanged': 'Signal when :py:class:`QgsAttributeEditorContext` mContext changed\n\n.. versionadded:: 3.4\n'}
    QgsWidgetWrapper.fromWidget = staticmethod(QgsWidgetWrapper.fromWidget)
    QgsWidgetWrapper.__group__ = ['editorwidgets', 'core']
except (NameError, AttributeError):
    pass
