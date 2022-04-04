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
#include "qgssettingsregistrycore.h"

#include <QPushButton>

typedef QHash<QgsVectorLayer *, QgsAttributeMap> CachedAttributesHash;
Q_GLOBAL_STATIC( CachedAttributesHash, sLastUsedValues )


QgsFeatureAction::QgsFeatureAction( const QString &name, QgsFeature &f, QgsVectorLayer *layer, QUuid actionId, int defaultAttr, QObject *parent )
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

  QgsAttributeEditorContext context( QgisApp::instance()->createAttributeEditorContext() );

  QgsDistanceArea myDa;

  myDa.setSourceCrs( mLayer->crs(), QgsProject::instance()->transformContext() );
  myDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  context.setDistanceArea( myDa );
  context.setFormMode( QgsAttributeEditorContext::StandaloneDialog );

  QgsAttributeDialog *dialog = new QgsAttributeDialog( mLayer, f, cloneFeature, parentWidget(), true, context );
  dialog->setWindowFlags( dialog->windowFlags() | Qt::Tool );

  dialog->setObjectName( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( f->id() ) );

  const QList<QgsAction> actions = mLayer->actions()->actions( QStringLiteral( "Feature" ) );
  if ( !actions.isEmpty() )
  {
    dialog->setContextMenuPolicy( Qt::ActionsContextMenu );

    QAction *a = new QAction( tr( "Run Actions" ), dialog );
    a->setEnabled( false );
    dialog->addAction( a );

    for ( const QgsAction &action : actions )
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

  const QString name( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( mFeature->id() ) );

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

    const int rv = dialog->exec();
    mFeature->setAttributes( dialog->feature()->attributes() );
    return rv;
  }
  else
  {
    const QString name( QStringLiteral( "featureactiondlg:%1:%2" ).arg( mLayer->id() ).arg( mFeature->id() ) );

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

bool QgsFeatureAction::addFeature( const QgsAttributeMap &defaultAttributes, bool showModal, QgsExpressionContextScope *scope SIP_TRANSFER, bool hideParent )
{
  if ( !mLayer || !mLayer->isEditable() )
    return false;

  const bool reuseAllLastValues = QgsSettingsRegistryCore::settingsDigitizingReuseLastValues.value();
  QgsDebugMsgLevel( QStringLiteral( "reuseAllLastValues: %1" ).arg( reuseAllLastValues ), 2 );

  const QgsFields fields = mLayer->fields();
  QgsAttributeMap initialAttributeValues;

  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( defaultAttributes.contains( idx ) )
    {
      initialAttributeValues.insert( idx, defaultAttributes.value( idx ) );
    }
    else if ( ( reuseAllLastValues || mLayer->editFormConfig().reuseLastValue( idx ) ) && sLastUsedValues()->contains( mLayer ) && ( *sLastUsedValues() )[ mLayer ].contains( idx ) )
    {
      // Only set initial attribute value if it's different from the default clause or we may trigger
      // unique constraint checks for no reason, see https://github.com/qgis/QGIS/issues/42909
      if ( mLayer->dataProvider() && mLayer->dataProvider()->defaultValueClause( idx ) != ( *sLastUsedValues() )[ mLayer ][idx] )
        initialAttributeValues.insert( idx, ( *sLastUsedValues() )[ mLayer ][idx] );
    }
  }

  // create new feature template - this will initialize the attributes to valid values, handling default
  // values and field constraints
  QgsExpressionContext context = mLayer->createExpressionContext();
  if ( scope )
    context.appendScope( scope );

  const QgsFeature newFeature = QgsVectorLayerUtils::createFeature( mLayer, mFeature->geometry(), initialAttributeValues,
                                &context );
  *mFeature = newFeature;

  //show the dialog to enter attribute values
  //only show if enabled in settings
  bool isDisabledAttributeValuesDlg = QgsSettingsRegistryCore::settingsDigitizingDisableEnterAttributeValuesDialog.value();

  // override application-wide setting if layer is non-spatial -- BECAUSE it's bad UX if
  // it appears that nothing happens when you click the add row button for a non-spatial layer. Unlike
  // spatial layers, where you can SEE the newly created spatial object on the map, creating a new
  // feature in a non-spatial layer otherwise seems to have no result.
  if ( !mLayer->isSpatial() )
    isDisabledAttributeValuesDlg = false;

  // override application-wide setting if layer has no fields
  if ( fields.count() == 0 )
    isDisabledAttributeValuesDlg = true;

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

  // finally, if this action has specifically forced suppression of the form, that overrides everything
  if ( mForceSuppressFormPopup )
    isDisabledAttributeValuesDlg = true;

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
    emit addFeatureFinished();
  }
  else
  {
    QgsAttributeDialog *dialog = newDialog( false );
    // delete the dialog when it is closed
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->setMode( QgsAttributeEditorContext::AddFeatureMode );
    dialog->setEditCommandMessage( text() );
    if ( scope )
      dialog->setExtraContextScope( new QgsExpressionContextScope( *scope ) );

    connect( dialog->attributeForm(), &QgsAttributeForm::featureSaved, this, &QgsFeatureAction::onFeatureSaved );

    if ( !showModal )
    {
      setParent( dialog ); // keep dialog until the dialog is closed and destructed
      connect( dialog, &QgsAttributeDialog::finished, this, &QgsFeatureAction::addFeatureFinished );
      dialog->show();

      if ( hideParent )
      {
        connect( this, &QgsFeatureAction::addFeatureFinished, this, &QgsFeatureAction::unhideParentWidget );
        hideParentWidget();
      }
      mFeature = nullptr;
      return true;
    }

    dialog->exec();
    emit addFeatureFinished();
  }

  // Will be set in the onFeatureSaved SLOT
  return mFeatureSaved;
}

void QgsFeatureAction::setForceSuppressFormPopup( bool force )
{
  mForceSuppressFormPopup = force;
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

  const QgsSettings settings;

  const QgsFields fields = mLayer->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    const QgsAttributes newValues = feature.attributes();
    QgsAttributeMap origValues = ( *sLastUsedValues() )[ mLayer ];
    if ( origValues[idx] != newValues.at( idx ) )
    {
      QgsDebugMsg( QStringLiteral( "saving %1 for %2" ).arg( ( *sLastUsedValues() )[ mLayer ][idx].toString() ).arg( idx ) );
      ( *sLastUsedValues() )[ mLayer ][idx] = newValues.at( idx );
    }
  }
}

void QgsFeatureAction::hideParentWidget()
{
  QWidget *dialog = parentWidget();
  if ( dialog )
  {
    QWidget *triggerWidget = dialog->parentWidget();
    if ( triggerWidget && triggerWidget->window()->objectName() != QStringLiteral( "QgisApp" ) )
      triggerWidget->window()->setVisible( false );
  }
}

void QgsFeatureAction::unhideParentWidget()
{
  QWidget *dialog = parentWidget();
  if ( dialog )
  {
    QWidget *triggerWidget = dialog->parentWidget();
    if ( triggerWidget )
      triggerWidget->window()->setVisible( true );
  }
}

QgsFeature QgsFeatureAction::feature() const
{
  return mFeature ? *mFeature : QgsFeature();
}
