# The following has been generated automatically from src/core/labeling/qgslabeling.h
# monkey patching scoped based enum
QgsLabeling.LinePlacementFlag.OnLine.__doc__ = "Labels can be placed directly over a line feature."
QgsLabeling.LinePlacementFlag.AboveLine.__doc__ = "Labels can be placed above a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed below the line feature."
QgsLabeling.LinePlacementFlag.BelowLine.__doc__ = "Labels can be placed below a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed above the line feature."
QgsLabeling.LinePlacementFlag.MapOrientation.__doc__ = "Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather than the feature's orientation. For example, AboveLine will always result in label's being placed above a line, regardless of the line's direction."
QgsLabeling.LinePlacementFlag.__doc__ = 'Line placement flags, which control how candidates are generated for a linear feature.\n\n' + '* ``OnLine``: ' + QgsLabeling.LinePlacementFlag.OnLine.__doc__ + '\n' + '* ``AboveLine``: ' + QgsLabeling.LinePlacementFlag.AboveLine.__doc__ + '\n' + '* ``BelowLine``: ' + QgsLabeling.LinePlacementFlag.BelowLine.__doc__ + '\n' + '* ``MapOrientation``: ' + QgsLabeling.LinePlacementFlag.MapOrientation.__doc__
# --
