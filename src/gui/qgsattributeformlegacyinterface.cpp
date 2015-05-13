/***************************************************************************
    qgsattributeformlegacyinterface.cpp
     --------------------------------------
    Date                 : 13.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeformlegacyinterface.h"

#include "qgspythonrunner.h"
#include "qgsattributeform.h"

#include <QString>
#include <QDateTime>
#include <QRegExp>

QgsAttributeFormLegacyInterface::QgsAttributeFormLegacyInterface( const QString& function, const QString& pyFormName, QgsAttributeForm* form )
    : QgsAttributeFormInterface( form )
    , mPyFunctionName( function )
    , mPyFormVarName( pyFormName )
{
  static int sLayerCounter = 0;
  mPyLayerVarName = QString( "_qgis_layer_%1_%2" ).arg( form->layer()->id() ).arg( sLayerCounter++ );
  mPyLayerVarName.replace( QRegExp( "[^a-zA-Z0-9_]" ), "_" ); // clean identifier

  QString initLayer = QString( "%1 = sip.wrapinstance( %2, qgis.core.QgsVectorLayer )" )
                      .arg( mPyLayerVarName )
                      .arg(( unsigned long ) form->layer() );

  QgsPythonRunner::run( initLayer );
}

QgsAttributeFormLegacyInterface::~QgsAttributeFormLegacyInterface()
{
  QString delLayer = QString( "del %1" ).arg( mPyLayerVarName );
  QgsPythonRunner::run( delLayer );
}

void QgsAttributeFormLegacyInterface::featureChanged()
{
  QDialogButtonBox* buttonBox = form()->findChild<QDialogButtonBox*>();
  if ( buttonBox )
  {
    // If the init function did not call disconnect, we do it here before reconnecting
    // If it did call disconnect, then the call will just do nothing
    QObject::disconnect( buttonBox, SIGNAL( accepted() ), form(), SLOT( accept() ) );
    QObject::connect( buttonBox, SIGNAL( accepted() ), form(), SLOT( accept() ) );
  }

  // Generate the unique ID of this feature. We used to use feature ID but some providers
  // return a ID that is an invalid python variable when we have new unsaved features.
  QDateTime dt = QDateTime::currentDateTime();
  QString pyFeatureVarName = QString( "_qgis_feature_%1" ).arg( dt.toString( "yyyyMMddhhmmsszzz" ) );
  QString initFeature = QString( "%1 = sip.wrapinstance( %2, qgis.core.QgsFeature )" )
                        .arg( pyFeatureVarName )
                        .arg(( unsigned long ) & form()->feature() );

  QgsPythonRunner::run( initFeature );

  QString expr = QString( "%1( %2, %3, %4)" )
                 .arg( mPyFunctionName )
                 .arg( mPyFormVarName )
                 .arg( mPyLayerVarName )
                 .arg( pyFeatureVarName );

  QgsPythonRunner::run( expr );

  QString delFeature = QString( "del %1" ).arg( pyFeatureVarName );
  QgsPythonRunner::run( delFeature );
}
