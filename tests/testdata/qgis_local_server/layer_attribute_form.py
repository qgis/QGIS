from qgis.PyQt.QtWidgets import QLabel


def formOpen(dialog, layer, feature):
    label = dialog.findChild(QLabel, 'label')
    assert label.text() == 'Swimming Monkey'
    label.setText('Flying Monkey')
