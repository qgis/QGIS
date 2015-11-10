import os
from PyQt4 import uic, QtCore, QtGui
from processing.algs.qgis.postgis_utils import GeoDB
from qgis.core import *
from PyQt4.QtGui import QMessageBox

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgPostgisTableSelector.ui'))


class PostgisTableSelector(BASE, WIDGET):

    def __init__(self, parent, tablename):
        super(PostgisTableSelector, self).__init__(parent)
        self.connection = None
        self.table = None
        self.schema = None
        self.setupUi(self)
        settings = QtCore.QSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        names = settings.childGroups()
        settings.endGroup()
        for n in names:
            item = ConnectionItem(n)
            self.treeConnections.addTopLevelItem(item)

        def itemExpanded(item):
            try:
                item.populateSchemas()
            except:
                pass

        self.treeConnections.itemExpanded.connect(itemExpanded)

        self.textTableName.setText(tablename)

        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

    def cancelPressed(self):
        self.close()

    def okPressed(self):
        if self.textTableName.text().strip() == "":
            self.textTableName.setStyleSheet("QLineEdit{background: yellow}")
            return
        item = self.treeConnections.currentItem()
        if isinstance(item, ConnectionItem):
            QMessageBox.warning(self, "Wrong selection", "Select a schema item in the tree")
            return
        self.schema = item.text(0)
        self.table = self.textTableName.text().strip()
        self.connection = item.parent().text(0)
        self.close()


class ConnectionItem(QtGui.QTreeWidgetItem):

    connIcon = QtGui.QIcon(os.path.dirname(__file__) + '/../images/postgis.png')
    schemaIcon = QtGui.QIcon(os.path.dirname(__file__) + '/../images/namespace.png')

    def __init__(self, connection):
        QtGui.QTreeWidgetItem.__init__(self)
        self.setChildIndicatorPolicy(QtGui.QTreeWidgetItem.ShowIndicator)
        self.connection = connection
        self.setText(0, connection)
        self.setIcon(0, self.connIcon)

    def populateSchemas(self):
        if self.childCount() != 0:
            return
        settings = QtCore.QSettings()
        connSettings = '/PostgreSQL/connections/' + self.connection
        database = settings.value(connSettings + '/database')
        user = settings.value(connSettings + '/username')
        host = settings.value(connSettings + '/host')
        port = settings.value(connSettings + '/port')
        passwd = settings.value(connSettings + '/password')
        uri = QgsDataSourceURI()
        uri.setConnection(host, str(port), database, user, passwd)
        connInfo = uri.connectionInfo()
        (success, user, passwd) = QgsCredentials.instance().get(connInfo, None, None)
        if success:
            QgsCredentials.instance().put(connInfo, user, passwd)
            geodb = GeoDB(host, int(port), database, user, passwd)
            schemas = geodb.list_schemas()
            for oid, name, owner, perms in schemas:
                item = QtGui.QTreeWidgetItem()
                item.setText(0, name)
                item.setIcon(0, self.schemaIcon)
                self.addChild(item)
