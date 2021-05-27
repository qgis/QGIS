# The following has been generated automatically from src/core/labeling/qgslabellinesettings.h
# monkey patching scoped based enum
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolLeftRight.__doc__ = "Place direction symbols on left/right of label"
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove.__doc__ = "Place direction symbols on above label"
QgsLabelLineSettings.DirectionSymbolPlacement.SymbolBelow.__doc__ = "Place direction symbols on below label"
QgsLabelLineSettings.DirectionSymbolPlacement.__doc__ = 'Placement options for direction symbols.\n\n' + '* ``SymbolLeftRight``: ' + QgsLabelLineSettings.DirectionSymbolPlacement.SymbolLeftRight.__doc__ + '\n' + '* ``SymbolAbove``: ' + QgsLabelLineSettings.DirectionSymbolPlacement.SymbolAbove.__doc__ + '\n' + '* ``SymbolBelow``: ' + QgsLabelLineSettings.DirectionSymbolPlacement.SymbolBelow.__doc__
# --
# monkey patching scoped based enum
QgsLabelLineSettings.AnchorType.HintOnly.__doc__ = "Line anchor is a hint for preferred placement only, but other placements close to the hint are permitted"
QgsLabelLineSettings.AnchorType.Strict.__doc__ = "Line anchor is a strict placement, and other placements are not permitted"
QgsLabelLineSettings.AnchorType.__doc__ = 'Line anchor types\n\n' + '* ``HintOnly``: ' + QgsLabelLineSettings.AnchorType.HintOnly.__doc__ + '\n' + '* ``Strict``: ' + QgsLabelLineSettings.AnchorType.Strict.__doc__
# --
# monkey patching scoped based enum
QgsLabelLineSettings.AnchorClipping.UseVisiblePartsOfLine.__doc__ = "Only visible parts of lines are considered when calculating the line anchor for labels"
QgsLabelLineSettings.AnchorClipping.UseEntireLine.__doc__ = "Entire original feature line geometry is used when calculating the line anchor for labels"
QgsLabelLineSettings.AnchorClipping.__doc__ = 'Clipping behavior for line anchor calculation.\n\n.. versionadded:: 3.20\n\n' + '* ``UseVisiblePartsOfLine``: ' + QgsLabelLineSettings.AnchorClipping.UseVisiblePartsOfLine.__doc__ + '\n' + '* ``UseEntireLine``: ' + QgsLabelLineSettings.AnchorClipping.UseEntireLine.__doc__
# --
