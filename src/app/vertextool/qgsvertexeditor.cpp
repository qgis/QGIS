/***************************************************************************
                               qgsvertexeditor.cpp
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

#include "qgsapplication.h"
#include "qgsvertexeditor.h"
#include "qgscoordinateutils.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgslockedfeature.h"
#include "qgsvectorlayer.h"
#include "qgsgeometryutils.h"
#include "qgsproject.h"
#include "qgscoordinatetransform.h"
#include "qgsdoublevalidator.h"
#include "qgspanelwidgetstack.h"

#include <QClipboard>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVector2D>
#include <QCheckBox>
#include <QStackedWidget>
#include <QMenu>

static const int MIN_RADIUS_ROLE = Qt::UserRole + 1;


QgsVertexEditorModel::QgsVertexEditorModel( QgsMapCanvas *canvas, QObject *parent )
  : QAbstractTableModel( parent )
  , mCanvas( canvas )
{
  QWidget *parentWidget = qobject_cast< QWidget * >( parent );
  if ( parentWidget )
    mWidgetFont = parentWidget->font();
}

void QgsVertexEditorModel::setFeature( QgsLockedFeature *lockedFeature )
{
  beginResetModel();

  mLockedFeature = lockedFeature;
  if ( mLockedFeature && mLockedFeature->layer() )
  {
    const QgsWkbTypes::Type layerWKBType = mLockedFeature->layer()->wkbType();

    mHasZ = QgsWkbTypes::hasZ( layerWKBType );
    mHasM = QgsWkbTypes::hasM( layerWKBType );

    if ( mHasZ )
      mZCol = 2;

    if ( mHasM )
      mMCol = 2 + ( mHasZ ? 1 : 0 );

    if ( mHasR )
      mRCol = 2 + ( mHasZ ? 1 : 0 ) + ( mHasM ? 1 : 0 );
  }

  endResetModel();
}

int QgsVertexEditorModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() || !mLockedFeature )
    return 0;

  return mLockedFeature->vertexMap().count();
}

int QgsVertexEditorModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  if ( !mLockedFeature )
    return 0;
  else
    return 2 + ( mHasZ ? 1 : 0 ) + ( mHasM ? 1 : 0 ) + ( mHasR ? 1 : 0 );
}

QVariant QgsVertexEditorModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !mLockedFeature ||
       ( role != Qt::DisplayRole && role != Qt::EditRole && role != MIN_RADIUS_ROLE && role != Qt::FontRole ) )
    return QVariant();

  if ( index.row() >= mLockedFeature->vertexMap().count() )
    return QVariant();

  if ( index.column() >= columnCount() )
    return QVariant();

  //get QgsVertexEntry for row
  const QgsVertexEntry *vertex = mLockedFeature->vertexMap().at( index.row() );
  if ( !vertex )
  {
    return QVariant();
  }

  if ( role == Qt::FontRole )
  {
    double r = 0;
    double minRadius = 0;
    QFont font = mWidgetFont;
    bool fontChanged = false;
    if ( vertex->isSelected() )
    {
      font.setBold( true );
      fontChanged = true;
    }
    if ( calcR( index.row(), r, minRadius ) )
    {
      font.setItalic( true );
      fontChanged = true;
    }
    if ( fontChanged )
    {
      return font;
    }
    else
    {
      return QVariant();
    }
  }

  if ( role == MIN_RADIUS_ROLE )
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
  else if ( index.column() == mMCol )
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

QVariant QgsVertexEditorModel::headerData( int section, Qt::Orientation orientation, int role ) const
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
      else if ( section == mMCol )
        return QVariant( tr( "m" ) );
      else if ( section == mRCol )
        return QVariant( tr( "r" ) );
      else
        return QVariant();
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    if ( orientation == Qt::Vertical )
    {
      return QVariant( tr( "Vertex %1" ).arg( section ) );
    }
    else
    {
      if ( section == 0 )
        return QVariant( tr( "X Coordinate" ) );
      else if ( section == 1 )
        return QVariant( tr( "Y Coordinate" ) );
      else if ( section == mZCol )
        return QVariant( tr( "Z Coordinate" ) );
      else if ( section == mMCol )
        return QVariant( tr( "M Value" ) );
      else if ( section == mRCol )
        return QVariant( tr( "Radius Value" ) );
      else
        return QVariant();
    }
  }
  else
  {
    return QVariant();
  }
}

bool QgsVertexEditorModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( !mLockedFeature || !mLockedFeature->layer() || index.row() >= mLockedFeature->vertexMap().count() )
  {
    return false;
  }

  // Get double value wrt current locale.
  const double doubleValue { QgsDoubleValidator::toDouble( value.toString() ) };

  double x = ( index.column() == 0 ? doubleValue : mLockedFeature->vertexMap().at( index.row() )->point().x() );
  double y = ( index.column() == 1 ? doubleValue : mLockedFeature->vertexMap().at( index.row() )->point().y() );

  if ( index.column() == mRCol ) // radius modified
  {
    if ( index.row() == 0 || index.row() >= mLockedFeature->vertexMap().count() - 1 )
      return false;

    const double x1 = mLockedFeature->vertexMap().at( index.row() - 1 )->point().x();
    const double y1 = mLockedFeature->vertexMap().at( index.row() - 1 )->point().y();
    const double x2 = x;
    const double y2 = y;
    const double x3 = mLockedFeature->vertexMap().at( index.row() + 1 )->point().x();
    const double y3 = mLockedFeature->vertexMap().at( index.row() + 1 )->point().y();

    QgsPoint result;
    if ( QgsGeometryUtils::segmentMidPoint( QgsPoint( x1, y1 ), QgsPoint( x3, y3 ), result, doubleValue, QgsPoint( x2, y2 ) ) )
    {
      x = result.x();
      y = result.y();
    }
  }
  const double z = ( index.column() == mZCol ? doubleValue : mLockedFeature->vertexMap().at( index.row() )->point().z() );
  const double m = ( index.column() == mMCol ? doubleValue : mLockedFeature->vertexMap().at( index.row() )->point().m() );
  const QgsPoint p( QgsWkbTypes::PointZM, x, y, z, m );

  mLockedFeature->layer()->beginEditCommand( QObject::tr( "Moved vertices" ) );
  mLockedFeature->layer()->moveVertex( p, mLockedFeature->featureId(), index.row() );
  mLockedFeature->layer()->endEditCommand();
  mLockedFeature->layer()->triggerRepaint();

  return false;
}

Qt::ItemFlags QgsVertexEditorModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractTableModel::flags( index );

  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

bool QgsVertexEditorModel::calcR( int row, double &r, double &minRadius ) const
{
  if ( row <= 0 || !mLockedFeature || row >= mLockedFeature->vertexMap().count() - 1 )
    return false;

  const QgsVertexEntry *entry = mLockedFeature->vertexMap().at( row );

  const bool curvePoint = ( entry->vertexId().type == Qgis::VertexType::Curve );
  if ( !curvePoint )
    return false;

  const QgsPoint &p1 = mLockedFeature->vertexMap().at( row - 1 )->point();
  const QgsPoint &p2 = mLockedFeature->vertexMap().at( row )->point();
  const QgsPoint &p3 = mLockedFeature->vertexMap().at( row + 1 )->point();

  double cx, cy;
  QgsGeometryUtils::circleCenterRadius( p1, p2, p3, r, cx, cy );

  double x13 = p3.x() - p1.x(), y13 = p3.y() - p1.y();
  minRadius = 0.5 * std::sqrt( x13 * x13 + y13 * y13 );

  return true;
}

//
// QgsVertexEditorWidget
//

QgsVertexEditorWidget::QgsVertexEditorWidget( QgsMapCanvas *canvas )
  : QgsPanelWidget()
  , mCanvas( canvas )
  , mVertexModel( new QgsVertexEditorModel( mCanvas, this ) )
{
  setPanelTitle( tr( "Vertex Editor" ) );
  setObjectName( QStringLiteral( "VertexEditor" ) );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );

  mStackedWidget = new QStackedWidget();
  mPageHint = new QWidget();
  mStackedWidget->addWidget( mPageHint );

  QVBoxLayout *pageHintLayout = new QVBoxLayout();
  mHintLabel = new QLabel();
  mHintLabel->setText( QStringLiteral( "%1\n\n%2" ).arg( tr( "Right click on an editable feature to show its table of vertices." ),
                       tr( "When a feature is bound to this panel, dragging a rectangle to select vertices on the canvas will only select those of the bound feature." ) ) );
  mHintLabel->setWordWrap( true );

  pageHintLayout->addStretch();
  pageHintLayout->addWidget( mHintLabel );
  pageHintLayout->addStretch();
  mPageHint->setLayout( pageHintLayout );

  mPageTable = new QWidget();
  mStackedWidget->addWidget( mPageTable );

  QVBoxLayout *pageTableLayout = new QVBoxLayout();
  pageTableLayout->setContentsMargins( 0, 0, 0, 0 );

  mTableView = new QTableView();
  mTableView->setSelectionMode( QTableWidget::ExtendedSelection );
  mTableView->setSelectionBehavior( QTableWidget::SelectRows );
  mTableView->setModel( mVertexModel );
  connect( mTableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsVertexEditorWidget::updateVertexSelection );

  pageTableLayout->addWidget( mTableView );
  mPageTable->setLayout( pageTableLayout );

  mStackedWidget->setCurrentWidget( mPageHint );
  layout->addWidget( mStackedWidget );

  setLayout( layout );

  mWidgetMenu = new QMenu( this );
  QAction *autoPopupAction = new QAction( tr( "Auto-open Table" ), this );
  autoPopupAction->setCheckable( true );
  autoPopupAction->setChecked( QgsVertexEditor::settingAutoPopupVertexEditorDock.value() );
  connect( autoPopupAction, &QAction::toggled, this, [ = ]( bool checked )
  {
    QgsVertexEditor::settingAutoPopupVertexEditorDock.setValue( checked );
  } );
  mWidgetMenu->addAction( autoPopupAction );
}

void QgsVertexEditorWidget::updateEditor( QgsLockedFeature *lockedFeature )
{
  mLockedFeature = lockedFeature;

  mVertexModel->setFeature( mLockedFeature );

  updateTableSelection();

  if ( mLockedFeature )
  {
    mStackedWidget->setCurrentWidget( mPageTable );

    connect( mLockedFeature, &QgsLockedFeature::selectionChanged, this, &QgsVertexEditorWidget::updateTableSelection );

    if ( mLockedFeature->layer() )
    {
      const QgsCoordinateReferenceSystem crs = mLockedFeature->layer()->crs();
      mTableView->setItemDelegateForColumn( 0, new CoordinateItemDelegate( crs, this ) );
      mTableView->setItemDelegateForColumn( 1, new CoordinateItemDelegate( crs, this ) );
      mTableView->setItemDelegateForColumn( 2, new CoordinateItemDelegate( crs, this ) );
      mTableView->setItemDelegateForColumn( 3, new CoordinateItemDelegate( crs, this ) );
      mTableView->setItemDelegateForColumn( 4, new CoordinateItemDelegate( crs, this ) );
    }
  }
  else
  {
    mStackedWidget->setCurrentWidget( mPageHint );
  }
}

QMenu *QgsVertexEditorWidget::menuButtonMenu()
{
  return mWidgetMenu;
}

QString QgsVertexEditorWidget::menuButtonTooltip() const
{
  return tr( "Options" );
}

void QgsVertexEditorWidget::updateTableSelection()
{
  if ( !mLockedFeature || mUpdatingVertexSelection || mUpdatingTableSelection )
    return;

  mUpdatingTableSelection = true;
  const QList<QgsVertexEntry *> &vertexMap = mLockedFeature->vertexMap();
  int firstSelectedRow = -1;
  QItemSelection selection;
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      if ( firstSelectedRow < 0 )
        firstSelectedRow = i;
      selection.select( mVertexModel->index( i, 0 ), mVertexModel->index( i, mVertexModel->columnCount() - 1 ) );
    }
  }
  mTableView->selectionModel()->select( selection, QItemSelectionModel::ClearAndSelect );

  if ( firstSelectedRow >= 0 )
    mTableView->scrollTo( mVertexModel->index( firstSelectedRow, 0 ), QAbstractItemView::PositionAtTop );

  mUpdatingTableSelection = false;
}

void QgsVertexEditorWidget::updateVertexSelection( const QItemSelection &, const QItemSelection & )
{
  if ( !mLockedFeature || mUpdatingVertexSelection || mUpdatingTableSelection )
    return;

  mUpdatingVertexSelection = true;

  mLockedFeature->deselectAllVertices();

  const QgsCoordinateTransform t( mLockedFeature->layer()->crs(), mCanvas->mapSettings().destinationCrs(), QgsProject::instance() );
  std::unique_ptr<QgsRectangle> bbox;
  const QModelIndexList indexList = mTableView->selectionModel()->selectedRows();
  for ( const QModelIndex &index : indexList )
  {
    const int vertexIdx = index.row();
    mLockedFeature->selectVertex( vertexIdx );

    // create a bounding box of selected vertices
    const QgsPointXY point( mLockedFeature->vertexMap().at( vertexIdx )->point() );
    if ( !bbox )
      bbox.reset( new QgsRectangle( point, point ) );
    else
      bbox->combineExtentWith( point );
  }

  //ensure that newly selected vertices are visible in canvas
  if ( bbox )
  {
    try
    {
      QgsRectangle transformedBbox = t.transform( *bbox );
      const QgsRectangle canvasExtent = mCanvas->mapSettings().visibleExtent();
      transformedBbox.combineExtentWith( canvasExtent );
      mCanvas->setExtent( transformedBbox, true );
      mCanvas->refresh();
    }
    catch ( QgsCsException &cse )
    {
      QgsMessageLog::logMessage( QObject::tr( "Simplify transform error caught: %1" ).arg( cse.what() ), QObject::tr( "CRS" ) );
    }
  }

  mUpdatingVertexSelection = false;
}

void QgsVertexEditorWidget::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    emit deleteSelectedRequested();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
  else if ( e->matches( QKeySequence::Copy ) )
  {
    if ( !mTableView->selectionModel()->hasSelection() )
      return;
    QString text;
    QItemSelectionRange range = mTableView->selectionModel()->selection().first();
    for ( int i = range.top(); i <= range.bottom(); ++i )
    {
      QStringList rowContents;
      for ( int j = range.left(); j <= range.right(); ++j )
        rowContents << mVertexModel->index( i, j ).data().toString();
      text += rowContents.join( '\t' );
      text += '\n';
    }
    QApplication::clipboard()->setText( text );
  }
}


//
// QgsVertexEditor
//

QgsVertexEditor::QgsVertexEditor( QgsMapCanvas *canvas )
{
  setWindowTitle( tr( "Vertex Editor" ) );
  setObjectName( QStringLiteral( "VertexEditor" ) );

  QgsPanelWidgetStack *stack = new QgsPanelWidgetStack();
  setWidget( stack );

  mWidget = new QgsVertexEditorWidget( canvas );
  stack->setMainPanel( mWidget );

  connect( mWidget, &QgsVertexEditorWidget::deleteSelectedRequested, this, &QgsVertexEditor::deleteSelectedRequested );
}

void QgsVertexEditor::updateEditor( QgsLockedFeature *lockedFeature )
{
  mWidget->updateEditor( lockedFeature );
}

void QgsVertexEditor::closeEvent( QCloseEvent *event )
{
  QgsDockWidget::closeEvent( event );

  emit editorClosed();
}

//
// CoordinateItemDelegate
//

CoordinateItemDelegate::CoordinateItemDelegate( const QgsCoordinateReferenceSystem &crs, QObject *parent )
  : QStyledItemDelegate( parent ), mCrs( crs )
{

}

QString CoordinateItemDelegate::displayText( const QVariant &value, const QLocale & ) const
{
  return QLocale().toString( value.toDouble(), 'f', displayDecimalPlaces() );
}

QWidget *CoordinateItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = new QLineEdit( parent );
  QgsDoubleValidator *validator = new QgsDoubleValidator( lineEdit );
  if ( index.data( MIN_RADIUS_ROLE ).isValid() )
    validator->setBottom( index.data( MIN_RADIUS_ROLE ).toDouble() );
  lineEdit->setValidator( validator );
  return lineEdit;
}

void CoordinateItemDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor );
  if ( lineEdit->hasAcceptableInput() )
  {
    QStyledItemDelegate::setModelData( editor, model, index );
  }
}

void CoordinateItemDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QLineEdit *lineEdit = qobject_cast<QLineEdit *>( editor );
  if ( lineEdit && index.isValid() )
  {
    lineEdit->setText( displayText( index.data( ).toDouble( ), QLocale() ).replace( QLocale().groupSeparator(), QString( ) ) );
  }
}

int CoordinateItemDelegate::displayDecimalPlaces() const
{
  return QgsCoordinateUtils::calculateCoordinatePrecisionForCrs( mCrs, QgsProject::instance() );
}
