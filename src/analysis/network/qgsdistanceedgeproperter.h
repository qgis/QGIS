/***************************************************************************
  qgsedgeproperter.h
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

#ifndef QGSEDGEDISTANCEPROPERTERH
#define QGSEDGEDISTANCEPROPERTERH

// QT4 includes
#include <QVariant>

// QGIS includes
#include <qgsedgeproperter.h>

class ANALYSIS_EXPORT QgsDistanceEdgeProperter : public QgsEdgeProperter
{
  public:
    virtual QVariant property( double distance, const QgsFeature& ) const;
};
#endif //QGSEDGEDISTANCEPROPERTYH
