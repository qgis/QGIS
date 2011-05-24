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

#ifndef QGSEDGEPROPERTERH
#define QGSEDGEPROPERTERH

// QT4 includes
#include <QVariant>

// QGIS includes
#include <qgsfeature.h>
#include <qgslabel.h>

class ANALYSIS_EXPORT QgsEdgeProperter
{
  public:
    QgsEdgeProperter()
    { }

    virtual QgsAttributeList requiredAttributes() const
    { return QgsAttributeList(); }
    
    virtual QVariant property( double distance, const QgsFeature& f ) const
    { return QVariant(); }
};

class ANALYSIS_EXPORT QgsEdgeDistanceProperter : public QgsEdgeProperter
{
  public:
    virtual QVariant property( double distance, const QgsFeature& ) const
    {
      return QVariant( distance );
    }
};
#endif //QGSEDGEPROPERTYH
