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

#include <qbrush.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <qdom.h>
#include <qstringlist.h>

class QString;

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
    QPicture marker ( QString fullName, int size, QPen pen, QBrush brush, int oversampling = 1 );

private:

    /**Constructor*/
    QgsMarkerCatalogue();

    static QgsMarkerCatalogue *mMarkerCatalogue;

    /** List of availabel markers*/
    QStringList mList;

    /** Hard coded */
    QPicture hardMarker ( QString name, int size, QPen pen, QBrush brush, int oversample = 1 );

    /** Hard coded */
    QPicture svgMarker ( QString name, int size );
};
    
#endif // QGSMARKERCATALOGUE_H


