/***************************************************************************
    qgscolorbrewercolorrampdialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorbrewercolorrampdialog.h"

#include "qgscolorramp.h"
#include "qgssymbollayerutils.h"
#include "qgshelp.h"

#include <QAbstractButton>
#include <QDialogButtonBox>

#if 0 // unused
static void updateColorButton( QAbstractButton *button, QColor color )
{
  QPixmap p( 20, 20 );
  p.fill( color );
  button->setIcon( QIcon( p ) );
}
#endif

/////////


QgsColorBrewerColorRampWidget::QgsColorBrewerColorRampWidget( const QgsColorBrewerColorRamp &ramp, QWidget *parent )
  : QgsPanelWidget( parent )
  , mRamp( ramp )
{

  setupUi( this );

  const QSize iconSize( 50, 16 );
  cboSchemeName->setIconSize( iconSize );

  const QStringList schemes = QgsColorBrewerColorRamp::listSchemeNames();
  const auto constSchemes = schemes;
  for ( const QString &schemeName : constSchemes )
  {
    // create a preview icon using five color variant
    QgsColorBrewerColorRamp *r = new QgsColorBrewerColorRamp( schemeName, 5 );
    const QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( r, iconSize );
    delete r;
    cboSchemeName->addItem( icon, schemeName );
  }

  updateUi();
  connect( cboSchemeName, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorBrewerColorRampWidget::setSchemeName );
  connect( cboColors, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsColorBrewerColorRampWidget::setColors );
}

void QgsColorBrewerColorRampWidget::setRamp( const QgsColorBrewerColorRamp &ramp )
{
  mRamp = ramp;
  updateUi();
  emit changed();
}

void QgsColorBrewerColorRampWidget::populateVariants()
{
  const QString oldVariant = cboColors->currentText();

  cboColors->clear();
  const QString schemeName = cboSchemeName->currentText();
  const QList<int> variants = QgsColorBrewerColorRamp::listSchemeVariants( schemeName );
  const auto constVariants = variants;
  for ( const int variant : constVariants )
  {
    cboColors->addItem( QString::number( variant ) );
  }

  // try to set the original variant again (if exists)
  int idx = cboColors->findText( oldVariant );
  if ( idx == -1 ) // not found?
  {
    // use the last item
    idx = cboColors->count() - 1;
  }
  cboColors->setCurrentIndex( idx );
}

void QgsColorBrewerColorRampWidget::updatePreview()
{
  const QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( &mRamp, size ) );
}

void QgsColorBrewerColorRampWidget::updateUi()
{
  whileBlocking( cboSchemeName )->setCurrentIndex( cboSchemeName->findText( mRamp.schemeName() ) );
  populateVariants();
  whileBlocking( cboColors )->setCurrentIndex( cboColors->findText( QString::number( mRamp.colors() ) ) );
  updatePreview();
}

void QgsColorBrewerColorRampWidget::setSchemeName()
{
  // populate list of variants
  populateVariants();

  mRamp.setSchemeName( cboSchemeName->currentText() );
  updatePreview();
  emit changed();
}

void QgsColorBrewerColorRampWidget::setColors()
{
  const int num = cboColors->currentText().toInt();
  mRamp.setColors( num );
  updatePreview();
  emit changed();
}

QgsColorBrewerColorRampDialog::QgsColorBrewerColorRampDialog( const QgsColorBrewerColorRamp &ramp, QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsColorBrewerColorRampWidget( ramp );
  connect( mWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );
  vLayout->addWidget( mWidget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsColorBrewerColorRampDialog::showHelp );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );
  setWindowTitle( tr( "ColorBrewer Ramp" ) );
  connect( mWidget, &QgsColorBrewerColorRampWidget::changed, this, &QgsColorBrewerColorRampDialog::changed );
}

QDialogButtonBox *QgsColorBrewerColorRampDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsColorBrewerColorRampDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/style_manager.html#setting-a-color-ramp" ) );
}
