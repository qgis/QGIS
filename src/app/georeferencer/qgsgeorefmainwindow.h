/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef QGSGEOREFMAINWINDOW_H
#define QGSGEOREFMAINWINDOW_H

#include "ui_qgsgeorefpluginguibase.h"
#include "qgsgeoreftransform.h"

#include "qgsgcplist.h"
#include "qgsmapcoordsdialog.h"
#include "qgsimagewarper.h"
#include "qgscoordinatereferencesystem.h"

#include <QPointer>

class QAction;
class QActionGroup;
class QIcon;
class QPlainTextEdit;
class QLabel;

class QgisInterface;
class QgsGeorefDataPoint;
class QgsGCPListWidget;
class QgsMapTool;
class QgsMapCanvas;
class QgsMapCoordsDialog;
class QgsPointXY;
class QgsRasterLayer;
class QgsRectangle;
class QgsMessageBar;
class QgsGeorefToolAddPoint;
class QgsGeorefToolDeletePoint;
class QgsGeorefToolMovePoint;
class QgsGeorefToolMovePoint;

class QgsGeorefDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    QgsGeorefDockWidget( const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
};

class QgsGeoreferencerMainWindow : public QMainWindow, private Ui::QgsGeorefPluginGuiBase
{
    Q_OBJECT

  public:
    QgsGeoreferencerMainWindow( QWidget *parent = nullptr, Qt::WindowFlags fl = nullptr );
    ~QgsGeoreferencerMainWindow() override;

  protected:
    void closeEvent( QCloseEvent * ) override;

  private slots:
    // file
    void reset();
    void openRaster();
    void doGeoreference();
    void generateGDALScript();
    bool getTransformSettings();

    // edit
    void setAddPointTool();
    void setDeletePointTool();
    void setMovePointTool();

    // view
    void setZoomInTool();
    void setZoomOutTool();
    void zoomToLayerTool();
    void zoomToLast();
    void zoomToNext();
    void setPanTool();
    void linkGeorefToQgis( bool link );
    void linkQGisToGeoref( bool link );

    // gcps
    void addPoint( const QgsPointXY &pixelCoords, const QgsPointXY &mapCoords,
                   bool enable = true, bool finalize = true );
    void deleteDataPoint( QPoint pixelCoords );
    void deleteDataPoint( int index );
    void showCoordDialog( const QgsPointXY &pixelCoords );

    void selectPoint( QPoint );
    void movePoint( QPoint );
    void releasePoint( QPoint );

    void loadGCPsDialog();
    void saveGCPsDialog();

    // settings
    void showRasterPropertiesDialog();
    void showGeorefConfigDialog();

    // comfort
    void jumpToGCP( uint theGCPIndex );
    void extentsChangedGeorefCanvas();
    void extentsChangedQGisCanvas();

    // canvas info
    void showMouseCoords( const QgsPointXY &pt );
    void updateMouseCoordinatePrecision();

    // Histogram stretch
    void localHistogramStretch();
    void fullHistogramStretch();


    // when one Layer is removed
    void layerWillBeRemoved( const QString &layerId );
    void extentsChanged(); // Use for need add again Raster (case above)

    bool updateGeorefTransform();

  private:
    enum SaveGCPs
    {
      GCPSAVE,
      GCPSILENTSAVE,
      GCPDISCARD,
      GCPCANCEL
    };

    // gui
    void createActions();
    void createActionGroups();
    void createMapCanvas();
    void createMenus();
    void createDockWidgets();
    QLabel *createBaseLabelStatus();
    void createStatusBar();
    void setupConnections();
    void removeOldLayer();

    // Mapcanvas Plugin
    void addRaster( const QString &file );

    // settings
    void readSettings();
    void writeSettings();

    // gcp points
    bool loadGCPs( /*bool verbose = true*/ );
    void saveGCPs();
    QgsGeoreferencerMainWindow::SaveGCPs checkNeedGCPSave();

    // georeference
    bool georeference();
    bool writeWorldFile( const QgsPointXY &origin, double pixelXSize, double pixelYSize, double rotation );
    bool writePDFReportFile( const QString &fileName, const QgsGeorefTransform &transform );
    bool writePDFMapFile( const QString &fileName, const QgsGeorefTransform &transform );
    void updateTransformParamLabel();

    // gdal script
    void showGDALScript( const QStringList &commands );
    QString generateGDALtranslateCommand( bool generateTFW = true );
    /* Generate command-line for gdalwarp based on current GCPs and given parameters.
     * For values in the range 1 to 3, the parameter "order" prescribes the degree of the interpolating polynomials to use,
     * a value of -1 indicates that thin plate spline interpolation should be used for warping.*/
    QString generateGDALwarpCommand( const QString &resampling, const QString &compress, bool useZeroForTrans, int order,
                                     double targetResX, double targetResY );

    // utils
    bool checkReadyGeoref();
    QgsRectangle transformViewportBoundingBox( const QgsRectangle &canvasExtent, QgsGeorefTransform &t,
        bool rasterToWorld = true, uint numSamples = 4 );
    QString convertTransformEnumToString( QgsGeorefTransform::TransformParametrisation transform );
    QString convertResamplingEnumToString( QgsImageWarper::ResamplingMethod resampling );
    int polynomialOrder( QgsGeorefTransform::TransformParametrisation transform );
    QString guessWorldFileName( const QString &rasterFileName );
    bool checkFileExisting( const QString &fileName, const QString &title, const QString &question );
    bool equalGCPlists( const QgsGCPList &list1, const QgsGCPList &list2 );
    void logTransformOptions();
    void logRequaredGCPs();
    void clearGCPData();

    /**
     * Calculates root mean squared error for the currently active
     * ground control points and transform method.
     * Note that the RMSE measure is adjusted for the degrees of freedom of the
     * used polynomial transform.
     * \param error out: the mean error
     * \returns true in case of success
     */
    bool calculateMeanError( double &error ) const;

    //! Docks / undocks this window
    void dockThisWindow( bool dock );

    QGridLayout *mCentralLayout = nullptr;

    QgsMessageBar *mMessageBar = nullptr;
    QMenu *mPanelMenu = nullptr;
    QMenu *mToolbarMenu = nullptr;

    QgsGCPListWidget *mGCPListWidget = nullptr;
    QLineEdit *mScaleEdit = nullptr;
    QLabel *mScaleLabel = nullptr;
    QLabel *mCoordsLabel = nullptr;
    QLabel *mTransformParamLabel = nullptr;
    QLabel *mEPSG = nullptr;
    unsigned int mMousePrecisionDecimalPlaces;

    QString mRasterFileName;
    QString mModifiedRasterFileName;
    QString mWorldFileName;
    QString mTranslatedRasterFileName;
    QString mGCPpointsFileName;
    QgsCoordinateReferenceSystem mProjection;
    QString mPdfOutputFile;
    QString mPdfOutputMapFile;
    QString mSaveGcp;
    double  mUserResX, mUserResY;  // User specified target scale

    QgsGeorefTransform::TransformParametrisation mTransformParam;
    QgsImageWarper::ResamplingMethod mResamplingMethod;
    QgsGeorefTransform mGeorefTransform;
    QString mCompressionMethod;

    QgsGCPList mPoints;
    QgsGCPList mInitialPoints;
    QgsMapCanvas *mCanvas = nullptr;
    QgsRasterLayer *mLayer = nullptr;
    bool mAgainAddRaster;

    QgsMapTool *mToolZoomIn = nullptr;
    QgsMapTool *mToolZoomOut = nullptr;
    QgsMapTool *mToolPan = nullptr;
    QgsMapTool *mPrevQgisMapTool = nullptr;
    QgsGeorefToolAddPoint *mToolAddPoint = nullptr;
    QgsGeorefToolDeletePoint *mToolDeletePoint = nullptr;
    QgsGeorefToolMovePoint *mToolMovePoint = nullptr;
    QgsGeorefToolMovePoint *mToolMovePointQgis = nullptr;

    QgsGeorefDataPoint *mMovingPoint = nullptr;
    QgsGeorefDataPoint *mMovingPointQgis = nullptr;
    QPointer<QgsMapCoordsDialog> mMapCoordsDialog;

    bool mUseZeroForTrans;
    bool mExtentsChangedRecursionGuard;
    bool mGCPsDirty;
    bool mLoadInQgis;


    QgsDockWidget *mDock = nullptr;
    int messageTimeout();
};

#endif
