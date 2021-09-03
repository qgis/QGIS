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
#include "qgshelp.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgssvgselectorwidget.h"
#include "qgsgui.h"

#include <QPainter>
#include <cmath>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSvgRenderer>

QgsDecorationNorthArrowDialog::QgsDecorationNorthArrowDialog( QgsDecorationNorthArrow &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationNorthArrowDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationNorthArrowDialog::buttonBox_rejected );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationNorthArrowDialog::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationNorthArrowDialog::showHelp );

  spinSize->setValue( mDeco.mSize );

  // signal/slot connection defined in 'designer' causes the slider to
  // be moved to reflect the change in the spinbox.
  spinAngle->setValue( mDeco.mRotationInt );

  // automatic
  cboxAutomatic->setChecked( mDeco.mAutomatic );
  spinAngle->setEnabled( !mDeco.mAutomatic );
  sliderRotation->setEnabled( !mDeco.mAutomatic );

  connect( cboxAutomatic, &QAbstractButton::toggled, this, [ = ]( bool checked )
  {
    spinAngle->setEnabled( !checked );
    sliderRotation->setEnabled( !checked );
  } );
  connect( spinAngle, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsDecorationNorthArrowDialog::spinAngle_valueChanged );
  connect( sliderRotation, &QSlider::valueChanged, this, &QgsDecorationNorthArrowDialog::sliderRotation_valueChanged );

  // placement
  cboPlacement->addItem( tr( "Top Left" ), QgsDecorationItem::TopLeft );
  cboPlacement->addItem( tr( "Top Center" ), QgsDecorationItem::TopCenter );
  cboPlacement->addItem( tr( "Top Right" ), QgsDecorationItem::TopRight );
  cboPlacement->addItem( tr( "Bottom Left" ), QgsDecorationItem::BottomLeft );
  cboPlacement->addItem( tr( "Bottom Center" ), QgsDecorationItem::BottomCenter );
  cboPlacement->addItem( tr( "Bottom Right" ), QgsDecorationItem::BottomRight );
  connect( cboPlacement, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    spinHorizontal->setMinimum( cboPlacement->currentData() == QgsDecorationItem::TopCenter || cboPlacement->currentData() == QgsDecorationItem::BottomCenter ? -100 : 0 );
  } );
  cboPlacement->setCurrentIndex( cboPlacement->findData( mDeco.placement() ) );

  spinHorizontal->setClearValue( 0 );
  spinHorizontal->setValue( mDeco.mMarginHorizontal );
  spinVertical->setValue( mDeco.mMarginVertical );
  wgtUnitSelection->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPercentage << QgsUnitTypes::RenderPixels );
  wgtUnitSelection->setUnit( mDeco.mMarginUnit );

  // enabled
  grpEnable->setChecked( mDeco.enabled() );

  mSvgPathLineEdit->setText( mDeco.mSvgPath );
  connect( mSvgPathLineEdit, &QLineEdit::textChanged, this, &QgsDecorationNorthArrowDialog::updateSvgPath );
  connect( mSvgSelectorBtn, &QPushButton::clicked, this, [ = ]
  {
    QgsSvgSelectorDialog svgDlg( this );
    svgDlg.setWindowTitle( tr( "Select SVG file" ) );
    svgDlg.svgSelector()->setSvgPath( mSvgPathLineEdit->text().trimmed() );
    if ( svgDlg.exec() == QDialog::Accepted )
    {
      const QString svgPath = svgDlg.svgSelector()->currentSvgPath();
      if ( !svgPath.isEmpty() )
      {
        updateSvgPath( svgPath );
      }
    }
  } );

  pbnChangeColor->setAllowOpacity( true );
  pbnChangeColor->setColor( mDeco.mColor );
  pbnChangeColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeColor->setColorDialogTitle( tr( "Select North Arrow Fill Color" ) );
  pbnChangeOutlineColor->setAllowOpacity( true );
  pbnChangeOutlineColor->setColor( mDeco.mOutlineColor );
  pbnChangeOutlineColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeOutlineColor->setColorDialogTitle( tr( "Select North Arrow Outline Color" ) );
  connect( pbnChangeColor, &QgsColorButton::colorChanged, this, [ = ]( QColor ) { drawNorthArrow(); } );
  connect( pbnChangeOutlineColor, &QgsColorButton::colorChanged, this, [ = ]( QColor ) { drawNorthArrow(); } );

  drawNorthArrow();
}

void QgsDecorationNorthArrowDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#north-arrow" ) );
}

void QgsDecorationNorthArrowDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationNorthArrowDialog::buttonBox_rejected()
{
  reject();
}


void QgsDecorationNorthArrowDialog::spinAngle_valueChanged( int spinAngle )
{
  sliderRotation->setValue( spinAngle );
  drawNorthArrow();
}

void QgsDecorationNorthArrowDialog::sliderRotation_valueChanged( int rotationValue )
{
  spinAngle->setValue( rotationValue );
  drawNorthArrow();
}

void QgsDecorationNorthArrowDialog::apply()
{
  mDeco.mColor = pbnChangeColor->color();
  mDeco.mOutlineColor = pbnChangeOutlineColor->color();
  mDeco.mSize = spinSize->value();
  mDeco.mRotationInt = sliderRotation->value();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mAutomatic = cboxAutomatic->isChecked();
  mDeco.mMarginHorizontal = spinHorizontal->value();
  mDeco.mMarginVertical = spinVertical->value();
  mDeco.update();
}

void QgsDecorationNorthArrowDialog::updateSvgPath( const QString &svgPath )
{
  if ( mSvgPathLineEdit->text() != svgPath )
  {
    mSvgPathLineEdit->setText( svgPath );
  }
  mDeco.mSvgPath = svgPath;

  const QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( svgPath, QgsProject::instance()->pathResolver() );
  const bool validSvg = QFileInfo::exists( resolvedPath );

  // draw red text for path field if invalid (path can't be resolved)
  mSvgPathLineEdit->setStyleSheet( QString( !validSvg ? "QLineEdit{ color: rgb(225, 0, 0); }" : "" ) );
  mSvgPathLineEdit->setToolTip( !validSvg ? tr( "File not found" ) : resolvedPath );

  drawNorthArrow();
}

void QgsDecorationNorthArrowDialog::drawNorthArrow()
{
  const int rotation = spinAngle->value();
  const double maxLength = 64;
  QSvgRenderer svg;

  const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( mDeco.svgPath(), maxLength, pbnChangeColor->color(), pbnChangeOutlineColor->color(), 1.0, 1.0 );
  svg.load( svgContent );

  if ( svg.isValid() )
  {
    QSize size( maxLength, maxLength );
    const QRectF viewBox = svg.viewBoxF();
    if ( viewBox.height() > viewBox.width() )
    {
      size.setWidth( maxLength * viewBox.width() / viewBox.height() );
    }
    else
    {
      size.setHeight( maxLength * viewBox.height() / viewBox.width() );
    }

    QPixmap  myPainterPixmap( maxLength, maxLength );
    myPainterPixmap.fill( Qt::transparent );

    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );

    myQPainter.setRenderHint( QPainter::SmoothPixmapTransform );

    const double centerXDouble = size.width() / 2.0;
    const double centerYDouble = size.height() / 2.0;
    //save the current canvas rotation
    myQPainter.save();
    myQPainter.translate( ( maxLength - size.width() ) / 2, ( maxLength - size.height() ) / 2 );

    //rotate the canvas
    myQPainter.rotate( rotation );
    //work out how to shift the image so that it appears in the center of the canvas
    //(x cos a + y sin a - x, -x sin a + y cos a - y)
    const double myRadiansDouble = ( M_PI / 180 ) * rotation;
    const int xShift = static_cast<int>( (
                                           ( centerXDouble * std::cos( myRadiansDouble ) ) +
                                           ( centerYDouble * std::sin( myRadiansDouble ) )
                                         ) - centerXDouble );
    const int yShift = static_cast<int>( (
                                           ( -centerXDouble * std::sin( myRadiansDouble ) ) +
                                           ( centerYDouble * std::cos( myRadiansDouble ) )
                                         ) - centerYDouble );

    //draw the pixmap in the proper position
    myQPainter.translate( xShift, yShift );
    svg.render( &myQPainter, QRectF( 0, 0, size.width(), size.height() ) );

    //unrotate the canvas again
    myQPainter.restore();
    myQPainter.end();

    pixmapLabel->setPixmap( myPainterPixmap );
  }
  else
  {
    QPixmap  myPainterPixmap( 200, 200 );
    myPainterPixmap.fill( Qt::transparent );
    QPainter myQPainter;
    myQPainter.begin( &myPainterPixmap );
    const QFont myQFont( QStringLiteral( "time" ), 12, QFont::Bold );
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

void QgsDecorationNorthArrowDialog::resizeEvent( QResizeEvent *resizeEvent )
{
  Q_UNUSED( resizeEvent )
  drawNorthArrow();
}
