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
#ifndef QGSPOINTDIALOG_H
#define QGSPOINTDIALOG_H

#include <vector>

#include <QDialog>
#include <QString>

#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>

#include <ui_qgspointdialogbase.h>

class QAction;
class QActionGroup;
class QIcon;
class QgsGeorefDataPoint;
class QgsMapTool;
class QgisInterface;

class QgsPointDialog : public QDialog, private Ui::QgsPointDialogBase
{
    Q_OBJECT
  public:
    QgsPointDialog( QString layerPath, QgisInterface* theQgisInterface,
                    QWidget* parent = 0, Qt::WFlags fl = 0 );

    QgsPointDialog( QgisInterface* theQgisInterface, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QgsPointDialog();

    /**Opens a new image file in mCanvas*/
    void openImageFile( QString layerPath );
    void showCoordDialog( QgsPoint& pixelCoords );
    void deleteDataPoint( QgsPoint& pixelCoords );
    static QWidget* findMainWindow();

  public slots:

    void addPoint( const QgsPoint& pixelCoords, const QgsPoint& mapCoords );
    void on_pbnClose_clicked();
    void on_pbnSelectRaster_clicked();
    void on_pbnGenerateWorldFile_clicked();
    void on_pbnGenerateAndLoad_clicked();
    void on_pbnSelectWorldFile_clicked();
    void on_pbnSelectModifiedRaster_clicked();
    void on_pbnSaveGCPs_clicked();
    void on_pbnLoadGCPs_clicked();
    void on_cmbTransformType_currentIndexChanged( const QString& );
    void on_leSelectModifiedRaster_textChanged( const QString & );
    void zoomIn();
    void zoomOut();
    void zoomToLayer();
    void pan();
    void addPoint();
    void deletePoint();
//    void enableRelevantControls( void );

  private:

    void initialize();
    bool generateWorldFileAndWarp();
    bool helmertWarp();
    void loadGCPs( QString & );
    void saveGCPs( std::vector<QgsPoint>, std::vector<QgsPoint> );
    QString guessWorldFileName( const QString& raster );

    void enableModifiedRasterControls( bool state );
    void enableControls( bool state );
    QIcon getThemeIcon( const QString theName );

    QActionGroup* mMapToolGroup;
    QAction* mActionZoomIn;
    QAction* mActionZoomOut;
    QAction* mActionZoomToLayer;
    QAction* mActionPan;
    QAction* mActionAddPoint;
    QAction* mActionDeletePoint;

    QgsMapCanvas* mCanvas;
    QgsRasterLayer* mLayer;

    QgsMapTool* mToolZoomIn;
    QgsMapTool* mToolZoomOut;
    QgsMapTool* mToolPan;
    QgsMapTool* mToolAddPoint;
    QgsMapTool* mToolDeletePoint;

    QString mProjBehaviour, mProjectCRS;
    int mProjectCrsId;
    /**dialog to enter reference point*/
//    QgsPointDialog* mPointDialog;
    /**Flag if plugin windows have been arranged with button*/
//    QSize origSize;
//    QPoint origPos;

//  std::vector<QgsPoint> mPixelCoords, mMapCoords;
//  std::vector<QString> mAcetateIDs;
    std::vector<QgsGeorefDataPoint*> mPoints;
    QgisInterface* mIface;
    int mAcetateCounter;
};

#endif
