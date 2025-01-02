/***************************************************************************
     qgsgeorefconfigdialog.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QCloseEvent>
#include <QSizeF>

#include "qgssettings.h"
#include "qgsgeorefconfigdialog.h"
#include "moc_qgsgeorefconfigdialog.cpp"
#include "qgis.h"
#include "qgsgui.h"

QgsGeorefConfigDialog::QgsGeorefConfigDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsGeorefConfigDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsGeorefConfigDialog::buttonBox_rejected );

  mPaperSizeComboBox->addItem( tr( "A5 (148x210 mm)" ), QSizeF( 148, 210 ) );
  mPaperSizeComboBox->addItem( tr( "A4 (210x297 mm)" ), QSizeF( 210, 297 ) );
  mPaperSizeComboBox->addItem( tr( "A3 (297x420 mm)" ), QSizeF( 297, 420 ) );
  mPaperSizeComboBox->addItem( tr( "A2 (420x594 mm)" ), QSizeF( 420, 594 ) );
  mPaperSizeComboBox->addItem( tr( "A1 (594x841 mm)" ), QSizeF( 594, 841 ) );
  mPaperSizeComboBox->addItem( tr( "A0 (841x1189 mm)" ), QSizeF( 841, 1189 ) );
  mPaperSizeComboBox->addItem( tr( "B5 (176 x 250 mm)" ), QSizeF( 176, 250 ) );
  mPaperSizeComboBox->addItem( tr( "B4 (250 x 353 mm)" ), QSizeF( 250, 353 ) );
  mPaperSizeComboBox->addItem( tr( "B3 (353 x 500 mm)" ), QSizeF( 353, 500 ) );
  mPaperSizeComboBox->addItem( tr( "B2 (500 x 707 mm)" ), QSizeF( 500, 707 ) );
  mPaperSizeComboBox->addItem( tr( "B1 (707 x 1000 mm)" ), QSizeF( 707, 1000 ) );
  mPaperSizeComboBox->addItem( tr( "B0 (1000 x 1414 mm)" ), QSizeF( 1000, 1414 ) );
  // North american formats
  mPaperSizeComboBox->addItem( tr( "Legal (8.5x14 inches)" ), QSizeF( 215.9, 355.6 ) );
  mPaperSizeComboBox->addItem( tr( "ANSI A (Letter; 8.5x11 inches)" ), QSizeF( 215.9, 279.4 ) );
  mPaperSizeComboBox->addItem( tr( "ANSI B (Tabloid; 11x17 inches)" ), QSizeF( 279.4, 431.8 ) );
  mPaperSizeComboBox->addItem( tr( "ANSI C (17x22 inches)" ), QSizeF( 431.8, 558.8 ) );
  mPaperSizeComboBox->addItem( tr( "ANSI D (22x34 inches)" ), QSizeF( 558.8, 863.6 ) );
  mPaperSizeComboBox->addItem( tr( "ANSI E (34x44 inches)" ), QSizeF( 863.6, 1117.6 ) );
  mPaperSizeComboBox->addItem( tr( "Arch A (9x12 inches)" ), QSizeF( 228.6, 304.8 ) );
  mPaperSizeComboBox->addItem( tr( "Arch B (12x18 inches)" ), QSizeF( 304.8, 457.2 ) );
  mPaperSizeComboBox->addItem( tr( "Arch C (18x24 inches)" ), QSizeF( 457.2, 609.6 ) );
  mPaperSizeComboBox->addItem( tr( "Arch D (24x36 inches)" ), QSizeF( 609.6, 914.4 ) );
  mPaperSizeComboBox->addItem( tr( "Arch E (36x48 inches)" ), QSizeF( 914.4, 1219.2 ) );
  mPaperSizeComboBox->addItem( tr( "Arch E1 (30x42 inches)" ), QSizeF( 762, 1066.8 ) );

  readSettings();
}

void QgsGeorefConfigDialog::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  switch ( e->type() )
  {
    case QEvent::LanguageChange:
      retranslateUi( this );
      break;
    default:
      break;
  }
}

void QgsGeorefConfigDialog::buttonBox_accepted()
{
  writeSettings();
  accept();
}

void QgsGeorefConfigDialog::buttonBox_rejected()
{
  reject();
}

void QgsGeorefConfigDialog::readSettings()
{
  const QgsSettings s;
  mShowIDsCheckBox->setChecked( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowId" ) ).toBool() );
  mShowCoordsCheckBox->setChecked( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowCoords" ) ).toBool() );
  mShowDockedCheckBox->setChecked( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowDocked" ) ).toBool() );

  if ( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ) ).toString() == QLatin1String( "mapUnits" ) )
  {
    mMapUnitsButton->setChecked( true );
  }
  else
  {
    mPixelsButton->setChecked( true );
  }

  mLeftMarginSpinBox->setValue( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/LeftMarginPDF" ), "2.0" ).toDouble() );
  mRightMarginSpinBox->setValue( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/RightMarginPDF" ), "2.0" ).toDouble() );

  const double currentWidth = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/WidthPDFMap" ), "297" ).toDouble();
  const double currentHeight = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/HeightPDFMap" ), "420" ).toDouble();

  int paperIndex = 2; //default to A3
  for ( int i = 0; i < mPaperSizeComboBox->count(); ++i )
  {
    const double itemWidth = mPaperSizeComboBox->itemData( i ).toSizeF().width();
    const double itemHeight = mPaperSizeComboBox->itemData( i ).toSizeF().height();
    if ( qgsDoubleNear( itemWidth, currentWidth ) && qgsDoubleNear( itemHeight, currentHeight ) )
    {
      paperIndex = i;
      break;
    }
  }
  mPaperSizeComboBox->setCurrentIndex( paperIndex );
}

void QgsGeorefConfigDialog::writeSettings()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowId" ), mShowIDsCheckBox->isChecked() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowCoords" ), mShowCoordsCheckBox->isChecked() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowDocked" ), mShowDockedCheckBox->isChecked() );
  if ( mPixelsButton->isChecked() )
  {
    s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ), "pixels" );
  }
  else
  {
    s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ), "mapUnits" );
  }
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/LeftMarginPDF" ), mLeftMarginSpinBox->value() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/RightMarginPDF" ), mRightMarginSpinBox->value() );

  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/WidthPDFMap" ), mPaperSizeComboBox->currentData().toSizeF().width() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/HeightPDFMap" ), mPaperSizeComboBox->currentData().toSizeF().height() );
}
