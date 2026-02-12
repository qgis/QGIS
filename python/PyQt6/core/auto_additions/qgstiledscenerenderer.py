# The following has been generated automatically from src/core/tiledscene/qgstiledscenerenderer.h
try:
    QgsTiledSceneRenderer.load = staticmethod(QgsTiledSceneRenderer.load)
    QgsTiledSceneRenderer.__virtual_methods__ = ['flags', 'startRender', 'stopRender', 'createLegendNodes', 'legendRuleKeys']
    QgsTiledSceneRenderer.__abstract_methods__ = ['type', 'clone', 'save', 'renderTriangle', 'renderLine']
    QgsTiledSceneRenderer.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
try:
    QgsTiledSceneRenderContext.__group__ = ['tiledscene']
except (NameError, AttributeError):
    pass
