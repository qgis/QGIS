/***************************************************************************
    qgsgrassselect.h  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
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
#ifndef QGSGRASSSELECT_H
#define QGSGRASSSELECT_H
#include "qgsgrassselectbase.h"
/*! \class QgsGrassSelect
 * \brief Dialog to select GRASS layer.
 *
 */
class QgsGrassSelect: public QgsGrassSelectBase
{
public:
    //! Constructor
    //QgsGrassSelect(QWidget *parent = 0, int type = VECTOR );
    QgsGrassSelect(int type = VECTOR );
    //! Destructor
    ~QgsGrassSelect();

    enum TYPE { VECTOR, RASTER };

    //! OK 
    void accept (void);
    
    //! Open dialog for Gisdbase
    void getGisdbase(void);
 
    //! Reset combobox of locations for current Gisdbase
    void setLocations (void );
    
    //! Reset combobox of mapsets for current Location
    void setMapsets (void );

    //! Reset combobox of maps for current Gisdbase + Location
    void setMaps (void );

    //! Reset combobox of layers for current Gisdbase + Location + Map
    void setLayers (void );

    QString  gisdbase;
    QString  location;
    QString  mapset;
    QString  map;
    QString  layer;

private:
    int type; // map type (mapset element)
    static bool first; // called first time
    static QString lastGisdbase; // Last selected values
    static QString lastLocation;
    static QString lastMapset;
    static QString lastMap;
    static QString lastLayer;
};


#endif // QGSGRASSSELECT_H
