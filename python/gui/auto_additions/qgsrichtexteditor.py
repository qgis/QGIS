# The following has been generated automatically from src/gui/qgsrichtexteditor.h
# monkey patching scoped based enum
QgsRichTextEditor.Mode.QTextDocument.__doc__ = "Default mode, exposes the Qt supported HTML/CSS subset"
QgsRichTextEditor.Mode.QgsTextRenderer.__doc__ = "QGIS text renderer mode, exposes the HTML/CSS subset supported by the QgsTextRenderer class"
QgsRichTextEditor.Mode.PlainText.__doc__ = "Plain text mode"
QgsRichTextEditor.Mode.__doc__ = """Widget modes.

.. versionadded:: 3.40

* ``QTextDocument``: Default mode, exposes the Qt supported HTML/CSS subset
* ``QgsTextRenderer``: QGIS text renderer mode, exposes the HTML/CSS subset supported by the QgsTextRenderer class
* ``PlainText``: Plain text mode

"""
# --
QgsRichTextEditor.Mode.baseClass = QgsRichTextEditor
try:
    QgsRichTextEditor.__attribute_docs__ = {'textChanged': 'Emitted when the text contents are changed.\n\n.. versionadded:: 3.26\n'}
except (NameError, AttributeError):
    pass
