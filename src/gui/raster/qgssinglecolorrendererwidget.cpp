/***************************************************************************
                         qgssinglecolorrendererwidget.cpp
                         ---------------------------------
    begin                : April 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssinglecolorrendererwidget.h"
#include "qgssinglecolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

QgsSingleColorRendererWidget::QgsSingleColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    connect( mColor, &QgsColorButton::colorChanged, this, &QgsSingleColorRendererWidget::colorChanged );

    setFromRenderer( layer->renderer() );
  }
}

QgsRasterRenderer *QgsSingleColorRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }

  QgsSingleColorRenderer *renderer = new QgsSingleColorRenderer( provider, mColor->color() );
  return renderer;
}

void QgsSingleColorRendererWidget::colorChanged( const QColor & )
{
  emit widgetChanged();
}

void QgsSingleColorRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsSingleColorRenderer *scr = dynamic_cast<const QgsSingleColorRenderer *>( r );
  if ( scr )
  {
    mColor->setColor( scr->color() );
  }
  else
  {
    mColor->setColor( QColor( 0, 0, 0 ) );
  }
}
