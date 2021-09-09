/***************************************************************************
  qgsdecorationimagedialog.cpp
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdecorationimagedialog.h"
#include "qgsdecorationimage.h"
#include "qgsimagecache.h"
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

QgsDecorationImageDialog::QgsDecorationImageDialog( QgsDecorationImage &deco, QWidget *parent )
  : QDialog( parent )
  , mDeco( deco )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsDecorationImageDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsDecorationImageDialog::buttonBox_rejected );

  QPushButton *applyButton = buttonBox->button( QDialogButtonBox::Apply );
  connect( applyButton, &QAbstractButton::clicked, this, &QgsDecorationImageDialog::apply );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDecorationImageDialog::showHelp );

  spinSize->setValue( mDeco.mSize );

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
  connect( grpEnable, &QGroupBox::toggled, this, [ = ] { updateEnabledColorButtons(); } );

  wgtImagePath->setFilePath( mDeco.imagePath() );
  connect( wgtImagePath, &QgsFileWidget::fileChanged, this, &QgsDecorationImageDialog::updateImagePath );
  updateImagePath( mDeco.imagePath() );

  pbnChangeColor->setAllowOpacity( true );
  pbnChangeColor->setColor( mDeco.mColor );
  pbnChangeColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeColor->setColorDialogTitle( tr( "Select SVG Image Fill Color" ) );
  pbnChangeOutlineColor->setAllowOpacity( true );
  pbnChangeOutlineColor->setColor( mDeco.mOutlineColor );
  pbnChangeOutlineColor->setContext( QStringLiteral( "gui" ) );
  pbnChangeOutlineColor->setColorDialogTitle( tr( "Select SVG Image Outline Color" ) );
  connect( pbnChangeColor, &QgsColorButton::colorChanged, this, [ = ]( QColor ) { drawImage(); } );
  connect( pbnChangeOutlineColor, &QgsColorButton::colorChanged, this, [ = ]( QColor ) { drawImage(); } );

  drawImage();
}

void QgsDecorationImageDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#image" ) );
}

void QgsDecorationImageDialog::buttonBox_accepted()
{
  apply();
  accept();
}

void QgsDecorationImageDialog::buttonBox_rejected()
{
  reject();
}

void QgsDecorationImageDialog::apply()
{
  mDeco.mColor = pbnChangeColor->color();
  mDeco.mOutlineColor = pbnChangeOutlineColor->color();
  mDeco.mSize = spinSize->value();
  mDeco.setPlacement( static_cast< QgsDecorationItem::Placement>( cboPlacement->currentData().toInt() ) );
  mDeco.mMarginUnit = wgtUnitSelection->unit();
  mDeco.setEnabled( grpEnable->isChecked() );
  mDeco.mMarginHorizontal = spinHorizontal->value();
  mDeco.mMarginVertical = spinVertical->value();
  mDeco.update();
}

void QgsDecorationImageDialog::updateEnabledColorButtons()
{

  if ( mDeco.mImageFormat == QgsDecorationImage::FormatSVG )
  {
    QColor defaultFill, defaultStroke;
    double defaultStrokeWidth, defaultFillOpacity, defaultStrokeOpacity;
    bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultStrokeColor, hasDefaultStrokeWidth, hasDefaultStrokeOpacity;
    bool hasFillParam, hasFillOpacityParam, hasStrokeParam, hasStrokeWidthParam, hasStrokeOpacityParam;
    QgsApplication::svgCache()->containsParams( mDeco.imagePath(), hasFillParam, hasDefaultFillColor, defaultFill,
        hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
        hasStrokeParam, hasDefaultStrokeColor, defaultStroke,
        hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
        hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );

    pbnChangeColor->setEnabled( grpEnable->isChecked() && hasFillParam );
    pbnChangeColor->setAllowOpacity( hasFillOpacityParam );
    if ( hasFillParam )
    {
      QColor fill = hasDefaultFillColor ? defaultFill : pbnChangeColor->color();
      fill.setAlphaF( hasFillOpacityParam && hasDefaultFillOpacity ? defaultFillOpacity : 1.0 );
      pbnChangeColor->setColor( fill );
    }
    pbnChangeOutlineColor->setEnabled( grpEnable->isChecked() && hasStrokeParam );
    pbnChangeOutlineColor->setAllowOpacity( hasStrokeOpacityParam );
    if ( hasStrokeParam )
    {
      QColor stroke = hasDefaultStrokeColor ? defaultStroke : pbnChangeOutlineColor->color();
      stroke.setAlphaF( hasStrokeOpacityParam && hasDefaultStrokeOpacity ? defaultStrokeOpacity : 1.0 );
      pbnChangeOutlineColor->setColor( stroke );
    }
  }
  else
  {
    pbnChangeColor->setEnabled( false );
    pbnChangeOutlineColor->setEnabled( false );
  }
}

void QgsDecorationImageDialog::updateImagePath( const QString &imagePath )
{
  if ( mDeco.imagePath() != imagePath )
    mDeco.setImagePath( imagePath );

  updateEnabledColorButtons();
  drawImage();
}

void QgsDecorationImageDialog::drawImage()
{
  bool missing = false;
  const double maxLength = 64;
  QSize size( maxLength, maxLength );
  QSvgRenderer svg;
  QImage img;
  switch ( mDeco.mImageFormat )
  {
    case QgsDecorationImage::FormatSVG:
    {
      const QByteArray &svgContent = QgsApplication::svgCache()->svgContent( mDeco.imagePath(), maxLength, pbnChangeColor->color(), pbnChangeOutlineColor->color(), 1.0, 1.0 );
      svg.load( svgContent );

      if ( svg.isValid() )
      {
        const QRectF viewBox = svg.viewBoxF();
        if ( viewBox.height() > viewBox.width() )
        {
          size.setWidth( maxLength * viewBox.width() / viewBox.height() );
        }
        else
        {
          size.setHeight( maxLength * viewBox.height() / viewBox.width() );
        }
      }
      else
      {
        // SVG can't be parsed
        missing = true;
      }
      break;
    }

    case QgsDecorationImage::FormatRaster:
    {
      const QSize originalSize = QgsApplication::imageCache()->originalSize( mDeco.imagePath() );
      if ( originalSize.isValid() )
      {
        if ( originalSize.height() > originalSize.width() )
        {
          size.setWidth( originalSize.width() * maxLength / originalSize.height() );
          size.setHeight( maxLength );
        }
        else
        {
          size.setWidth( maxLength );
          size.setHeight( originalSize.height() * maxLength / originalSize.width() );
        }
        bool cached;
        img = QgsApplication::imageCache()->pathAsImage( mDeco.imagePath(), size, true, 1.0, cached );
      }
      else
      {
        // Image can't be read
        missing = true;
      }
      break;
    }

    case QgsDecorationImage::FormatUnknown:
      // Broken / unavailable image
      missing = true;
  }

  if ( !missing )
  {
    QPixmap  px( maxLength, maxLength );
    px.fill( Qt::transparent );

    QPainter painter;
    painter.begin( &px );
    painter.setRenderHint( QPainter::SmoothPixmapTransform );
    switch ( mDeco.mImageFormat )
    {
      case QgsDecorationImage::FormatSVG:
        svg.render( &painter, QRectF( 0, 0, size.width(), size.height() ) );
        break;
      case QgsDecorationImage::FormatRaster:
        painter.drawImage( QRectF( 0, 0, size.width(), size.height() ), img );
        break;
      case QgsDecorationImage::FormatUnknown:
        // Nothing to do here
        break;
    }
    painter.end();

    pixmapLabel->setPixmap( px );
  }
  else
  {
    QPixmap  px( 200, 200 );
    px.fill( Qt::transparent );
    QPainter painter;
    painter.begin( &px );
    const QFont font( QStringLiteral( "time" ), 12, QFont::Bold );
    painter.setFont( font );
    painter.setPen( Qt::red );
    painter.drawText( 10, 20, tr( "Pixmap not found" ) );
    painter.end();
    pixmapLabel->setPixmap( px );
  }
}

//
// Called when the widget has been resized.
//

void QgsDecorationImageDialog::resizeEvent( QResizeEvent *resizeEvent )
{
  Q_UNUSED( resizeEvent )
  drawImage();
}
