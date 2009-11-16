/***************************************************************************
                           qgsrenderer.cpp

                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrenderer.h"

#include <QBrush>
#include <QColor>
#include <QMatrix>
#include <QString>


QColor QgsRenderer::mSelectionColor = QColor( 0, 0, 0 );

QgsRenderer::QgsRenderer()
{

}

QgsRenderer::~QgsRenderer()
{
}

void QgsRenderer::setSelectionColor( QColor color )
{
  mSelectionColor = color;
}

QColor QgsRenderer::selectionColor()
{
  return mSelectionColor;
}

bool QgsRenderer::containsPixmap() const
{
  //default implementation returns true only for points
  switch ( mGeometryType )
  {
    case QGis::Point:
      return true;
    default:
      return false;
  }
}

void QgsRenderer::scaleBrush( QBrush& b, double rasterScaleFactor )
{
  if ( rasterScaleFactor != 1.0 )
  {
    QMatrix m;
    m.scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
    b.setMatrix( m );
  }
}
