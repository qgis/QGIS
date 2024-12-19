/***************************************************************************
  qgsgraphbuilder.h
  --------------------------------------
  Date                 : 2010-10-25
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

#ifndef QGSGRAPHBUILDER_H
#define QGSGRAPHBUILDER_H

#include "qgsgraphbuilderinterface.h"
#include "qgis_sip.h"

#include "qgsspatialindex.h"
#include "qgis_analysis.h"

class QgsDistanceArea;
class QgsCoordinateTransform;
class QgsGraph;

/**
* \ingroup analysis
* \class QgsGraphBuilder
* \brief This class used for making the QgsGraph object
*/

class ANALYSIS_EXPORT QgsGraphBuilder : public QgsGraphBuilderInterface SIP_NODEFAULTCTORS
{
  public:
    /**
     * Default constructor
     */
    QgsGraphBuilder( const QgsCoordinateReferenceSystem &crs, bool otfEnabled = true, double topologyTolerance = 0.0, const QString &ellipsoidID = "WGS84" );

    ~QgsGraphBuilder() override;

    /*
     * MANDATORY BUILDER PROPERTY DECLARATION
     */
    void addVertex( int id, const QgsPointXY &pt ) override;

    void addEdge( int pt1id, const QgsPointXY &pt1, int pt2id, const QgsPointXY &pt2, const QVector<QVariant> &prop ) override;

    /**
     * Returns the generated QgsGraph.
     *
     * The builder is left in its current state.
     *
     * \see takeGraph()
     */
    QgsGraph graph() const;

    /**
     * Takes the generated graph from the builder, resetting the builder back to its initial
     * state ready for additional graph construction.
     *
     * \since QGIS 3.22
     */
    QgsGraph *takeGraph() SIP_FACTORY;

  private:
    std::unique_ptr<QgsGraph> mGraph;

    QgsGraphBuilder( const QgsGraphBuilder & ) = delete;
    QgsGraphBuilder &operator=( const QgsGraphBuilder & ) = delete;
};

// clazy:excludeall=qstring-allocations

#endif // QGSGRAPHBUILDER_H
