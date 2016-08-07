/***************************************************************************
    qgsvectorcolorbrewercolorrampdialog.cpp
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

#include "qgsvectorcolorbrewercolorrampdialog.h"

#include "qgsvectorcolorramp.h"
#include "qgssymbollayerutils.h"
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


QgsVectorColorBrewerColorRampDialog::QgsVectorColorBrewerColorRampDialog( QgsVectorColorBrewerColorRamp* ramp, QWidget* parent )
    : QDialog( parent )
    , mRamp( ramp )
{

  setupUi( this );

  QSize iconSize( 50, 16 );
  cboSchemeName->setIconSize( iconSize );

  QStringList schemes = QgsVectorColorBrewerColorRamp::listSchemeNames();
  Q_FOREACH ( const QString& schemeName, schemes )
  {
    // create a preview icon using five color variant
    QgsVectorColorBrewerColorRamp* r = new QgsVectorColorBrewerColorRamp( schemeName, 5 );
    QIcon icon = QgsSymbolLayerUtils::colorRampPreviewIcon( r, iconSize );
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

void QgsVectorColorBrewerColorRampDialog::populateVariants()
{
  QString oldVariant = cboColors->currentText();

  cboColors->clear();
  QString schemeName = cboSchemeName->currentText();
  QList<int> variants = QgsVectorColorBrewerColorRamp::listSchemeVariants( schemeName );
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

void QgsVectorColorBrewerColorRampDialog::updatePreview()
{
  QSize size( 300, 40 );
  lblPreview->setPixmap( QgsSymbolLayerUtils::colorRampPreviewPixmap( mRamp, size ) );
}

void QgsVectorColorBrewerColorRampDialog::setSchemeName()
{
  // populate list of variants
  populateVariants();

  mRamp->setSchemeName( cboSchemeName->currentText() );
  updatePreview();
}

void QgsVectorColorBrewerColorRampDialog::setColors()
{
  int num = cboColors->currentText().toInt();
  mRamp->setColors( num );
  updatePreview();
}
