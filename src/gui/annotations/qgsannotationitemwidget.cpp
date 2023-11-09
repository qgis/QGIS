/***************************************************************************
                             qgsannotationitemwidget.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationitemwidget.h"

QgsAnnotationItemBaseWidget::QgsAnnotationItemBaseWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{

}

bool QgsAnnotationItemBaseWidget::setItem( QgsAnnotationItem *item )
{
  return setNewItem( item );
}

void QgsAnnotationItemBaseWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsAnnotationItemBaseWidget::context() const
{
  return mContext;
}

void QgsAnnotationItemBaseWidget::focusDefaultWidget()
{
}

bool QgsAnnotationItemBaseWidget::setNewItem( QgsAnnotationItem * )
{
  return false;
}
