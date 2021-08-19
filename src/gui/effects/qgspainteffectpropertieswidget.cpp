/***************************************************************************
    qgslayerpropertieswidget.cpp
    ----------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Arunmozhi
    email                : aruntheguy at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspainteffectpropertieswidget.h"

#include <QFile>
#include <QStandardItem>
#include <QKeyEvent>
#include <QMessageBox>

#include "qgspainteffectregistry.h"
#include "qgspainteffect.h"
#include "qgspainteffectwidget.h"
#include "qgseffectstack.h"
#include "qgsapplication.h"
#include "qgslogger.h"

static bool _initWidgetFunction( const QString &name, QgsPaintEffectWidgetFunc f )
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();

  QgsPaintEffectAbstractMetadata *abstractMetadata = registry->effectMetadata( name );
  if ( !abstractMetadata )
  {
    QgsDebugMsg( QStringLiteral( "Failed to find paint effect entry in registry: %1" ).arg( name ) );
    return false;
  }
  QgsPaintEffectMetadata *metadata = dynamic_cast<QgsPaintEffectMetadata *>( abstractMetadata );
  if ( !metadata )
  {
    QgsDebugMsg( QStringLiteral( "Failed to cast paint effect's metadata: " ) .arg( name ) );
    return false;
  }
  metadata->setWidgetFunction( f );
  return true;
}

static void _initWidgetFunctions()
{
  static bool sInitialized = false;
  if ( sInitialized )
    return;

  _initWidgetFunction( QStringLiteral( "blur" ), QgsBlurWidget::create );
  _initWidgetFunction( QStringLiteral( "dropShadow" ), QgsShadowEffectWidget::create );
  _initWidgetFunction( QStringLiteral( "innerShadow" ), QgsShadowEffectWidget::create );
  _initWidgetFunction( QStringLiteral( "drawSource" ), QgsDrawSourceWidget::create );
  _initWidgetFunction( QStringLiteral( "outerGlow" ), QgsGlowWidget::create );
  _initWidgetFunction( QStringLiteral( "innerGlow" ), QgsGlowWidget::create );
  _initWidgetFunction( QStringLiteral( "transform" ), QgsTransformWidget::create );
  _initWidgetFunction( QStringLiteral( "color" ), QgsColorEffectWidget::create );

  sInitialized = true;
}


QgsPaintEffectPropertiesWidget::QgsPaintEffectPropertiesWidget( QgsPaintEffect *effect, QWidget *parent )
  : QWidget( parent )
  , mEffect( effect )
{
  setupUi( this );
  _initWidgetFunctions();

  populateEffectTypes();
  // update effect type combo box
  if ( effect )
  {
    const int idx = mEffectTypeCombo->findData( effect->type() );
    mEffectTypeCombo->setCurrentIndex( idx );
  }
  // set the corresponding widget
  updateEffectWidget( effect );
  connect( mEffectTypeCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPaintEffectPropertiesWidget::effectTypeChanged );
}


void QgsPaintEffectPropertiesWidget::populateEffectTypes()
{
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  const QStringList types = registry->effects();

  const auto constTypes = types;
  for ( const QString &type : constTypes )
  {
    //don't show stack effect
    if ( type == QLatin1String( "effectStack" ) )
      continue;

    mEffectTypeCombo->addItem( registry->effectMetadata( type )->visibleName(), type );
  }
}

void QgsPaintEffectPropertiesWidget::updateEffectWidget( QgsPaintEffect *effect )
{
  if ( !effect )
  {
    stackedWidget->setCurrentWidget( pageDummy );
    return;
  }

  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    if ( QgsPaintEffectWidget *pew = qobject_cast< QgsPaintEffectWidget * >( stackedWidget->currentWidget() ) )
      disconnect( pew, &QgsPaintEffectWidget::changed, this, &QgsPaintEffectPropertiesWidget::emitSignalChanged );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QgsPaintEffectAbstractMetadata *am = registry->effectMetadata( effect->type() );
  if ( am )
  {
    QgsPaintEffectWidget *w = am->createWidget();
    if ( w )
    {
      w->setPaintEffect( effect );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, &QgsPaintEffectWidget::changed, this, &QgsPaintEffectPropertiesWidget::emitSignalChanged );
      return;
    }
  }
  // When anything is not right
  stackedWidget->setCurrentWidget( pageDummy );
}

void QgsPaintEffectPropertiesWidget::effectTypeChanged()
{
  QgsPaintEffect *effect = mEffect;
  if ( !effect )
    return;

  const QString newEffectType = mEffectTypeCombo->currentData().toString();
  if ( effect->type() == newEffectType )
    return;

  // get creation function for new effect from registry
  QgsPaintEffectRegistry *registry = QgsApplication::paintEffectRegistry();
  QgsPaintEffectAbstractMetadata *am = registry->effectMetadata( newEffectType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change effect to a new (with different type)
  // base new effect on existing effect's properties
  QgsPaintEffect *newEffect = am->createPaintEffect( effect->properties() );
  if ( !newEffect )
    return;

  updateEffectWidget( newEffect );
  emit changeEffect( newEffect );

  mEffect = newEffect;
}

void QgsPaintEffectPropertiesWidget::emitSignalChanged()
{
  emit changed();
}
