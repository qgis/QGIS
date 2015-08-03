/***************************************************************************
                               qgsnodeeditor.cpp
                               -----------------
        begin                : Tue Mar 24 2015
        copyright            : (C) 2015 Sandro Mani / Sourcepole AG
        email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnodeeditor.h"
#include "qgsmapcanvas.h"
#include "qgsselectedfeature.h"
#include "qgsvertexentry.h"
#include "qgsvectorlayer.h"
#include "qgsgeometryutils.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QVector2D>

static const int MinRadiusRole = Qt::UserRole + 1;


class CoordinateItemDelegate : public QStyledItemDelegate
{
  public:

    QString displayText( const QVariant & value, const QLocale & locale ) const override
    {
      return locale.toString( value.toDouble(), 'f', 4 );
    }

  protected:

    QWidget* createEditor( QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index ) const override
    {
      QLineEdit* lineEdit = new QLineEdit( parent );
      QDoubleValidator* validator = new QDoubleValidator();
      if ( !index.data( MinRadiusRole ).isNull() )
        validator->setBottom( index.data( MinRadiusRole ).toDouble() );
      lineEdit->setValidator( validator );
      return lineEdit;
    }

    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override
    {
      QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor );
      if ( lineEdit->hasAcceptableInput() )
      {
        QStyledItemDelegate::setModelData( editor, model, index );
      }
    }
};


QgsNodeEditor::QgsNodeEditor(
  QgsVectorLayer *layer,
  QgsSelectedFeature *selectedFeature,
  QgsMapCanvas *canvas )
{
  setWindowTitle( tr( "Vertex editor" ) );
  setFeatures( features() ^ QDockWidget::DockWidgetClosable );

  mLayer = layer;
  mSelectedFeature = selectedFeature;
  mCanvas = canvas;

  mTableWidget = new QTableWidget( 0, 6, this );
  mTableWidget->setHorizontalHeaderLabels( QStringList() << "id" << "x" << "y" << "z" << "m" << "r" );
  mTableWidget->setSelectionMode( QTableWidget::ExtendedSelection );
  mTableWidget->setSelectionBehavior( QTableWidget::SelectRows );
  mTableWidget->verticalHeader()->hide();
  mTableWidget->horizontalHeader()->setResizeMode( 1, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 2, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 3, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 4, QHeaderView::Stretch );
  mTableWidget->horizontalHeader()->setResizeMode( 5, QHeaderView::Stretch );
  mTableWidget->setItemDelegateForColumn( 1, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 2, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 3, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 4, new CoordinateItemDelegate() );
  mTableWidget->setItemDelegateForColumn( 5, new CoordinateItemDelegate() );

  setWidget( mTableWidget );

  connect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );
  connect( mSelectedFeature, SIGNAL( vertexMapChanged() ), this, SLOT( rebuildTable() ) );
  connect( mTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( updateNodeSelection() ) );
  connect( mTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( tableValueChanged( int, int ) ) );

  rebuildTable();
}

void QgsNodeEditor::rebuildTable()
{
  QFont curvePointFont = mTableWidget->font();
  curvePointFont.setItalic( true );

  mTableWidget->blockSignals( true );
  mTableWidget->setRowCount( 0 );
  int row = 0;
  bool hasR = false;
  foreach ( const QgsVertexEntry* entry, mSelectedFeature->vertexMap() )
  {
    mTableWidget->insertRow( row );

    QTableWidgetItem* idItem = new QTableWidgetItem();
    idItem->setData( Qt::DisplayRole, row );
    idItem->setFlags( idItem->flags() ^ Qt::ItemIsEditable );
    mTableWidget->setItem( row, 0, idItem );

    QTableWidgetItem* xItem = new QTableWidgetItem();
    xItem->setData( Qt::EditRole, entry->point().x() );
    mTableWidget->setItem( row, 1, xItem );

    QTableWidgetItem* yItem = new QTableWidgetItem();
    yItem->setData( Qt::EditRole, entry->point().y() );
    mTableWidget->setItem( row, 2, yItem );

    QTableWidgetItem* zItem = new QTableWidgetItem();
    zItem->setData( Qt::EditRole, entry->point().z() );
    mTableWidget->setItem( row, 3, zItem );

    QTableWidgetItem* mItem = new QTableWidgetItem();
    mItem->setData( Qt::EditRole, entry->point().m() );
    mTableWidget->setItem( row, 4, mItem );

    QTableWidgetItem* rItem = new QTableWidgetItem();
    mTableWidget->setItem( row, 5, rItem );

    bool curvePoint = ( entry->vertexId().type == QgsVertexId::CurveVertex );
    if ( curvePoint )
    {
      idItem->setFont( curvePointFont );
      xItem->setFont( curvePointFont );
      yItem->setFont( curvePointFont );
      zItem->setFont( curvePointFont );
      mItem->setFont( curvePointFont );
      rItem->setFont( curvePointFont );

      const QgsPointV2& p1 = mSelectedFeature->vertexMap()[row - 1]->point();
      const QgsPointV2& p2 = mSelectedFeature->vertexMap()[row]->point();
      const QgsPointV2& p3 = mSelectedFeature->vertexMap()[row + 1]->point();

      double r, cx, cy;
      QgsGeometryUtils::circleCenterRadius( p1, p2, p3, r, cx, cy );
      rItem->setData( Qt::EditRole, r );

      double x13 = p3.x() - p1.x(), y13 = p3.y() - p1.y();
      rItem->setData( MinRadiusRole, 0.5 * qSqrt( x13 * x13 + y13 * y13 ) );

      hasR = true;
    }
    else
    {
      rItem->setFlags( rItem->flags() & ~( Qt::ItemIsSelectable | Qt::ItemIsEnabled ) );
    }

    ++row;
  }
  mTableWidget->setColumnHidden( 3, !mSelectedFeature->vertexMap()[0]->point().is3D() );
  mTableWidget->setColumnHidden( 4, !mSelectedFeature->vertexMap()[0]->point().isMeasure() );
  mTableWidget->setColumnHidden( 5, !hasR );
  mTableWidget->resizeColumnToContents( 0 );
  mTableWidget->blockSignals( false );
}

void QgsNodeEditor::tableValueChanged( int row, int col )
{
  int nodeIdx = mTableWidget->item( row, 0 )->data( Qt::DisplayRole ).toInt();
  double x, y;
  if ( col == 5 ) // radius modified
  {
    double r = mTableWidget->item( row, 5 )->data( Qt::EditRole ).toDouble();
    double x1 = mTableWidget->item( row - 1, 1 )->data( Qt::EditRole ).toDouble();
    double y1 = mTableWidget->item( row - 1, 2 )->data( Qt::EditRole ).toDouble();
    double x2 = mTableWidget->item( row    , 1 )->data( Qt::EditRole ).toDouble();
    double y2 = mTableWidget->item( row    , 2 )->data( Qt::EditRole ).toDouble();
    double x3 = mTableWidget->item( row + 1, 1 )->data( Qt::EditRole ).toDouble();
    double y3 = mTableWidget->item( row + 1, 2 )->data( Qt::EditRole ).toDouble();

    QgsPointV2 result;
    QgsGeometryUtils::segmentMidPoint( QgsPointV2( x1, y1 ), QgsPointV2( x3, y3 ), result, r, QgsPointV2( x2, y2 ) );
    x = result.x();
    y = result.y();
  }
  else
  {
    x = mTableWidget->item( row, 1 )->data( Qt::EditRole ).toDouble();
    y = mTableWidget->item( row, 2 )->data( Qt::EditRole ).toDouble();
  }
  double z = mTableWidget->item( row, 3 )->data( Qt::EditRole ).toDouble();
  double m = mTableWidget->item( row, 4 )->data( Qt::EditRole ).toDouble();
  QgsPointV2 p( QgsWKBTypes::PointZM, x, y, z, m );

  mLayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  mLayer->moveVertex( p, mSelectedFeature->featureId(), nodeIdx );
  mLayer->endEditCommand();
  mCanvas->refresh();
}

void QgsNodeEditor::updateTableSelection()
{
  mTableWidget->blockSignals( true );
  mTableWidget->clearSelection();
  const QList<QgsVertexEntry*>& vertexMap = mSelectedFeature->vertexMap();
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      mTableWidget->selectRow( i );
    }
  }
  mTableWidget->blockSignals( false );
}

void QgsNodeEditor::updateNodeSelection()
{
  disconnect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );

  mSelectedFeature->deselectAllVertexes();
  foreach ( const QModelIndex& index, mTableWidget->selectionModel()->selectedRows() )
  {
    int nodeIdx = mTableWidget->item( index.row(), 0 )->data( Qt::DisplayRole ).toInt();
    mSelectedFeature->selectVertex( nodeIdx );
  }

  connect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );
}
