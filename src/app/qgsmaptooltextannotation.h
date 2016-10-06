/***************************************************************************
                              qgsmaptooltextannotation.h
                              -------------------------
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

#ifndef QGSMAPTOOLTEXTANNOTATION_H
#define QGSMAPTOOLTEXTANNOTATION_H

#include "qgsmaptoolannotation.h"

class APP_EXPORT QgsMapToolTextAnnotation: public QgsMapToolAnnotation
{
    Q_OBJECT

  public:
    QgsMapToolTextAnnotation( QgsMapCanvas* canvas );
    ~QgsMapToolTextAnnotation();

  protected:
    QgsAnnotationItem* createItem( QMouseEvent* e ) override;
};

#endif // QGSMAPTOOLTEXTANNOTATION_H
