import os
from qgis.PyQt.QtWidgets import QAction, QMenu
from PyQt4.QtGui import QIcon
from processing.core.alglist import algList
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from qgis.utils import iface
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.AlgorithmExecutor import runalg
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.core.Processing import Processing

algorithmsToolbar = None
menusSettingsGroup = 'Menus'

defaultMenuEntries = {}
vectorMenu = Processing.tr('Vect&or')
analysisToolsMenu = vectorMenu + "/" + Processing.tr('&Analysis Tools')
defaultMenuEntries.update({'qgis:distancematrix': analysisToolsMenu,
                           'qgis:sumlinelengths': analysisToolsMenu,
                           'qgis:pointsinpolygon': analysisToolsMenu,
                           'qgis:listuniquevalues': analysisToolsMenu,
                           'qgis:basicstatisticsfornumericfields': analysisToolsMenu,
                           'qgis:basicstatisticsfortextfields': analysisToolsMenu,
                           'qgis:nearestneighbouranalysis': analysisToolsMenu,
                           'qgis:meancoordinates': analysisToolsMenu,
                           'qgis:lineintersecions': analysisToolsMenu})
researchToolsMenu = vectorMenu + "/" + Processing.tr('&Research Tools')
defaultMenuEntries.update({'qgis:randomselection': researchToolsMenu,
                           'qgis:randomselectionwithinsubsets': researchToolsMenu,
                           'qgis:randompointsinextent': researchToolsMenu,
                           'qgis:randompointsinlayerbounds': researchToolsMenu,
                           'qgis:randompointsinsidepolygonsfixed': researchToolsMenu,
                           'qgis:randompointsinsidepolygonsvariable': researchToolsMenu,
                           'qgis:regularpoints': researchToolsMenu,
                           'qgis:vectorgrid': researchToolsMenu,
                           'qgis:selectbylocation': researchToolsMenu,
                           'qgis:polygonfromlayerextent': researchToolsMenu})

geoprocessingToolsMenu = vectorMenu + "/" + Processing.tr('&Geoprocessing Tools')
defaultMenuEntries.update({'qgis:convexhull': geoprocessingToolsMenu,
                           'qgis:fixeddistancebuffer': geoprocessingToolsMenu,
                           'qgis:variabledistancebuffer': geoprocessingToolsMenu,
                           'qgis:intersection': geoprocessingToolsMenu,
                           'qgis:union': geoprocessingToolsMenu,
                           'qgis:symmetricaldifference': geoprocessingToolsMenu,
                           'qgis:clip': geoprocessingToolsMenu,
                           'qgis:difference': geoprocessingToolsMenu,
                           'qgis:dissolve': geoprocessingToolsMenu,
                           'qgis:eliminatesliverpolygons': geoprocessingToolsMenu})
geometryToolsMenu = vectorMenu + "/" + Processing.tr('G&eometry Tools')
defaultMenuEntries.update({'qgis:checkvalidity': geometryToolsMenu,
                           'qgis:exportaddgeometrycolumns': geometryToolsMenu,
                           'qgis:polygoncentroids': geometryToolsMenu,
                           'qgis:delaunaytriangulation': geometryToolsMenu,
                           'qgis:voronoipolygons': geometryToolsMenu,
                           'qgis:simplifygeometries': geometryToolsMenu,
                           'qgis:densifygeometries': geometryToolsMenu,
                           'qgis:multiparttosingleparts': geometryToolsMenu,
                           'qgis:singlepartstomultipart': geometryToolsMenu,
                           'qgis:polygonstolines': geometryToolsMenu,
                           'qgis:linestopolygons': geometryToolsMenu,
                           'qgis:extractnodes': geometryToolsMenu})
managementToolsMenu = vectorMenu + "/" + Processing.tr('&Data Management Tools')
defaultMenuEntries.update({'qgis:definecurrentprojection': managementToolsMenu,
                           'qgis:joinattributesbylocation': managementToolsMenu,
                           'qgis:splitvectorlayer': managementToolsMenu,
                           'qgis:mergevectorlayers': managementToolsMenu,
                           'qgis:createspatialindex': managementToolsMenu})
"""
rasterMenu = Processing.tr('&Raster')
projectionsMenu = rasterMenu + "/" + Processing.tr('Projections')
defaultMenuEntries.update({'gdalogr:warpreproject':projectionsMenu,
                           'gdalogr:assignprojection':projectionsMenu,
                           'gdalogr:extractprojection':projectionsMenu})
conversionMenu = rasterMenu + "/" + Processing.tr('Conversion')
defaultMenuEntries.update({'gdalogr:rasterize':conversionMenu,
                           'gdalogr:rasterize_over':conversionMenu,
                           'gdalogr:polygonize':conversionMenu,
                           'gdalogr:translate':conversionMenu,
                           'gdalogr:rgbtopct':conversionMenu,
                           'gdalogr:pcttorgb':conversionMenu})
extractionMenu = rasterMenu + "/" + Processing.tr('Extraction')
defaultMenuEntries.update({'gdalogr:contour':extractionMenu,
                           'gdalogr:cliprasterbyextent':extractionMenu,
                           'gdalogr:cliprasterbymasklayer':extractionMenu})
analysisMenu = rasterMenu + "/" + Processing.tr('Analysis')
defaultMenuEntries.update({'gdalogr:sieve':analysisMenu, 'gdalogr:nearblack':analysisMenu,
                           'gdalogr:fillnodata':analysisMenu,
                           'gdalogr:proximity':analysisMenu,
                           'gdalogr:griddatametrics':analysisMenu,
                           'gdalogr:gridaverage':analysisMenu,
                           'gdalogr:gridinvdist':analysisMenu,
                           'gdalogr:gridnearestneighbor':analysisMenu,
                           'gdalogr:aspect':analysisMenu,
                           'gdalogr:hillshade':analysisMenu,
                           'gdalogr:roughness':analysisMenu,
                           'gdalogr:slope':analysisMenu,
                           'gdalogr:tpi':analysisMenu,
                           'gdalogr:tri':analysisMenu})
miscMenu = rasterMenu + "/" + Processing.tr('Miscellaneous')
defaultMenuEntries.update({'gdalogr:buildvirtualraster':miscMenu,
                           'gdalogr:merge':miscMenu,
                           'gdalogr:rasterinfo':miscMenu,
                           'gdalogr:overviews':miscMenu,
                           'gdalogr:tileindex':miscMenu})
"""


def initializeMenus():
    for provider in algList.providers:
        for alg in provider.algs:
            d = defaultMenuEntries.get(alg.commandLineName(), "")
            setting = Setting(menusSettingsGroup, "MENU_" + alg.commandLineName(),
                              "Menu path", d)
            ProcessingConfig.addSetting(setting)
            setting = Setting(menusSettingsGroup, "BUTTON_" + alg.commandLineName(),
                              "Add button", False)
            ProcessingConfig.addSetting(setting)
            setting = Setting(menusSettingsGroup, "ICON_" + alg.commandLineName(),
                              "Icon", "", valuetype=Setting.FILE)
            ProcessingConfig.addSetting(setting)

    ProcessingConfig.readSettings()


def updateMenus():
    removeMenus()
    createMenus()


def createMenus():
    for provider in list(algList.algs.values()):
        for alg in list(provider.values()):
            menuPath = ProcessingConfig.getSetting("MENU_" + alg.commandLineName())
            addButton = ProcessingConfig.getSetting("BUTTON_" + alg.commandLineName())
            icon = ProcessingConfig.getSetting("ICON_" + alg.commandLineName())
            if icon and os.path.exists(icon):
                icon = QIcon(icon)
            else:
                icon = None
            if menuPath:
                paths = menuPath.split("/")
                addAlgorithmEntry(alg, paths[0], paths[-1], addButton=addButton, icon=icon)


def removeMenus():
    for provider in list(algList.algs.values()):
        for alg in list(provider.values()):
            menuPath = ProcessingConfig.getSetting("MENU_" + alg.commandLineName())
            if menuPath:
                paths = menuPath.split("/")
                removeAlgorithmEntry(alg, paths[0], paths[-1])


def addAlgorithmEntry(alg, menuName, submenuName, actionText=None, icon=None, addButton=False):
    action = QAction(icon or alg.getIcon(), actionText or alg.i18n_name or alg.name, iface.mainWindow())
    action.triggered.connect(lambda: _executeAlgorithm(alg))
    action.setObjectName("mProcessingUserMenu_%s" % alg.commandLineName())

    if menuName:
        menu = getMenu(menuName, iface.mainWindow().menuBar())
        submenu = getMenu(submenuName, menu)
        submenu.addAction(action)

    if addButton:
        global algorithmsToolbar
        if algorithmsToolbar is None:
            algorithmsToolbar = iface.addToolBar('ProcessingAlgorithms')
        algorithmsToolbar.addAction(action)


def removeAlgorithmEntry(alg, menuName, submenuName, actionText=None, delButton=True):
    if menuName:
        menu = getMenu(menuName, iface.mainWindow().menuBar())
        subMenu = getMenu(submenuName, menu)
        action = findAction(subMenu.actions(), alg, actionText)
        if action is not None:
            subMenu.removeAction(action)

        if len(subMenu.actions()) == 0:
            subMenu.deleteLater()

    if delButton:
        global algorithmsToolbar
        if algorithmsToolbar is not None:
            action = findAction(algorithmsToolbar.actions(), alg, actionText)
            if action is not None:
                algorithmsToolbar.removeAction(action)


def _executeAlgorithm(alg):
    message = alg.checkBeforeOpeningParametersDialog()
    if message:
        dlg = MessageDialog()
        dlg.setTitle(Processing.tr('Missing dependency'))
        dlg.setMessage(
            Processing.tr('<h3>Missing dependency. This algorithm cannot '
                          'be run :-( </h3>\n%s') % message)
        dlg.exec_()
        return
    alg = alg.getCopy()
    if (alg.getVisibleParametersCount() + alg.getVisibleOutputsCount()) > 0:
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
    else:
        progress = MessageBarProgress()
        runalg(alg, progress)
        handleAlgorithmResults(alg, progress)
        progress.close()


def getMenu(name, parent):
    menus = [c for c in parent.children() if isinstance(c, QMenu) and c.title() == name]
    if menus:
        return menus[0]
    else:
        return parent.addMenu(name)


def findAction(actions, alg, actionText=None):
    for action in actions:
        if action.text() in [actionText, alg.i18n_name, alg.name]:
            return action
    return None
