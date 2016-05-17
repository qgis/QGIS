/***************************************************************************
    qgsvectorcolorbrewercolorrampv2dialog.cpp
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

#include "qgsvectorcolorbrewercolorrampv2dialog.h"

#include "qgsvectorcolorrampv2.h"

#include <QAbstractButton>

#if 0 // unused
static void updateColorButton( QAbstractButton* button, QColor color )
{
  QPixmap p( 20, 20 );
  p.fill( color );
  button->setIcon( QIcon( p ) );
}
#endif

/////////


QgsVectorColorBrewerColorRampV2Dialog::QgsVectorColorBrewerColorRampV2Dialog( QgsVectorColorBrewerColorRampV2* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( ramp )
{

  setupUi( this );

  QSize iconSize( 50, 16 );
  cboSchemeName->setIconSize( iconSize );

  QStringList schemes = QgsVectorColorBrewerColorRampV2::listSchemeNames();
  Q_FOREACH ( const QString& schemeName, schemes )
  {
    // create a preview icon using five color variant
    QgsVectorColorBrewerColorRampV2* r = new QgsVectorColorBrewerColorRampV2( schemeName, 5 );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( r, iconSize );
    delete r;
    cboSchemeName->addItem( icon, schemeName );
  }

  cboSchemeName->setCurrentIndex( cboSchemeName->findText( ramp->schemeName() ) );
  populateVariants();
  cboColors->setCurrentIndex( cboColors->findText( QString::number( ramp->colors() ) ) );

  connect( cboSchemeName, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setSchemeName() ) );
  connect( cboColors, SIGNAL( currentIndexChanged( int ) ), this, SLOT( setColors() ) );

  updatePreview();
}

void QgsVectorColorBrewerColorRampV2Dialog::populateVariants()
{
  QString oldVariant = cboColors->currentText();

  cboColors->clear();
  QString schemeName = cboSchemeName->currentText();
  QList<int> variants = QgsVectorColorBrewerColorRampV2::listSchemeVariants( schemeName );
  Q_FOREACH ( int variant, variants )
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

void QgsVectorColorBrewerColorRampV2Dialog::updatePreview()
{
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerV2Utils::colorRampPreviewPixmap( mRamp, size ) );
}

void QgsVectorColorBrewerColorRampV2Dialog::setSchemeName()
{
  // populate list of variants
  populateVariants();

  mRamp->setSchemeName( cboSchemeName->currentText() );
  updatePreview();
}

void QgsVectorColorBrewerColorRampV2Dialog::setColors()
{
  int num = cboColors->currentText().toInt();
  mRamp->setColors( num );
  updatePreview();
}
