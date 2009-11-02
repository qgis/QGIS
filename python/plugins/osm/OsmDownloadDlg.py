"""@package OsmDownloadDlg
Module provides simple way how to download OSM data.
First user is asked to choose download region, output file etc.

Then HTTP connection to OpenStreetMap server is created and download operation is started.

Note that OpenStreetMap server you are downloading OSM data from (~api.openstreetmap.org)
has fixed limitations of how much data you can get. As written on wiki.openstreetmap.org
neither latitude nor longitude extent of downloaded region can be larger than 0.25 degrees.

Each error response from OSM server is caught by OSM Plugin and display to its user.
"""


from ui_OsmDownloadDlg import Ui_OsmDownloadDlg
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtNetwork import *
from PyQt4 import *
from time import *
from qgis.core import *


class OsmDownloadDlg(QDialog, Ui_OsmDownloadDlg):
    """This is the main class of this module.
    It's direct descendant of "OSM Download" dialog.

    It provides simple way how to download OSM data.
    First user is asked to choose download region, output file etc.

    Then HTTP connection to OpenStreetMap server is created and download operation is started.

    Note that OpenStreetMap server you are downloading OSM data from (~api.openstreetmap.org)
    has fixed limitations of how much data you can get. As written on wiki.openstreetmap.org
    neither latitude nor longitude extent of downloaded region can be larger than 0.25 degrees.

    Each error response from OSM server is caught by OSM Plugin and display to its user.
    """

    def __init__(self, plugin):
        """The constructor.

        Performs initialization of OSM Download dialog and inner structures.
        Default download region is set according to current canvas extent.

        @param plugin is pointer to instance of OSM Plugin.
        """

        QDialog.__init__(self, None)
        self.setupUi(self)
        self.dbm=plugin.dbm

        self.urlHost="api.openstreetmap.org"
        self.urlPathPrefix="/api/0.6/map?bbox="

        self.downloadButton.setDefault(True)
        self.downloadButton.setEnabled(False)

        # determining default area for download
        if QgsMapLayerRegistry.instance().count()>0:
            currentExtent=plugin.canvas.extent()
        else:
            # if no layer is loaded default download extent is "part of the Prague city center" :-D
            currentExtent=QgsRectangle(14.4271398308,50.0768156358,14.4324358906,50.0812613868)

        # check whether the extent needs to be projected back to WGS84
        mapRenderer = plugin.canvas.mapRenderer()
        if mapRenderer.hasCrsTransformEnabled():
          crsMap=mapRenderer.destinationSrs()
          crsWgs84=QgsCoordinateReferenceSystem(4326)
          xform=QgsCoordinateTransform(crsMap, crsWgs84)
          currentExtent=xform.transformBoundingBox(currentExtent)

        self.latFromLineEdit.setText(QString("%1").arg(currentExtent.yMinimum(),0,'f',10))
        self.latToLineEdit.setText(QString("%1").arg(currentExtent.yMaximum(),0,'f',10))
        self.lonFromLineEdit.setText(QString("%1").arg(currentExtent.xMinimum(),0,'f',10))
        self.lonToLineEdit.setText(QString("%1").arg(currentExtent.xMaximum(),0,'f',10))

        # create object for http connection
        self.outFile=None
        self.httpGetId=0
        self.httpSuccess=False
        self.errMessage=None
        self.finished=False
        self.responseHeader=""

        # connect all important signals to slots
        self.connectDlgSignals()

        self.helpButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_questionMark.png"))

        # set special font for extentInfoLabel
        myFont = self.extentInfoLabel.font()
        myFont.setPointSize( myFont.pointSize()+1 )
        myFont.setBold(True)
        self.extentInfoLabel.setFont(myFont)

        # generating default name for output file
        defaultFileName=self.generateDefFileName()
        self.destdirLineEdit.setText(defaultFileName)
        self.destdirLineEdit.setEnabled(True)
        self.downloadButton.setEnabled(True)

        # check default extent
        self.checkExtent()

        # load default values to combobox determining style for custom renderer
        self.styles=["Small scale","Medium scale","Large scale"]
        self.styleCombo.addItems(self.styles)

        # just determine if "replace data" checkbox should be checked
        if not plugin.dbm.currentKey:
            self.chkReplaceData.setEnabled(False)


    def downloadFile(self):
        """Function starts thw whole download process.

        It's called after click() signal is emitted on Download button.
        """

        if self.finished:
            return

        self.downloadButton.setEnabled(False)
        self.disconnectDlgSignals()

        # finding out which area should be downloaded, and to where
        urlPath = self.urlPathPrefix + self.lonFromLineEdit.text() + "," + self.latFromLineEdit.text() + "," + self.lonToLineEdit.text() + "," + self.latToLineEdit.text()
        fileName = self.destdirLineEdit.text()

        # remove the old database file
        if QFile.exists(fileName+".db"):
            QFile.remove(fileName+".db")

        self.outFile=QFile(fileName)
        if not self.outFile.open(QIODevice.WriteOnly):
            QMessageBox.information(self, self.tr("OSM Download"),
                                          self.tr("Unable to save the file %1: %2.")
                                          .arg(fileName).arg(self.outFile.errorString()))
            self.outFile = None
            return

        # creating progress dialog for download
        self.progressDialog=QProgressDialog(self)
        # !!! don't set progress dialog modal !!! it would cause serious problems!
        self.progressDialog.setAutoClose(False)
        self.progressDialog.setWindowTitle(self.tr("OSM Download"))
        self.connect(self.progressDialog,SIGNAL("canceled()"), self.progressDlgCanceled)

        self.setEnabled(False)
        self.progressDialog.setEnabled(True)
        self.progressDialog.show()
        self.progressDialog.setLabelText(self.tr("Waiting for OpenStreetMap server ..."))
        self.progressDialog.setMaximum(1)
        self.progressDialog.setValue(0)

        # create object for http connection
        self.http=QHttp(self)

        # catching http signals!
        self.connect(self.http,SIGNAL("requestFinished(int, bool)"), self.httpRequestFinished)
        self.connect(self.http,SIGNAL("dataReadProgress(int, int)"), self.updateDataReadProgress)
        self.connect(self.http,SIGNAL("responseHeaderReceived(QHttpResponseHeader)"), self.readResponseHeader)
        self.connect(self.http,SIGNAL("stateChanged(int)"), self.stateChanged)
        self.connect(self.http,SIGNAL("done(bool)"), self.httpDone)

        self.setProxy()
        self.http.setHost(self.urlHost, 80)
        self.httpGetId=self.http.get(urlPath, self.outFile)


    def httpRequestFinished(self, requestId, error):
        """Function is called when requestFinished(...) signal is emitted
        on global HTTP connection object.

        @param requestId identifier of http request that was finished
        @param error True if error occured on given request; False otherwise
        """

        if self.finished:
            return


    def readResponseHeader(self, responseHeader):
        """Function is called when responseHeaderReceived(...) signal is emitted
        on global HTTP connection object.

        If statusCode of responseHeader doesn't equal to 200, function cancels the whole connection.

        @param responseHeader header of HTTP response from the OSM server
        """

        if self.finished:
            return

        if responseHeader.statusCode() != 200:
            self.cancelDownload(self.tr("Download process failed. OpenStreetMap server response: %1 - %2")
                    .arg(responseHeader.reasonPhrase())
                    .arg(responseHeader.value("Error")))


    def updateDataReadProgress(self, bytesRead, totalBytes):
        """Function is called after dataReadProgress(...) signal is emitted on global HTTP connection object.

        It updates progress dialog.

        @param bytesRead total number of bytes that has been already read through the HTTP connection
        @param totalBytes total number of bytes that will be received through HTTP connection
        """

        if self.finished:
            return

        # note that progress dialog mustn't be modal!
        self.progressDialog.setMaximum(totalBytes)
        self.progressDialog.setValue(bytesRead)


    def stateChanged(self,newState):
        """Function is called after stateChanged(...) signal is emitted on HTTP connection.

        OSM Downloader does actully nothing in here.
        Maybe in future function will be used.

        @param newState number representing new state of the connection
        """

        if self.finished:
            return


    def httpDone(self,error):
        """Function is called after done(...) signal is emitted on HTTP connection.
        (Done signal is emitted immediatelly after all requests of HTTP connection
        are finished ~ emits an requestFinished(...) signal).

        @param error True if error occured on any of HTTP connection requests; False otherwise
        """

        if self.finished:
            return
        self.finished=True
        self.outFile.flush()
        self.outFile.close()

        # we are no more interested in signals emitted on QHttp object
        self.disconnect(self.http,SIGNAL("done(bool)"), self.httpDone)
        self.disconnect(self.http,SIGNAL("requestFinished(int, bool)"), self.httpRequestFinished)
        self.disconnect(self.http,SIGNAL("dataReadProgress(int, int)"), self.updateDataReadProgress)
        self.disconnect(self.http,SIGNAL("responseHeaderReceived(QHttpResponseHeader)"), self.readResponseHeader)
        self.disconnect(self.http,SIGNAL("stateChanged(int)"), self.stateChanged)

        del self.http
        self.http=None

        # request was not aborted
        if error:
            self.httpSuccess=False
            # remove output file
            if self.outFile and self.outFile.exists():
                self.outFile.remove()
                del self.outFile
                self.outFile=None

            # and tell user
            if self.errMessage==None:
                self.errMessage="Check your internet connection"
            QMessageBox.information(self, self.tr("OSM Download Error")
                ,self.tr("Download failed: %1.").arg(self.errMessage))
        else:
            self.httpSuccess=True

        # well, download process has finished successfully;
        # close progress dialog and the whole download dialog
        self.progressDialog.close()
        self.close()


    def cancelDownload(self,errMessage=None):
        """Function aborts global HTTP connection.

        It gets an error message and just store it into member variable.
        It will be displayed to Quantum GIS user later after done(...) will be emitted.

        @param errMessage error message ~ the reason why connection is canceled
        """

        if self.finished:
            return

        self.errMessage=errMessage
        # stop http communication
        self.http.abort()


    ####################################################################################
    ############ NON-HTTP FUNCTIONS ####################################################
    ####################################################################################

    def showChooseDirectoryDialog(self):
        """Function just shows dialog for directory selection.

        Only OSM files can be selected.
        """

        if self.finished:
            return

        # display file open dialog and get absolute path to selected directory
        fileSelected = QFileDialog.getSaveFileName(self, "Choose file to save","download.osm", "OSM Files (*.osm)");
        # insert selected directory path into line edit control
        if not fileSelected.isNull():
            self.destdirLineEdit.setText(fileSelected)
            self.downloadButton.setEnabled(True)


    def generateDefFileName(self):
        """This function creates default name for output file.

        It's called mainly from downloader initialization.
        Default name is always unique. It consist of current timestamp and a postfix.
        """

        if self.finished:
            return

        prefix=QDir.tempPath() + "/"
        if self.dbm.currentKey:
            key=QString(self.dbm.currentKey)
            p=key.lastIndexOf("/")
            prefix=key.left(p+1)

        timestring=strftime("%y%m%d_%H%M%S",localtime(time()))
        return prefix.append(QString(timestring)).append("_downloaded.osm")


    def autoLoadClicked(self):
        """Function is called after clicking on AutoLoad checkbox.
        """

        if self.finished:
            return

        if not self.autoLoadCheckBox.isChecked():
            self.chkCustomRenderer.setEnabled(False)
            self.chkReplaceData.setEnabled(False)
        else:
            self.chkCustomRenderer.setEnabled(True)
            self.chkReplaceData.setEnabled(True)


    def showExtentHelp(self):
        """Function is called after clicking on Help button.
        It shows basic information on downloading. 
        """

        if self.finished:
            return

        mb=QMessageBox()
        mb.setMinimumWidth(390)
        mb.information(self, self.tr("Getting data"),self.tr("The OpenStreetMap server you are downloading OSM data from (~ api.openstreetmap.org) has fixed limitations of how much data you can get. As written at <http://wiki.openstreetmap.org/wiki/Getting_Data> neither latitude nor longitude extent of downloaded region can be larger than 0.25 degrees. Note that Quantum GIS allows you to specify any extent you want, but OpenStreetMap server will reject all request that won't satisfy downloading limitations."))


    def checkExtent(self):
        """Function checks if extent, currently set on dialog, is valid.

        It's called whenever download region changed.
        Result of checking is displayed on dialog.
        """

        if self.finished:
            return

        lim = 0.25        # download limitations of openstreetmap server in degrees

        # get coordinates that are currently set
        latFrom = self.latFromLineEdit.text().toDouble()[0]
        lonFrom = self.lonFromLineEdit.text().toDouble()[0]
        latTo = self.latToLineEdit.text().toDouble()[0]
        lonTo = self.lonToLineEdit.text().toDouble()[0]

        # tested conditions
        largeLatExt = False
        largeLonExt = False

        if abs(latTo-latFrom)>lim:
            largeLatExt = True
        if abs(lonTo-lonFrom)>lim:
            largeLonExt = True

        if largeLatExt and largeLonExt:
            self.extentInfoLabel.setText(self.tr("Both extents are too large!"))
        elif largeLatExt:
            self.extentInfoLabel.setText(self.tr("Latitude extent is too large!"))
        elif largeLonExt:
            self.extentInfoLabel.setText(self.tr("Longitude extent is too large!"))
        else:
            self.extentInfoLabel.setText(self.tr("OK! Area is probably acceptable to server."))


    def progressDlgCanceled(self):
        """Function is called after progress dialog is canceled.

        It aborts HTTP connection.
        """

        if self.finished:
            return

        # cancel download with no message for user
        self.cancelDownload()


    def setProxy(self):
        """Function sets proxy to HTTP connection of downloader.

        HTTP connection object is not given in function parameter,
        because it's global - accessible for the whole downloader.
        """

        if self.finished:
            return

        # getting and setting proxy information
        settings=QSettings()
        proxyHost=QString()
        proxyUser=QString()
        proxyPassword=QString()
        proxyPort=0
        proxyType=QNetworkProxy.NoProxy
        proxyEnabled=settings.value("proxy/proxyEnabled",QVariant(0)).toBool()

        if proxyEnabled:

            proxyHost=settings.value("proxy/proxyHost",QVariant("")).toString()
            proxyPort=settings.value("proxy/proxyPort",QVariant(8080)).toInt()[0]
            proxyUser=settings.value("proxy/proxyUser",QVariant("")).toString()
            proxyPassword=settings.value("proxy/proxyPassword",QVariant("")).toString()
            proxyTypeString=settings.value("proxy/proxyType",QVariant("")).toString()

            if proxyTypeString=="DefaultProxy":
                proxyType=QNetworkProxy.DefaultProxy
            elif proxyTypeString=="Socks5Proxy":
                proxyType=QNetworkProxy.Socks5Proxy
            elif proxyTypeString=="HttpProxy":
                proxyType=QNetworkProxy.HttpProxy
            elif proxyTypeString=="HttpCachingProxy":
                proxyType=QNetworkProxy.HttpCachingProxy
            elif proxyTypeString=="FtpCachingProxy":
                proxyType=QNetworkProxy.FtpCachingProxy

            self.proxy=QNetworkProxy()
            self.proxy.setType(proxyType)
            self.proxy.setHostName(proxyHost)
            self.proxy.setPort(proxyPort)
            self.http.setProxy(self.proxy)


    def connectDlgSignals(self):
        """Function connects neccessary signals to appropriate slots.
        """

        # whenever extent coordinates are changed, currently set extent has to be tested for validity
        # (coz openstreetmap has some limitations for how large area and how much data can be downloaded at once)
        self.connect(self.latFromLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.connect(self.latToLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.connect(self.lonFromLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.connect(self.lonToLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)

        self.connect(self.helpButton, SIGNAL("clicked()"), self.showExtentHelp)
        self.connect(self.downloadButton, SIGNAL("clicked()"), self.downloadFile)
        self.connect(self.cancelButton, SIGNAL("clicked()"), self.close)
        self.connect(self.choosedirButton, SIGNAL("clicked()"), self.showChooseDirectoryDialog)
        self.connect(self.autoLoadCheckBox, SIGNAL("clicked()"), self.autoLoadClicked)


    def disconnectDlgSignals(self):
        """Function disconnects connected signals.
        """

        self.disconnect(self.latFromLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.disconnect(self.latToLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.disconnect(self.lonFromLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)
        self.disconnect(self.lonToLineEdit, SIGNAL("textChanged(const QString &)"), self.checkExtent)

        self.disconnect(self.helpButton, SIGNAL("clicked()"), self.showExtentHelp)
        self.disconnect(self.downloadButton, SIGNAL("clicked()"), self.downloadFile)
        self.disconnect(self.choosedirButton, SIGNAL("clicked()"), self.showChooseDirectoryDialog)
        self.disconnect(self.autoLoadCheckBox, SIGNAL("clicked()"), self.autoLoadClicked)



