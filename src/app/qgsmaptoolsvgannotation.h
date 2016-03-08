/***************************************************************************
                              qgsmaptoolsvgannotation.h
                              -------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSVGANNOTATION_H
#define QGSMAPTOOLSVGANNOTATION_H

#include "qgsmaptoolannotation.h"

class APP_EXPORT QgsMapToolSvgAnnotation: public QgsMapToolAnnotation
{
    Q_OBJECT

  public:
    QgsMapToolSvgAnnotation( QgsMapCanvas* canvas );
    ~QgsMapToolSvgAnnotation();
  protected:
    QgsAnnotationItem* createItem( QMouseEvent* e ) override;
};

#endif // QGSMAPTOOLSVGANNOTATION_H
