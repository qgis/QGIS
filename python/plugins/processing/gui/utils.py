from qgis.utils import iface
from PyQt4 import QtGui
from processing.core.Processing import Processing
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog

algorithmsToolbar = None


def addAlgorithmEntry(algname, menuName, submenuName, actionText=None, icon=None, addButton=False):
    alg = Processing.getAlgorithm(algname)
    if alg is None:
        return
    if menuName:
        menu = getMenu(menuName, iface.mainWindow().menuBar())
        submenu = getMenu(submenuName, menu)
        action = QtGui.QAction(icon or alg.getIcon(), actionText or alg.name, iface.mainWindow())
        action.triggered.connect(lambda: _executeAlgorithm(alg))
        submenu.addAction(action)
    if addButton:
        global algorithmsToolbar
        if algorithmsToolbar is None:
            algorithmsToolbar = iface.addToolBar("ProcessingAlgorithms")
        algorithmsToolbar.addAction(action)


def _executeAlgorithm(alg):
    message = alg.checkBeforeOpeningParametersDialog()
    if message:
        dlg = MessageDialog()
        dlg.setTitle(tr('Missing dependency'))
        dlg.setMessage(
            tr('<h3>Missing dependency. This algorithm cannot '
               'be run :-( </h3>\n%s') % message)
        dlg.exec_()
        return
    alg = alg.getCopy()
    dlg = alg.getCustomParametersDialog()
    if not dlg:
        dlg = AlgorithmDialog(alg)
    canvas = iface.mapCanvas()
    prevMapTool = canvas.mapTool()
    dlg.show()
    dlg.exec_()
    if canvas.mapTool() != prevMapTool:
        try:
            canvas.mapTool().reset()
        except:
            pass
        canvas.setMapTool(prevMapTool)


def getMenu(name, parent):
    menus = [c for c in parent.children() if isinstance(c, QtGui.QMenu)]
    menusDict = {m.title(): m for m in menus}
    if name in menusDict:
        return menusDict[name]
    else:
        menu = QtGui.QMenu(name, parent)
        parent.addMenu(menu)
        return menu
