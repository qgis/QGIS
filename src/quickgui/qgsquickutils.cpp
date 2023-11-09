/***************************************************************************
  qgsquickutils.cpp
  --------------------------------------
  Date                 : 7.11.2022
  Copyright            : (C) 2022 by Tomas Mizera
  Email                : tomas.mizera (at) lutraconsulting.co.uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickutils.h"

QgsQuickUtils::QgsQuickUtils( QObject *parent )
  : QObject( parent )
{
}

QgsPoint QgsQuickUtils::toQgsPoint( const QPointF &point )
{
  return QgsPoint( point );
}
