# The following has been generated automatically from src/core/qgsstringutils.h
try:
    QgsStringUtils.__attribute_docs__ = {'UNACCENT_MAP': "Lookup table used by :py:func:`~QgsStringUtils.unaccent`. Generated at build time from\nPostgreSQL's unaccent rules.\n\nThis hash map contains mappings from accented/special characters to their\nASCII equivalents. The table includes:\n\n- Diacritical marks (é→e, ñ→n, ü→u, etc.)\n- Ligatures (Æ→AE, œ→oe, etc.)\n- Special letters (ß→ss, ł→l, etc.)\n- Compatibility characters (℃→°C, ℗→(P), etc.)\n- Full-width characters (＃→#, etc.)"}
    QgsStringUtils.__annotations__ = {'UNACCENT_MAP': 'Dict[str, str]'}
    QgsStringUtils.capitalize = staticmethod(QgsStringUtils.capitalize)
    QgsStringUtils.ampersandEncode = staticmethod(QgsStringUtils.ampersandEncode)
    QgsStringUtils.levenshteinDistance = staticmethod(QgsStringUtils.levenshteinDistance)
    QgsStringUtils.longestCommonSubstring = staticmethod(QgsStringUtils.longestCommonSubstring)
    QgsStringUtils.hammingDistance = staticmethod(QgsStringUtils.hammingDistance)
    QgsStringUtils.soundex = staticmethod(QgsStringUtils.soundex)
    QgsStringUtils.fuzzyScore = staticmethod(QgsStringUtils.fuzzyScore)
    QgsStringUtils.insertLinks = staticmethod(QgsStringUtils.insertLinks)
    QgsStringUtils.isUrl = staticmethod(QgsStringUtils.isUrl)
    QgsStringUtils.wordWrap = staticmethod(QgsStringUtils.wordWrap)
    QgsStringUtils.substituteVerticalCharacters = staticmethod(QgsStringUtils.substituteVerticalCharacters)
    QgsStringUtils.htmlToMarkdown = staticmethod(QgsStringUtils.htmlToMarkdown)
    QgsStringUtils.qRegExpEscape = staticmethod(QgsStringUtils.qRegExpEscape)
    QgsStringUtils.truncateMiddleOfString = staticmethod(QgsStringUtils.truncateMiddleOfString)
    QgsStringUtils.containsByWord = staticmethod(QgsStringUtils.containsByWord)
    QgsStringUtils.createUnaccentMap = staticmethod(QgsStringUtils.createUnaccentMap)
    QgsStringUtils.unaccent = staticmethod(QgsStringUtils.unaccent)
except (NameError, AttributeError):
    pass
try:
    QgsStringReplacement.fromProperties = staticmethod(QgsStringReplacement.fromProperties)
except (NameError, AttributeError):
    pass
