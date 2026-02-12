# The following has been generated automatically from src/core/labeling/qgslabellinesettings.h
# monkey patching scoped based enum
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolLeftRight.__doc__ = "Place direction symbols on left/right of label"
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove.__doc__ = "Place direction symbols on above label"
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolBelow.__doc__ = "Place direction symbols on below label"
QgsLabelLineSettings.DirectionSymbolPlacement.__doc__ = """Placement options for direction symbols.

* ``SymbolLeftRight``: Place direction symbols on left/right of label
* ``SymbolAbove``: Place direction symbols on above label
* ``SymbolBelow``: Place direction symbols on below label

"""
# --
QgsLabelLineSettings.DirectionSymbolPlacement.baseClass = QgsLabelLineSettings
# monkey patching scoped based enum
QgsLabelLineSettings.AnchorType.HintOnly.__doc__ = "Line anchor is a hint for preferred placement only, but other placements close to the hint are permitted"
QgsLabelLineSettings.AnchorType.Strict.__doc__ = "Line anchor is a strict placement, and other placements are not permitted"
QgsLabelLineSettings.AnchorType.__doc__ = """Line anchor types

* ``HintOnly``: Line anchor is a hint for preferred placement only, but other placements close to the hint are permitted
* ``Strict``: Line anchor is a strict placement, and other placements are not permitted

"""
# --
QgsLabelLineSettings.AnchorType.baseClass = QgsLabelLineSettings
# monkey patching scoped based enum
QgsLabelLineSettings.AnchorClipping.UseVisiblePartsOfLine.__doc__ = "Only visible parts of lines are considered when calculating the line anchor for labels"
QgsLabelLineSettings.AnchorClipping.UseEntireLine.__doc__ = "Entire original feature line geometry is used when calculating the line anchor for labels"
QgsLabelLineSettings.AnchorClipping.__doc__ = """Clipping behavior for line anchor calculation.

.. versionadded:: 3.20

* ``UseVisiblePartsOfLine``: Only visible parts of lines are considered when calculating the line anchor for labels
* ``UseEntireLine``: Entire original feature line geometry is used when calculating the line anchor for labels

"""
# --
QgsLabelLineSettings.AnchorClipping.baseClass = QgsLabelLineSettings
# monkey patching scoped based enum
QgsLabelLineSettings.AnchorTextPoint.StartOfText.__doc__ = "Anchor using start of text"
QgsLabelLineSettings.AnchorTextPoint.CenterOfText.__doc__ = "Anchor using center of text"
QgsLabelLineSettings.AnchorTextPoint.EndOfText.__doc__ = "Anchor using end of text"
QgsLabelLineSettings.AnchorTextPoint.FollowPlacement.__doc__ = "Automatically set the anchor point based on the lineAnchorPercent() value. Values <25% will use the start of text, values > 75% will use the end of text, and values in between will use the center of the text"
QgsLabelLineSettings.AnchorTextPoint.__doc__ = """Anchor point of label text.

.. versionadded:: 3.26

* ``StartOfText``: Anchor using start of text
* ``CenterOfText``: Anchor using center of text
* ``EndOfText``: Anchor using end of text
* ``FollowPlacement``: Automatically set the anchor point based on the lineAnchorPercent() value. Values <25% will use the start of text, values > 75% will use the end of text, and values in between will use the center of the text

"""
# --
QgsLabelLineSettings.AnchorTextPoint.baseClass = QgsLabelLineSettings
try:
    QgsLabelLineSettings.__group__ = ['labeling']
except (NameError, AttributeError):
    pass
