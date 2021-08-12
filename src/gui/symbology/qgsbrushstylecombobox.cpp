/***************************************************************************
    qgsbrushstylecombobox.cpp
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

#include "qgsbrushstylecombobox.h"
#include "qgsguiutils.h"

#include <QList>
#include <QPair>

#include <QBrush>
#include <QPainter>
#include <QPen>

QgsBrushStyleComboBox::QgsBrushStyleComboBox( QWidget *parent )
  : QComboBox( parent )
{
  QList < QPair<Qt::BrushStyle, QString> > styles;
  styles << qMakePair( Qt::SolidPattern, tr( "Solid" ) )
         << qMakePair( Qt::NoBrush, tr( "No Brush" ) )
         << qMakePair( Qt::HorPattern, tr( "Horizontal" ) )
         << qMakePair( Qt::VerPattern, tr( "Vertical" ) )
         << qMakePair( Qt::CrossPattern, tr( "Cross" ) )
         << qMakePair( Qt::BDiagPattern, tr( "BDiagonal" ) )
         << qMakePair( Qt::FDiagPattern, tr( "FDiagonal" ) )
         << qMakePair( Qt::DiagCrossPattern, tr( "Diagonal X" ) )
         << qMakePair( Qt::Dense1Pattern, tr( "Dense 1" ) )
         << qMakePair( Qt::Dense2Pattern, tr( "Dense 2" ) )
         << qMakePair( Qt::Dense3Pattern, tr( "Dense 3" ) )
         << qMakePair( Qt::Dense4Pattern, tr( "Dense 4" ) )
         << qMakePair( Qt::Dense5Pattern, tr( "Dense 5" ) )
         << qMakePair( Qt::Dense6Pattern, tr( "Dense 6" ) )
         << qMakePair( Qt::Dense7Pattern, tr( "Dense 7" ) );

  const int iconSize = QgsGuiUtils::scaleIconSize( 16 );
  setIconSize( QSize( iconSize * 2, iconSize ) );

  for ( int i = 0; i < styles.count(); i++ )
  {
    const Qt::BrushStyle style = styles.at( i ).first;
    const QString name = styles.at( i ).second;
    addItem( iconForBrush( style ), name, QVariant( static_cast<int>( style ) ) );
  }

  setCurrentIndex( 1 );

}


Qt::BrushStyle QgsBrushStyleComboBox::brushStyle() const
{
  return ( Qt::BrushStyle ) currentData().toInt();
}

void QgsBrushStyleComboBox::setBrushStyle( Qt::BrushStyle style )
{
  const int idx = findData( QVariant( static_cast<int>( style ) ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}

QIcon QgsBrushStyleComboBox::iconForBrush( Qt::BrushStyle style )
{
  QPixmap pix( iconSize() );
  QPainter p;
  pix.fill( Qt::transparent );

  p.begin( &pix );
  const QBrush brush( QColor( 100, 100, 100 ), style );
  p.setBrush( brush );
  const QPen pen( Qt::NoPen );
  p.setPen( pen );
  p.drawRect( QRect( QPoint( 0, 0 ), iconSize() ) );
  p.end();

  return QIcon( pix );
}
