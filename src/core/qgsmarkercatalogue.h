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
class QPainter;
class QPicture;

/** Catalogue of point symbols */
class QgsMarkerCatalogue{

public:
    //! Destructor
    ~QgsMarkerCatalogue();

    //! Access to canonical QgsMarkerCatalogue instance
    static QgsMarkerCatalogue *instance();
    
    /**List of available markers*/
    QStringList list();
    
    /** Returns pixmap of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     */
    QPixmap pixmapMarker (QString fullName, int size, QPen pen, QBrush brush, bool qtBug = true );
    /** Returns qpicture of the marker
     * \param fullName full name, e.g. hard:circle, svg:/home/usr1/marker1.svg
     */
    QPicture pictureMarker (QString fullName, int size, QPen pen, QBrush brush, bool qtBug = true );

    /* Returns a pixmap given a filename of a svg marker 
     * NOTE: this method needs to be public static for QgsMarkerDialog::visualizeMarkers */
    static void svgMarker (QPainter * thepPainter, QString name, int size );
private:

    /**Constructor*/
    QgsMarkerCatalogue();

    static QgsMarkerCatalogue *mMarkerCatalogue;

    /** List of availabel markers*/
    QStringList mList;

    /** Hard coded */
    void hardMarker (QPainter * thepPainter, QString name, int size, QPen pen, QBrush brush, bool qtBug = true );

};
    
#endif // QGSMARKERCATALOGUE_H


