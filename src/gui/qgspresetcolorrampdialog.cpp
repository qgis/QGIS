/***************************************************************************
    qgspresetcolorrampdialog.cpp
    ----------------------------
    begin                : September 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspresetcolorrampdialog.h"

#include "qgssymbollayerutils.h"
#include "qgscolordialog.h"
#include <QFileDialog>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QMessageBox>

QgsPresetColorRampWidget::QgsPresetColorRampWidget( const QgsPresetSchemeColorRamp& ramp, QWidget* parent )
    : QgsPanelWidget( parent )
    , mRamp( ramp )
{
  setupUi( this );
  mTreeColors->setScheme( &mRamp );

  connect( mButtonCopyColors, SIGNAL( clicked() ), mTreeColors, SLOT( copyColors() ) );
  connect( mButtonRemoveColor, SIGNAL( clicked() ), mTreeColors, SLOT( removeSelection() ) );
  connect( mButtonPasteColors, SIGNAL( clicked() ), mTreeColors, SLOT( pasteColors() ) );
  connect( mButtonImportColors, SIGNAL( clicked( bool ) ), mTreeColors, SLOT( showImportColorsDialog() ) );
  connect( mButtonExportColors, SIGNAL( clicked( bool ) ), mTreeColors, SLOT( showExportColorsDialog() ) );

  connect( mTreeColors->model(), SIGNAL( dataChanged( QModelIndex, QModelIndex, QVector<int> ) ), this, SLOT( schemeChanged() ) );
  connect( mTreeColors->model(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( schemeChanged() ) );

  updatePreview();
}

QgsPresetColorRampWidget::~QgsPresetColorRampWidget()
{
}

QgsPresetSchemeColorRamp QgsPresetColorRampWidget::ramp() const
{
  return mRamp;
}

void QgsPresetColorRampWidget::setRamp( const QgsPresetSchemeColorRamp& ramp )
{
  mRamp = ramp;
  mTreeColors->setScheme( &mRamp );
  updatePreview();
  emit changed();
}

void QgsPresetColorRampWidget::updatePreview()
{
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size ) );
}

void QgsPresetColorRampWidget::setColors()
{
  updatePreview();
  emit changed();
}

void QgsPresetColorRampWidget::on_mButtonAddColor_clicked()
{
  if ( dockMode() )
  {
    mTreeColors->addColor( QgsRecentColorScheme::lastUsedColor(), QgsSymbolLayerUtils::colorToName( QgsRecentColorScheme::lastUsedColor() ), true );

    QgsCompoundColorWidget* colorWidget = new QgsCompoundColorWidget( this, QgsRecentColorScheme::lastUsedColor(), QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Select Color" ) );
    colorWidget->setAllowAlpha( true );
    connect( colorWidget, SIGNAL( currentColorChanged( QColor ) ), this, SLOT( newColorChanged( QColor ) ) );
    openPanel( colorWidget );
  }
  else
  {
    QColor newColor = QgsColorDialog::getColor( QColor(), this->parentWidget(), tr( "Select Color" ), true );
    if ( !newColor.isValid() )
    {
      return;
    }
    activateWindow();

    mTreeColors->addColor( newColor, QgsSymbolLayerUtils::colorToName( newColor ) );
  }
}

void QgsPresetColorRampWidget::schemeChanged()
{
  mTreeColors->saveColorsToScheme();
  updatePreview();
  emit changed();
}

void QgsPresetColorRampWidget::newColorChanged( const QColor& color )
{
  int row = mTreeColors->model()->rowCount() - 1;
  QModelIndex colorIndex = mTreeColors->model()->index( row, 0 );
  mTreeColors->model()->setData( colorIndex, color );
}

QgsPresetColorRampDialog::QgsPresetColorRampDialog( const QgsPresetSchemeColorRamp& ramp, QWidget* parent )
    : QDialog( parent )
{
  QVBoxLayout* vLayout = new QVBoxLayout();
  mWidget = new QgsPresetColorRampWidget( ramp );
  vLayout->addWidget( mWidget );
  QDialogButtonBox* bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( bbox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  connect( mWidget, SIGNAL( changed() ), this, SIGNAL( changed() ) );
}
