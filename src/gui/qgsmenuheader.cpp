/***************************************************************************
                             qgsmenuheader.cpp
                             -----------------
    begin                : June 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmenuheader.h"
#include "qgis.h"
#include <QPainter>
#include <QApplication>

QgsMenuHeader::QgsMenuHeader( const QString &text, QWidget *parent )
  : QWidget( parent )
  , mText( text )
{
  int textMinWidth = fontMetrics().boundingRect( mText ).width();
  mTextHeight = fontMetrics().height();
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  mLabelMargin = Qgis::UI_SCALE_FACTOR * fontMetrics().width( QStringLiteral( "." ) );
#else
  mLabelMargin = Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( '.' );
#endif
  mMinWidth = 2 * mLabelMargin + textMinWidth;
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  updateGeometry();
}

QSize QgsMenuHeader::minimumSizeHint() const
{
  return QSize( mMinWidth, mTextHeight + mLabelMargin );
}

QSize QgsMenuHeader::sizeHint() const
{
  return QSize( mMinWidth, mTextHeight + mLabelMargin );
}

void QgsMenuHeader::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  QPalette pal = QPalette( qApp->palette() );
  QColor headerBgColor = pal.color( QPalette::Mid );
  QColor headerTextColor = pal.color( QPalette::BrightText );

  //draw header background
  painter.setBrush( headerBgColor );
  painter.setPen( Qt::NoPen );
  painter.drawRect( QRect( 0, 0, width(), mTextHeight + mLabelMargin ) );

  //draw header text
  painter.setPen( headerTextColor );
  painter.drawText( QPoint( mLabelMargin, 0.25 * mLabelMargin + mTextHeight ), mText );
  painter.end();
}

QgsMenuHeaderWidgetAction::QgsMenuHeaderWidgetAction( const QString &text, QObject *parent )
  : QWidgetAction( parent )
{
  QgsMenuHeader *w = new QgsMenuHeader( text, nullptr );
  setDefaultWidget( w ); //transfers ownership
}
