/***************************************************************************
                         qgsbasicrelationwidgetfactory.cpp
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

#include "qgsbasicrelationwidgetfactory.h"
#include "qgsbasicrelationconfigwidget.h"

QgsBasicRelationWidgetFactory::QgsBasicRelationWidgetFactory()
{

}

QString QgsBasicRelationWidgetFactory::type() const
{
  return QStringLiteral( "basic" );
}

QString QgsBasicRelationWidgetFactory::name() const
{
  return QStringLiteral( "Relation Editor" );
}

QgsRelationWidget *QgsBasicRelationWidgetFactory::create( const QVariantMap &config, QWidget *parent ) const
{
  return new QgsBasicRelationWidget( config, parent );
}


QgsRelationConfigWidget *QgsBasicRelationWidgetFactory::configWidget( const QgsRelation &relation, QWidget *parent ) const
{
  return static_cast<QgsRelationConfigWidget *>( new QgsBasicRelationConfigWidget( relation, parent ) );
}

