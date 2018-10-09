/***************************************************************************
  qgsgraphdirector.h
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

#ifndef QGSGRAPHDIRECTOR_H
#define QGSGRAPHDIRECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

#include "qgis.h"
#include "qgsfeedback.h"
#include "qgsnetworkstrategy.h"
#include "qgis_analysis.h"

class QgsGraphBuilderInterface;
class QgsPoint;

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgsvectorlayerdirector.h>
% End
#endif

/**
 * \ingroup analysis
 * \class QgsGraphDirector
 * \brief Determine making the graph. QgsGraphBuilder and QgsGraphDirector implemented
 * using "builder" design patter.
 */
class ANALYSIS_EXPORT QgsGraphDirector : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsVectorLayerDirector * >( sipCpp ) != NULL )
      sipType = sipType_QgsVectorLayerDirector;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT

  public:

    ~QgsGraphDirector() override
    {
      qDeleteAll( mStrategies );
    }

    /**
     * Make a graph using QgsGraphBuilder
     *
     * \param builder the graph builder
     * \param additionalPoints list of points that should be snapped to the graph
     * \param snappedPoints list of snapped points
     * \param feedback feedback object for reporting progress
     * \note if snappedPoints[i] == QgsPointXY(0.0,0.0) then snapping failed.
     */
    virtual void makeGraph( QgsGraphBuilderInterface *builder,
                            const QVector< QgsPointXY > &additionalPoints,
                            QVector< QgsPointXY > &snappedPoints SIP_OUT,
                            QgsFeedback *feedback = nullptr ) const
    {
      Q_UNUSED( builder );
      Q_UNUSED( additionalPoints );
      Q_UNUSED( snappedPoints );
      Q_UNUSED( feedback );
    }

    //! Add optimization strategy
    void addStrategy( QgsNetworkStrategy *prop SIP_TRANSFER )
    {
      mStrategies.push_back( prop );
    }

    //! Returns director name
    virtual QString name() const = 0;

  protected:
    QList<QgsNetworkStrategy *> mStrategies;
};

#endif // QGSGRAPHDIRECTOR_H
