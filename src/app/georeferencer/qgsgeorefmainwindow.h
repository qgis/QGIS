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
#include "qgssettingstree.h"

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
class QgsScreenHelper;
class QgsSettingsEntryBool;
class QgsSettingsEntryString;
template<class T> class QgsSettingsEntryEnumFlag;


class QgsGeorefDockWidget : public QgsDockWidget
{
    Q_OBJECT
  public:
    QgsGeorefDockWidget( const QString &title, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
};

class APP_EXPORT QgsGeoreferencerMainWindow : public QMainWindow, private Ui::QgsGeorefPluginGuiBase
{
    Q_OBJECT

  public:
    static inline QgsSettingsTreeNode *sTreeGeoreferencer = QgsSettingsTree::sTreeApp->createChildNode( QStringLiteral( "georeferencer" ) );

    static const QgsSettingsEntryEnumFlag<QgsImageWarper::ResamplingMethod> *settingResamplingMethod;
    static const QgsSettingsEntryString *settingCompressionMethod;
    static const QgsSettingsEntryBool *settingUseZeroForTransparent;
    static const QgsSettingsEntryEnumFlag<QgsGcpTransformerInterface::TransformMethod> *settingTransformMethod;
    static const QgsSettingsEntryBool *settingSaveGcps;
    static const QgsSettingsEntryBool *settingLoadInProject;
    static const QgsSettingsEntryString *settingLastSourceFolder;
    static const QgsSettingsEntryString *settingLastRasterFileFilter;

    QgsGeoreferencerMainWindow( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );
    ~QgsGeoreferencerMainWindow() override;

  protected:
    void closeEvent( QCloseEvent * ) override;
    void dropEvent( QDropEvent *event ) override;
    void dragEnterEvent( QDragEnterEvent *event ) override;

  private slots:
    // file
    void reset();
    void openLayer( Qgis::LayerType layerType, const QString &fileName = QString() );
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

    void loadSource( Qgis::LayerType layerType, const QString &uri, const QString &provider );

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

    void postProcessGeoreferencedLayer( const QString &fileName, Qgis::LayerType type, const QString &provider );

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

    QgsScreenHelper *mScreenHelper = nullptr;

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

    QgsGeorefDataPoint *mMovingPoint = nullptr;
    QgsGeorefDataPoint *mMovingPointQgis = nullptr;
    QgsGeorefDataPoint *mNewlyAddedPoint = nullptr;
    QPointer<QgsMapCoordsDialog> mMapCoordsDialog;

    bool mUseZeroForTrans = false;
    bool mExtentsChangedRecursionGuard;
    bool mGCPsDirty;
    bool mLoadInQgis = true;


    QgsDockWidget *mDock = nullptr;

    friend class TestQgsGeoreferencer;
};

#endif
