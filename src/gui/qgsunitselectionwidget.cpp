/***************************************************************************
    qgsunitselectionwidget.h
    -------------------
    begin                : Mar 24, 2014
    copyright            : (C) 2014 Sandro Mani
    email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsunitselectionwidget.h"
#include <QDialogButtonBox>

QgsMapUnitScaleWidget::QgsMapUnitScaleWidget( QWidget* parent )
    : QgsPanelWidget( parent )
    , mBlockSignals( true )
{
  setupUi( this );
  mComboBoxMinScale->setScale( 0.0000001 );
  mComboBoxMaxScale->setScale( 1 );
  mSpinBoxMinSize->setShowClearButton( false );
  mSpinBoxMaxSize->setShowClearButton( false );
  connect( mCheckBoxMinScale, SIGNAL( toggled( bool ) ), this, SLOT( configureMinComboBox() ) );
  connect( mCheckBoxMaxScale, SIGNAL( toggled( bool ) ), this, SLOT( configureMaxComboBox() ) );
  connect( mComboBoxMinScale, SIGNAL( scaleChanged( double ) ), this, SLOT( configureMaxComboBox() ) );
  connect( mComboBoxMinScale, SIGNAL( scaleChanged( double ) ), mComboBoxMaxScale, SLOT( setMinScale( double ) ) );
  connect( mComboBoxMaxScale, SIGNAL( scaleChanged( double ) ), this, SLOT( configureMinComboBox() ) );

  connect( mCheckBoxMinSize, SIGNAL( toggled( bool ) ), mSpinBoxMinSize, SLOT( setEnabled( bool ) ) );
  connect( mCheckBoxMaxSize, SIGNAL( toggled( bool ) ), mSpinBoxMaxSize, SLOT( setEnabled( bool ) ) );

  // notification of setting changes
  connect( mCheckBoxMinScale, SIGNAL( toggled( bool ) ), this, SLOT( settingsChanged() ) );
  connect( mCheckBoxMaxScale, SIGNAL( toggled( bool ) ), this, SLOT( settingsChanged() ) );
  connect( mComboBoxMinScale, SIGNAL( scaleChanged( double ) ), this, SLOT( settingsChanged() ) );
  connect( mComboBoxMaxScale, SIGNAL( scaleChanged( double ) ), this, SLOT( settingsChanged() ) );
  connect( mCheckBoxMinSize, SIGNAL( toggled( bool ) ), this, SLOT( settingsChanged() ) );
  connect( mCheckBoxMaxSize, SIGNAL( toggled( bool ) ), this, SLOT( settingsChanged() ) );
  connect( mSpinBoxMinSize, SIGNAL( valueChanged( double ) ), this, SLOT( settingsChanged() ) );
  connect( mSpinBoxMaxSize, SIGNAL( valueChanged( double ) ), this, SLOT( settingsChanged() ) );
  mBlockSignals = false;
}

void QgsMapUnitScaleWidget::setMapUnitScale( const QgsMapUnitScale &scale )
{
  // can't block signals on the widgets themselves, some use them to update
  // internal states
  mBlockSignals = true;
  mComboBoxMinScale->setScale( scale.minScale > 0.0 ? scale.minScale : 0.0000001 );
  mCheckBoxMinScale->setChecked( scale.minScale > 0.0 );
  mComboBoxMinScale->setEnabled( scale.minScale > 0.0 );
  mComboBoxMaxScale->setScale( scale.maxScale > 0.0 ? scale.maxScale : 1.0 );
  mCheckBoxMaxScale->setChecked( scale.maxScale > 0.0 );
  mComboBoxMaxScale->setEnabled( scale.maxScale > 0.0 );

  mCheckBoxMinSize->setChecked( scale.minSizeMMEnabled );
  mSpinBoxMinSize->setEnabled( scale.minSizeMMEnabled );
  mSpinBoxMinSize->setValue( scale.minSizeMM );

  mCheckBoxMaxSize->setChecked( scale.maxSizeMMEnabled );
  mSpinBoxMaxSize->setEnabled( scale.maxSizeMMEnabled );
  mSpinBoxMaxSize->setValue( scale.maxSizeMM );
  mBlockSignals = false;

  settingsChanged();
}

void QgsMapUnitScaleWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mComboBoxMinScale->setMapCanvas( canvas );
  mComboBoxMinScale->setShowCurrentScaleButton( true );
  mComboBoxMaxScale->setMapCanvas( canvas );
  mComboBoxMaxScale->setShowCurrentScaleButton( true );
}

void QgsMapUnitScaleWidget::configureMinComboBox()
{
  mComboBoxMinScale->setEnabled( mCheckBoxMinScale->isChecked() );
  if ( mCheckBoxMinScale->isChecked() && mComboBoxMinScale->scale() > mComboBoxMaxScale->scale() )
  {
    mComboBoxMinScale->setScale( mComboBoxMaxScale->scale() );
  }
}

void QgsMapUnitScaleWidget::configureMaxComboBox()
{
  mComboBoxMaxScale->setEnabled( mCheckBoxMaxScale->isChecked() );
  if ( mCheckBoxMaxScale->isChecked() && mComboBoxMaxScale->scale() < mComboBoxMinScale->scale() )
  {
    mComboBoxMaxScale->setScale( mComboBoxMinScale->scale() );
  }
}

void QgsMapUnitScaleWidget::settingsChanged()
{
  if ( mBlockSignals )
    return;

  emit mapUnitScaleChanged( mapUnitScale() );
}

QgsMapUnitScale QgsMapUnitScaleWidget::mapUnitScale() const
{
  QgsMapUnitScale scale;
  scale.minScale = mCheckBoxMinScale->isChecked() ? mComboBoxMinScale->scale() : 0;
  scale.maxScale = mCheckBoxMaxScale->isChecked() ? mComboBoxMaxScale->scale() : 0;
  scale.minSizeMMEnabled = mCheckBoxMinSize->isChecked();
  scale.minSizeMM = mSpinBoxMinSize->value();
  scale.maxSizeMMEnabled = mCheckBoxMaxSize->isChecked();
  scale.maxSizeMM = mSpinBoxMaxSize->value();
  return scale;
}





QgsUnitSelectionWidget::QgsUnitSelectionWidget( QWidget *parent )
    : QWidget( parent )
    , mCanvas( nullptr )
{
  mMapUnitIdx = -1;

  setupUi( this );
  mMapScaleButton->setVisible( false );
  mMapScaleButton->setToolTip( tr( "Adjust scaling range" ) );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mUnitCombo );

  connect( mUnitCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( toggleUnitRangeButton() ) );
  connect( mMapScaleButton, SIGNAL( clicked() ), this, SLOT( showDialog() ) );
  connect( mUnitCombo, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( changed() ) );
}

void QgsUnitSelectionWidget::setUnits( const QStringList &units, int mapUnitIdx )
{
  blockSignals( true );
  mUnitCombo->addItems( units );
  mMapUnitIdx = mapUnitIdx;
  blockSignals( false );
}

void QgsUnitSelectionWidget::setUnits( const QgsUnitTypes::RenderUnitList &units )
{
  blockSignals( true );
  mUnitCombo->clear();

  //instead of iterating over the units list, we specifically check for presence of unit types
  //to ensure that the widget always keeps the same order for units, regardless of the
  //order specified in the units list
  mMapUnitIdx = -1;
  if ( units.contains( QgsUnitTypes::RenderMillimeters ) )
  {
    mUnitCombo->addItem( tr( "Millimeter" ), QgsUnitTypes::RenderMillimeters );
  }
  if ( units.contains( QgsUnitTypes::RenderPixels ) )
  {
    mUnitCombo->addItem( tr( "Pixels" ), QgsUnitTypes::RenderPixels );
  }
  if ( units.contains( QgsUnitTypes::RenderMapUnits ) )
  {
    mUnitCombo->addItem( tr( "Map unit" ), QgsUnitTypes::RenderMapUnits );
  }
  if ( units.contains( QgsUnitTypes::RenderPercentage ) )
  {
    mUnitCombo->addItem( tr( "Percentage" ), QgsUnitTypes::RenderPercentage );
  }
  blockSignals( false );
}

QgsUnitTypes::RenderUnit QgsUnitSelectionWidget::unit() const
{
  if ( mUnitCombo->count() == 0 )
    return QgsUnitTypes::RenderUnknownUnit;

  QVariant currentData = mUnitCombo->currentData();
  if ( currentData.isValid() )
  {
    return ( QgsUnitTypes::RenderUnit ) currentData.toInt();
  }
  //unknown
  return QgsUnitTypes::RenderUnknownUnit;
}

void QgsUnitSelectionWidget::setUnit( int unitIndex )
{
  blockSignals( true );
  mUnitCombo->setCurrentIndex( unitIndex );
  blockSignals( false );
}

void QgsUnitSelectionWidget::setUnit( QgsUnitTypes::RenderUnit unit )
{
  int idx = mUnitCombo->findData( QVariant(( int ) unit ) );
  mUnitCombo->setCurrentIndex( idx == -1 ? 0 : idx );
}

void QgsUnitSelectionWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

void QgsUnitSelectionWidget::showDialog()
{
  QgsPanelWidget* panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsMapUnitScaleWidget* widget = new QgsMapUnitScaleWidget( panel );
    widget->setPanelTitle( tr( "Adjust scaling range" ) );
    widget->setMapCanvas( mCanvas );
    widget->setMapUnitScale( mMapUnitScale );
    connect( widget, SIGNAL( mapUnitScaleChanged( QgsMapUnitScale ) ), this, SLOT( widgetChanged( QgsMapUnitScale ) ) );
    panel->openPanel( widget );
    return;
  }

  QgsMapUnitScaleDialog dlg( this );
  dlg.setMapUnitScale( mMapUnitScale );
  dlg.setMapCanvas( mCanvas );
  if ( dlg.exec() == QDialog::Accepted )
  {
    if ( mMapUnitScale != dlg.getMapUnitScale() )
    {
      mMapUnitScale = dlg.getMapUnitScale();
      emit changed();
    }
  }
}

void QgsUnitSelectionWidget::toggleUnitRangeButton()
{
  if ( unit() != QgsUnitTypes::RenderUnknownUnit )
  {
    mMapScaleButton->setVisible( unit() == QgsUnitTypes::RenderMapUnits );
  }
  else
  {
    mMapScaleButton->setVisible( mMapUnitIdx != -1 && mUnitCombo->currentIndex() == mMapUnitIdx );
  }
}

void QgsUnitSelectionWidget::widgetChanged( const QgsMapUnitScale& scale )
{
  mMapUnitScale = scale;
  emit changed();
}


QgsMapUnitScaleDialog::QgsMapUnitScaleDialog( QWidget* parent )
    : QDialog( parent )
    , mWidget( nullptr )
{
  QVBoxLayout* vLayout = new QVBoxLayout();
  mWidget = new QgsMapUnitScaleWidget();
  vLayout->addWidget( mWidget );
  QDialogButtonBox* bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( bbox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
}

QgsMapUnitScale QgsMapUnitScaleDialog::getMapUnitScale() const
{
  return mWidget->mapUnitScale();
}

void QgsMapUnitScaleDialog::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mWidget->setMapUnitScale( scale );
}

void QgsMapUnitScaleDialog::setMapCanvas( QgsMapCanvas* canvas )
{
  mWidget->setMapCanvas( canvas );
}
