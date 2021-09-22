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
#include "qgshelp.h"
#include <QDialogButtonBox>

QgsMapUnitScaleWidget::QgsMapUnitScaleWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  mComboBoxMinScale->setScale( 10000000.0 );
  mComboBoxMaxScale->setScale( 1 );
  mSpinBoxMinSize->setShowClearButton( false );
  mSpinBoxMaxSize->setShowClearButton( false );
  connect( mCheckBoxMinScale, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::configureMinComboBox );
  connect( mCheckBoxMaxScale, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::configureMaxComboBox );
  connect( mComboBoxMinScale, &QgsScaleWidget::scaleChanged, this, &QgsMapUnitScaleWidget::configureMaxComboBox );
  connect( mComboBoxMinScale, &QgsScaleWidget::scaleChanged, mComboBoxMaxScale, &QgsScaleWidget::setMinScale );
  connect( mComboBoxMaxScale, &QgsScaleWidget::scaleChanged, this, &QgsMapUnitScaleWidget::configureMinComboBox );

  connect( mCheckBoxMinSize, &QCheckBox::toggled, mSpinBoxMinSize, &QgsDoubleSpinBox::setEnabled );
  connect( mCheckBoxMaxSize, &QCheckBox::toggled, mSpinBoxMaxSize, &QgsDoubleSpinBox::setEnabled );

  // notification of setting changes
  connect( mCheckBoxMinScale, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mCheckBoxMaxScale, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mComboBoxMinScale, &QgsScaleWidget::scaleChanged, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mComboBoxMaxScale, &QgsScaleWidget::scaleChanged, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mCheckBoxMinSize, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mCheckBoxMaxSize, &QCheckBox::toggled, this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mSpinBoxMinSize, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsMapUnitScaleWidget::settingsChanged );
  connect( mSpinBoxMaxSize, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsMapUnitScaleWidget::settingsChanged );
  mBlockSignals = false;
}

void QgsMapUnitScaleWidget::setMapUnitScale( const QgsMapUnitScale &scale )
{
  // can't block signals on the widgets themselves, some use them to update
  // internal states
  mBlockSignals = true;
  mComboBoxMinScale->setScale( scale.minScale > 0.0 ? scale.minScale : 10000000 );
  mCheckBoxMinScale->setChecked( scale.minScale != 0.0 );
  mComboBoxMinScale->setEnabled( scale.minScale != 0.0 );
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
  if ( mCheckBoxMinScale->isChecked() && mComboBoxMinScale->scale() < mComboBoxMaxScale->scale() )
  {
    mComboBoxMinScale->setScale( mComboBoxMaxScale->scale() );
  }
}

void QgsMapUnitScaleWidget::configureMaxComboBox()
{
  mComboBoxMaxScale->setEnabled( mCheckBoxMaxScale->isChecked() );
  if ( mCheckBoxMaxScale->isChecked() && mComboBoxMaxScale->scale() > mComboBoxMinScale->scale() )
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

{
  mMapUnitIdx = -1;

  setupUi( this );
  mMapScaleButton->setVisible( false );
  mMapScaleButton->setToolTip( tr( "Adjust scaling range" ) );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mUnitCombo );

  connect( mUnitCombo, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, &QgsUnitSelectionWidget::toggleUnitRangeButton );
  connect( mMapScaleButton, &QToolButton::clicked, this, &QgsUnitSelectionWidget::showDialog );
  connect( mUnitCombo, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::currentIndexChanged ), this, &QgsUnitSelectionWidget::changed );
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
    mUnitCombo->addItem( tr( "Millimeters" ), QgsUnitTypes::RenderMillimeters );
  }
  if ( units.contains( QgsUnitTypes::RenderPoints ) )
  {
    mUnitCombo->addItem( tr( "Points" ), QgsUnitTypes::RenderPoints );
  }
  if ( units.contains( QgsUnitTypes::RenderPixels ) )
  {
    mUnitCombo->addItem( tr( "Pixels" ), QgsUnitTypes::RenderPixels );
  }
  if ( units.contains( QgsUnitTypes::RenderMetersInMapUnits ) )
  {
    mUnitCombo->addItem( tr( "Meters at Scale" ), QgsUnitTypes::RenderMetersInMapUnits );
  }
  if ( units.contains( QgsUnitTypes::RenderMapUnits ) )
  {
    mUnitCombo->addItem( tr( "Map Units" ), QgsUnitTypes::RenderMapUnits );
  }
  if ( units.contains( QgsUnitTypes::RenderPercentage ) )
  {
    mUnitCombo->addItem( tr( "Percentage" ), QgsUnitTypes::RenderPercentage );
  }
  if ( units.contains( QgsUnitTypes::RenderInches ) )
  {
    mUnitCombo->addItem( tr( "Inches" ), QgsUnitTypes::RenderInches );
  }
  blockSignals( false );
}

QgsUnitTypes::RenderUnit QgsUnitSelectionWidget::unit() const
{
  if ( mUnitCombo->count() == 0 )
    return QgsUnitTypes::RenderUnknownUnit;

  const QVariant currentData = mUnitCombo->currentData();
  if ( currentData.isValid() )
  {
    return static_cast< QgsUnitTypes::RenderUnit >( currentData.toInt() );
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
  const int idx = mUnitCombo->findData( QVariant( static_cast< int >( unit ) ) );
  mUnitCombo->setCurrentIndex( idx == -1 ? 0 : idx );
}

void QgsUnitSelectionWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

void QgsUnitSelectionWidget::showDialog()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsMapUnitScaleWidget *widget = new QgsMapUnitScaleWidget( panel );
    widget->setPanelTitle( tr( "Adjust Scaling Range" ) );
    widget->setMapCanvas( mCanvas );
    widget->setMapUnitScale( mMapUnitScale );
    connect( widget, &QgsMapUnitScaleWidget::mapUnitScaleChanged, this, &QgsUnitSelectionWidget::widgetChanged );
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
    mMapScaleButton->setVisible( mShowMapScaleButton && unit() == QgsUnitTypes::RenderMapUnits );
  }
  else
  {
    mMapScaleButton->setVisible( mShowMapScaleButton && mMapUnitIdx != -1 && mUnitCombo->currentIndex() == mMapUnitIdx );
  }
}

void QgsUnitSelectionWidget::widgetChanged( const QgsMapUnitScale &scale )
{
  mMapUnitScale = scale;
  emit changed();
}

bool QgsUnitSelectionWidget::showMapScaleButton() const
{
  return mShowMapScaleButton;
}

void QgsUnitSelectionWidget::setShowMapScaleButton( bool show )
{
  mShowMapScaleButton = show;
  if ( !show )
    mMapScaleButton->hide();
}


QgsMapUnitScaleDialog::QgsMapUnitScaleDialog( QWidget *parent )
  : QDialog( parent )

{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsMapUnitScaleWidget();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsMapUnitScaleDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsMapUnitScaleDialog::reject );
  connect( bbox, &QDialogButtonBox::helpRequested, this, &QgsMapUnitScaleDialog::showHelp );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Adjust Scaling Range" ) );
}

QgsMapUnitScale QgsMapUnitScaleDialog::getMapUnitScale() const
{
  return mWidget->mapUnitScale();
}

void QgsMapUnitScaleDialog::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mWidget->setMapUnitScale( scale );
}

void QgsMapUnitScaleDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mWidget->setMapCanvas( canvas );
}

void QgsMapUnitScaleDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#unit-selector" ) );
}
