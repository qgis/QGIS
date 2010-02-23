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
/* $Id$ */

#ifndef QGSGEOREFPLUGINGUI_H
#define QGSGEOREFPLUGINGUI_H

#include "ui_qgsgeorefpluginguibase.h"
#include "qgsgeoreftransform.h"

#include "qgsgcplist.h"
#include "qgsmapcoordsdialog.h"
#include "qgsimagewarper.h"

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

class QgsGeorefPluginGui : public QMainWindow, private Ui::QgsGeorefPluginGuiBase
{
  Q_OBJECT

public:
  QgsGeorefPluginGui( QgisInterface* theQgisInterface, QWidget* parent = 0, Qt::WFlags fl = 0 );
  ~QgsGeorefPluginGui();

protected:
  void closeEvent(QCloseEvent *);

private slots:
  // file
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
  void deleteDataPoint(int index);
  void showCoordDialog( const QgsPoint &pixelCoords );

  void selectPoint(const QPoint &);
  void movePoint(const QPoint &);
  void releasePoint(const QPoint &);

  void loadGCPsDialog();
  void saveGCPsDialog();

  // settings
  void showRasterPropertiesDialog();
  void showGeorefConfigDialog();

  // plugin info
  void contextHelp();

  // comfort
  void jumpToGCP(uint theGCPIndex);
  void extentsChangedGeorefCanvas();
  void extentsChangedQGisCanvas();

  // canvas info
  void showMouseCoords(const QgsPoint pt);
  void updateMouseCoordinatePrecision();

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
  void createStatusBar();
  void setupConnections();

  // settings
  void readSettings();
  void writeSettings();

  // gcp points
  void loadGCPs(/*bool verbose = true*/);
  void saveGCPs();
  QgsGeorefPluginGui::SaveGCPs checkNeedGCPSave();

  // georeference
  bool georeference();
  bool writeWorldFile(QgsPoint origin, double pixelXSize, double pixelYSize);

  // gdal script
  void showGDALScript(int argNum...);
  QString gdal_translateCommand(bool generateTFW = true);
  QString gdalwarpCommandGCP(QString resampling, QString compress, bool useZeroForTrans, int order,
                             double targetResX, double targetResY);
  QString gdalwarpCommandTPS(QString resampling, QString compress, bool useZeroForTrans,
                             double targetResX, double targetResY);

  // log
  void showMessageInLog(const QString &description, const QString &msg);
  void clearLog();

  // utils
  bool checkReadyGeoref();
  bool updateGeorefTransform();
  QgsRectangle transformViewportBoundingBox(const QgsRectangle &canvasExtent, const QgsGeorefTransform &t,
                                            bool rasterToWorld = true, uint numSamples = 4);
  QString convertTransformEnumToString(QgsGeorefTransform::TransformParametrisation transform);
  QString convertResamplingEnumToString(QgsImageWarper::ResamplingMethod resampling);
  int polynomeOrder(QgsGeorefTransform::TransformParametrisation transform);
  QString guessWorldFileName( const QString &rasterFileName );
  QIcon getThemeIcon( const QString &theName );
  bool checkFileExisting(QString fileName, QString title, QString question);
  bool equalGCPlists(const QgsGCPList &list1, const QgsGCPList &list2);
  void logTransformOptions();
  void logRequaredGCPs();
  void clearGCPData();


  QMenu *mPanelMenu;
  QMenu *mToolbarMenu;

//  QPlainTextEdit *mLogViewer;
  QAction *mActionHelp;

  QgsGCPListWidget *mGCPListWidget;
  QLineEdit *mScaleEdit;
  QLabel *mScaleLabel;
  QLabel *mCoordsLabel;
  QLabel *mTransformParamLabel;
  unsigned int mMousePrecisionDecimalPlaces;

  QString mRasterFileName;
  QString mModifiedRasterFileName;
  QString mWorldFileName;
  QString mTranslatedRasterFileName;
  QString mGCPpointsFileName;
  QString mProjection;
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

  QgsMapTool *mToolZoomIn;
  QgsMapTool *mToolZoomOut;
  QgsMapTool *mToolPan;
  QgsMapTool *mToolAddPoint;
  QgsMapTool *mToolDeletePoint;
  QgsMapTool *mToolMovePoint;

  QgsGeorefDataPoint *mMovingPoint;
  QPointer<QgsMapCoordsDialog> mMapCoordsDialog;

  bool mUseZeroForTrans;
  bool mExtentsChangedRecursionGuard;
  bool mGCPsDirty;
  bool mLoadInQgis;
};
#endif
