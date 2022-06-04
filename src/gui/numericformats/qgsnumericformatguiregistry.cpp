/***************************************************************************
                         qgsnumericformatguiregistry.cpp
                         -------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnumericformatguiregistry.h"
#include "qgis.h"
#include "qgsnumericformatwidget.h"

///@cond PRIVATE
class QgsBasicNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsBasicNumericFormatWidget( format );
    }
};

class QgsBearingNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsBearingNumericFormatWidget( format );
    }
};

class QgsGeographicCoordinateNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsGeographicCoordinateNumericFormatWidget( format );
    }
};

class QgsCurrencyNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsCurrencyNumericFormatWidget( format );
    }
};

class QgsPercentageNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsPercentageNumericFormatWidget( format );
    }
};

class QgsScientificNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsScientificNumericFormatWidget( format );
    }
};

class QgsFractionNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:

    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const
    {
      return new QgsFractionNumericFormatWidget( format );
    }
};
///@endcond

QgsNumericFormatGuiRegistry::QgsNumericFormatGuiRegistry()
{
  addFormatConfigurationWidgetFactory( QStringLiteral( "basic" ), new QgsBasicNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "bearing" ), new QgsBearingNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "currency" ), new QgsCurrencyNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "percentage" ), new QgsPercentageNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "scientific" ), new QgsScientificNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "fraction" ), new QgsFractionNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( QStringLiteral( "geographiccoordinate" ), new QgsGeographicCoordinateNumericFormatConfigurationWidgetFactory() );
}

QgsNumericFormatGuiRegistry::~QgsNumericFormatGuiRegistry()
{
  qDeleteAll( mFormatConfigurationWidgetFactories );
}

void QgsNumericFormatGuiRegistry::addFormatConfigurationWidgetFactory( const QString &id, QgsNumericFormatConfigurationWidgetFactory *factory )
{
  mFormatConfigurationWidgetFactories.insert( id, factory );
}

void QgsNumericFormatGuiRegistry::removeFormatConfigurationWidgetFactory( const QString &id )
{
  delete mFormatConfigurationWidgetFactories.value( id );
  mFormatConfigurationWidgetFactories.remove( id );
}

QgsNumericFormatWidget *QgsNumericFormatGuiRegistry::formatConfigurationWidget( const QgsNumericFormat *format ) const
{
  if ( !format )
    return nullptr;

  if ( !mFormatConfigurationWidgetFactories.contains( format->id() ) )
    return nullptr;

  return mFormatConfigurationWidgetFactories.value( format->id() )->create( format );
}
