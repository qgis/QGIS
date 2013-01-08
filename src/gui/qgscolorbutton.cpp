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
#include <QPainter>

/*!
  \class QgsColorButton

  \brief The QgsColorButton class provides a tool button widget displaying
  a color which can be altered by calling QColorDialog::getColor.

  A subclass of QToolButton is needed to draw the button content because
  some platforms such as Mac OS X and Windows XP enforce a consistent
  GUI look by always using the button color of the current style and
  not allowing button backgrounds to be changed on a button by button basis.

  This class is a simplified version of QtColorButton, an internal class used
  by Qt Designer to do the same thing.
*/

QgsColorButton::QgsColorButton( QWidget *parent, QString cdt, QColorDialog::ColorDialogOptions cdo )
    : QToolButton( parent )
    , mColorDialogTitle( cdt )
    , mColorDialogOptions( cdo )
{
  setToolButtonStyle( Qt::ToolButtonTextOnly ); // decrease default button height
  connect( this, SIGNAL( clicked() ), this, SLOT( onButtonClicked() ) );
}

QgsColorButton::~QgsColorButton()
{}

void QgsColorButton::onButtonClicked()
{
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( color(), 0, mColorDialogTitle, mColorDialogOptions );
#else
  QColor newColor = QColorDialog::getColor( color() );
#endif
  if ( newColor.isValid() )
  {
    setColor( newColor );
  }
}

/*!
  Paints button in response to a paint event.
*/
void QgsColorButton::paintEvent( QPaintEvent *e )
{
  QToolButton::paintEvent( e );
  if (
#ifdef Q_WS_MAC
    // Mac shows color only a when a window is active
    isActiveWindow() &&
#endif
    isEnabled() )
  {
    QPainter p( this );
    int margin = 2;  // Leave some space for highlighting
    QRect r = rect().adjusted( margin, margin, -margin, -margin );
    p.fillRect( r, mColor );
  }
}

void QgsColorButton::setColor( const QColor &color )
{
  QColor oldColor = mColor;

  mColor = color;
  update();

  if ( oldColor != mColor )
  {
    emit( colorChanged( mColor ) );
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

//////////////////

QgsColorButtonV2::QgsColorButtonV2( QWidget* parent )
    : QPushButton( parent )
{
}

QgsColorButtonV2::QgsColorButtonV2( QString text, QWidget* parent )
    : QPushButton( text, parent )
{
}

void QgsColorButtonV2::setColor( const QColor &color )
{
  mColor = color;

  QPixmap pixmap( iconSize() );
  pixmap.fill( QColor( 0, 0, 0, 0 ) );

  QRect rect( 1, 1, iconSize().width() - 2, iconSize().height() - 2 );

  // draw a slightly rounded rectangle
  QPainter p;
  p.begin( &pixmap );
  p.setPen( Qt::NoPen );
  p.setRenderHint( QPainter::Antialiasing );
  p.setBrush( color );
  p.drawRoundedRect( rect, 4, 4 );
  p.end();

  // set this pixmap as icon
  setIcon( QIcon( pixmap ) );
}
