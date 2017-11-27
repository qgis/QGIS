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

#include "qgsmaptoolformannotation.h"
#include "qgsformannotation.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolFormAnnotation::QgsMapToolFormAnnotation( QgsMapCanvas *canvas ): QgsMapToolAnnotation( canvas )
{

}

QgsAnnotation *QgsMapToolFormAnnotation::createItem() const
{
  return new QgsFormAnnotation();
}

