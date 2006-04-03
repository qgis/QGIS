/***************************************************************************
                             qgsmarkercatalogue.h
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
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
#ifndef QGSMARKERCATALOGUE_H
#define QGSMARKERCATALOGUE_H

#include <iostream>

class QString;
class QStringList;
class QPicture;
class QPixmap;
class QPen;
class QBrush;

/** Catalogue of point symbols */
class QgsMarkerCatalogue{

public:
    //! Destructor
    ~QgsMarkerCatalogue();

    //! Access to canonical QgsMarkerCatalogue instance
    static QgsMarkerCatalogue *instance();
    
    /**List of available markers*/
    QStringList list();
    
    /** Returns picture of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     */
    QPixmap marker ( QString fullName, int size, QPen pen, QBrush brush, bool qtBug = true );

    /* Returns a pixmap given a filename of a svg marker */
    static QPixmap svgMarker ( QString name, int size );
private:

    /**Constructor*/
    QgsMarkerCatalogue();

    static QgsMarkerCatalogue *mMarkerCatalogue;

    /** List of availabel markers*/
    QStringList mList;

    /** Hard coded */
    QPicture hardMarker ( QString name, int size, QPen pen, QBrush brush, bool qtBug = true );

};
    
#endif // QGSMARKERCATALOGUE_H


