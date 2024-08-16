# The following has been generated automatically from src/core/scalebar/qgsscalebarrenderer.h
# monkey patching scoped based enum
QgsScaleBarRenderer.Flag.FlagUsesLineSymbol.__doc__ = "Renderer utilizes the scalebar line symbol (see QgsScaleBarSettings.lineSymbol() )"
QgsScaleBarRenderer.Flag.FlagUsesFillSymbol.__doc__ = "Renderer utilizes the scalebar fill symbol (see QgsScaleBarSettings.fillSymbol() )"
QgsScaleBarRenderer.Flag.FlagUsesAlternateFillSymbol.__doc__ = "Renderer utilizes the alternate scalebar fill symbol (see QgsScaleBarSettings.alternateFillSymbol() )"
QgsScaleBarRenderer.Flag.FlagRespectsUnits.__doc__ = "Renderer respects the QgsScaleBarSettings.units() setting"
QgsScaleBarRenderer.Flag.FlagRespectsMapUnitsPerScaleBarUnit.__doc__ = "Renderer respects the QgsScaleBarSettings.mapUnitsPerScaleBarUnit() setting"
QgsScaleBarRenderer.Flag.FlagUsesUnitLabel.__doc__ = "Renderer uses the QgsScaleBarSettings.unitLabel() setting"
QgsScaleBarRenderer.Flag.FlagUsesSegments.__doc__ = "Renderer uses the scalebar segments"
QgsScaleBarRenderer.Flag.FlagUsesLabelBarSpace.__doc__ = "Renderer uses the QgsScaleBarSettings.labelBarSpace() setting"
QgsScaleBarRenderer.Flag.FlagUsesLabelVerticalPlacement.__doc__ = "Renderer uses the QgsScaleBarSettings.labelVerticalPlacement() setting"
QgsScaleBarRenderer.Flag.FlagUsesLabelHorizontalPlacement.__doc__ = "Renderer uses the QgsScaleBarSettings.labelHorizontalPlacement() setting"
QgsScaleBarRenderer.Flag.FlagUsesAlignment.__doc__ = "Renderer uses the QgsScaleBarSettings.alignment() setting"
QgsScaleBarRenderer.Flag.FlagUsesSubdivisions.__doc__ = "Renderer uses the scalebar subdivisions (see QgsScaleBarSettings.numberOfSubdivisions() )"
QgsScaleBarRenderer.Flag.FlagUsesDivisionSymbol.__doc__ = "Renderer utilizes the scalebar division symbol (see QgsScaleBarSettings.divisionLineSymbol() )"
QgsScaleBarRenderer.Flag.FlagUsesSubdivisionSymbol.__doc__ = "Renderer utilizes the scalebar subdivision symbol (see QgsScaleBarSettings.subdivisionLineSymbol() )"
QgsScaleBarRenderer.Flag.FlagUsesSubdivisionsHeight.__doc__ = "Renderer uses the scalebar subdivisions height (see QgsScaleBarSettings.subdivisionsHeight() )"
QgsScaleBarRenderer.Flag.__doc__ = "Flags which control scalebar renderer behavior.\n\n.. versionadded:: 3.14\n\n" + '* ``FlagUsesLineSymbol``: ' + QgsScaleBarRenderer.Flag.FlagUsesLineSymbol.__doc__ + '\n' + '* ``FlagUsesFillSymbol``: ' + QgsScaleBarRenderer.Flag.FlagUsesFillSymbol.__doc__ + '\n' + '* ``FlagUsesAlternateFillSymbol``: ' + QgsScaleBarRenderer.Flag.FlagUsesAlternateFillSymbol.__doc__ + '\n' + '* ``FlagRespectsUnits``: ' + QgsScaleBarRenderer.Flag.FlagRespectsUnits.__doc__ + '\n' + '* ``FlagRespectsMapUnitsPerScaleBarUnit``: ' + QgsScaleBarRenderer.Flag.FlagRespectsMapUnitsPerScaleBarUnit.__doc__ + '\n' + '* ``FlagUsesUnitLabel``: ' + QgsScaleBarRenderer.Flag.FlagUsesUnitLabel.__doc__ + '\n' + '* ``FlagUsesSegments``: ' + QgsScaleBarRenderer.Flag.FlagUsesSegments.__doc__ + '\n' + '* ``FlagUsesLabelBarSpace``: ' + QgsScaleBarRenderer.Flag.FlagUsesLabelBarSpace.__doc__ + '\n' + '* ``FlagUsesLabelVerticalPlacement``: ' + QgsScaleBarRenderer.Flag.FlagUsesLabelVerticalPlacement.__doc__ + '\n' + '* ``FlagUsesLabelHorizontalPlacement``: ' + QgsScaleBarRenderer.Flag.FlagUsesLabelHorizontalPlacement.__doc__ + '\n' + '* ``FlagUsesAlignment``: ' + QgsScaleBarRenderer.Flag.FlagUsesAlignment.__doc__ + '\n' + '* ``FlagUsesSubdivisions``: ' + QgsScaleBarRenderer.Flag.FlagUsesSubdivisions.__doc__ + '\n' + '* ``FlagUsesDivisionSymbol``: ' + QgsScaleBarRenderer.Flag.FlagUsesDivisionSymbol.__doc__ + '\n' + '* ``FlagUsesSubdivisionSymbol``: ' + QgsScaleBarRenderer.Flag.FlagUsesSubdivisionSymbol.__doc__ + '\n' + '* ``FlagUsesSubdivisionsHeight``: ' + QgsScaleBarRenderer.Flag.FlagUsesSubdivisionsHeight.__doc__
# --
try:
    QgsScaleBarRenderer.__attribute_docs__ = {'segmentWidth': 'The width, in millimeters, of each individual segment drawn.\n\n.. note::\n\n   The number of map units per segment needs to be set via :py:class:`QgsScaleBarSettings`.setUnitsPerSegment.', 'size': 'Destination size for scalebar. This is used for scalebars which\nalter their appearance or alignment based on the desired scalebar\nsize (e.g. correctly aligning text in a numeric scale bar).', 'scale': 'Scale denominator', 'flags': 'Scalebar renderer flags'}
except NameError:
    pass
QgsScaleBarRenderer.ScaleBarContext.__doc__ = """Contains parameters regarding scalebar calculations.

.. note::

   The need to attribute the parameters vary depending on the targeted scalebar."""
try:
    QgsScaleBarRenderer.__group__ = ['scalebar']
except NameError:
    pass
try:
    QgsScaleBarRenderer.ScaleBarContext.__group__ = ['scalebar']
except NameError:
    pass
