# The following has been generated automatically from src/core/symbology/qgsstyle.h
# monkey patching scoped based enum
QgsStyle.SymbolTableColumn.Id.__doc__ = "Symbol ID"
QgsStyle.SymbolTableColumn.Name.__doc__ = "Symbol Name"
QgsStyle.SymbolTableColumn.XML.__doc__ = "Symbol definition (as XML)"
QgsStyle.SymbolTableColumn.FavoriteId.__doc__ = "Symbol is favorite flag"
QgsStyle.SymbolTableColumn.__doc__ = """Columns available in the Symbols table.

.. versionadded:: 3.40.

* ``Id``: Symbol ID
* ``Name``: Symbol Name
* ``XML``: Symbol definition (as XML)
* ``FavoriteId``: Symbol is favorite flag

"""
# --
QgsStyle.SymbolTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.TagTableColumn.Id.__doc__ = "Tag ID"
QgsStyle.TagTableColumn.Name.__doc__ = "Tag name"
QgsStyle.TagTableColumn.__doc__ = """Columns available in the Tags table.

.. versionadded:: 3.40.

* ``Id``: Tag ID
* ``Name``: Tag name

"""
# --
QgsStyle.TagTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.TagmapTableColumn.TagId.__doc__ = "Tag ID"
QgsStyle.TagmapTableColumn.SymbolId.__doc__ = "Symbol ID"
QgsStyle.TagmapTableColumn.__doc__ = """Columns available in the tag to symbol table.

.. versionadded:: 3.40.

* ``TagId``: Tag ID
* ``SymbolId``: Symbol ID

"""
# --
QgsStyle.TagmapTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.ColorRampTableColumn.Id.__doc__ = "Color ramp ID"
QgsStyle.ColorRampTableColumn.Name.__doc__ = "Color ramp name"
QgsStyle.ColorRampTableColumn.XML.__doc__ = "Color ramp definition (as XML)"
QgsStyle.ColorRampTableColumn.FavoriteId.__doc__ = "Color ramp is favorite flag"
QgsStyle.ColorRampTableColumn.__doc__ = """Columns available in the color ramp table.

.. versionadded:: 3.40.

* ``Id``: Color ramp ID
* ``Name``: Color ramp name
* ``XML``: Color ramp definition (as XML)
* ``FavoriteId``: Color ramp is favorite flag

"""
# --
QgsStyle.ColorRampTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.TextFormatTableColumn.Id.__doc__ = "Text format ID"
QgsStyle.TextFormatTableColumn.Name.__doc__ = "Text format name"
QgsStyle.TextFormatTableColumn.XML.__doc__ = "Text format definition (as XML)"
QgsStyle.TextFormatTableColumn.FavoriteId.__doc__ = "Text format is favorite flag"
QgsStyle.TextFormatTableColumn.__doc__ = """Columns available in the text format table.

.. versionadded:: 3.40.

* ``Id``: Text format ID
* ``Name``: Text format name
* ``XML``: Text format definition (as XML)
* ``FavoriteId``: Text format is favorite flag

"""
# --
QgsStyle.TextFormatTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.LabelSettingsTableColumn.Id.__doc__ = "Label settings ID"
QgsStyle.LabelSettingsTableColumn.Name.__doc__ = "Label settings name"
QgsStyle.LabelSettingsTableColumn.XML.__doc__ = "Label settings definition (as XML)"
QgsStyle.LabelSettingsTableColumn.FavoriteId.__doc__ = "Label settings is favorite flag"
QgsStyle.LabelSettingsTableColumn.__doc__ = """Columns available in the label settings table.

.. versionadded:: 3.40.

* ``Id``: Label settings ID
* ``Name``: Label settings name
* ``XML``: Label settings definition (as XML)
* ``FavoriteId``: Label settings is favorite flag

"""
# --
QgsStyle.LabelSettingsTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.SmartGroupTableColumn.Id.__doc__ = "Smart group ID"
QgsStyle.SmartGroupTableColumn.Name.__doc__ = "Smart group name"
QgsStyle.SmartGroupTableColumn.XML.__doc__ = "Smart group definition (as XML)"
QgsStyle.SmartGroupTableColumn.__doc__ = """Columns available in the smart group table.

.. versionadded:: 3.40.

* ``Id``: Smart group ID
* ``Name``: Smart group name
* ``XML``: Smart group definition (as XML)

"""
# --
QgsStyle.SmartGroupTableColumn.baseClass = QgsStyle
# monkey patching scoped based enum
QgsStyle.TextFormatContext.Labeling.__doc__ = "Text format used in labeling"
QgsStyle.TextFormatContext.__doc__ = """Text format context.

.. versionadded:: 3.20

* ``Labeling``: Text format used in labeling

"""
# --
try:
    QgsStyle.__attribute_docs__ = {'initialized': 'Emitted when the style database has been fully initialized.\n\nThis signals is only emitted by the :py:func:`QgsStyle.defaultStyle()` instance,\nand only when the :py:func:`~QgsStyle.defaultStyle` has been lazily initialized.\n\n.. versionadded:: 3.36\n', 'aboutToBeDestroyed': 'Emitted just before the style object is destroyed.\n\nEmitted in the destructor when the style is about to be deleted,\nbut it is still in a perfectly valid state: the last chance for\nother pieces of code for some cleanup if they use the style.\n\n.. versionadded:: 3.26\n', 'symbolSaved': 'Emitted every time a new symbol has been added to the database.\nEmitted whenever a symbol has been added to the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`symbolRemoved`\n\n.. seealso:: :py:func:`rampAdded`\n\n.. seealso:: :py:func:`symbolChanged`\n', 'symbolChanged': "Emitted whenever a symbol's definition is changed. This does not include\nname or tag changes.\n\n.. seealso:: :py:func:`symbolSaved`\n\n.. versionadded:: 3.4\n", 'groupsModified': 'Emitted every time a tag or smartgroup has been added, removed, or renamed\n', 'entityTagsChanged': "Emitted whenever an ``entity``'s tags are changed.\n\n.. versionadded:: 3.4\n", 'favoritedChanged': 'Emitted whenever an ``entity`` is either favorited or un-favorited.\n\n.. versionadded:: 3.4\n', 'entityAdded': 'Emitted every time a new entity has been added to the database.\n\n.. versionadded:: 3.14\n', 'entityRemoved': 'Emitted whenever an entity of the specified type is removed from the style and the database\nhas been updated as a result.\n\n.. versionadded:: 3.14\n', 'entityRenamed': 'Emitted whenever a entity of the specified type has been renamed from ``oldName`` to ``newName``\n\n.. versionadded:: 3.14\n', 'entityChanged': "Emitted whenever an entity's definition is changed. This does not include\nname or tag changes.\n\n.. versionadded:: 3.14\n", 'symbolRemoved': 'Emitted whenever a symbol has been removed from the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`symbolSaved`\n\n.. seealso:: :py:func:`rampRemoved`\n\n.. versionadded:: 3.4\n', 'symbolRenamed': 'Emitted whenever a symbol has been renamed from ``oldName`` to ``newName``\n\n.. seealso:: :py:func:`rampRenamed`\n\n.. versionadded:: 3.4\n', 'rampRenamed': 'Emitted whenever a color ramp has been renamed from ``oldName`` to ``newName``\n\n.. seealso:: :py:func:`symbolRenamed`\n\n.. versionadded:: 3.4\n', 'rampAdded': 'Emitted whenever a color ramp has been added to the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`rampRemoved`\n\n.. seealso:: :py:func:`symbolSaved`\n\n.. versionadded:: 3.4\n', 'rampRemoved': 'Emitted whenever a color ramp has been removed from the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`rampAdded`\n\n.. seealso:: :py:func:`symbolRemoved`\n\n.. versionadded:: 3.4\n', 'rampChanged': "Emitted whenever a color ramp's definition is changed. This does not include\nname or tag changes.\n\n.. seealso:: :py:func:`rampAdded`\n\n.. versionadded:: 3.4\n", 'textFormatRenamed': 'Emitted whenever a text format has been renamed from ``oldName`` to ``newName``\n\n.. seealso:: :py:func:`symbolRenamed`\n\n.. versionadded:: 3.10\n', 'textFormatAdded': 'Emitted whenever a text format has been added to the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`textFormatRemoved`\n\n.. seealso:: :py:func:`symbolSaved`\n\n.. versionadded:: 3.10\n', 'textFormatRemoved': 'Emitted whenever a text format has been removed from the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`textFormatAdded`\n\n.. seealso:: :py:func:`symbolRemoved`\n\n.. versionadded:: 3.10\n', 'textFormatChanged': "Emitted whenever a text format's definition is changed. This does not include\nname or tag changes.\n\n.. seealso:: :py:func:`textFormatAdded`\n\n.. versionadded:: 3.10\n", 'labelSettingsRenamed': 'Emitted whenever label settings have been renamed from ``oldName`` to ``newName``\n\n.. seealso:: :py:func:`symbolRenamed`\n\n.. versionadded:: 3.10\n', 'labelSettingsAdded': 'Emitted whenever label settings have been added to the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`labelSettingsRemoved`\n\n.. seealso:: :py:func:`symbolSaved`\n\n.. versionadded:: 3.10\n', 'labelSettingsRemoved': 'Emitted whenever label settings have been removed from the style and the database\nhas been updated as a result.\n\n.. seealso:: :py:func:`labelSettingsAdded`\n\n.. seealso:: :py:func:`symbolRemoved`\n\n.. versionadded:: 3.10\n', 'labelSettingsChanged': "Emitted whenever a label setting's definition is changed. This does not include\nname or tag changes.\n\n.. seealso:: :py:func:`labelSettingsAdded`\n\n.. versionadded:: 3.10\n", 'rebuildIconPreviews': 'Emitted whenever icon previews for entities in the style must be rebuilt.\n\n.. versionadded:: 3.26\n'}
    QgsStyle.defaultStyle = staticmethod(QgsStyle.defaultStyle)
    QgsStyle.defaultTextFormatForProject = staticmethod(QgsStyle.defaultTextFormatForProject)
    QgsStyle.isXmlStyleFile = staticmethod(QgsStyle.isXmlStyleFile)
    QgsStyle.__signal_arguments__ = {'symbolSaved': ['name: str', 'symbol: QgsSymbol'], 'symbolChanged': ['name: str'], 'entityTagsChanged': ['entity: QgsStyle.StyleEntity', 'name: str', 'newTags: List[str]'], 'favoritedChanged': ['entity: QgsStyle.StyleEntity', 'name: str', 'isFavorite: bool'], 'entityAdded': ['entity: QgsStyle.StyleEntity', 'name: str'], 'entityRemoved': ['entity: QgsStyle.StyleEntity', 'name: str'], 'entityRenamed': ['entity: QgsStyle.StyleEntity', 'oldName: str', 'newName: str'], 'entityChanged': ['entity: QgsStyle.StyleEntity', 'name: str'], 'symbolRemoved': ['name: str'], 'symbolRenamed': ['oldName: str', 'newName: str'], 'rampRenamed': ['oldName: str', 'newName: str'], 'rampAdded': ['name: str'], 'rampRemoved': ['name: str'], 'rampChanged': ['name: str'], 'textFormatRenamed': ['oldName: str', 'newName: str'], 'textFormatAdded': ['name: str'], 'textFormatRemoved': ['name: str'], 'textFormatChanged': ['name: str'], 'labelSettingsRenamed': ['oldName: str', 'newName: str'], 'labelSettingsAdded': ['name: str'], 'labelSettingsRemoved': ['name: str'], 'labelSettingsChanged': ['name: str']}
    QgsStyle.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleEntityInterface.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleSymbolEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleColorRampEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleTextFormatEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleLabelSettingsEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleLegendPatchShapeEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsStyleSymbol3DEntity.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
