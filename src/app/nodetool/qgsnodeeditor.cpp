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
#include <QKeyEvent>
#include <QLineEdit>
#include <QVector2D>

static const int MinRadiusRole = Qt::UserRole + 1;


QgsNodeEditorModel::QgsNodeEditorModel( QgsVectorLayer* layer, QgsSelectedFeature* selectedFeature, QgsMapCanvas* canvas, QObject* parent )
    : QAbstractTableModel( parent )
    , mLayer( layer )
    , mSelectedFeature( selectedFeature )
    , mCanvas( canvas )
    , mHasZ( false )
    , mHasM( false )
    , mHasR( true ) //always show for now - avoids scanning whole feature for curves TODO - avoid this
    , mZCol( -1 )
    , mMCol( -1 )
    , mRCol( -1 )
{

  if ( !mSelectedFeature->vertexMap().isEmpty() )
  {
    mHasZ = mSelectedFeature->vertexMap().at( 0 )->point().is3D();
    mHasM = mSelectedFeature->vertexMap().at( 0 )->point().isMeasure();

    if ( mHasZ )
      mZCol = 2;

    if ( mHasM )
      mMCol = 2 + ( mHasZ ? 1 : 0 );

    if ( mHasR )
      mRCol = 2 + ( mHasZ ? 1 : 0 ) + ( mHasM ? 1 : 0 );
  }

  QWidget* parentWidget = dynamic_cast< QWidget* >( parent );
  if ( parentWidget )
  {
    mWidgetFont = parentWidget->font();
  }

  connect( mSelectedFeature, SIGNAL( vertexMapChanged() ), this, SLOT( featureChanged() ) );
}

int QgsNodeEditorModel::rowCount( const QModelIndex& parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mSelectedFeature->vertexMap().count();
}

int QgsNodeEditorModel::columnCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent );
  return 2 + ( mHasZ ? 1 : 0 ) + ( mHasM ? 1 : 0 ) + ( mHasR ? 1 : 0 );
}

QVariant QgsNodeEditorModel::data( const QModelIndex& index, int role ) const
{
  if ( !index.isValid() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole && role != MinRadiusRole && role != Qt::FontRole ) )
    return QVariant();

  if ( index.row() >= mSelectedFeature->vertexMap().count() )
    return QVariant();

  if ( index.column() >= columnCount() )
    return QVariant();

  //get QgsVertexEntry for row
  const QgsVertexEntry* vertex = mSelectedFeature->vertexMap().at( index.row() );
  if ( !vertex )
  {
    return QVariant();
  }

  if ( role == Qt::FontRole )
  {
    double r = 0;
    double minRadius = 0;
    if ( calcR( index.row(), r, minRadius ) )
    {
      QFont curvePointFont = mWidgetFont;
      curvePointFont.setItalic( true );
      return curvePointFont;
    }
    else
    {
      return QVariant();
    }
  }

  if ( role == MinRadiusRole )
  {
    if ( index.column() == mRCol )
    {
      double r = 0;
      double minRadius = 0;
      if ( calcR( index.row(), r, minRadius ) )
      {
        return minRadius;
      }
    }
    return QVariant();
  }

  if ( index.column() == 0 )
    return vertex->point().x();
  else if ( index.column() == 1 )
    return vertex->point().y();
  else if ( index.column() == mZCol )
    return vertex->point().z();
  else if ( index.column() ==  mMCol )
    return vertex->point().m();
  else if ( index.column() == mRCol )
  {
    double r = 0;
    double minRadius = 0;
    if ( calcR( index.row(), r, minRadius ) )
    {
      return r;
    }
    return QVariant();
  }
  else
  {
    return QVariant();
  }

}

QVariant QgsNodeEditorModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole )
  {
    if ( orientation == Qt::Vertical ) //row
    {
      return QVariant( section );
    }
    else
    {
      if ( section == 0 )
        return QVariant( tr( "x" ) );
      else if ( section == 1 )
        return QVariant( tr( "y" ) );
      else if ( section == mZCol )
        return QVariant( tr( "z" ) );
      else if ( section ==  mMCol )
        return QVariant( tr( "m" ) );
      else if ( section == mRCol )
        return QVariant( tr( "r" ) );
      else
        return QVariant();
    }
  }
  else
  {
    return QVariant();
  }
}

bool QgsNodeEditorModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mSelectedFeature->vertexMap().count() )
  {
    return false;
  }

  double x = ( index.column() == 0 ? value.toDouble() : mSelectedFeature->vertexMap().at( index.row() )->point().x() );
  double y = ( index.column() == 1 ? value.toDouble() : mSelectedFeature->vertexMap().at( index.row() )->point().y() );

  if ( index.column() == mRCol ) // radius modified
  {
    if ( index.row() == 0 || index.row() >= mSelectedFeature->vertexMap().count() - 1 )
      return false;

    double r = value.toDouble();
    double x1 = mSelectedFeature->vertexMap().at( index.row() - 1 )->point().x();
    double y1 = mSelectedFeature->vertexMap().at( index.row() - 1 )->point().y();
    double x2 = x;
    double y2 = y;
    double x3 = mSelectedFeature->vertexMap().at( index.row() + 1 )->point().x();
    double y3 = mSelectedFeature->vertexMap().at( index.row() + 1 )->point().y();

    QgsPointV2 result;
    if ( QgsGeometryUtils::segmentMidPoint( QgsPointV2( x1, y1 ), QgsPointV2( x3, y3 ), result, r, QgsPointV2( x2, y2 ) ) )
    {
      x = result.x();
      y = result.y();
    }
  }
  double z = ( index.column() == mZCol ? value.toDouble() : mSelectedFeature->vertexMap().at( index.row() )->point().z() );
  double m = ( index.column() == mMCol ? value.toDouble() : mSelectedFeature->vertexMap().at( index.row() )->point().m() );
  QgsPointV2 p( QgsWKBTypes::PointZM, x, y, z, m );

  mLayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  mLayer->moveVertex( p, mSelectedFeature->featureId(), index.row() );
  mLayer->endEditCommand();
  mLayer->triggerRepaint();

  return false;
}

Qt::ItemFlags QgsNodeEditorModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

bool QgsNodeEditorModel::calcR( int row, double& r, double& minRadius ) const
{
  if ( row <= 0 || row >= mSelectedFeature->vertexMap().count() - 1 )
    return false;

  const QgsVertexEntry* entry = mSelectedFeature->vertexMap().at( row );

  bool curvePoint = ( entry->vertexId().type == QgsVertexId::CurveVertex );
  if ( !curvePoint )
    return false;

  const QgsPointV2& p1 = mSelectedFeature->vertexMap().at( row - 1 )->point();
  const QgsPointV2& p2 = mSelectedFeature->vertexMap().at( row )->point();
  const QgsPointV2& p3 = mSelectedFeature->vertexMap().at( row + 1 )->point();

  double cx, cy;
  QgsGeometryUtils::circleCenterRadius( p1, p2, p3, r, cx, cy );

  double x13 = p3.x() - p1.x(), y13 = p3.y() - p1.y();
  minRadius = 0.5 * qSqrt( x13 * x13 + y13 * y13 );

  return true;
}

void QgsNodeEditorModel::featureChanged()
{
  //TODO - avoid reset
  reset();
}

QgsNodeEditor::QgsNodeEditor(
  QgsVectorLayer *layer,
  QgsSelectedFeature *selectedFeature,
  QgsMapCanvas *canvas )
    : mUpdatingTableSelection( false )
    , mUpdatingNodeSelection( false )
{
  setWindowTitle( tr( "Vertex Editor" ) );

  mLayer = layer;
  mSelectedFeature = selectedFeature;
  mCanvas = canvas;

  mTableView = new QTableView( this );
  mNodeModel = new QgsNodeEditorModel( mLayer, mSelectedFeature, mCanvas, this );
  mTableView->setModel( mNodeModel );

  mTableView->setSelectionMode( QTableWidget::ExtendedSelection );
  mTableView->setSelectionBehavior( QTableWidget::SelectRows );
  mTableView->setItemDelegateForColumn( 0, new CoordinateItemDelegate() );
  mTableView->setItemDelegateForColumn( 1, new CoordinateItemDelegate() );
  mTableView->setItemDelegateForColumn( 2, new CoordinateItemDelegate() );
  mTableView->setItemDelegateForColumn( 3, new CoordinateItemDelegate() );
  mTableView->setItemDelegateForColumn( 4, new CoordinateItemDelegate() );

  setWidget( mTableView );

  connect( mSelectedFeature, SIGNAL( selectionChanged() ), this, SLOT( updateTableSelection() ) );
  connect( mTableView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( updateNodeSelection( QItemSelection, QItemSelection ) ) );
}

void QgsNodeEditor::updateTableSelection()
{
  if ( mUpdatingNodeSelection )
    return;

  mUpdatingTableSelection = true;
  mTableView->selectionModel()->clearSelection();
  const QList<QgsVertexEntry*>& vertexMap = mSelectedFeature->vertexMap();
  int firstSelectedRow = -1;
  QItemSelection selection;
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      if ( firstSelectedRow < 0 )
        firstSelectedRow = i;
      selection.select( mNodeModel->index( i, 0 ), mNodeModel->index( i, mNodeModel->columnCount() - 1 ) );
    }
  }
  mTableView->selectionModel()->select( selection, QItemSelectionModel::Select );

  if ( firstSelectedRow >= 0 )
    mTableView->scrollTo( mNodeModel->index( firstSelectedRow, 0 ), QAbstractItemView::PositionAtTop );

  mUpdatingTableSelection = false;
}

void QgsNodeEditor::updateNodeSelection( const QItemSelection& selected, const QItemSelection& )
{
  if ( mUpdatingTableSelection )
    return;

  mUpdatingNodeSelection = true;

  mSelectedFeature->deselectAllVertexes();
  Q_FOREACH ( const QModelIndex& index, mTableView->selectionModel()->selectedRows() )
  {
    int nodeIdx = index.row();
    mSelectedFeature->selectVertex( nodeIdx );
  }

  //ensure that newly selected node is visible in canvas
  if ( !selected.indexes().isEmpty() )
  {
    int newRow = selected.indexes().first().row();
    zoomToNode( newRow );
  }

  mUpdatingNodeSelection = false;
}

void QgsNodeEditor::zoomToNode( int idx )
{
  double x = mSelectedFeature->vertexMap().at( idx )->point().x();
  double y = mSelectedFeature->vertexMap().at( idx )->point().y();
  QgsPoint newCenter( x, y );

  QgsCoordinateTransform t( mLayer->crs(), mCanvas->mapSettings().destinationCrs() );
  QgsPoint tCenter = t.transform( newCenter );

  QPolygonF ext = mCanvas->mapSettings().visiblePolygon();
  //close polygon
  ext.append( ext.first() );
  QScopedPointer< QgsGeometry > extGeom( QgsGeometry::fromQPolygonF( ext ) );
  QScopedPointer< QgsGeometry > nodeGeom( QgsGeometry::fromPoint( tCenter ) );
  if ( !nodeGeom->within( extGeom.data() ) )
  {
    mCanvas->setCenter( tCenter );
    mCanvas->refresh();
  }
}

void QgsNodeEditor::keyPressEvent( QKeyEvent * e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    emit deleteSelectedRequested();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}

//
// CoordinateItemDelegate
//

QString CoordinateItemDelegate::displayText( const QVariant& value, const QLocale& locale ) const
{
  return locale.toString( value.toDouble(), 'f', 4 );
}

QWidget*CoordinateItemDelegate::createEditor( QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index ) const
{
  QLineEdit* lineEdit = new QLineEdit( parent );
  QDoubleValidator* validator = new QDoubleValidator();
  if ( !index.data( MinRadiusRole ).isNull() )
    validator->setBottom( index.data( MinRadiusRole ).toDouble() );
  lineEdit->setValidator( validator );
  return lineEdit;
}

void CoordinateItemDelegate::setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
  QLineEdit* lineEdit = qobject_cast<QLineEdit*>( editor );
  if ( lineEdit->hasAcceptableInput() )
  {
    QStyledItemDelegate::setModelData( editor, model, index );
  }
}
