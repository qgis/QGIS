/***************************************************************************
                              qgsmaptoolformannotation.cpp
                              -------------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolhtmlannotation.h"
#include "moc_qgsmaptoolhtmlannotation.cpp"
#include "qgshtmlannotation.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolHtmlAnnotation::QgsMapToolHtmlAnnotation( QgsMapCanvas *canvas )
  : QgsMapToolAnnotation( canvas )
{
}

QgsAnnotation *QgsMapToolHtmlAnnotation::createItem() const
{
  return new QgsHtmlAnnotation();
}
