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

#include "qgsmaptoolhtmlannotation.h"
#include "qgshtmlannotation.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolHtmlAnnotation::QgsMapToolHtmlAnnotation( QgsMapCanvas *canvas ): QgsMapToolAnnotation( canvas )
{

}

QgsAnnotation *QgsMapToolHtmlAnnotation::createItem() const
{
  return new QgsHtmlAnnotation();
}

