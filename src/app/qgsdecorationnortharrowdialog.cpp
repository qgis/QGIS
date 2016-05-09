/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "qgsdecorationnortharrowdialog.h"
#include "qgsdecorationnortharrow.h"
#include "qgslogger.h"
#include "qgscontexthelp.h"

#include <QPainter>
#include <QSettings>
#include <cmath>
#include <QDialogButtonBox>
#include <QPushButton>

QgsDecorationNorthArrowDialog::QgsDecorationNorthArrowDialog( QgsDecorationNorthArrow& deco, QWidget* parent )
    : QDialog( parent )
    , mDeco( deco )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/DecorationNorthArrow/geometry" ).toByteArray() );

  QPushButton* applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, SIGNAL( clicked() ), this, SLOT( apply() ) );

  // rotation
  rotatePixmap( mDeco.mRotationInt );
  // signal/slot connection defined in 'designer' causes the slider to
  // be moved to reflect the change in the spinbox.
  spinAngle->setValue( mDeco.mRotationInt );

  // placement
  cboPlacement->addItem( tr( "Top left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom right" ), QgsDecorationItem::BottomRight );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );
  spinHorizontal->setValue( mDeco.mMarginHorizontal );
  spinVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits( QgsSymbolV2::OutputUnitList() << QgsSymbolV2::MM << QgsSymbolV2::Percentage << QgsSymbolV2::Pixel );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  // enabled
  grpEnable->setChecked( mDeco.enabled() );

  // automatic
  cboxAutomatic->setChecked( mDeco.mAutomatic );
}

QgsDecorationNorthArrowDialog::~QgsDecorationNorthArrowDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/DecorationNorthArrow/geometry", saveGeometry() );
}

void QgsDecorationNorthArrowDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run( metaObject()->className() );
}

void QgsDecorationNorthArrowDialog::on_buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationNorthArrowDialog::on_buttonBox_rejected()
{
  reject();
}


void QgsDecorationNorthArrowDialog::on_spinAngle_valueChanged( int theInt )
{
  Q_UNUSED( theInt );
}

void QgsDecorationNorthArrowDialog::on_sliderRotation_valueChanged( int theInt )
{
  rotatePixmap( theInt );
}

void QgsDecorationNorthArrowDialog::apply()
{
  mDeco.mRotationInt = sliderRotation->value();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->itemData( cboPlacement->currentIndex() ).toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mAutomatic = cboxAutomatic->isChecked();
  mDeco.mMarginHorizontal = spinHorizontal->value();
  mDeco.mMarginVertical = spinVertical->value();
  mDeco.update();
}

void QgsDecorationNorthArrowDialog::rotatePixmap( int theRotationInt )
{
  QPixmap myQPixmap;
  QString myFileNameQString = ":/images/north_arrows/default.png";
// QgsDebugMsg(QString("Trying to load %1").arg(myFileNameQString));
  if ( myQPixmap.load( myFileNameQString ) )
  {
    QPixmap  myPainterPixmap( myQPixmap.height(), myQPixmap.width() );
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );

    myQPainter.setRenderHint( QPainter::SmoothPixmapTransform );

    double centerXDouble = myQPixmap.width() / 2.0;
    double centerYDouble = myQPixmap.height() / 2.0;
    //save the current canvas rotation
    myQPainter.save();
    //myQPainter.translate( (int)centerXDouble, (int)centerYDouble );

    //rotate the canvas
    myQPainter.rotate( theRotationInt );
    //work out how to shift the image so that it appears in the center of the canvas
    //(x cos a + y sin a - x, -x sin a + y cos a - y)
    const double PI = 3.14159265358979323846;
    double myRadiansDouble = ( PI / 180 ) * theRotationInt;
    int xShift = static_cast<int>((
                                    ( centerXDouble * cos( myRadiansDouble ) ) +
                                    ( centerYDouble * sin( myRadiansDouble ) )
                                  ) - centerXDouble );
    int yShift = static_cast<int>((
                                    ( -centerXDouble * sin( myRadiansDouble ) ) +
                                    ( centerYDouble * cos( myRadiansDouble ) )
                                  ) - centerYDouble );

    //draw the pixmap in the proper position
    myQPainter.drawPixmap( xShift, yShift, myQPixmap );

    //unrotate the canvas again
    myQPainter.restore();
    myQPainter.end();

    pixmapLabel->setPixmap( myPainterPixmap );
  }
  else
  {
    QPixmap  myPainterPixmap( 200, 200 );
    myPainterPixmap.fill();
    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );
    QFont myQFont( "time", 12, QFont::Bold );
    myQPainter.setFont( myQFont );
    myQPainter.setPen( Qt::red );
    myQPainter.drawText( 10, 20, tr( "Pixmap not found" ) );
    myQPainter.end();
    pixmapLabel->setPixmap( myPainterPixmap );
  }
}

//
// Called when the widget has been resized.
//

void QgsDecorationNorthArrowDialog::resizeEvent( QResizeEvent *theResizeEvent )
{
  Q_UNUSED( theResizeEvent );
  rotatePixmap( sliderRotation->value() );
}
