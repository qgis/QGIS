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

#include <vector>
class QString;
class QCloseEvent;
#include <qlabel.h>

// Must be here, so that it is included to moc file
#include "../../src/qgisapp.h"
#include "../../src/qgisiface.h"
#include "../../src/qgspoint.h"
#include "../../src/qgsspatialrefsys.h"
#include "../../widgets/projectionselector/qgsprojectionselector.h"

class QgsGrassProvider;
#include "qgsgrassplugin.h"
#include "qgsgrassnewmapsetbase.h"
#include "qgsgrassselect.h"
#include "qgsgrassattributes.h"

extern "C" {
#include <gis.h>
#include <gprojects.h>
#include <Vect.h>
}

/*! \class QgsGrassNewMapset
 *  \brief GRASS vector edit.
 *
 */
class QgsGrassNewMapset : public QgsGrassNewMapsetBase
{
    Q_OBJECT;

public:

    enum PAGE { 
		DATABASE, 
		LOCATION, 
		PROJECTION,
		REGION,
		MAPSET, 
		FINISH
    	      };
    
    //! Constructor
    QgsGrassNewMapset ( QgisApp *qgisApp, QgisIface *iface,
                   QgsGrassPlugin *plugin, 
	           QWidget * parent = 0, const char * name = 0, WFlags f = 0 );

    //! Destructor
    ~QgsGrassNewMapset();

    //! Is running 
    static bool isRunning();

    //! Close
    void close(); 

public slots:
    //! Browse database
    void browseDatabase();

    //! Database changed
    void databaseChanged();
    
    /***************** LOCATION *****************/
    //! Set location page
    void setLocationPage ( );

    //! Set locations
    void setLocations ( );

    //! Location radio switched
    void locationRadioSwitched ( );

    //! Existing location selection
    void existingLocationChanged(const QString&);

    //! New location name has changed
    void newLocationChanged();

    //! Check location
    void checkLocation();

    /***************** PROJECTION ****************/
    //! Set projection page, called when entered from location page
    void setProjectionPage();

    //! Projection selected
    void sridSelected(QString);
    void projectionSelected();

    //! Location radio switched
    void projRadioSwitched ( );

    //! Set GRASS projection structures for currently selected projection
    // or PROJECTION_XY if 'not defined' is selected
    void setGrassProjection();
    
    /******************* REGION ******************/
    //! Set region page, called when entered from projection
    void setRegionPage();

    //! Set default GRASS region for current projection
    void setGrassRegionDefaults();

    //! Region Changed
    void regionChanged();

    //! Set current QGIS region
    void setCurrentRegion();

    //! Set region selected in combo box
    void setSelectedRegion();

    //! Draw current region on map
    void drawRegion();
    void clearRegion();

    /******************* MAPSET *******************/
    //! Set existing mapsets
    void setMapsets();

    //! Mapset name changed
    void mapsetChanged();

    /******************** FINISH ******************/
    //! Set finish page
    void setFinishPage();

    //! Finish / accept 
    void accept ();

    //! Create new mapset
    void createMapset();

    //! New page was selected
    void pageSelected ( const QString & );

    //! Close event
    void closeEvent(QCloseEvent *e);

    //! Key event
    void keyPressEvent ( QKeyEvent * e );

    //! Set error line
    void setError ( QLabel *line, const QString &err ); 
private:
    //! QGIS application
    QgisApp *mQgisApp; 
    
    //! Pointer to the QGIS interface object
    QgisIface *mIface;

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
    QgsSpatialRefSys mSrs;
    //bool mSrsSet; 
};

#endif // QGSGRASSNEWMAPSET_H
