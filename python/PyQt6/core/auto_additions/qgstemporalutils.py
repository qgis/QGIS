# The following has been generated automatically from src/core/qgstemporalutils.h
try:
    QgsTemporalUtils.AnimationExportSettings.__attribute_docs__ = {'animationRange': 'Dictates the overall temporal range of the animation.', 'frameDuration': 'Duration of individual export frames', 'outputDirectory': 'Destination directory for created image files.', 'fileNameTemplate': "The filename template for exporting the frames.\n\nThis must be in format ``prefix####.format``, where number of\n````#`` ``characters represents how many 0's should be left-padded to the frame number\ne.g. ``my###.jpg`` will create frames ``my001.jpg``, ``my002.jpg``, etc", 'decorations': 'List of decorations to draw onto exported frames.', 'availableTemporalRanges': 'Contains the list of all available temporal ranges which have data available.\n\nThe list can be a list of non-contiguous ranges (i.e. containing gaps)\nwhich together describe the complete range of times which contain data.\n\nThis list is required whenever the :py:class:`QgsUnitTypes`.TemporalIrregularStep interval is used\nfor an animation.\n\n.. versionadded:: 3.30', 'frameRate': 'Target animation frame rate in frames per second.\n\n.. versionadded:: 3.26'}
    QgsTemporalUtils.AnimationExportSettings.__doc__ = """Contains settings relating to exporting animations"""
except (NameError, AttributeError):
    pass
try:
    QgsTemporalUtils.calculateTemporalRangeForProject = staticmethod(QgsTemporalUtils.calculateTemporalRangeForProject)
    QgsTemporalUtils.usedTemporalRangesForProject = staticmethod(QgsTemporalUtils.usedTemporalRangesForProject)
    QgsTemporalUtils.exportAnimation = staticmethod(QgsTemporalUtils.exportAnimation)
    QgsTemporalUtils.calculateFrameTime = staticmethod(QgsTemporalUtils.calculateFrameTime)
    QgsTemporalUtils.calculateDateTimesUsingDuration = staticmethod(QgsTemporalUtils.calculateDateTimesUsingDuration)
    QgsTemporalUtils.calculateDateTimesFromISO8601 = staticmethod(QgsTemporalUtils.calculateDateTimesFromISO8601)
except (NameError, AttributeError):
    pass
