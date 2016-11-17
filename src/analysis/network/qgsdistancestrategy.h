/***************************************************************************
  qgsdistancestrategy.h
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

#ifndef QGSDISTANCESTRATEGY_H
#define QGSDISTANCESTRATEGY_H

#include <qgsstrategy.h>

/** \ingroup analysis
 * \class QgsDistanceStrategy
 * \brief Strategy for caclucating edge cost based on its length. Should be
 * used for finding shortest path between two points.
 */
class ANALYSIS_EXPORT QgsDistanceStrategy : public QgsStrategy
{
  public:
    virtual QVariant cost( double distance, const QgsFeature& ) const override;
};

#endif // QGSDISTANCEARCPROPERTER_H
