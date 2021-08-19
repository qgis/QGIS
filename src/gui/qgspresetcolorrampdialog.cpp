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

QgsPresetColorRampWidget::QgsPresetColorRampWidget( const QgsPresetSchemeColorRamp &ramp, QWidget *parent )
  : QgsPanelWidget( parent )
  , mRamp( ramp )
{
  setupUi( this );
  connect( mButtonAddColor, &QToolButton::clicked, this, &QgsPresetColorRampWidget::mButtonAddColor_clicked );
  mTreeColors->setScheme( &mRamp );

  connect( mButtonCopyColors, &QAbstractButton::clicked, mTreeColors, &QgsColorSchemeList::copyColors );
  connect( mButtonRemoveColor, &QAbstractButton::clicked, mTreeColors, &QgsColorSchemeList::removeSelection );
  connect( mButtonPasteColors, &QAbstractButton::clicked, mTreeColors, &QgsColorSchemeList::pasteColors );
  connect( mButtonImportColors, &QAbstractButton::clicked, mTreeColors, &QgsColorSchemeList::showImportColorsDialog );
  connect( mButtonExportColors, &QAbstractButton::clicked, mTreeColors, &QgsColorSchemeList::showExportColorsDialog );

  connect( mTreeColors->model(), &QAbstractItemModel::dataChanged, this, &QgsPresetColorRampWidget::schemeChanged );
  connect( mTreeColors->model(), &QAbstractItemModel::rowsRemoved, this, &QgsPresetColorRampWidget::schemeChanged );

  updatePreview();
}

QgsPresetSchemeColorRamp QgsPresetColorRampWidget::ramp() const
{
  return mRamp;
}

void QgsPresetColorRampWidget::setRamp( const QgsPresetSchemeColorRamp &ramp )
{
  mRamp = ramp;
  mTreeColors->setScheme( &mRamp );
  updatePreview();
  emit changed();
}

void QgsPresetColorRampWidget::updatePreview()
{
  const QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size ) );
}

void QgsPresetColorRampWidget::setColors()
{
  updatePreview();
  emit changed();
}

void QgsPresetColorRampWidget::mButtonAddColor_clicked()
{
  if ( dockMode() )
  {
    mTreeColors->addColor( QgsRecentColorScheme::lastUsedColor(), QgsSymbolLayerUtils::colorToName( QgsRecentColorScheme::lastUsedColor() ), true );

    QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( this, QgsRecentColorScheme::lastUsedColor(), QgsCompoundColorWidget::LayoutVertical );
    colorWidget->setPanelTitle( tr( "Select Color" ) );
    colorWidget->setAllowOpacity( true );
    connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, &QgsPresetColorRampWidget::newColorChanged );
    openPanel( colorWidget );
  }
  else
  {
    const QColor newColor = QgsColorDialog::getColor( QColor(), this->parentWidget(), tr( "Select Color" ), true );
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

void QgsPresetColorRampWidget::newColorChanged( const QColor &color )
{
  const int row = mTreeColors->model()->rowCount() - 1;
  const QModelIndex colorIndex = mTreeColors->model()->index( row, 0 );
  mTreeColors->model()->setData( colorIndex, color );
}

QgsPresetColorRampDialog::QgsPresetColorRampDialog( const QgsPresetSchemeColorRamp &ramp, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsPresetColorRampWidget( ramp );
  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsPresetColorRampDialog::showHelp );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "Color Presets Ramp" ) );
  connect( mWidget, &QgsPresetColorRampWidget::changed, this, &QgsPresetColorRampDialog::changed );
}

QDialogButtonBox *QgsPresetColorRampDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsPresetColorRampDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/style_manager.html#setting-a-color-ramp" ) );
}
