/***************************************************************************
    qgsgrassnewmapset.h  - New GRASS Mapset wizard
                             -------------------
    begin                : October, 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSNEWMAPSET_H
#define QGSGRASSNEWMAPSET_H

#include "ui_qgsgrassnewmapsetbase.h"
#include "qgscoordinatereferencesystem.h"

class QgsGrassPlugin;
class QgisInterface;
class QgsPoint;
class QgsProjectionSelector;

extern "C"
{
#include <grass/gis.h>
}


/*! \class QgsGrassNewMapset
 *  \brief GRASS vector edit.
 *
 */
class QgsGrassNewMapset : public QWizard, private Ui::QgsGrassNewMapsetBase
{
    Q_OBJECT

  public:

    enum PAGE
    {
      DATABASE,
      LOCATION,
      CRS,
      REGION,
      MAPSET,
      FINISH
    };

    //! Constructor
    QgsGrassNewMapset( QgisInterface *iface,
                       QgsGrassPlugin *plugin,
                       QWidget * parent = 0, Qt::WindowFlags f = 0 );

    //! Destructor
    ~QgsGrassNewMapset();

    //! Next page
    int nextId() const;

    //! Is running
    static bool isRunning();

    //! Close
    void close();

  public slots:
    //! Browse database
    void on_mDatabaseButton_clicked() { browseDatabase(); }
    void browseDatabase();

    //! Database changed
    void on_mDatabaseLineEdit_returnPressed() { databaseChanged(); }
    void on_mDatabaseLineEdit_textChanged() { databaseChanged(); }
    void databaseChanged();

    /***************** LOCATION *****************/
    //! Set location page
    void setLocationPage();

    //! Set locations
    void setLocations();

    //! Location radio switched
    void on_mCreateLocationRadioButton_clicked() { locationRadioSwitched(); }
    void on_mSelectLocationRadioButton_clicked() { locationRadioSwitched(); }
    void locationRadioSwitched();

    //! Existing location selection
    void on_mLocationComboBox_textChanged( const QString &txt )
    {
      existingLocationChanged( txt );
    }
    void existingLocationChanged( const QString& );

    //! New location name has changed
    void on_mLocationLineEdit_returnPressed() { newLocationChanged(); }
    void on_mLocationLineEdit_textChanged() { newLocationChanged(); }
    void newLocationChanged();

    //! Check location
    void checkLocation();

    /***************** CRS ****************/
    //! Set projection page, called when entered from location page
    void setProjectionPage();

    //! Projection selected
    void sridSelected( QString );
    void projectionSelected();

    //! Location radio switched
    void on_mNoProjRadioButton_clicked() { projRadioSwitched(); }
    void on_mProjRadioButton_clicked() { projRadioSwitched(); }
    void projRadioSwitched();

    //! Set GRASS projection structures for currently selected projection
    // or CRS_XY if 'not defined' is selected
    void setGrassProjection();

    /******************* REGION ******************/
    //! Set region page, called when entered from projection
    void setRegionPage();

    //! Set default GRASS region for current projection
    void setGrassRegionDefaults();

    //! Region Changed
    void on_mNorthLineEdit_returnPressed() { regionChanged(); }
    void on_mNorthLineEdit_textChanged() { regionChanged(); }
    void on_mSouthLineEdit_returnPressed() { regionChanged(); }
    void on_mSouthLineEdit_textChanged() { regionChanged(); }
    void on_mEastLineEdit_returnPressed() { regionChanged(); }
    void on_mEastLineEdit_textChanged() { regionChanged(); }
    void on_mWestLineEdit_returnPressed() { regionChanged(); }
    void on_mWestLineEdit_textChanged() { regionChanged(); }
    void regionChanged();

    //! Set current QGIS region
    void on_mCurrentRegionButton_clicked() { setCurrentRegion(); }
    void setCurrentRegion();

    //! Set region selected in combo box
    void on_mRegionButton_clicked() { setSelectedRegion(); }
    void setSelectedRegion();

    //! Draw current region on map
    void drawRegion();
    void clearRegion();

    /******************* MAPSET *******************/
    //! Set existing mapsets
    void setMapsets();

    //! Mapset name changed
    void on_mMapsetLineEdit_returnPressed() { mapsetChanged(); }
    void on_mMapsetLineEdit_textChanged() { mapsetChanged(); }
    void mapsetChanged();

    /******************** FINISH ******************/
    //! Set finish page
    void setFinishPage();

    //! Finish / accept
    void accept();

    //! Create new mapset
    void createMapset();

    //! New page was selected
    void pageSelected( int index );

    //! Close event
    void closeEvent( QCloseEvent *e );

    //! Key event
    void keyPressEvent( QKeyEvent * e );

    //! Set error line
    void setError( QLabel *line, const QString &err );
  private:
    //! Pointer to the QGIS interface object
    QgisInterface *mIface;

    //! Plugin
    QgsGrassPlugin *mPlugin;

    //! Editing is already running
    static bool mRunning;

    //! Projection selector
    QgsProjectionSelector *mProjectionSelector;

    //! GRASS projection
    struct Cell_head mCellHead;
    struct Key_Value *mProjInfo;
    struct Key_Value *mProjUnits;

    //! Previous page
    int mPreviousPage;

    //! Was the region page modified by user
    bool mRegionModified;

    //! Check region seting
    void checkRegion();

    //! Region map
    QPixmap mPixmap;

    //! Read predefined locations from GML
    void loadRegions();

    //! Locations were initialized
    bool mRegionsInited;

    std::vector<QgsPoint> mRegionsPoints;
    //std::vector<double> mRegionsPoints;

    //! Last projection used for region
    QgsCoordinateReferenceSystem mSrs;
    //bool mSrsSet;
};

#endif // QGSGRASSNEWMAPSET_H
