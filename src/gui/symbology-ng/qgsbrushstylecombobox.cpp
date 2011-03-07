
#include "qgsbrushstylecombobox.h"

#include <QList>
#include <QPair>

#include <QBrush>
#include <QPainter>
#include <QPen>

QgsBrushStyleComboBox::QgsBrushStyleComboBox( QWidget* parent )
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

  setIconSize( QSize( 32, 16 ) );

  for ( int i = 0; i < styles.count(); i++ )
  {
    Qt::BrushStyle style = styles.at( i ).first;
    QString name = styles.at( i ).second;
    addItem( iconForBrush( style ), name, QVariant( style ) );
  }

  setCurrentIndex( 1 );

}


Qt::BrushStyle QgsBrushStyleComboBox::brushStyle() const
{
  return ( Qt::BrushStyle ) itemData( currentIndex() ).toInt();
}

void QgsBrushStyleComboBox::setBrushStyle( Qt::BrushStyle style )
{
  int idx = findData( QVariant( style ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}

QIcon QgsBrushStyleComboBox::iconForBrush( Qt::BrushStyle style )
{
  QPixmap pix( iconSize() );
  QPainter p;
  pix.fill( Qt::transparent );

  p.begin( &pix );
  QBrush brush( QColor( 100, 100, 100 ), style );
  p.setBrush( brush );
  QPen pen( Qt::NoPen );
  p.setPen( pen );
  p.drawRect( QRect( QPoint( 0, 0 ), iconSize() ) );
  p.end();

  return QIcon( pix );
}
