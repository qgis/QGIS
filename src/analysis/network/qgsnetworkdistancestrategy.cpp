/***************************************************************************
  qgsdistancestrategy.h
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*    *
*   it under the terms of the GNU General Public License as published by   *
*         *
*                                      *
*                                                                          *
***************************************************************************/

#include "qgsnetworkdistancestrategy.h"

QVariant QgsNetworkDistanceStrategy::cost( double distance, const QgsFeature &f ) const
{
  Q_UNUSED( f )
  return QVariant( distance );
}
