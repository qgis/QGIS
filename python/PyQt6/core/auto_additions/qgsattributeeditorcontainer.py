# The following has been generated automatically from src/core/editform/qgsattributeeditorcontainer.h
try:
    QgsAttributeEditorContainer.__virtual_methods__ = ['addChildElement', 'setIsGroupBox', 'isGroupBox', 'findElements']
    QgsAttributeEditorContainer.__overridden_methods__ = ['clone']
    import functools as _functools
    __wrapped_QgsAttributeEditorContainer_addChildElement = QgsAttributeEditorContainer.addChildElement
    def __QgsAttributeEditorContainer_addChildElement_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAttributeEditorContainer_addChildElement(self, arg)
    QgsAttributeEditorContainer.addChildElement = _functools.update_wrapper(__QgsAttributeEditorContainer_addChildElement_wrapper, QgsAttributeEditorContainer.addChildElement)

    QgsAttributeEditorContainer.__group__ = ['editform']
except (NameError, AttributeError):
    pass
