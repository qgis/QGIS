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
/* $Id$ */

#include "qgsfeatureaction.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsidentifyresults.h"
#include "qgsattributedialog.h"
#include "qgslogger.h"

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
  mLayer->actions()->doAction( mAction, mFeature.attributeMap(), mIdx );
}

QgsAttributeDialog *QgsFeatureAction::newDialog( bool cloneFeature )
{
  QgsFeature *f = cloneFeature ? new QgsFeature( mFeature ) : &mFeature;
  QgsAttributeDialog *dialog = new QgsAttributeDialog( mLayer, f, cloneFeature );

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
    QgsAttributeMap src = mFeature.attributeMap();

    if ( dialog->exec() )
    {
      mLayer->beginEditCommand( text() );

      const QgsAttributeMap &dst = mFeature.attributeMap();
      for ( QgsAttributeMap::const_iterator it = dst.begin(); it != dst.end(); it++ )
      {
        if ( !src.contains( it.key() ) || it.value() != src[it.key()] )
        {
          mLayer->changeAttributeValue( mFeature.id(), it.key(), it.value() );
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
  const QgsFieldMap fields = mLayer->pendingFields();
  for ( QgsFieldMap::const_iterator it = fields.constBegin(); it != fields.constEnd(); ++it )
  {
    if ( reuseLastValues && mLastUsedValues.contains( mLayer ) && mLastUsedValues[ mLayer ].contains( it.key() ) )
    {
      QgsDebugMsg( QString( "reusing %1 for %2" ).arg( mLastUsedValues[ mLayer ][ it.key()].toString() ).arg( it.key() ) );
      mFeature.addAttribute( it.key(), mLastUsedValues[ mLayer ][ it.key()] );
    }
    else
    {
      mFeature.addAttribute( it.key(), provider->defaultValue( it.key() ) );
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
    QgsAttributeMap origValues;
    if ( reuseLastValues )
      origValues = mFeature.attributeMap();

    QgsAttributeDialog *dialog = newDialog( false );
    if ( dialog->exec() )
    {
      if ( reuseLastValues )
      {
        for ( QgsFieldMap::const_iterator it = fields.constBegin(); it != fields.constEnd(); ++it )
        {
          const QgsAttributeMap &newValues = mFeature.attributeMap();
          if ( newValues.contains( it.key() )
               && origValues.contains( it.key() )
               && origValues[ it.key()] != newValues[ it.key()] )
          {
            QgsDebugMsg( QString( "saving %1 for %2" ).arg( mLastUsedValues[ mLayer ][ it.key()].toString() ).arg( it.key() ) );
            mLastUsedValues[ mLayer ][ it.key()] = newValues[ it.key()];
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
