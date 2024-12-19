/***************************************************************************
    qgsclassificationmethodregistry.h
    ---------------------
    begin                : September 2019
    copyright            : (C) 2019 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QIcon>

#include "qgsclassificationmethodregistry.h"

// classification methods
#include "qgsclassificationcustom.h"
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationquantile.h"
#include "qgsclassificationjenks.h"
#include "qgsclassificationstandarddeviation.h"
#include "qgsclassificationprettybreaks.h"
#include "qgsclassificationlogarithmic.h"
#include "qgsclassificationfixedinterval.h"

QgsClassificationMethodRegistry::QgsClassificationMethodRegistry()
{
  addMethod( new QgsClassificationEqualInterval() );
  addMethod( new QgsClassificationQuantile() );
  addMethod( new QgsClassificationJenks() );
  addMethod( new QgsClassificationStandardDeviation() );
  addMethod( new QgsClassificationPrettyBreaks() );
  addMethod( new QgsClassificationLogarithmic() );
  addMethod( new QgsClassificationFixedInterval() );
}

QgsClassificationMethodRegistry::~QgsClassificationMethodRegistry()
{
  qDeleteAll( mMethods );
}

bool QgsClassificationMethodRegistry::addMethod( QgsClassificationMethod *method )
{
  if ( mMethods.contains( method->id() ) )
    return false;

  mMethods.insert( method->id(), method );
  return true;
}

QgsClassificationMethod *QgsClassificationMethodRegistry::method( const QString &id )
{
  QgsClassificationMethod *method = mMethods.value( id, new QgsClassificationCustom() );
  return method->clone();
}

QMap<QString, QString> QgsClassificationMethodRegistry::methodNames() const
{
  QMap<QString, QString> methods;
  for ( const QgsClassificationMethod *method : std::as_const( mMethods ) )
    methods.insert( method->name(), method->id() );
  return methods;
}

QIcon QgsClassificationMethodRegistry::icon( const QString &id ) const
{
  QgsClassificationMethod *method = mMethods.value( id, nullptr );
  if ( method )
    return method->icon();
  else
    return QIcon();
}

