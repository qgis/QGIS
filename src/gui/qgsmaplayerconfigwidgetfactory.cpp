/***************************************************************************
    qgsmaplayerconfigwidgetfactoryfactory.cpp
     ----------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerconfigwidgetfactory.h"

QgsMapLayerConfigWidgetFactory::QgsMapLayerConfigWidgetFactory( const QString &title, const QIcon &icon )
  : mIcon( icon )
  , mTitle( title )
{
}

QString QgsMapLayerConfigWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QString();
}

bool QgsMapLayerConfigWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return true;
}
