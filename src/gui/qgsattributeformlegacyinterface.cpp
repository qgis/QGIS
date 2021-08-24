/***************************************************************************
    qgsattributeformlegacyinterface.cpp
     --------------------------------------
    Date                 : 13.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include <QRegularExpression>

QgsAttributeFormLegacyInterface::QgsAttributeFormLegacyInterface( const QString &function, const QString &pyFormName, QgsAttributeForm *form )
  : QgsAttributeFormInterface( form )
  , mPyFunctionName( function )
  , mPyFormVarName( pyFormName )
{
  static int sLayerCounter = 0;
  mPyLayerVarName = QStringLiteral( "_qgis_layer_%1_%2" ).arg( form->layer()->id() ).arg( sLayerCounter++ );
  const thread_local QRegularExpression reClean( QRegularExpression( "[^a-zA-Z0-9_]" ) );
  mPyLayerVarName.replace( reClean, QStringLiteral( "_" ) ); // clean identifier

  const QString initLayer = QStringLiteral( "%1 = sip.wrapinstance( %2, qgis.core.QgsVectorLayer )" )
                            .arg( mPyLayerVarName )
                            .arg( ( quint64 ) form->layer() );

  QgsPythonRunner::run( initLayer );
}

QgsAttributeFormLegacyInterface::~QgsAttributeFormLegacyInterface()
{
  const QString delLayer = QStringLiteral( "del %1" ).arg( mPyLayerVarName );
  QgsPythonRunner::run( delLayer );
}

void QgsAttributeFormLegacyInterface::featureChanged()
{
  QDialogButtonBox *buttonBox = form()->findChild<QDialogButtonBox *>();
  if ( buttonBox )
  {
    // If the init function did not call disconnect, we do it here before reconnecting
    // If it did call disconnect, then the call will just do nothing
    QObject::disconnect( buttonBox, &QDialogButtonBox::accepted, form(), &QgsAttributeForm::save );
    QObject::connect( buttonBox, &QDialogButtonBox::accepted, form(), &QgsAttributeForm::save );
  }

  // Generate the unique ID of this feature. We used to use feature ID but some providers
  // return a ID that is an invalid python variable when we have new unsaved features.
  const QDateTime dt = QDateTime::currentDateTime();
  const QString pyFeatureVarName = QStringLiteral( "_qgis_feature_%1" ).arg( dt.toString( QStringLiteral( "yyyyMMddhhmmsszzz" ) ) );
  const QString initFeature = QStringLiteral( "%1 = sip.wrapinstance( %2, qgis.core.QgsFeature )" )
                              .arg( pyFeatureVarName )
                              .arg( ( quint64 ) & form()->feature() );

  QgsPythonRunner::run( initFeature );

  const QString expr = QStringLiteral( "%1( %2, %3, %4)" )
                       .arg( mPyFunctionName,
                             mPyFormVarName,
                             mPyLayerVarName,
                             pyFeatureVarName );

  QgsPythonRunner::run( expr );

  const QString delFeature = QStringLiteral( "del %1" ).arg( pyFeatureVarName );
  QgsPythonRunner::run( delFeature );
}
