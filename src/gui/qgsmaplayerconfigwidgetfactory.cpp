/***************************************************************************
    qgsmaplayerconfigwidgetfactoryfactory.cpp
     ----------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerconfigwidgetfactory.h"

QgsMapLayerConfigWidgetFactory::QgsMapLayerConfigWidgetFactory( const QString &title, const QIcon &icon )
  : mIcon( icon )
  , mTitle( title )
{
}

bool QgsMapLayerConfigWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return true;
}
