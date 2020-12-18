/***************************************************************************
                         qgsrelationconfigbasewidget.cpp
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationconfigwidget.h"
#include "qgsexpressioncontextutils.h"

QgsRelationConfigWidget::QgsRelationConfigWidget( const QgsRelation &relation, QWidget *parent )
  : QWidget( parent )
  , mRelation( relation )
{
}

QgsVectorLayer *QgsRelationConfigWidget::layer()
{
  return mLayer;
}

QgsRelation QgsRelationConfigWidget::relation() const
{
  return mRelation;
}


