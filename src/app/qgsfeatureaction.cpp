/***************************************************************************
                     qgsfeatureaction.cpp  -  description
                              -------------------
      begin                : 2010-09-20
      copyright            : (C) 2010 by Juergen E. Fischer
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

#include "qgisapp.h"
#include "qgsattributedialog.h"
#include "qgsdistancearea.h"
#include "qgsfeatureaction.h"
#include "qgsguivectorlayertools.h"
#include "qgsidentifyresultsdialog.h"
#include "qgslogger.h"
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsactionmanager.h"
#include "qgsaction.h"
#include "qgsvectorlayerutils.h"
#include "qgssettings.h"

#include <QPushButton>

QgsFeatureAction::QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *layer, const QUuid &actionId, int defaultAttr, QObject *parent )
  : QAction( name, parent )
  , mLayer( layer )
  , mFeature( &f )
  , mActionId( actionId )
  , mIdx( defaultAttr )
  , mFeatureSaved( false )
{
}

void QgsFeatureAction::execute()
{
  mLayer->actions()->doAction( mActionId, *mFeature, mIdx );
}

QgsAttributeDialog *QgsFeatureAction::newDialog( bool cloneFeature )
{
  QgsFeature *f = cloneFeature ? new QgsFeature( *mFeature ) : mFeature;

  QgsAttributeEditorContext context;

  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  context.setDistanceArea( myDa );
  context.setVectorLayerTools( QgisApp::instance()->vectorLayerTools() );
  context.setMapCanvas( QgisApp::instance()->mapCanvas() );
  context.setFormMode( QgsAttributeEditorContext::StandaloneDialog );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( mLayer, f, cloneFeature, parentWidget(), true, context );
  dialog->setWindowFlags( dialog->windowFlags() | Qt::Tool );
  dialog->setObjectName( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( f->id() ) );

  QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Feature" ) );
  if ( !actions.isEmpty() )
  {
    dialog->setContextMenuPolicy( Qt::ActionsContextMenu );

    QAction *a = new QAction( tr( "Run Actions" ), dialog );
    a->setEnabled( false );
    dialog->addAction( a );

    Q_FOREACH ( const QgsAction &action, actions )
    {
      if ( !action.runable() )
        continue;

      if ( !mLayer->isEditable() && action.isEnabledOnlyWhenEditable() )
        continue;

      QgsFeature &feat = const_cast<QgsFeature &>( *dialog->feature() );
      QgsFeatureAction *a = new QgsFeatureAction( action.name(), feat, mLayer, action.id(), -1, dialog );
      dialog->addAction( a );
      connect( a, &QAction::triggered, a, &QgsFeatureAction::execute );

      QAbstractButton *pb = dialog->findChild<QAbstractButton *>( action.name() );
      if ( pb )
        connect( pb, &QAbstractButton::clicked, a, &QgsFeatureAction::execute );
    }
  }

  return dialog;
}

bool QgsFeatureAction::viewFeatureForm( QgsHighlight *h )
{
  if ( !mLayer || !mFeature )
    return false;

  QString name( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( mFeature->id() ) );

  QgsAttributeDialog *dialog = QgisApp::instance()->findChild<QgsAttributeDialog *>( name );
  if ( dialog )
  {
    delete h;
    dialog->raise();
    dialog->activateWindow();
    return true;
  }

  dialog = newDialog( true );
  dialog->setHighlight( h );
  // delete the dialog when it is closed
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();

  return true;
}

bool QgsFeatureAction::editFeature( bool showModal )
{
  if ( !mLayer )
    return false;

  if ( showModal )
  {
    std::unique_ptr<QgsAttributeDialog> dialog( newDialog( false ) );

    if ( !mFeature->isValid() )
      dialog->setMode( QgsAttributeEditorContext::AddFeatureMode );

    int rv = dialog->exec();
    mFeature->setAttributes( dialog->feature()->attributes() );
    return rv;
  }
  else
  {
    QString name( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( mFeature->id() ) );

    QgsAttributeDialog *dialog = QgisApp::instance()->findChild<QgsAttributeDialog *>( name );
    if ( dialog )
    {
      dialog->raise();
      return true;
    }

    dialog = newDialog( false );

    if ( !mFeature->isValid() )
      dialog->setMode( QgsAttributeEditorContext::AddFeatureMode );

    // delete the dialog when it is closed
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->show();
  }

  return true;
}

bool QgsFeatureAction::addFeature( const QgsAttributeMap &defaultAttributes, bool showModal, QgsExpressionContextScope *scope SIP_TRANSFER )
{
  if ( !mLayer || !mLayer->isEditable() )
    return false;

  QgsSettings settings;
  bool reuseLastValues = settings.value( QStringLiteral( "qgis/digitizing/reuseLastValues" ), false ).toBool();
  QgsDebugMsg( QString( "reuseLastValues: %1" ).arg( reuseLastValues ) );

  QgsFields fields = mLayer->fields();
  QgsAttributeMap initialAttributeValues;

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( defaultAttributes.contains( idx ) )
    {
      initialAttributeValues.insert( idx, defaultAttributes.value( idx ) );
    }
    else if ( reuseLastValues && sLastUsedValues.contains( mLayer ) && sLastUsedValues[ mLayer ].contains( idx ) )
    {
      initialAttributeValues.insert( idx, sLastUsedValues[ mLayer ][idx] );
    }
  }

  // create new feature template - this will initialize the attributes to valid values, handling default
  // values and field constraints
  QgsExpressionContext context = mLayer->createExpressionContext();
  if ( scope )
    context.appendScope( scope );

  QgsFeature newFeature = QgsVectorLayerUtils::createFeature( mLayer, mFeature->geometry(), initialAttributeValues,
                          &context );
  *mFeature = newFeature;

  //show the dialog to enter attribute values
  //only show if enabled in settings and layer has fields
  bool isDisabledAttributeValuesDlg = ( fields.count() == 0 ) || settings.value( QStringLiteral( "qgis/digitizing/disable_enter_attribute_values_dialog" ), false ).toBool();

  // override application-wide setting with any layer setting
  switch ( mLayer->editFormConfig().suppress() )
  {
    case QgsEditFormConfig::SuppressOn:
      isDisabledAttributeValuesDlg = true;
      break;
    case QgsEditFormConfig::SuppressOff:
      isDisabledAttributeValuesDlg = false;
      break;
    case QgsEditFormConfig::SuppressDefault:
      break;
  }
  if ( isDisabledAttributeValuesDlg )
  {
    mLayer->beginEditCommand( text() );
    mFeatureSaved = mLayer->addFeature( *mFeature );

    if ( mFeatureSaved )
    {
      mLayer->endEditCommand();
      mLayer->triggerRepaint();
    }
    else
    {
      mLayer->destroyEditCommand();
    }
  }
  else
  {
    QgsAttributeDialog *dialog = newDialog( false );
    // delete the dialog when it is closed
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setMode( QgsAttributeEditorContext::AddFeatureMode );
    dialog->setEditCommandMessage( text() );

    connect( dialog->attributeForm(), &QgsAttributeForm::featureSaved, this, &QgsFeatureAction::onFeatureSaved );

    if ( !showModal )
    {
      setParent( dialog ); // keep dialog until the dialog is closed and destructed
      dialog->show();
      mFeature = nullptr;
      return true;
    }

    dialog->exec();
  }

  // Will be set in the onFeatureSaved SLOT
  return mFeatureSaved;
}

void QgsFeatureAction::onFeatureSaved( const QgsFeature &feature )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( sender() );
  Q_UNUSED( form ) // only used for Q_ASSERT
  Q_ASSERT( form );

  // Assign provider generated values
  if ( mFeature )
    *mFeature = feature;

  mFeatureSaved = true;

  QgsSettings settings;
  bool reuseLastValues = settings.value( QStringLiteral( "qgis/digitizing/reuseLastValues" ), false ).toBool();
  QgsDebugMsg( QString( "reuseLastValues: %1" ).arg( reuseLastValues ) );

  if ( reuseLastValues )
  {
    QgsFields fields = mLayer->fields();
    for ( int idx = 0; idx < fields.count(); ++idx )
    {
      QgsAttributes newValues = feature.attributes();
      QgsAttributeMap origValues = sLastUsedValues[ mLayer ];
      if ( origValues[idx] != newValues.at( idx ) )
      {
        QgsDebugMsg( QString( "saving %1 for %2" ).arg( sLastUsedValues[ mLayer ][idx].toString() ).arg( idx ) );
        sLastUsedValues[ mLayer ][idx] = newValues.at( idx );
      }
    }
  }
}

QHash<QgsVectorLayer *, QgsAttributeMap> QgsFeatureAction::sLastUsedValues;
