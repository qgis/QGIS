# The following has been generated automatically from src/core/layertree/qgslayertreegroup.h
try:
    QgsLayerTreeGroup.readXml = staticmethod(QgsLayerTreeGroup.readXml)
    QgsLayerTreeGroup.__overridden_methods__ = ['name', 'setName', 'writeXml', 'dump', 'clone', 'resolveReferences', 'setItemVisibilityCheckedRecursive']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayerTreeGroup_addChildNode = QgsLayerTreeGroup.addChildNode
    def __QgsLayerTreeGroup_addChildNode_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayerTreeGroup_addChildNode(self, arg)
    QgsLayerTreeGroup.addChildNode = _functools.update_wrapper(__QgsLayerTreeGroup_addChildNode_wrapper, QgsLayerTreeGroup.addChildNode)

    QgsLayerTreeGroup.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
