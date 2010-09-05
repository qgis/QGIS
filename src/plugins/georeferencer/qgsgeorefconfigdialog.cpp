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
/* $Id$ */
#include <QCloseEvent>
#include <QSettings>
#include <QSizeF>

#include "qgsgeorefconfigdialog.h"

QgsGeorefConfigDialog::QgsGeorefConfigDialog( QWidget *parent ) :
    QDialog( parent )
{
  setupUi( this );

  readSettings();

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
  mPaperSizeComboBox->addItem( tr( "Arch E1 (30x42 inches)" ) , QSizeF( 762, 1066.8 ) );

  mPaperSizeComboBox->setCurrentIndex( 2 ); //A3

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

void QgsGeorefConfigDialog::on_buttonBox_accepted()
{
  writeSettings();
  accept();
}

void QgsGeorefConfigDialog::on_buttonBox_rejected()
{
  reject();
}

void QgsGeorefConfigDialog::readSettings()
{
  QSettings s;
  if ( s.value( "/Plugin-GeoReferencer/Config/ShowId" ).toBool() )
  {
    mShowIDsCheckBox->setChecked( true );
  }
  else
  {
    mShowIDsCheckBox->setChecked( false );
  }

  if ( s.value( "/Plugin-GeoReferencer/Config/ShowCoords" ).toBool() )
  {
    mShowCoordsCheckBox->setChecked( true );
  }
  else
  {
    mShowCoordsCheckBox->setChecked( false );
  }

  if ( s.value( "/Plugin-GeoReferencer/Config/ShowDocked" ).toBool() )
  {
    mShowDockedCheckBox->setChecked( true );
  }
  else
  {
    mShowDockedCheckBox->setChecked( false );
  }

  if ( s.value( "/Plugin-GeoReferencer/Config/ResidualUnits" ).toString() == "mapUnits" )
  {
    mMapUnitsButton->setChecked( true );
  }
  else
  {
    mPixelsButton->setChecked( true );
  }

  mLeftMarginSpinBox->setValue( s.value( "/Plugin-GeoReferencer/Config/LeftMarginPDF", "2.0" ).toDouble() );
  mRightMarginSpinBox->setValue( s.value( "/Plugin-GeoReferencer/Config/RightMarginPDF", "2.0" ).toDouble() );
}

void QgsGeorefConfigDialog::writeSettings()
{
  QSettings s;
  s.setValue( "/Plugin-GeoReferencer/Config/ShowId", mShowIDsCheckBox->isChecked() );
  s.setValue( "/Plugin-GeoReferencer/Config/ShowCoords", mShowCoordsCheckBox->isChecked() );
  s.setValue( "/Plugin-GeoReferencer/Config/ShowDocked", mShowDockedCheckBox->isChecked() );
  if ( mPixelsButton->isChecked() )
  {
    s.setValue( "/Plugin-GeoReferencer/Config/ResidualUnits", "pixels" );
  }
  else
  {
    s.setValue( "/Plugin-GeoReferencer/Config/ResidualUnits", "mapUnits" );
  }
  s.setValue( "/Plugin-GeoReferencer/Config/LeftMarginPDF", mLeftMarginSpinBox->value() );
  s.setValue( "/Plugin-GeoReferencer/Config/RightMarginPDF", mRightMarginSpinBox->value() );

  s.setValue( "/Plugin-GeoReferencer/Config/WidthPDFMap", mPaperSizeComboBox->itemData( mPaperSizeComboBox->currentIndex() ).toSizeF().width() );
  s.setValue( "/Plugin-GeoReferencer/Config/HeightPDFMap", mPaperSizeComboBox->itemData( mPaperSizeComboBox->currentIndex() ).toSizeF().height() );

}

