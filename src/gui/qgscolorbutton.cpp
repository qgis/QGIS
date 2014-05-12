/***************************************************************************
    qgscolorbutton.cpp - Button which displays a color
     --------------------------------------
    Date                 : 12-Dec-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorbutton.h"
#include "qgscolordialog.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssymbollayerv2utils.h"

#include <QPainter>
#include <QSettings>
#include <QTemporaryFile>
#include <QMouseEvent>
#include <QMenu>
#include <QClipboard>

/*!
  \class QgsColorButton

  \brief A cross platform button subclass for selecting colors. Will open a color chooser dialog when clicked.
  Offers live updates to button from color chooser dialog

  A subclass of QPushButton is needed to draw the button content because
  some platforms such as Mac OS X and Windows XP enforce a consistent
  GUI look by always using the button color of the current style and
  not allowing button backgrounds to be changed on a button by button basis.
  Therefore, a wholely stylesheet-based button is used for the no-text variant.

  This class is a simplified version of QtColorButton, an internal class used
  by Qt Designer to do the same thing.
*/

QgsColorButton::QgsColorButton( QWidget *parent, QString cdt, QColorDialog::ColorDialogOptions cdo )
    : QPushButton( parent )
    , mColorDialogTitle( cdt.isEmpty() ? tr( "Select Color" ) : cdt )
    , mColor( Qt::black )
    , mColorDialogOptions( cdo )
    , mAcceptLiveUpdates( true )
    , mTempPNG( NULL )
    , mColorSet( false )
{
  connect( this, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
}

QgsColorButton::~QgsColorButton()
{
  if ( mTempPNG.exists() )
    mTempPNG.remove();
}

const QPixmap& QgsColorButton::transpBkgrd()
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

void QgsColorButton::onButtonClicked()
{
  //QgsDebugMsg( "entered" );
  QColor newColor;
  QSettings settings;
  if ( mAcceptLiveUpdates && settings.value( "/qgis/live_color_dialogs", false ).toBool() )
  {
    newColor = QgsColorDialog::getLiveColor(
                 color(), this, SLOT( setValidColor( const QColor& ) ),
                 this->parentWidget(), mColorDialogTitle, mColorDialogOptions );
  }
  else
  {
    newColor = QColorDialog::getColor( color(), this->parentWidget(), mColorDialogTitle, mColorDialogOptions );
  }
  setValidColor( newColor );

  // reactivate button's window
  activateWindow();
}

void QgsColorButton::mousePressEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    showContextMenu( e );
  }
  else
  {
    QPushButton::mousePressEvent( e );
  }
}

void QgsColorButton::showContextMenu( QMouseEvent *event )
{
  QMenu colorContextMenu;

  QAction* copyAsHexAction = new QAction( tr( "Copy color" ), 0 );
  colorContextMenu.addAction( copyAsHexAction );
  QAction* copyAsRgbAction = new QAction( tr( "Copy as rgb" ), 0 );
  colorContextMenu.addAction( copyAsRgbAction );
  QAction* copyAsRgbaAction = new QAction( tr( "Copy as rgba" ), 0 );
  if ( mColorDialogOptions & QColorDialog::ShowAlphaChannel )
  {
    //alpha enabled, so add rgba action
    colorContextMenu.addAction( copyAsRgbaAction );
  }

  QString clipboardText = QApplication::clipboard()->text();
  QAction* pasteColorAction = new QAction( tr( "Paste color" ), 0 );
  pasteColorAction->setEnabled( false );
  colorContextMenu.addSeparator();
  colorContextMenu.addAction( pasteColorAction );
  QColor clipColor;
  if ( !( clipboardText.isEmpty() ) )
  {
    bool hasAlpha = false;
    clipColor = QgsSymbolLayerV2Utils::parseColorWithAlpha( clipboardText, hasAlpha );

    if ( clipColor.isValid() )
    {
      if ( !hasAlpha )
      {
        //clipboard color has no explicit alpha component, so keep existing alpha
        clipColor.setAlpha( mColor.alpha() );
      }
      pasteColorAction->setEnabled( true );
    }
  }

  QAction* selectedAction = colorContextMenu.exec( event->globalPos( ) );
  if ( selectedAction == copyAsHexAction )
  {
    //copy color as hex code
    QString colorString = mColor.name();
    QApplication::clipboard()->setText( colorString );
  }
  else if ( selectedAction == copyAsRgbAction )
  {
    //copy color as rgb
    QString colorString = QString( "rgb(%1,%2,%3)" ).arg( mColor.red() ). arg( mColor.green() ).arg( mColor.blue() );
    QApplication::clipboard()->setText( colorString );
  }
  else if ( selectedAction == copyAsRgbaAction )
  {
    //copy color as rgba
    QString colorString = QString( "rgba(%1,%2,%3,%4)" ).arg( mColor.red() ).arg( mColor.green() ).arg( mColor.blue() ).arg( QString::number( mColor.alphaF(), 'f', 2 ) );
    QApplication::clipboard()->setText( colorString );
  }
  else if ( selectedAction == pasteColorAction )
  {
    //paste color
    setColor( clipColor );
  }

  delete copyAsHexAction;
  delete copyAsRgbAction;
  delete copyAsRgbaAction;
  delete pasteColorAction;
}

void QgsColorButton::setValidColor( const QColor& newColor )
{
  if ( newColor.isValid() )
  {
    setColor( newColor );
  }
}

void QgsColorButton::changeEvent( QEvent* e )
{
  if ( e->type() == QEvent::EnabledChange )
  {
    setButtonBackground();
  }
  QPushButton::changeEvent( e );
}

#if 0 // causes too many cyclical updates, but may be needed on some platforms
void QgsColorButton::paintEvent( QPaintEvent* e )
{
  QPushButton::paintEvent( e );

  if ( !mBackgroundSet )
  {
    setButtonBackground();
  }
}
#endif

void QgsColorButton::showEvent( QShowEvent* e )
{
  setButtonBackground();
  QPushButton::showEvent( e );
}

void QgsColorButton::setColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }
  QColor oldColor = mColor;
  mColor = color;

  // handle when initially set color is same as default (Qt::black); consider it a color change
  if ( oldColor != mColor || ( mColor == QColor( Qt::black ) && !mColorSet ) )
  {
    setButtonBackground();
    if ( isEnabled() )
    {
      // TODO: May be beneficial to have the option to set color without emitting this signal.
      //       Now done by blockSignals( bool ) where button is used
      emit colorChanged( mColor );
    }
  }
  mColorSet = true;
}

void QgsColorButton::setButtonBackground()
{
  if ( !text().isEmpty() )
  {
    // generate icon pixmap for regular pushbutton
    setFlat( false );

    QPixmap pixmap;
    pixmap = QPixmap( iconSize() );
    pixmap.fill( QColor( 0, 0, 0, 0 ) );

    int iconW = iconSize().width();
    int iconH = iconSize().height();
    QRect rect( 0, 0, iconW, iconH );

    // QPainterPath::addRoundRect has flaws, draw chamfered corners instead
    QPainterPath roundRect;
    int chamfer = 3;
    int inset = 1;
    roundRect.moveTo( chamfer, inset );
    roundRect.lineTo( iconW - chamfer, inset );
    roundRect.lineTo( iconW - inset, chamfer );
    roundRect.lineTo( iconW - inset, iconH - chamfer );
    roundRect.lineTo( iconW - chamfer, iconH - inset );
    roundRect.lineTo( chamfer, iconH - inset );
    roundRect.lineTo( inset, iconH - chamfer );
    roundRect.lineTo( inset, chamfer );
    roundRect.closeSubpath();

    QPainter p;
    p.begin( &pixmap );
    p.setRenderHint( QPainter::Antialiasing );
    p.setClipPath( roundRect );
    p.setPen( Qt::NoPen );
    if ( mColor.alpha() < 255 )
    {
      p.drawTiledPixmap( rect, transpBkgrd() );
    }
    p.setBrush( mColor );
    p.drawRect( rect );
    p.end();

    // set this pixmap as icon
    setIcon( QIcon( pixmap ) );
  }
  else
  {
    // generate temp background image file with checkerboard canvas to be used via stylesheet

    // set flat, or inline spacing (widget margins) needs to be manually calculated and set
    setFlat( true );

    bool useAlpha = ( mColorDialogOptions & QColorDialog::ShowAlphaChannel );

    // in case margins need to be adjusted
    QString margin = QString( "%1px %2px %3px %4px" ).arg( 0 ).arg( 0 ).arg( 0 ).arg( 0 );

    //QgsDebugMsg( QString( "%1 margin: %2" ).arg( objectName() ).arg( margin ) );

    QString bkgrd = QString( " background-color: rgba(%1,%2,%3,%4);" )
                    .arg( mColor.red() )
                    .arg( mColor.green() )
                    .arg( mColor.blue() )
                    .arg( useAlpha ? mColor.alpha() : 255 );

    if ( useAlpha && mColor.alpha() < 255 )
    {
      QPixmap pixmap = transpBkgrd();
      QRect rect( 0, 0, pixmap.width(), pixmap.height() );

      QPainter p;
      p.begin( &pixmap );
      p.setRenderHint( QPainter::Antialiasing );
      p.setPen( Qt::NoPen );
      p.setBrush( mColor );
      p.drawRect( rect );
      p.end();

      if ( mTempPNG.open() )
      {
        mTempPNG.setAutoRemove( false );
        pixmap.save( mTempPNG.fileName(), "PNG" );
        mTempPNG.close();
      }

      bkgrd = QString( " background-image: url(%1);" ).arg( mTempPNG.fileName() );
    }

    //QgsDebugMsg( QString( "%1" ).arg( bkgrd ) );

    // TODO: get OS-style focus color and switch border to that color when button in focus
    setStyleSheet( QString( "QgsColorButton{"
                            " %1"
                            " background-position: top left;"
                            " background-origin: content;"
                            " background-clip: content;"
                            " padding: 2px;"
                            " margin: %2;"
                            " outline: none;"
                            " border-style: %4;"
                            " border-width: 1px;"
                            " border-color: rgb(%3,%3,%3);"
                            " border-radius: 3px;} "
                            "QgsColorButton:pressed{"
                            " %1"
                            " background-position: top left;"
                            " background-origin: content;"
                            " background-clip: content;"
                            " padding: 1px;"
                            " margin: %2;"
                            " outline: none;"
                            " border-style: inset;"
                            " border-width: 2px;"
                            " border-color: rgb(128,128,128);"
                            " border-radius: 4px;} " )
                   .arg( bkgrd )
                   .arg( margin )
                   .arg( isEnabled() ? "128" : "110" )
                   .arg( isEnabled() ? "outset" : "dotted" ) );
  }
}

QColor QgsColorButton::color() const
{
  return mColor;
}

void QgsColorButton::setColorDialogOptions( QColorDialog::ColorDialogOptions cdo )
{
  mColorDialogOptions = cdo;
}

QColorDialog::ColorDialogOptions QgsColorButton::colorDialogOptions()
{
  return mColorDialogOptions;
}

void QgsColorButton::setColorDialogTitle( QString cdt )
{
  mColorDialogTitle = cdt;
}

QString QgsColorButton::colorDialogTitle()
{
  return mColorDialogTitle;
}
