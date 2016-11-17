/***************************************************************************
  qgsspeedstrategy.h
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

#ifndef QGSSPEEDSTRATEGY_H
#define QGSSPEEDSTRATEGY_H

#include <qgsstrategy.h>

/** \ingroup analysis
 * \class QgsSpeedStrategy
 * \note added in QGIS 3.0
 * \brief Strategy for caclucating edge cost based on travel time. Should be
 * used for finding fastest path between two points.
 */
class ANALYSIS_EXPORT QgsSpeedStrategy : public QgsStrategy
{
  public:

    /**
     * Default constructor
     */
    QgsSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor );

    //! Returns edge cost
    QVariant cost( double distance, const QgsFeature& f ) const override;

    /**
     * Returns list of the source layer attributes needed for cost calculation.
     * This method called by QgsGraphDirector.
     */
    QgsAttributeList requiredAttributes() const override;

  private:
    int mAttributeId;
    double mDefaultValue;
    double mToMetricFactor;

};

#endif // QGSSPEEDSTRATEGY_H
