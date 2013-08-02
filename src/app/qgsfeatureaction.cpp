/***************************************************************************
                     qgsfeatureaction.cpp  -  description
                              -------------------
      begin                : 2010-09-20
      copyright            : (C) 2010 by Jürgen E. Fischer
      email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeatureaction.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsattributedialog.h"
#include "qgslogger.h"
#include "qgsdistancearea.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"

#include <QPushButton>
#include <QSettings>

QgsFeatureAction::QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *layer, int action, int defaultAttr, QObject *parent )
    : QAction( name, parent )
    , mLayer( layer )
    , mFeature( f )
    , mAction( action )
    , mIdx( defaultAttr )
{
}

void QgsFeatureAction::execute()
{
  mLayer->actions()->doAction( mAction, mFeature, mIdx );
}

QgsAttributeDialog *QgsFeatureAction::newDialog( bool cloneFeature )
{
  QgsFeature *f = cloneFeature ? new QgsFeature( mFeature ) : &mFeature;

  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( mLayer, f, cloneFeature, myDa );

  if ( mLayer->actions()->size() > 0 )
  {
    dialog->dialog()->setContextMenuPolicy( Qt::ActionsContextMenu );

    QAction *a = new QAction( tr( "Run actions" ), dialog->dialog() );
    a->setEnabled( false );
    dialog->dialog()->addAction( a );

    for ( int i = 0; i < mLayer->actions()->size(); i++ )
    {
      const QgsAction &action = mLayer->actions()->at( i );

      if ( !action.runable() )
        continue;

      QgsFeatureAction *a = new QgsFeatureAction( action.name(), *f, mLayer, i, -1, dialog->dialog() );
      dialog->dialog()->addAction( a );
      connect( a, SIGNAL( triggered() ), a, SLOT( execute() ) );

      QAbstractButton *pb = dialog->dialog()->findChild<QAbstractButton *>( action.name() );
      if ( pb )
        connect( pb, SIGNAL( clicked() ), a, SLOT( execute() ) );
    }
  }

  return dialog;
}

bool QgsFeatureAction::viewFeatureForm( QgsHighlight *h )
{
  if ( !mLayer )
    return false;

  QgsAttributeDialog *dialog = newDialog( true );
  dialog->setHighlight( h );
  dialog->show();

  return true;
}

bool QgsFeatureAction::editFeature()
{
  bool res = false;

  if ( !mLayer )
    return res;

  QgsAttributeDialog *dialog = newDialog( false );

  if ( !mLayer->isEditable() )
  {
    res = dialog->exec();
  }
  else
  {
    QgsAttributes src = mFeature.attributes();

    if ( dialog->exec() )
    {
      mLayer->beginEditCommand( text() );

      const QgsAttributes &dst = mFeature.attributes();
      for ( int i = 0; i < dst.count(); ++i )
      {
        if ( dst[i] != src[i] )
        {
          mLayer->changeAttributeValue( mFeature.id(), i, dst[i] );
        }
      }

      mLayer->endEditCommand();
      res = true;
    }
    else
    {
      res = false;
    }
  }

  delete dialog;
  return res;
}

bool QgsFeatureAction::addFeature()
{
  if ( !mLayer || !mLayer->isEditable() )
    return false;

  QgsVectorDataProvider *provider = mLayer->dataProvider();

  QSettings settings;
  bool reuseLastValues = settings.value( "/qgis/digitizing/reuseLastValues", false ).toBool();
  QgsDebugMsg( QString( "reuseLastValues: %1" ).arg( reuseLastValues ) );

  // add the fields to the QgsFeature
  const QgsFields& fields = mLayer->pendingFields();
  mFeature.initAttributes( fields.count() );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( reuseLastValues && mLastUsedValues.contains( mLayer ) && mLastUsedValues[ mLayer ].contains( idx ) )
    {
      QgsDebugMsg( QString( "reusing %1 for %2" ).arg( mLastUsedValues[ mLayer ][idx].toString() ).arg( idx ) );
      mFeature.setAttribute( idx, mLastUsedValues[ mLayer ][idx] );
    }
    else
    {
      mFeature.setAttribute( idx, provider->defaultValue( idx ) );
    }
  }

  bool res = false;

  mLayer->beginEditCommand( text() );

  // show the dialog to enter attribute values
  bool isDisabledAttributeValuesDlg = settings.value( "/qgis/digitizing/disable_enter_attribute_values_dialog", false ).toBool();
  if ( isDisabledAttributeValuesDlg )
  {
    res = mLayer->addFeature( mFeature );
  }
  else
  {
    QgsAttributes origValues;
    if ( reuseLastValues )
      origValues = mFeature.attributes();

    QgsAttributeDialog *dialog = newDialog( false );
    if ( dialog->exec() )
    {
      if ( reuseLastValues )
      {
        for ( int idx = 0; idx < fields.count(); ++idx )
        {
          const QgsAttributes &newValues = mFeature.attributes();
          if ( origValues[idx] != newValues[idx] )
          {
            QgsDebugMsg( QString( "saving %1 for %2" ).arg( mLastUsedValues[ mLayer ][idx].toString() ).arg( idx ) );
            mLastUsedValues[ mLayer ][idx] = newValues[idx];
          }
        }
      }

      res = mLayer->addFeature( mFeature );
    }
    else
    {
      QgsDebugMsg( "Adding feature to layer failed" );
      res = false;
    }
  }

  if ( res )
    mLayer->endEditCommand();
  else
    mLayer->destroyEditCommand();

  return res;
}

QMap<QgsVectorLayer *, QgsAttributeMap> QgsFeatureAction::mLastUsedValues;
