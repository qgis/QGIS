/***************************************************************************
    qgspenstylecombobox.cpp
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

#include "qgspenstylecombobox.h"

#include "qgsapplication.h"
#include "qgsguiutils.h"

#include <QList>
#include <QPair>

#include <QPainter>
#include <QPen>

QgsPenStyleComboBox::QgsPenStyleComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList < QPair<Qt::PenStyle, QString> > styles;
  styles << qMakePair( Qt::SolidLine, tr( "Solid Line" ) )
         << qMakePair( Qt::NoPen, tr( "No Pen" ) )
         << qMakePair( Qt::DashLine, tr( "Dash Line" ) )
         << qMakePair( Qt::DotLine, tr( "Dot Line" ) )
         << qMakePair( Qt::DashDotLine, tr( "Dash Dot Line" ) )
         << qMakePair( Qt::DashDotDotLine, tr( "Dash Dot Dot Line" ) );

  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  setIconSize( QSize( iconSize * 2, iconSize ) );

  for ( int i = 0; i < styles.count(); i++ )
  {
    const Qt::PenStyle style = styles.at( i ).first;
    const QString name = styles.at( i ).second;
    addItem( iconForPen( style ), name, QVariant( ( int ) style ) );
  }
}

Qt::PenStyle QgsPenStyleComboBox::penStyle() const
{
  return ( Qt::PenStyle ) currentData().toInt();
}

void QgsPenStyleComboBox::setPenStyle( Qt::PenStyle style )
{
  const int idx = findData( QVariant( ( int ) style ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}

QIcon QgsPenStyleComboBox::iconForPen( Qt::PenStyle style )
{
  QPixmap pix( iconSize() );
  QPainter p;
  pix.fill( Qt::transparent );

  p.begin( &pix );
  QPen pen( style );
  pen.setWidth( 2 );
  p.setPen( pen );
  const double mid = iconSize().height() / 2.0;
  p.drawLine( 0, mid, iconSize().width(), mid );
  p.end();

  return QIcon( pix );
}


/////////
// join

QgsPenJoinStyleComboBox::QgsPenJoinStyleComboBox( QWidget *parent )
  : QComboBox( parent )
{
  const QString path = QgsApplication::defaultThemePath();
  addItem( QIcon( path + "/join_bevel.svg" ), tr( "Bevel" ), QVariant( Qt::BevelJoin ) );
  addItem( QIcon( path + "/join_miter.svg" ), tr( "Miter" ), QVariant( Qt::MiterJoin ) );
  addItem( QIcon( path + "/join_round.svg" ), tr( "Round" ), QVariant( Qt::RoundJoin ) );
}

Qt::PenJoinStyle QgsPenJoinStyleComboBox::penJoinStyle() const
{
  return ( Qt::PenJoinStyle ) currentData().toInt();
}

void QgsPenJoinStyleComboBox::setPenJoinStyle( Qt::PenJoinStyle style )
{
  const int idx = findData( QVariant( style ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}


/////////
// cap

QgsPenCapStyleComboBox::QgsPenCapStyleComboBox( QWidget *parent )
  : QComboBox( parent )
{
  const QString path = QgsApplication::defaultThemePath();
  addItem( QIcon( path + "/cap_square.svg" ), tr( "Square" ), QVariant( Qt::SquareCap ) );
  addItem( QIcon( path + "/cap_flat.svg" ), tr( "Flat" ), QVariant( Qt::FlatCap ) );
  addItem( QIcon( path + "/cap_round.svg" ), tr( "Round" ), QVariant( Qt::RoundCap ) );
}

Qt::PenCapStyle QgsPenCapStyleComboBox::penCapStyle() const
{
  return ( Qt::PenCapStyle ) currentData().toInt();
}

void QgsPenCapStyleComboBox::setPenCapStyle( Qt::PenCapStyle style )
{
  const int idx = findData( QVariant( style ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}
