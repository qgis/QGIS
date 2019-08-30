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


#include "qgsclassificationmethodregistry.h"

// classification methods
#include "qgsclassificationcustom.h"
#include "qgsclassificationequalinterval.h"
#include "qgsclassificationquantile.h"
#include "qgsclassificationjenks.h"
#include "qgsclassificationstandarddeviation.h"
#include "qgsclassificationprettybreaks.h"

QgsClassificationMethodRegistry::QgsClassificationMethodRegistry()
{
  addMethod( new QgsClassificationEqualInterval() );
  addMethod( new QgsClassificationQuantile() );
  addMethod( new QgsClassificationJenks() );
  addMethod( new QgsClassificationStandardDeviation() );
  addMethod( new QgsClassificationPrettyBreaks() );
}

void QgsClassificationMethodRegistry::addMethod( QgsClassificationMethod *method )
{
  mMethods.insert( method->id(), method );
}

QgsClassificationMethod *QgsClassificationMethodRegistry::method( const QString &id )
{
  QgsClassificationMethod *method = mMethods.value( id, new QgsClassificationCustom() );
  return method->clone();
}

QMap<QString, QString> QgsClassificationMethodRegistry::methodNames() const
{
  QMap<QString, QString> methods;
  for ( const QgsClassificationMethod *method : qgis::as_const( mMethods ) )
    methods.insert( method->id(), method->name() );
  return methods;
}

