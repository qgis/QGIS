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

static bool _initWidgetFunction( const QString& name, QgsPaintEffectWidgetFunc f )
{
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();

  QgsPaintEffectAbstractMetadata* abstractMetadata = registry->effectMetadata( name );
  if ( !abstractMetadata )
  {
    QgsDebugMsg( QString( "Failed to find paint effect entry in registry: %1" ).arg( name ) );
    return false;
  }
  QgsPaintEffectMetadata* metadata = dynamic_cast<QgsPaintEffectMetadata*>( abstractMetadata );
  if ( !metadata )
  {
    QgsDebugMsg( QString( "Failed to cast paint effect's metadata: " ) .arg( name ) );
    return false;
  }
  metadata->setWidgetFunction( f );
  return true;
}

static void _initWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  _initWidgetFunction( "blur", QgsBlurWidget::create );
  _initWidgetFunction( "dropShadow", QgsShadowEffectWidget::create );
  _initWidgetFunction( "innerShadow", QgsShadowEffectWidget::create );
  _initWidgetFunction( "drawSource", QgsDrawSourceWidget::create );
  _initWidgetFunction( "outerGlow", QgsGlowWidget::create );
  _initWidgetFunction( "innerGlow", QgsGlowWidget::create );
  _initWidgetFunction( "transform", QgsTransformWidget::create );
  _initWidgetFunction( "color", QgsColorEffectWidget::create );

  initialized = true;
}


QgsPaintEffectPropertiesWidget::QgsPaintEffectPropertiesWidget( QgsPaintEffect* effect, QWidget *parent )
    : QWidget( parent )
    , mEffect( effect )
{
  setupUi( this );
  _initWidgetFunctions();

  populateEffectTypes();
  // update effect type combo box
  if ( effect )
  {
    int idx = mEffectTypeCombo->findData( effect->type() );
    mEffectTypeCombo->setCurrentIndex( idx );
  }
  // set the corresponding widget
  updateEffectWidget( effect );
  connect( mEffectTypeCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( effectTypeChanged() ) );
}


void QgsPaintEffectPropertiesWidget::populateEffectTypes()
{
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QStringList types = registry->effects();

  Q_FOREACH ( const QString& type, types )
  {
    //don't show stack effect
    if ( type == "effectStack" )
      continue;

    mEffectTypeCombo->addItem( registry->effectMetadata( type )->visibleName(), type );
  }
}

void QgsPaintEffectPropertiesWidget::updateEffectWidget( QgsPaintEffect* effect )
{
  if ( !effect )
  {
    stackedWidget->setCurrentWidget( pageDummy );
    return;
  }

  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    disconnect( stackedWidget->currentWidget(), SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QgsPaintEffectAbstractMetadata* am = registry->effectMetadata( effect->type() );
  if ( am )
  {
    QgsPaintEffectWidget* w = am->createWidget();
    if ( w )
    {
      w->setPaintEffect( effect );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start receiving updates from widget
      connect( w, SIGNAL( changed() ), this, SLOT( emitSignalChanged() ) );
      return;
    }
  }
  // When anything is not right
  stackedWidget->setCurrentWidget( pageDummy );
}

void QgsPaintEffectPropertiesWidget::effectTypeChanged()
{
  QgsPaintEffect* effect = mEffect;
  if ( !effect )
    return;

  QString newEffectType = mEffectTypeCombo->itemData( mEffectTypeCombo->currentIndex() ).toString();
  if ( effect->type() == newEffectType )
    return;

  // get creation function for new effect from registry
  QgsPaintEffectRegistry* registry = QgsPaintEffectRegistry::instance();
  QgsPaintEffectAbstractMetadata* am = registry->effectMetadata( newEffectType );
  if ( !am ) // check whether the metadata is assigned
    return;

  // change effect to a new (with different type)
  // base new effect on existing effect's properties
  QgsPaintEffect* newEffect = am->createPaintEffect( effect->properties() );
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
