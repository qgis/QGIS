/***************************************************************************
    qgsgrassregion.h  -  Edit region
                             -------------------
    begin                : August, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSREGION_H
#define QGSGRASSREGION_H

#include <qpointarray.h>

class QgsGrassProvider;
class QgsGrassPlugin;
class QgisApp;
class QgisIface;
class QgsMapCanvas;
class QButtonGroup;
class QgsPoint;
#include "qgsgrassregionbase.h"

extern "C" {
#include <gis.h>
}

/*! \class QgsGrassRegion
 *  \brief GRASS attributes.
 *
 */
class QgsGrassRegion: public QgsGrassRegionBase
{
    Q_OBJECT;

public:
    //! Constructor
    QgsGrassRegion ( QgsGrassPlugin *plugin, QgisApp *qgisApp, QgisIface *interface, 
	             QWidget * parent = 0, const char * name = 0, 
		     WFlags f = 0 );

    //! Destructor
    ~QgsGrassRegion();

    //! Is Running
    static bool isRunning (void);

public slots:
    //! OK
    void accept ( void );

    //! Close
    void reject ( void );

    //! Called when rendering is finished
    void postRender ( QPainter * );

    //! Mouse event receiver
    void mouseEventReceiverMove ( QgsPoint & );

    //! Mouse event receiver
    void mouseEventReceiverClick ( QgsPoint & );

    //! Calculate region, called if any value is changed
    void adjust ( void );

    //! Value in GUI was changed
    void northChanged(const QString &str);
    void southChanged(const QString &str);
    void eastChanged(const QString &str);
    void westChanged(const QString &str);
    void NSResChanged(const QString &str);
    void EWResChanged(const QString &str);
    void rowsChanged(const QString &str);
    void colsChanged(const QString &str);

    void radioChanged ( void ) ;
    
    void changeColor ( void ) ;
    void changeWidth ( void ) ;

private:
    //! Editing is already running
    static bool mRunning;
    
    //! Pointer to plugin 
    QgsGrassPlugin *mPlugin;

    //! Pointer to vector provider 
    QgisApp *mQgisApp;

    //! Pointer to QGIS interface
    QgisIface *mInterface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    QButtonGroup *mNSRadioGroup;
    QButtonGroup *mEWRadioGroup;

    //! Current new region
    struct Cell_head mWindow;

    //! Display current state of new region in XOR mode
    void displayRegion(void);

    //! Region was displayed 
    bool mDisplayed;

    //! Old displayed region points
    QPointArray mPointArray;

    //! Draw region
    void draw ( double x1, double y1, double x2, double y2 );

    //! Status of input from canvas 
    bool mDraw;

    //! First corner coordinates
    double mX;
    double mY;
    
    //! Currently updating GUI, don't run *Changed methods
    bool mUpdatingGui;

    // Set region values in GUI from mWindow
    void setGuiValues( bool north = true, bool south = true, bool east = true, bool west = true, 
	               bool nsres = true, bool ewres = true, bool rows = true, bool cols = true );


    void restorePosition(void);
    
    void saveWindowLocation(void);
};

#endif // QGSGRASSREGION_H
