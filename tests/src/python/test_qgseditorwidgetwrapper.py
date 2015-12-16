from PyQt4.QtGui import QWidget

from qgis.gui import (
    QgsEditorWidgetWrapper,
    QgsEditorConfigWidget,
    QgsEditorWidgetFactory,
    QgsEditorWidgetRegistry
)

from utilities import (TestCase,
                       unittest,
                       getQgisTestApp
                       )

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class MyWidget(QWidget):

    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

    def value(self):
        return self.value

    def setValue(self, value):
        if value:
            self.value = value


class MyWidgetWrapper(QgsEditorWidgetWrapper):

    def createWidget(self, parent):
        return MyWidget(parent)

    def value(self):
        return self.widget().value()

    def setValue(self, value):
        self.widget().setValue(value)


class MyWidgetWrapperConfig(QgsEditorConfigWidget):

    def __init__(self, layer, fieldIndex, parent):
        QgsEditorConfigWidget.__init__(self, layer, fieldIndex, parent)

    def config(self):
        return {}

    def setConfig(self, config):
        pass


class MyWidgetWrapperFactory(QgsEditorWidgetFactory):

    def __init__(self):
        QgsEditorWidgetFactory.__init__(self, 'My test widget')

    def create(self, layer, fieldIndex, editor, parent):
        return MyWidgetWrapper(layer, fieldIndex, editor, parent)

    def configWidget(self, layer, fieldIndex, parent):
        return MyWidgetWrapperConfig(layer, fieldIndex, parent)


class WidgetTest(TestCase):

    @classmethod
    def setUpClass(cls):
        cls.widget_registry = QgsEditorWidgetRegistry.instance()
        cls.widget_registry.registerWidget('TestWidget', MyWidgetWrapperFactory())

    def testCreateWidget(self):
        ww = self.widget_registry.create('TestWidget', None, 0, {}, None, None)
        ww.config()

if __name__ == '__main__':
    unittest.main()
