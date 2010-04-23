/***************************************************************************
                          qgsrubberselectid.h
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.comm

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: $ */

#ifndef QGSRUBBERSELECTID_H
#define QGSRUBBERSELECTID_H

#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"

/**
* \class QgsRubberSelectId
* \brief This Class management the RubberBand for selected ID from query result
*/
class QgsRubberSelectId
{
public:
    /**
    * Constructor for a class RubberSelectedId.
    * @param mapCanvas Pointer to the iface.mapCanvas().
    */
    QgsRubberSelectId( QgsMapCanvas* mapCanvas);
    /**
    * \brief Destructor
    */
    ~QgsRubberSelectId();

    /**
    * \brief Set if is geometry polygon for rubber band
    * \param isPolygon   boolean for type geometry is polygon
    */
    void isGeometryNotPolygon(bool isPolygon);

    /**
    * \brief Reset rubber band
    */
    void reset();

    /**
    * \brief Set color for rubber band
    * \param colorRed     integer for value red (0 - 255)
    * \param colorGreen   integer for value green (0 - 255)
    * \param colorBlue    integer for value blue (0 - 255)
    * \param alfa         float for transparent(0.0 - 1)
    */
    void setColor( int colorRed, int colorGreen, int colorBlue, int width, float alfa );

    /**
    * \brief Create rubber band from geometry by feature
    * \param mLayer    pointer to QgsVectorLayer
    * \param fid       integer for ID for feature
    */
    void addFeature( QgsVectorLayer* mLayer, int fid );

    /**
    * \brief Show rubber band
    */
    void show();
private:
    //! RubberBand
    QgsRubberBand* mRubberBand;
    bool mIsPolygon;
    QgsMapCanvas* mMapCanvas;
};

#endif

