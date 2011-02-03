/***************************************************************************
  graphdirector.h
  --------------------------------------
  Date                 : 2010-10-18
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
#ifndef ROADGRAPH_GRAPHDIRECTOR
#define ROADGRAPH_GRAPHDIRECTOR

//QT4 includes

//QGIS includes
#include <qgsrectangle.h>

//forward declarations
class RgGraphBuilder;

/** 
 * \class RgGraphDirector
 * \brief Determine making the graph
 */
class RgGraphDirector
{
  public:
    //! Destructor
    virtual ~RgGraphDirector() { };

    /**
     * Make a graph using RgGraphBuilder
     *
     * @param builder   The graph builder
     *
     * @param additionalPoints  Vector of points that must be tied to the graph
     *
     * @param tiedPoints  Vector of tied points
     *
     * @note if tiedPoints[i]==QgsPoint(0.0,0.0) then tied failed.
     */
    virtual void makeGraph( RgGraphBuilder *builder, 
                            const QVector< QgsPoint >& additionalPoints, 
                            QVector< QgsPoint>& tiedPoints ) const = 0;

    /**
     * return Director name
     */
    virtual QString name() const = 0;
};
#endif //GRAPHDIRECTOR
