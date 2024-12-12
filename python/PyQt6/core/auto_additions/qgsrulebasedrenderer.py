# The following has been generated automatically from src/core/symbology/qgsrulebasedrenderer.h
QgsRuleBasedRenderer.FeatIsSelected = QgsRuleBasedRenderer.FeatureFlags.FeatIsSelected
QgsRuleBasedRenderer.FeatDrawMarkers = QgsRuleBasedRenderer.FeatureFlags.FeatDrawMarkers
QgsRuleBasedRenderer.Rule.Filtered = QgsRuleBasedRenderer.Rule.RenderResult.Filtered
QgsRuleBasedRenderer.Rule.Inactive = QgsRuleBasedRenderer.Rule.RenderResult.Inactive
QgsRuleBasedRenderer.Rule.Rendered = QgsRuleBasedRenderer.Rule.RenderResult.Rendered
try:
    QgsRuleBasedRenderer.RenderJob.__attribute_docs__ = {'ftr': 'Feature to render', 'symbol': 'Symbol to render feature with (not owned by this object).'}
    QgsRuleBasedRenderer.RenderJob.__doc__ = """A QgsRuleBasedRenderer rendering job, consisting of a feature to be rendered with a particular symbol."""
    QgsRuleBasedRenderer.RenderJob.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.RenderLevel.__attribute_docs__ = {'jobs': 'List of jobs to render, owned by this object.'}
    QgsRuleBasedRenderer.RenderLevel.__doc__ = """Render level: a list of jobs to be drawn at particular level for a QgsRuleBasedRenderer."""
    QgsRuleBasedRenderer.RenderLevel.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.create = staticmethod(QgsRuleBasedRenderer.create)
    QgsRuleBasedRenderer.createFromSld = staticmethod(QgsRuleBasedRenderer.createFromSld)
    QgsRuleBasedRenderer.refineRuleCategories = staticmethod(QgsRuleBasedRenderer.refineRuleCategories)
    QgsRuleBasedRenderer.refineRuleRanges = staticmethod(QgsRuleBasedRenderer.refineRuleRanges)
    QgsRuleBasedRenderer.refineRuleScales = staticmethod(QgsRuleBasedRenderer.refineRuleScales)
    QgsRuleBasedRenderer.convertFromRenderer = staticmethod(QgsRuleBasedRenderer.convertFromRenderer)
    QgsRuleBasedRenderer.convertToDataDefinedSymbology = staticmethod(QgsRuleBasedRenderer.convertToDataDefinedSymbology)
    QgsRuleBasedRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.Rule.createFromSld = staticmethod(QgsRuleBasedRenderer.Rule.createFromSld)
    QgsRuleBasedRenderer.Rule.create = staticmethod(QgsRuleBasedRenderer.Rule.create)
    QgsRuleBasedRenderer.Rule.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsRuleBasedRenderer.FeatureToRender.__doc__ = """Feature for rendering by a QgsRuleBasedRenderer. Contains a QgsFeature and some flags."""
    QgsRuleBasedRenderer.FeatureToRender.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
