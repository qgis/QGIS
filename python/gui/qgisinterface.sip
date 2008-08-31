
/** 
 * \class QgisInterface
 * \brief Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 * Only functionality exposed by QgisInterface can be used in plugins.
 * This interface has to be implemented with application specific details.
 *
 * QGIS implements it in QgisAppInterface class, 3rd party applications
 * could provide their own implementation to be able to use plugins.
 */

class QgisInterface : QObject
{
%TypeHeaderCode
#include <qgisinterface.h>
%End

  public:

    /** Constructor */
    QgisInterface();

    /** Virtual destructor */
    virtual ~QgisInterface();
    

  public slots: // TODO: do these functions really need to be slots?

    //! Zoom to full extent of map layers
    virtual void zoomFull()=0;
    //! Zoom to previous view extent
    virtual void zoomPrevious()=0;
    //! Zoome to extent of the active layer
    virtual void zoomActiveLayer()=0;

    //! Add a vector layer
    virtual QgsVectorLayer* addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
    //! Add a raster layer given a raster layer file name
    virtual QgsRasterLayer* addRasterLayer(QString rasterLayerPath, QString baseName = QString())=0;
    //! Add a WMS layer
    virtual QgsRasterLayer* addRasterLayer(const QString& url, const QString& layerName, const QString& providerKey, const QStringList& layers, const QStringList& styles, const QString& format, const QString& crs) = 0;

    //! Add a project
    virtual bool addProject(QString theProject)=0; 
    //! Start a blank project
    virtual void newProject(bool thePromptToSaveFlag=false)=0; 

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer()=0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon(QAction *qAction) =0;
    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon(QAction *qAction) = 0;
    //! Add toolbar with specified name
    virtual QToolBar* addToolBar(QString name)=0 /Factory/;
    /** Get the file toolbar - intended for use with plugins which
    *   add a new file type handler.
    */
    virtual QToolBar * fileToolBar()=0;

    // TODO: is this deprecated in favour of QgsContextHelp?
    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
    virtual void openURL(QString url, bool useQgisDocDirectory=true)=0;

    /** Return a pointer to the map canvas */
    virtual QgsMapCanvas * mapCanvas()=0;

    /** Return a pointer to the main window (instance of QgisApp in case of QGIS) */
    virtual QWidget * mainWindow()=0;

    /** Add action to the plugins menu */
    virtual void addPluginToMenu(QString name, QAction* action)=0;
    /** Remove action from the plugins menu */
    virtual void removePluginMenu(QString name, QAction* action)=0;

    /** Add a dock widget to the main window */
    virtual void addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget )=0;

    /** refresh legend of a layer */
    virtual void refreshLegend( QgsMapLayer * layer )=0;

  signals:
    /** Emited whenever current (selected) layer changes.
     *  The pointer to layer can be null if no layer is selected
     */
    void currentLayerChanged ( QgsMapLayer * layer );

};

