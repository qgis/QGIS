/***************************************************************************
  qgsstrategy.h
  --------------------------------------
  Date                 : 2011-04-01
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

#ifndef QGSSTRATERGY_H
#define QGSSTRATERGY_H

#include <QVariant>

#include <qgsfeature.h>
#include <qgsfeaturerequest.h>

/**
 * \ingroup analysis
 * \class QgsStrategy
 * \note added in QGIS 3.0
 * \brief QgsStrategy defines strategy used for calculation of the edge cost. For example it can
 * take into account travel distance, amount of time or money. Currently there are two strategies
 * implemented in the analysis library: QgsDistanceStrategy and QgsSpeedStrategy.
 * QgsStrategy implemented with strategy design pattern.
 */
class ANALYSIS_EXPORT QgsStrategy
{
  public:

    /**
     * Default constructor
     */
    QgsStrategy() {}

    virtual ~QgsStrategy() {}

    /**
     * Returns list of the source layer attributes needed for cost calculation.
     * This method called by QgsGraphDirector.
     * \return list of required attributes
     */
    virtual QgsAttributeList requiredAttributes() const { return QgsAttributeList(); }

    /**
     * Return edge cost
     */
    virtual QVariant cost( double distance, const QgsFeature &f ) const
    {
      Q_UNUSED( distance );
      Q_UNUSED( f );
      return QVariant();
    }
};

#endif // QGSSTRATERGY_H
