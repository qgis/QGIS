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
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsBasicNumericFormatWidget( format );
    }
};

class QgsBearingNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsBearingNumericFormatWidget( format );
    }
};

class QgsGeographicCoordinateNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsGeographicCoordinateNumericFormatWidget( format );
    }
};

class QgsCurrencyNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsCurrencyNumericFormatWidget( format );
    }
};

class QgsPercentageNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsPercentageNumericFormatWidget( format );
    }
};

class QgsScientificNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsScientificNumericFormatWidget( format );
    }
};

class QgsFractionNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsFractionNumericFormatWidget( format );
    }
};

class QgsExpressionBasedNumericFormatConfigurationWidgetFactory : public QgsNumericFormatConfigurationWidgetFactory
{
  public:
    QgsNumericFormatWidget *create( const QgsNumericFormat *format ) const override
    {
      return new QgsExpressionBasedNumericFormatWidget( format );
    }
};

///@endcond

QgsNumericFormatGuiRegistry::QgsNumericFormatGuiRegistry()
{
  addFormatConfigurationWidgetFactory( u"basic"_s, new QgsBasicNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"bearing"_s, new QgsBearingNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"currency"_s, new QgsCurrencyNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"percentage"_s, new QgsPercentageNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"scientific"_s, new QgsScientificNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"fraction"_s, new QgsFractionNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"geographiccoordinate"_s, new QgsGeographicCoordinateNumericFormatConfigurationWidgetFactory() );
  addFormatConfigurationWidgetFactory( u"expression"_s, new QgsExpressionBasedNumericFormatConfigurationWidgetFactory() );
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

  auto it = mFormatConfigurationWidgetFactories.constFind( format->id() );
  if ( it == mFormatConfigurationWidgetFactories.constEnd() )
    return nullptr;

  return it.value()->create( format );
}
