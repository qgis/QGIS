/***************************************************************************
                              qgsmaptoolformannotation.cpp
                              -------------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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

