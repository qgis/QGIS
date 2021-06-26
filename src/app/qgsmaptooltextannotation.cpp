/***************************************************************************
                              qgsmaptooltextannotation.cpp
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

#include "qgsmaptooltextannotation.h"
#include "qgstextannotation.h"
#include "qgsproject.h"
#include <QMouseEvent>

QgsMapToolTextAnnotation::QgsMapToolTextAnnotation( QgsMapCanvas *canvas ): QgsMapToolAnnotation( canvas )
{

}

QgsAnnotation *QgsMapToolTextAnnotation::createItem() const
{
  return new QgsTextAnnotation();
}

