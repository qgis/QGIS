/***************************************************************************
    qgspointcloudrendererwidget.cpp
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgspointcloudrendererwidget.h"

QgsPointCloudRendererWidget::QgsPointCloudRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style )
  : mLayer( layer )
  , mStyle( style )
{
}

void QgsPointCloudRendererWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsPointCloudRendererWidget::context() const
{
  return mContext;
}

