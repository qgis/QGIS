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
#include "qgssettingsentryenumflag.h"

#include <memory>

#include <QPointer>

class QAction;
class QActionGroup;
class QIcon;
class QPlainTextEdit;
class QLabel;

class QgisInterface;
class QgsDoubleSpinBox;
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
class QgsGCPCanvasItem;
class QgsGcpPoint;
class QgsMapLayer;

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

    static const inline QgsSettingsEntryEnumFlag<QgsImageWarper::ResamplingMethod> settingResamplingMethod = QgsSettingsEntryEnumFlag<QgsImageWarper::ResamplingMethod>( QStringLiteral( "resampling-method" ), QgsSettings::Prefix::APP_GEOREFERENCER, QgsImageWarper::ResamplingMethod::NearestNeighbour, QObject::tr( "Last used georeferencer resampling method" ) );
    static const inline QgsSettingsEntryString settingCompressionMethod = QgsSettingsEntryString( QStringLiteral( "compression-method" ), QgsSettings::Prefix::APP_GEOREFERENCER, QStringLiteral( "NONE" ), QObject::tr( "Last used georeferencer compression method" ) );
    static const inline QgsSettingsEntryBool settingUseZeroForTransparent = QgsSettingsEntryBool( QStringLiteral( "use-zero-for-transparent" ), QgsSettings::Prefix::APP_GEOREFERENCER, false, QObject::tr( "Last used georeferencer use-zero-as-transparent option" ) );
    static const inline QgsSettingsEntryEnumFlag<QgsGcpTransformerInterface::TransformMethod> settingTransformMethod = QgsSettingsEntryEnumFlag<QgsGcpTransformerInterface::TransformMethod>( QStringLiteral( "transform-method" ), QgsSettings::Prefix::APP_GEOREFERENCER, QgsGcpTransformerInterface::TransformMethod::Linear, QObject::tr( "Last used georeferencer transform method" ) );
    static const inline QgsSettingsEntryBool settingSaveGcps = QgsSettingsEntryBool( QStringLiteral( "save-gcp-points" ), QgsSettings::Prefix::APP_GEOREFERENCER, false, QObject::tr( "Whether georeferencer should automatically save .points files" ) );
    static const inline QgsSettingsEntryBool settingLoadInProject = QgsSettingsEntryBool( QStringLiteral( "load-result-in-project" ), QgsSettings::Prefix::APP_GEOREFERENCER, true, QObject::tr( "Whether georeferencer should automatically load results into the current project" ) );
    static const inline QgsSettingsEntryString settingLastSourceFolder = QgsSettingsEntryString( QStringLiteral( "last-source-folder" ), QgsSettings::Prefix::APP_GEOREFERENCER, QString(), QObject::tr( "Last used folder for georeferencer source files" ) );
    static const inline QgsSettingsEntryString settingLastRasterFileFilter = QgsSettingsEntryString( QStringLiteral( "last-raster-file-filter" ), QgsSettings::Prefix::APP_GEOREFERENCER, QString(), QObject::tr( "Last used raster file filter for georeferencer source files" ) );

    QgsGeoreferencerMainWindow( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );
    ~QgsGeoreferencerMainWindow() override;

  protected:
    void closeEvent( QCloseEvent * ) override;
    void dropEvent( QDropEvent *event ) override;
    void dragEnterEvent( QDragEnterEvent *event ) override;

  private slots:
    // file
    void reset();
    void openLayer( QgsMapLayerType layerType, const QString &fileName = QString() );
    void generateGDALScript();
    bool showTransformSettingsDialog();

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

    /**
     * Adds a new reference point.
     * \param sourceCoords MUST be in source layer coordinates, e.g. if source is already georeferenced then it is in layer coordinates NOT pixels
     * \param destinationMapCoords
     * \param destinationCrs
     * \param enable
     * \param finalize
     */
    void addPoint( const QgsPointXY &sourceCoords, const QgsPointXY &destinationMapCoords,
                   const QgsCoordinateReferenceSystem &destinationCrs, bool enable = true, bool finalize = true );

    void deleteDataPoint( QPoint pixelCoords );
    void deleteDataPoint( int index );
    void showCoordDialog( const QgsPointXY &sourceCoordinates );

    void selectPoint( QPoint );
    void movePoint( QPoint canvasPixels );
    void releasePoint( QPoint );

    void loadGCPsDialog();
    void saveGCPsDialog();

    // settings
    void showLayerPropertiesDialog();
    void showGeorefConfigDialog();

    // comfort
    void recenterOnPoint( const QgsPointXY &point );
    void extentsChangedGeorefCanvas();
    void extentsChangedQGisCanvas();
    void updateCanvasRotation();

    // canvas info
    void showMouseCoords( const QgsPointXY &pt );
    void updateMouseCoordinatePrecision();

    // Histogram stretch
    void localHistogramStretch();
    void fullHistogramStretch();

    bool updateGeorefTransform();
    void invalidateCanvasCoords();

  private:
    enum SaveGCPs
    {
      GCPSAVE,
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
    QFont statusBarFont();
    void createStatusBar();
    void setupConnections();
    void removeOldLayer();

    void loadSource( QgsMapLayerType layerType, const QString &uri, const QString &provider );

    // settings
    void readSettings();
    void writeSettings();

    // gcp points
    bool loadGCPs( QString &error );
    void saveGCPs();
    QgsGeoreferencerMainWindow::SaveGCPs checkNeedGCPSave();

    // georeference
    bool georeference();
    bool georeferenceRaster();
    bool georeferenceVector();

    bool writeWorldFile( const QgsPointXY &origin, double pixelXSize, double pixelYSize, double rotation );
    bool writePDFReportFile( const QString &fileName, const QgsGeorefTransform &transform );
    bool writePDFMapFile( const QString &fileName, const QgsGeorefTransform &transform );
    void updateTransformParamLabel();

    // gdal script
    void showGDALScript( const QStringList &commands );
    QString generateGDALtranslateCommand( bool generateTFW = true );
    QString generateGDALogr2ogrCommand() const;

    /**
     * Generate command-line for gdalwarp based on current GCPs and given parameters.
     * For values in the range 1 to 3, the parameter "order" prescribes the degree of the interpolating polynomials to use,
     * a value of -1 indicates that thin plate spline interpolation should be used for warping.
    */
    QString generateGDALwarpCommand( const QString &resampling, const QString &compress, bool useZeroForTrans, int order,
                                     double targetResX, double targetResY );

    // utils
    bool validate();
    QgsRectangle transformViewportBoundingBox( const QgsRectangle &canvasExtent, QgsGeorefTransform &t,
        bool rasterToWorld = true, uint numSamples = 4 );
    QString convertResamplingEnumToString( QgsImageWarper::ResamplingMethod resampling );
    int polynomialOrder( QgsGeorefTransform::TransformMethod transform );
    QString guessWorldFileName( const QString &sourceFileName );
    bool checkFileExisting( const QString &fileName, const QString &title, const QString &question );
    bool equalGCPlists( const QList<QgsGcpPoint> &list1, const QgsGCPList &list2 );
    void logTransformOptions();
    void logRequaredGCPs();
    void clearGCPData();

    /**
     * Calculates root mean squared error for the currently active
     * ground control points and transform method.
     * Note that the RMSE measure is adjusted for the degrees of freedom of the
     * used polynomial transform.
     * \param error out: the mean error
     * \returns TRUE in case of success
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
    QLabel *mRotationLabel = nullptr;
    QgsDoubleSpinBox *mRotationEdit = nullptr;
    unsigned int mMousePrecisionDecimalPlaces = 0;

    QString mFileName;
    QString mModifiedFileName;
    QString mWorldFileName;
    QString mTranslatedFileName;
    QString mGCPpointsFileName;
    QgsCoordinateReferenceSystem mTargetCrs;
    QgsCoordinateReferenceSystem mLastGCPProjection;
    QString mPdfOutputFile;
    QString mPdfOutputMapFile;
    bool mSaveGcp = false;
    double  mUserResX, mUserResY;  // User specified target scale

    QgsGcpTransformerInterface::TransformMethod mTransformMethod = QgsGcpTransformerInterface::TransformMethod::InvalidTransform;
    QgsImageWarper::ResamplingMethod mResamplingMethod;
    QgsGeorefTransform mGeorefTransform;
    QString mCompressionMethod = QStringLiteral( "NONE" );
    bool mCreateWorldFileOnly = false;

    QgsGCPList mPoints;
    QList< QgsGcpPoint > mSavedPoints;

    QgsMapCanvas *mCanvas = nullptr;
    std::unique_ptr< QgsMapLayer > mLayer;

    QgsMapTool *mToolZoomIn = nullptr;
    QgsMapTool *mToolZoomOut = nullptr;
    QgsMapTool *mToolPan = nullptr;
    QgsMapTool *mPrevQgisMapTool = nullptr;
    QgsGeorefToolAddPoint *mToolAddPoint = nullptr;
    QgsGeorefToolDeletePoint *mToolDeletePoint = nullptr;
    QgsGeorefToolMovePoint *mToolMovePoint = nullptr;
    QgsGeorefToolMovePoint *mToolMovePointQgis = nullptr;

    QgsGCPCanvasItem *mNewlyAddedPointItem = nullptr;

    QgsGeorefDataPoint *mMovingPoint = nullptr;
    QgsGeorefDataPoint *mMovingPointQgis = nullptr;
    QPointer<QgsMapCoordsDialog> mMapCoordsDialog;

    bool mUseZeroForTrans = false;
    bool mExtentsChangedRecursionGuard;
    bool mGCPsDirty;
    bool mLoadInQgis = true;


    QgsDockWidget *mDock = nullptr;
};

#endif
