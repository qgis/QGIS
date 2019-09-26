from qgis.PyQt.QtWidgets import QLabel, QWidget

def formOpen(dialog, layer, feature):

    label = dialog.findChild(QLabel, 'label')
    label.setText('Flying Monkey')
