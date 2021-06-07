/***************************************************************************
                              qgsmaptoolsvgannotation.cpp
                              ---------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsvgannotation.h"
#include "qgssvgannotation.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolSvgAnnotation::QgsMapToolSvgAnnotation( QgsMapCanvas *canvas ): QgsMapToolAnnotation( canvas )
{

}

QgsAnnotation *QgsMapToolSvgAnnotation::createItem() const
{
  return new QgsSvgAnnotation();
}
