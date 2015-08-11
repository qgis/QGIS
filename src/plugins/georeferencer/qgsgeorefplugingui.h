/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef QGSGEOREFPLUGINGUI_H
#define QGSGEOREFPLUGINGUI_H

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
class QgsPoint;
class QgsRasterLayer;
class QgsRectangle;
class QgsMessageBar;

class QgsGeorefDockWidget : public QDockWidget
{
    Q_OBJECT
  public:
    QgsGeorefDockWidget( const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0 );
};

class QgsGeorefPluginGui : public QMainWindow, private Ui::QgsGeorefPluginGuiBase
{
    Q_OBJECT

  public:
    QgsGeorefPluginGui( QgisInterface* theQgisInterface, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~QgsGeorefPluginGui();

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
    void linkGeorefToQGis( bool link );
    void linkQGisToGeoref( bool link );

    // gcps
    void addPoint( const QgsPoint& pixelCoords, const QgsPoint& mapCoords,
                   bool enable = true, bool refreshCanvas = true/*, bool verbose = true*/ );
    void deleteDataPoint( const QPoint &pixelCoords );
    void deleteDataPoint( int index );
    void showCoordDialog( const QgsPoint &pixelCoords );

    void selectPoint( const QPoint & );
    void movePoint( const QPoint & );
    void releasePoint( const QPoint & );

    void loadGCPsDialog();
    void saveGCPsDialog();

    // settings
    void showRasterPropertiesDialog();
    void showGeorefConfigDialog();

    // plugin info
    void contextHelp();

    // comfort
    void jumpToGCP( uint theGCPIndex );
    void extentsChangedGeorefCanvas();
    void extentsChangedQGisCanvas();

    // canvas info
    void showMouseCoords( const QgsPoint &pt );
    void updateMouseCoordinatePrecision();

    // Histogram stretch
    void localHistogramStretch();
    void fullHistogramStretch();


    // when one Layer is removed
    void layerWillBeRemoved( QString theLayerId );
    void extentsChanged(); // Use for need add again Raster (case above)

    bool updateGeorefTransform();

    void updateIconTheme( QString theme );

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
    QLabel* createBaseLabelStatus();
    void createStatusBar();
    void setupConnections();
    void removeOldLayer();

    // Mapcanvas Plugin
    void addRaster( QString file );

    // settings
    void readSettings();
    void writeSettings();

    // gcp points
    bool loadGCPs( /*bool verbose = true*/ );
    void saveGCPs();
    QgsGeorefPluginGui::SaveGCPs checkNeedGCPSave();

    // georeference
    bool georeference();
    bool writeWorldFile( QgsPoint origin, double pixelXSize, double pixelYSize, double rotation );
    bool writePDFReportFile( const QString& fileName, const QgsGeorefTransform& transform );
    bool writePDFMapFile( const QString& fileName, const QgsGeorefTransform& transform );
    void updateTransformParamLabel();

    // gdal script
    void showGDALScript( const QStringList& commands );
    QString generateGDALtranslateCommand( bool generateTFW = true );
    /* Generate command-line for gdalwarp based on current GCPs and given parameters.
     * For values in the range 1 to 3, the parameter "order" prescribes the degree of the interpolating polynomials to use,
     * a value of -1 indicates that thin plate spline interpolation should be used for warping.*/
    QString generateGDALwarpCommand( QString resampling, QString compress, bool useZeroForTrans, int order,
                                     double targetResX, double targetResY );

    // utils
    bool checkReadyGeoref();
    QgsRectangle transformViewportBoundingBox( const QgsRectangle &canvasExtent, QgsGeorefTransform &t,
        bool rasterToWorld = true, uint numSamples = 4 );
    QString convertTransformEnumToString( QgsGeorefTransform::TransformParametrisation transform );
    QString convertResamplingEnumToString( QgsImageWarper::ResamplingMethod resampling );
    int polynomialOrder( QgsGeorefTransform::TransformParametrisation transform );
    QString guessWorldFileName( const QString &rasterFileName );
    QIcon getThemeIcon( const QString &theName );
    bool checkFileExisting( QString fileName, QString title, QString question );
    bool equalGCPlists( const QgsGCPList &list1, const QgsGCPList &list2 );
    void logTransformOptions();
    void logRequaredGCPs();
    void clearGCPData();

    /**
     * Calculates root mean squared error for the currently active
     * ground control points and transform method.
     * Note that the RMSE measure is adjusted for the degrees of freedom of the
     * used polynomial transform.
     * @param error out: the mean error
     * @return true in case of success
     */
    bool calculateMeanError( double& error ) const;

    /** Docks / undocks this window*/
    void dockThisWindow( bool dock );

    QGridLayout* mCentralLayout;

    QgsMessageBar* mMessageBar;
    QMenu *mPanelMenu;
    QMenu *mToolbarMenu;

    QAction *mActionHelp;

    QgsGCPListWidget *mGCPListWidget;
    QLineEdit *mScaleEdit;
    QLabel *mScaleLabel;
    QLabel *mCoordsLabel;
    QLabel *mTransformParamLabel;
    QLabel *mEPSG;
    unsigned int mMousePrecisionDecimalPlaces;

    QString mRasterFileName;
    QString mModifiedRasterFileName;
    QString mWorldFileName;
    QString mTranslatedRasterFileName;
    QString mGCPpointsFileName;
    QgsCoordinateReferenceSystem mProjection;
    QString mPdfOutputFile;
    QString mPdfOutputMapFile;
    double  mUserResX, mUserResY;  // User specified target scale

    QgsGeorefTransform::TransformParametrisation mTransformParam;
    QgsImageWarper::ResamplingMethod mResamplingMethod;
    QgsGeorefTransform mGeorefTransform;
    QString mCompressionMethod;

    QgisInterface *mIface;

    QgsGCPList mPoints;
    QgsGCPList mInitialPoints;
    QgsMapCanvas *mCanvas;
    QgsRasterLayer *mLayer;
    bool mAgainAddRaster;

    QgsMapTool *mToolZoomIn;
    QgsMapTool *mToolZoomOut;
    QgsMapTool *mToolPan;
    QgsMapTool *mToolAddPoint;
    QgsMapTool *mToolDeletePoint;
    QgsMapTool *mToolMovePoint;
    QgsMapTool *mToolMovePointQgis;

    QgsGeorefDataPoint *mMovingPoint;
    QgsGeorefDataPoint *mMovingPointQgis;
    QPointer<QgsMapCoordsDialog> mMapCoordsDialog;

    bool mUseZeroForTrans;
    bool mExtentsChangedRecursionGuard;
    bool mGCPsDirty;
    bool mLoadInQgis;

    QDockWidget* mDock;
    int messageTimeout();
};

#endif
