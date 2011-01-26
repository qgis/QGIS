/***************************************************************************
  graphbuilder.h
  --------------------------------------
  Date                 : 2010-10-22
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/
#ifndef ROADGRAPH_GRAPHBUILDER
#define ROADGRAPH_GRAPHBUILDER

#include "utils.h"

//QT4 includes

//QGIS includes
#include <qgspoint.h>
#include <qgscoordinatereferencesystem.h>

//forward declarations

/**
* \class RgGraphDirector
* \brief Determine making the graph, contained the settings
*/
class RgGraphBuilder
{
  public:
    //! Destructor
    virtual ~RgGraphBuilder()
    {};
    /**
     * set source CRS
     */
    virtual void setSourceCrs( const QgsCoordinateReferenceSystem& crs ) = 0;

    /**
     * set destionation CRS
     */
    virtual void setDestinationCrs( const QgsCoordinateReferenceSystem& crs ) = 0;

    /**
     * add vertex
     */
    virtual void addVertex( const QgsPoint& pt ) = 0;

    /**
     * add arc
     */
    virtual void addArc( const QgsPoint& pt1, const QgsPoint& pt2, double speed ) = 0;

    /**
     * tie point
     * @param pt maps point
     * @param ok ok = false if tiePoint failed.
     * @return Graph vertex corresponding pt.
     * @note: graph can be modified
     */
    virtual QgsPoint tiePoint( const QgsPoint &pt, bool &ok ) = 0;

};
#endif //GRAPHBUILDER
