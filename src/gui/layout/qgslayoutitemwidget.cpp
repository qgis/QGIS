/***************************************************************************
                             qgslayoutitemwidget.cpp
                             ------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgslayout.h"


//
// QgsLayoutConfigObject
//

QgsLayoutConfigObject::QgsLayoutConfigObject( QWidget *parent, QgsLayoutObject *layoutObject )
  : QObject( parent )
  , mLayoutObject( layoutObject )
{
#if 0 //TODO
  connect( atlasComposition(), &QgsAtlasComposition::coverageLayerChanged,
           this, [ = ] { updateDataDefinedButtons(); } );
  connect( atlasComposition(), &QgsAtlasComposition::toggled, this, &QgsComposerConfigObject::updateDataDefinedButtons );
#endif
}

void QgsLayoutConfigObject::updateDataDefinedProperty()
{
  //match data defined button to item's data defined property
  QgsPropertyOverrideButton *ddButton = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  if ( !ddButton )
  {
    return;
  }
  QgsLayoutObject::DataDefinedProperty key = QgsLayoutObject::NoProperty;

  if ( ddButton->propertyKey() >= 0 )
    key = static_cast< QgsLayoutObject::DataDefinedProperty >( ddButton->propertyKey() );

  if ( key == QgsLayoutObject::NoProperty )
  {
    return;
  }

  //set the data defined property and refresh the item
  if ( mLayoutObject )
  {
    mLayoutObject->dataDefinedProperties().setProperty( key, ddButton->toProperty() );
    mLayoutObject->refresh();
  }
}

void QgsLayoutConfigObject::updateDataDefinedButtons()
{
#if 0 //TODO
  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    button->setVectorLayer( atlasCoverageLayer() );
  }
#endif
}

void QgsLayoutConfigObject::initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key )
{
  button->blockSignals( true );
  button->init( key, mLayoutObject->dataDefinedProperties(), QgsLayoutObject::propertyDefinitions(), coverageLayer() );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLayoutConfigObject::updateDataDefinedProperty );
  button->registerExpressionContextGenerator( mLayoutObject );
  button->blockSignals( false );
}

void QgsLayoutConfigObject::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  if ( !button )
    return;

  if ( button->propertyKey() < 0 || !mLayoutObject )
    return;

  QgsLayoutObject::DataDefinedProperty key = static_cast< QgsLayoutObject::DataDefinedProperty >( button->propertyKey() );
  whileBlocking( button )->setToProperty( mLayoutObject->dataDefinedProperties().property( key ) );
}

#if 0 // TODO
QgsAtlasComposition *QgsLayoutConfigObject::atlasComposition() const
{
  if ( !mLayoutObject )
  {
    return nullptr;
  }

  QgsComposition *composition = mComposerObject->composition();

  if ( !composition )
  {
    return nullptr;
  }

  return &composition->atlasComposition();
}
#endif

QgsVectorLayer *QgsLayoutConfigObject::coverageLayer() const
{
  if ( !mLayoutObject )
    return nullptr;

  QgsLayout *layout = mLayoutObject->layout();
  if ( !layout )
    return nullptr;

  return layout->context().layer();
}


//
// QgsLayoutItemBaseWidget
//

QgsLayoutItemBaseWidget::QgsLayoutItemBaseWidget( QWidget *parent, QgsLayoutObject *layoutObject )
  : QgsPanelWidget( parent )
  , mConfigObject( new QgsLayoutConfigObject( this, layoutObject ) )
{

}

void QgsLayoutItemBaseWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty property )
{
  mConfigObject->initializeDataDefinedButton( button, property );
}

void QgsLayoutItemBaseWidget::updateDataDefinedButton( QgsPropertyOverrideButton *button )
{
  mConfigObject->updateDataDefinedButton( button );
}

QgsVectorLayer *QgsLayoutItemBaseWidget::coverageLayer() const
{
  return mConfigObject->coverageLayer();
}

#if 0 //TODO
QgsAtlasComposition *QgsLayoutItemBaseWidget::atlasComposition() const
{
  return mConfigObject->atlasComposition();
}
#endif
