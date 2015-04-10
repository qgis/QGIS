/***************************************************************************
    qgsgrassselect.cpp  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassedit.h"
#include "qgsgrassattributes.h"
#include "qgsgrassedittools.h"
#include "qgsgrassplugin.h"
#include "qgsgrassutils.h"
#include "qgsgrassprovider.h"
#include "qgsgrass.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvasitem.h"
#include "qgsmaplayer.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QDebug>

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#define BOUND_BOX bound_box
#endif
}

class QgsGrassEditLayer : public QgsMapCanvasItem
{
  public:

    QgsGrassEditLayer( QgsMapCanvas* mapCanvas ): QgsMapCanvasItem( mapCanvas )
    {
    }

    virtual void paint( QPainter* p ) override
    {
      p->drawPixmap( 0, 0, mPixmap );
    }

    virtual QRectF boundingRect() const override
    {
      return QRectF( 0, 0, mMapCanvas->width(), mMapCanvas->height() );
  }

    virtual void updatePosition() override
    {
      setPos( QPointF( mPanningOffset ) );
    }

    QPixmap& pixmap() { return mPixmap; }

  private:

    QPixmap mPixmap;
};


#include <QItemDelegate>
class QgsGrassEditAttributeTableItemDelegate : public QItemDelegate
{
  public:
    QgsGrassEditAttributeTableItemDelegate( QObject *parent = 0 );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

QgsGrassEditAttributeTableItemDelegate::QgsGrassEditAttributeTableItemDelegate( QObject *parent )
    : QItemDelegate( parent )
{}

QWidget *QgsGrassEditAttributeTableItemDelegate::createEditor( QWidget *parent,
    const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QWidget *editor;
  if ( index.column() == 1 )
  {
    QComboBox *cb = new QComboBox( parent );
    cb->addItems( QStringList() << "integer" << "double precision" << "varchar" );
    editor = cb;
  }
  else
  {
    editor = QItemDelegate::createEditor( parent, option, index );
  }
  return editor;
}

void QgsGrassEditAttributeTableItemDelegate::setEditorData( QWidget *editor,
    const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    QComboBox *cb = static_cast<QComboBox *>( editor );
    cb->setCurrentIndex( cb->findData( index.model()->data( index ), Qt::DisplayRole ) );
  }
  else
  {
    QItemDelegate::setEditorData( editor, index );
  }
}

void QgsGrassEditAttributeTableItemDelegate::setModelData( QWidget *editor,
    QAbstractItemModel *model, const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    QComboBox *cb = static_cast<QComboBox *>( editor );
    model->setData( index, cb->currentText(), Qt::EditRole );
  }
  else
  {
    QItemDelegate::setModelData( editor, model, index );
  }
}


bool QgsGrassEdit::mRunning = false;

QgsGrassEdit::QgsGrassEdit( QgisInterface *iface, QgsMapLayer *layer, bool newMap,
                            QWidget *parent, Qt::WindowFlags f )
    : QMainWindow( parent, f )
    , QgsGrassEditBase()
    , mLayer( 0 )
    , mToolBar( 0 )
    , mSize( 0 )
    , mValid( false )
    , mInited( false )
    , mIface( iface )
    , mCanvas( 0 )
    , mProvider( 0 )
    , mTool( QgsGrassEdit::NONE )
    , mSuspend( false )
    , mEditPoints( 0 )
    , mPoints( 0 )
    , mCats( 0 )
    , mPixmap( 0 )
    , mTransform( 0 )
    , mSelectedLine( 0 )
    , mSelectedPart( 0 )
    , mAddVertexEnd( false )
    , mLineWidth( 0 )
    , mMarkerSize( 0 )
    , mAttributes( 0 )
    , mNewMap( newMap )
    , mNewPointAction( 0 )
    , mNewLineAction( 0 )
    , mNewBoundaryAction( 0 )
    , mNewCentroidAction( 0 )
    , mMoveVertexAction( 0 )
    , mAddVertexAction( 0 )
    , mDeleteVertexAction( 0 )
    , mMoveLineAction( 0 )
    , mSplitLineAction( 0 )
    , mDeleteLineAction( 0 )
    , mEditAttributesAction( 0 )
    , mCloseEditAction( 0 )
    , mMapTool( 0 )
    , mCanvasEdit( 0 )
    , mRubberBandLine( 0 )
    , mRubberBandIcon( 0 )
{
  QgsDebugMsg( "QgsGrassEdit()" );

  setupUi( this );

  mRunning = true;

  mProjectionEnabled = ( QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 ) != 0 );

  mCanvas = mIface->mapCanvas();

  if ( !isEditable( layer ) )
    return;

  //TODO dynamic_cast ?
  mLayer = ( QgsVectorLayer* )layer;

  //TODO dynamic_cast ?
  mProvider = ( QgsGrassProvider * ) mLayer->dataProvider();

  init();
}

bool QgsGrassEdit::isEditable( QgsMapLayer *layer )
{
  if ( !layer )
    return false;

  QgsDebugMsgLevel( "layer name: " + layer->name(), 3 );

  if ( layer->type() != QgsMapLayer::VectorLayer )
  {
    QgsDebugMsgLevel( "The selected layer is not vector.", 3 );
    return false;
  }

  //TODO dynamic_cast ?
  QgsVectorLayer *vector = ( QgsVectorLayer* )layer;

  QgsDebugMsgLevel( "Vector layer type: " + vector->providerType(), 3 );

  if ( vector->providerType() != "grass" )
  {
    QgsDebugMsgLevel( "The selected layer is not GRASS.", 3 );
    return false;
  }

  return true;
}

void QgsGrassEdit::keyPress( QKeyEvent *e )
{
  QgsDebugMsg( "entered." );
  // This does not work:
  //keyPressEvent(e);

  // TODO: this is not optimal
  switch ( e->key() )
  {
    case Qt::Key_F1: newPoint(); break;
    case Qt::Key_F2: newLine(); break;
    case Qt::Key_F3: newBoundary(); break;
    case Qt::Key_F4: newCentroid(); break;
    case Qt::Key_F5: moveVertex(); break;
    case Qt::Key_F6: addVertex(); break;
    case Qt::Key_F7: deleteVertex(); break;
    case Qt::Key_F9: moveLine(); break;
    case Qt::Key_F10: splitLine(); break;
    case Qt::Key_F11: deleteLine(); break;
    default: break;
  }
}


void QgsGrassEdit::init()
{
  if ( !( mProvider->isGrassEditable() ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "You are not owner of the mapset, cannot open the vector for editing." ) );
    return;
  }

  if ( !( mProvider->startEdit() ) )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot open vector for update." ) );
    return;
  }

  mRubberBandLine = new QgsRubberBand( mCanvas );
  mRubberBandIcon = new QgsVertexMarker( mCanvas );
  mRubberBandLine->setZValue( 20 );
  mRubberBandIcon->setZValue( 20 );

  connect( mCanvas, SIGNAL( keyPressed( QKeyEvent * ) ), this, SLOT( keyPress( QKeyEvent * ) ) );


  mToolBar = addToolBar( tr( "Edit tools" ) );

  mNewPointAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_new_point.png" ), tr( "New point" ), this );
  mNewPointAction->setShortcut( QKeySequence( Qt::Key_F1 ) );
  mToolBar->addAction( mNewPointAction );
  connect( mNewPointAction, SIGNAL( triggered() ), this, SLOT( newPoint() ) );

  mNewLineAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_new_line.png" ), tr( "New line" ), this );
  mNewLineAction->setShortcut( QKeySequence( Qt::Key_F2 ) );
  mToolBar->addAction( mNewLineAction );
  connect( mNewLineAction, SIGNAL( triggered() ), this, SLOT( newLine() ) );

  mNewBoundaryAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_new_boundary.png" ), tr( "New boundary" ), this );
  mNewBoundaryAction->setShortcut( QKeySequence( Qt::Key_F3 ) );
  mToolBar->addAction( mNewBoundaryAction );
  connect( mNewBoundaryAction, SIGNAL( triggered() ), this, SLOT( newBoundary() ) );

  mNewCentroidAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_new_centroid.png" ), tr( "New centroid" ), this );
  mNewCentroidAction->setShortcut( QKeySequence( Qt::Key_F4 ) );
  mToolBar->addAction( mNewCentroidAction );
  connect( mNewCentroidAction, SIGNAL( triggered() ), this, SLOT( newCentroid() ) );

  mMoveVertexAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_move_vertex.png" ), tr( "Move vertex" ), this );
  mMoveVertexAction->setShortcut( QKeySequence( Qt::Key_F5 ) );
  mToolBar->addAction( mMoveVertexAction );
  connect( mMoveVertexAction, SIGNAL( triggered() ), this, SLOT( moveVertex() ) );

  mAddVertexAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_add_vertex.png" ), tr( "Add vertex" ), this );
  mAddVertexAction->setShortcut( QKeySequence( Qt::Key_F6 ) );
  mToolBar->addAction( mAddVertexAction );
  connect( mAddVertexAction, SIGNAL( triggered() ), this, SLOT( addVertex() ) );

  mDeleteVertexAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_delete_vertex.png" ), tr( "Delete vertex" ), this );
  mDeleteVertexAction->setShortcut( QKeySequence( Qt::Key_F7 ) );
  mToolBar->addAction( mDeleteVertexAction );
  connect( mDeleteVertexAction, SIGNAL( triggered() ), this, SLOT( deleteVertex() ) );

  mMoveLineAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_move_line.png" ), tr( "Move element" ), this );
  mMoveLineAction->setShortcut( QKeySequence( Qt::Key_F9 ) );
  mToolBar->addAction( mMoveLineAction );
  connect( mMoveLineAction, SIGNAL( triggered() ), this, SLOT( moveLine() ) );

  mSplitLineAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_split_line.png" ), tr( "Split line" ), this );
  mSplitLineAction->setShortcut( QKeySequence( Qt::Key_F10 ) );
  mToolBar->addAction( mSplitLineAction );
  connect( mSplitLineAction, SIGNAL( triggered() ), this, SLOT( splitLine() ) );

  mDeleteLineAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_delete_line.png" ), tr( "Delete element" ), this );
  mDeleteLineAction->setShortcut( QKeySequence( Qt::Key_F11 ) );
  mToolBar->addAction( mDeleteLineAction );
  connect( mDeleteLineAction, SIGNAL( triggered() ), this, SLOT( deleteLine() ) );

  mEditAttributesAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_edit_attributes.png" ), tr( "Edit attributes" ), this );
  mToolBar->addAction( mEditAttributesAction );
  connect( mEditAttributesAction, SIGNAL( triggered() ), this, SLOT( editAttributes() ) );

  mCloseEditAction = new QAction(
    QgsGrassPlugin::getThemeIcon( "grass_close_edit.png" ), tr( "Close" ), this );
  mToolBar->addAction( mCloseEditAction );
  connect( mCloseEditAction, SIGNAL( triggered() ), this, SLOT( closeEdit() ) );

  mNewPointAction->setCheckable( true );
  mNewLineAction->setCheckable( true );
  mNewBoundaryAction->setCheckable( true );
  mNewCentroidAction->setCheckable( true );
  mMoveVertexAction->setCheckable( true );
  mAddVertexAction->setCheckable( true );
  mDeleteVertexAction->setCheckable( true );
  mMoveLineAction->setCheckable( true );
  mSplitLineAction->setCheckable( true );
  mDeleteLineAction->setCheckable( true );
  mEditAttributesAction->setCheckable( true );

  QActionGroup *ag = new QActionGroup( this );
  ag->addAction( mNewPointAction );
  ag->addAction( mNewLineAction );
  ag->addAction( mNewBoundaryAction );
  ag->addAction( mNewCentroidAction );
  ag->addAction( mMoveVertexAction );
  ag->addAction( mAddVertexAction );
  ag->addAction( mDeleteVertexAction );
  ag->addAction( mMoveLineAction );
  ag->addAction( mSplitLineAction );
  ag->addAction( mDeleteLineAction );
  ag->addAction( mEditAttributesAction );

  mEditPoints = Vect_new_line_struct();
  mPoints = Vect_new_line_struct();
  mCats = Vect_new_cats_struct();

  // Set lines symbology from map
  int nlines = mProvider->numLines();
  mLineSymb.resize( nlines + 1000 );
  for ( int line = 1; line <= nlines; line++ )
  {
    mLineSymb[line] = lineSymbFromMap( line );
  }

  // Set nodes symbology from map
  int nnodes = mProvider->numNodes();
  mNodeSymb.resize( nnodes + 1000 );
  for ( int node = 1; node <= nnodes; node++ )
  {
    mNodeSymb[node] = nodeSymbFromMap( node );
  }

  // Set default colors
  mSymb.resize( SYMB_COUNT );
  mSymb[SYMB_BACKGROUND].setColor( QColor( 255, 255, 255 ) );       // white
  mSymb[SYMB_HIGHLIGHT].setColor( QColor( 255, 255,   0 ) );        // yellow
  mSymb[SYMB_DYNAMIC].setColor( QColor( 125, 125, 125 ) );          // grey
  mSymb[SYMB_POINT].setColor( QColor( 0,   0,   0 ) );              // black
  mSymb[SYMB_LINE].setColor( QColor( 0,   0,   0 ) );               // black
  mSymb[SYMB_BOUNDARY_0].setColor( QColor( 255,   0,   0 ) );       // red
  mSymb[SYMB_BOUNDARY_1].setColor( QColor( 255, 125,   0 ) );       // orange
  mSymb[SYMB_BOUNDARY_2].setColor( QColor( 0, 255,   0 ) );         // green
  mSymb[SYMB_CENTROID_IN].setColor( QColor( 0, 255,   0 ) );        // green
  mSymb[SYMB_CENTROID_OUT].setColor( QColor( 255,   0,   0 ) );     // red
  mSymb[SYMB_CENTROID_DUPL].setColor( QColor( 255,   0, 255 ) );    // magenta
  mSymb[SYMB_NODE_1].setColor( QColor( 255,   0,   0 ) );           // red
  mSymb[SYMB_NODE_2].setColor( QColor( 0, 255,   0 ) );             // green

  // Set mSymbDisplay
  mSymbDisplay.resize( SYMB_COUNT );
  mSymbDisplay[SYMB_BACKGROUND] = true;
  mSymbDisplay[SYMB_HIGHLIGHT] = true;
  mSymbDisplay[SYMB_DYNAMIC] = true;
  mSymbDisplay[SYMB_POINT] = true;
  mSymbDisplay[SYMB_LINE] = true;
  mSymbDisplay[SYMB_BOUNDARY_0] = true;
  mSymbDisplay[SYMB_BOUNDARY_1] = true;
  mSymbDisplay[SYMB_BOUNDARY_2] = true;
  mSymbDisplay[SYMB_CENTROID_IN] = true;
  mSymbDisplay[SYMB_CENTROID_OUT] = true;
  mSymbDisplay[SYMB_CENTROID_DUPL] = true;
  mSymbDisplay[SYMB_NODE_1] = true;
  mSymbDisplay[SYMB_NODE_2] = true;

  // Set symbology names
  mSymbName.resize( SYMB_COUNT );
  mSymbName[SYMB_BACKGROUND]    = tr( "Background" );
  mSymbName[SYMB_HIGHLIGHT]     = tr( "Highlight" );
  mSymbName[SYMB_DYNAMIC]       = tr( "Dynamic" );
  mSymbName[SYMB_POINT]         = tr( "Point" );
  mSymbName[SYMB_LINE]          = tr( "Line" );
  mSymbName[SYMB_BOUNDARY_0]    = tr( "Boundary (no area)" );
  mSymbName[SYMB_BOUNDARY_1]    = tr( "Boundary (1 area)" );
  mSymbName[SYMB_BOUNDARY_2]    = tr( "Boundary (2 areas)" );
  mSymbName[SYMB_CENTROID_IN]   = tr( "Centroid (in area)" );
  mSymbName[SYMB_CENTROID_OUT]  = tr( "Centroid (outside area)" );
  mSymbName[SYMB_CENTROID_DUPL] = tr( "Centroid (duplicate in area)" );
  mSymbName[SYMB_NODE_1]        = tr( "Node (1 line)" );
  mSymbName[SYMB_NODE_2]        = tr( "Node (2 lines)" );

  // Restore symbology
  QString spath = "/GRASS/edit/symb/";
  QSettings settings;

  mLineWidth = settings.value(
                 spath + "lineWidth", 1 ).toInt();
  mSize = settings.value(
            spath + "markerSize", 9 ).toInt();
  mLineWidthSpinBox->setValue( mLineWidth );
  mMarkerSizeSpinBox->setValue( mSize );

  for ( int i = 0; i < SYMB_COUNT; i++ )
  {
    bool ok = settings.contains(
                spath + "display/" + QString::number( i ) );
    bool displ = settings.value(
                   spath + "display/" + QString::number( i ),
                   true ).toBool();
    if ( ok )
    {
      mSymbDisplay[i] = displ;
    }

    ok = settings.contains(
           spath + "color/" + QString::number( i ) );
    QString colorName = settings.value(
                          spath + "color/" + QString::number( i ),
                          "" ).toString();
    if ( ok )
    {
      QColor color( colorName );
      mSymb[i].setColor( color );
      // Use the 'dynamic' color for mRubberBand
      if ( i == SYMB_DYNAMIC )
      {
        mRubberBandLine->setColor( QColor( colorName ) );
      }
    }
    mSymb[i].setWidth( mLineWidth );
  }

  // Set Symbology in dialog
  symbologyList->setColumnWidth( 0, 40 );
  symbologyList->setColumnWidth( 1, 50 );
  symbologyList->setColumnWidth( 2, 200 );

  for ( int i = 0; i < SYMB_COUNT; i++ )
  {
    if ( i == SYMB_NODE_0 )
      continue;

    QPixmap pm( 40, 15 );
    pm.fill( mSymb[i].color() );
    QString index;
    index.sprintf( "%d", i );

    QTreeWidgetItem *item = new QTreeWidgetItem( symbologyList );
    if ( !( i == SYMB_BACKGROUND || i == SYMB_HIGHLIGHT || i == SYMB_DYNAMIC ) )
    {
      item->setCheckState( 0, mSymbDisplay[i] ? Qt::Checked : Qt::Unchecked );
    }
    item->setIcon( 1, pm );
    item->setText( 2, mSymbName[i] );
    item->setText( 3, index );
  }

  connect( symbologyList, SIGNAL( itemPressed( QTreeWidgetItem *, int ) ),
           this, SLOT( changeSymbology( QTreeWidgetItem *, int ) ) );

  // Init table tab
  mAttributeTable->setItemDelegate( new QgsGrassEditAttributeTableItemDelegate( this ) );
  mAttributeTable->verticalHeader()->hide();

  int ndblinks = mProvider->numDbLinks();

  if ( ndblinks > 0 )
  {
    for ( int i = 0; i < ndblinks; i++ )
    {
      int f = mProvider->dbLinkField( i );

      QString str;
      str.sprintf( "%d", f );
      mTableField->addItem( str );
      mFieldBox->addItem( str );
      if ( i == 0 )
      {
        setAttributeTable( f );
      }
    }
    mTableField->setCurrentIndex( 0 );
    mFieldBox->setCurrentIndex( 0 );
  }
  else
  {
    mTableField->addItem( "1" );
    setAttributeTable( 1 );

    mFieldBox->addItem( "1" );
  }

  connect( mAttributeTable, SIGNAL( cellChanged( int, int ) ), this, SLOT( columnTypeChanged( int, int ) ) );

  // Set variables
  mSelectedLine = 0;
  mAttributes = 0;

  // Read max cats
  for ( int i = 0; i < mProvider->cidxGetNumFields(); i++ )
  {
    int field = mProvider->cidxGetFieldNumber( i );
    if ( field > 0 )
    {
      int cat = mProvider->cidxGetMaxCat( i );
      MaxCat mc;
      mc.field = field;
      mc.maxCat = cat;
      mMaxCats.push_back( mc );
    }
  }

  connect( mCanvas, SIGNAL( renderComplete( QPainter * ) ), this, SLOT( postRender( QPainter * ) ) );

  mCanvasEdit = new QgsGrassEditLayer( mCanvas );

  mPixmap = &mCanvasEdit->pixmap();

  // Init GUI values
  mCatModeBox->addItem( tr( "Next not used" ), CAT_MODE_NEXT );
  mCatModeBox->addItem( tr( "Manual entry" ), CAT_MODE_MANUAL );
  mCatModeBox->addItem( tr( "No category" ), CAT_MODE_NOCAT );
  catModeChanged();

  // TODO: how to get keyboard events from canvas (shortcuts)

  restorePosition();

  mValid = true;
  mInited = true;
}

void QgsGrassEdit::attributeTableFieldChanged( void )
{
  QgsDebugMsg( "entered." );
  int field = mTableField->currentText().toInt();

  setAttributeTable( field );
}

void QgsGrassEdit::setAttributeTable( int field )
{
  mAttributeTable->setRowCount( 0 );

  QString key = mProvider->key( field );
  if ( !key.isEmpty() )   // Database link defined
  {
    QVector<QgsField> *cols = mProvider->columns( field );

    mAttributeTable->setRowCount( cols->size() );

    for ( int c = 0; c < cols->size(); c++ )
    {
      QgsField col = ( *cols )[c];

      QTableWidgetItem *ti;

      ti = new QTableWidgetItem( col.name() );
      ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
      mAttributeTable->setItem( c, 0, ti );

      ti = new QTableWidgetItem( col.typeName() );
      ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
      mAttributeTable->setItem( c, 1, ti );

      QString str;
      str.sprintf( "%d", col.length() );
      ti = new QTableWidgetItem( str );
      ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
      mAttributeTable->setItem( c, 2, ti );
    }

    delete cols;
  }
  else
  {
    mAttributeTable->setRowCount( 1 );

    QTableWidgetItem *ti;

    ti = new QTableWidgetItem( "cat" );
    mAttributeTable->setItem( 0, 0, ti );

    ti = new QTableWidgetItem( "integer" );
    ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
    mAttributeTable->setItem( 0, 1, ti );

    ti = new QTableWidgetItem( "" );
    ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
    mAttributeTable->setItem( 0, 2, ti );
  }
}

void QgsGrassEdit::addColumn( void )
{
  QgsDebugMsg( "entered." );
  int r = mAttributeTable->rowCount();
  mAttributeTable->setRowCount( r + 1 );

  QString cn;
  cn.sprintf( "column%d", r + 1 );

  QTableWidgetItem *ti;

  ti = new QTableWidgetItem( cn );
  mAttributeTable->setItem( r, 0, ti );

  ti = new QTableWidgetItem( "integer" );
  mAttributeTable->setItem( r, 1, ti );

  ti = new QTableWidgetItem( "20" );
  ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
  mAttributeTable->setItem( r, 2, ti );
}

void QgsGrassEdit::columnTypeChanged( int row, int col )
{
  QgsDebugMsg( QString( "row = %1 col = %2" ).arg( row ).arg( col ) );

  if ( col != 1 )
    return;

  QTableWidgetItem *ti = mAttributeTable->item( row, 2 );
  if ( ti )
  {
    if ( mAttributeTable->item( row, 1 )->text().compare( "varchar" ) == 0 )
    {
      ti->setFlags( ti->flags() | Qt::ItemIsEnabled );
    }
    else
    {
      ti->setFlags( ti->flags() & ~Qt::ItemIsEnabled );
    }
  }
}

void QgsGrassEdit::alterTable( void )
{
  QgsDebugMsg( "entered." );

  // Create new table if first column name is editable otherwise alter table
  int field = mTableField->currentText().toInt();

  QString sql;
  QString type;

  if ( mAttributeTable->item( 0, 0 )->flags() & Qt::ItemIsEnabled )
  {
    QgsDebugMsg( "Create new table" );

    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( i > 0 )
        sql.append( ", " );

      type = mAttributeTable->item( i, 1 )->text();
      sql.append( mAttributeTable->item( i, 0 )->text() + " " + type );

      if ( type.compare( "varchar" ) == 0 )
      {
        sql.append( " (" + mAttributeTable->item( i, 2 )->text() + ")" );
      }
    }

    QString error = mProvider->createTable( field, mAttributeTable->item( 0, 0 )->text(), sql );
    if ( !error.isEmpty() )
    {
      QMessageBox::warning( 0, tr( "Warning" ), error );
    }
    else
    {
      QMessageBox::information( 0, tr( "Info" ), tr( "The table was created" ) );
      QString str;
      str.sprintf( "%d", field );
      mFieldBox->addItem( str );
    }
  }
  else
  {
    QgsDebugMsg( "Alter table" );

    for ( int i = 0; i < mAttributeTable->rowCount(); i++ )
    {
      if ( !( mAttributeTable->item( i, 0 )->flags() & Qt::ItemIsEnabled ) )
        continue;

      type = mAttributeTable->item( i, 1 )->text();
      sql = mAttributeTable->item( i, 0 )->text() + " " + type;

      if ( type.compare( "varchar" ) == 0 )
      {
        sql.append( " (" + mAttributeTable->item( i, 2 )->text() + ")" );
      }

      QString error = mProvider->addColumn( field, sql );
      if ( !error.isEmpty() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), error );
      }
    }
  }

  setAttributeTable( field );
}

void QgsGrassEdit::changeSymbology( QTreeWidgetItem * item, int col )
{
  QgsDebugMsg( QString( "col = %1" ).arg( col ) );

  QSettings settings;

  if ( !item )
    return;

  int index = item->text( 3 ).toInt();

  if ( col == 0 )
  {
    if ( index == SYMB_BACKGROUND || index == SYMB_HIGHLIGHT || index == SYMB_DYNAMIC )
      return;

    mSymbDisplay[index] = item->checkState( 0 ) == Qt::Checked;

    //int ww = settings.readNumEntry("/GRASS/windows/edit/w", 420);
    QString sn;
    // TODO use a name instead of index
    sn.sprintf( "/GRASS/edit/symb/display/%d", index );
    settings.setValue( sn, ( bool )mSymbDisplay[index] );
  }
  else if ( col == 1 )
  {
    QColor color = QColorDialog::getColor( mSymb[index].color(), this );
    mSymb[index].setColor( color );

    QPixmap pm( 40, 15 );
    pm.fill( mSymb[index].color() );
    item->setIcon( 1, pm );

    QString sn;
    // TODO use a name instead of index
    sn.sprintf( "/GRASS/edit/symb/color/%d", index );
    settings.setValue( sn, mSymb[index].color().name() );
    // Use the 'dynamic' color for mRubberBand
    if ( index == SYMB_DYNAMIC )
    {
      mRubberBandLine->setColor( color );
    }
  }
}

void QgsGrassEdit::lineWidthChanged()
{
  QgsDebugMsg( "entered." );
  QSettings settings;
  mLineWidth = mLineWidthSpinBox->value();

  for ( int i = 0; i < SYMB_COUNT; i++ )
  {
    mSymb[i].setWidth( mLineWidth );
  }

  QString spath = "/GRASS/edit/symb/";
  settings.setValue( spath + "lineWidth", mLineWidth );
}

void QgsGrassEdit::markerSizeChanged()
{
  QgsDebugMsg( "entered." );
  QSettings settings;
  mSize = mMarkerSizeSpinBox->value();
  QString spath = "/GRASS/edit/symb/";
  settings.setValue( spath + "markerSize", mSize );
}

void QgsGrassEdit::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/GRASS/windows/edit/geometry" ).toByteArray() );
}

void QgsGrassEdit::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/GRASS/windows/edit/geometry", saveGeometry() );
}

void QgsGrassEdit::updateSymb( void )
{
  QgsDebugMsg( "entered." );

  // Set lines symbology from map
  unsigned int nlines = mProvider->numLines();
  if ( nlines + 1 >= mLineSymb.size() )
    mLineSymb.resize( nlines + 1000 );

  nlines = mProvider->numUpdatedLines();
  for ( unsigned int i = 0; i < nlines; i++ )
  {
    int line = mProvider->updatedLine( i );
    QgsDebugMsg( QString( "updated line = %1" ).arg( line ) );
    if ( !( mProvider->lineAlive( line ) ) )
      continue;
    mLineSymb[line] = lineSymbFromMap( line );
  }

  // Set nodes symbology from map
  unsigned int nnodes = mProvider->numNodes();
  if ( nnodes + 1 >= mNodeSymb.size() )
    mNodeSymb.resize( nnodes + 1000 );

  nnodes = mProvider->numUpdatedNodes();
  for ( unsigned int i = 0; i < nnodes; i++ )
  {
    int node = mProvider->updatedNode( i );
    if ( !( mProvider->nodeAlive( node ) ) )
      continue;
    mNodeSymb[node] = nodeSymbFromMap( node );
    QgsDebugMsg( QString( "node = %1 mNodeSymb = %2" ).arg( node ).arg( mNodeSymb[node] ) );
  }
}

int QgsGrassEdit::nodeSymbFromMap( int node )
{
  QgsDebugMsg( QString( "node = %1" ).arg( node ) );

  int nlines = mProvider->nodeNLines( node );

  int count = 0;

  for ( int i = 0; i < nlines; i++ )
  {
    int line = qAbs( mProvider->nodeLine( node, i ) );
    int type = mProvider->readLine( NULL, NULL, line );

    if ( type & GV_LINES )
      count++;
  }

  if ( count == 0 )
    return SYMB_NODE_0;
  else if ( count == 1 )
    return SYMB_NODE_1;

  return SYMB_NODE_2;
}

int QgsGrassEdit::lineSymbFromMap( int line )
{
  QgsDebugMsg( QString( "line = %1" ).arg( line ) );

  int type = mProvider->readLine( NULL, NULL, line );

  if ( type < 0 )
    return 0;

  switch ( type )
  {
    case GV_POINT:
      return SYMB_POINT;
      break;

    case GV_LINE:
      return SYMB_LINE;
      break;

    case GV_BOUNDARY:
      int left, right, nareas;

      if ( !( mProvider->lineAreas( line, &left, &right ) ) )
        return 0;

      /* Count areas on both sides */
      nareas = 0;
      if ( left > 0 || ( left < 0 && mProvider->isleArea( -left ) > 0 ) )
        nareas++;
      if ( right > 0 || ( right < 0 && mProvider->isleArea( -right ) > 0 ) )
        nareas++;
      if ( nareas == 0 )
        return SYMB_BOUNDARY_0;
      else if ( nareas == 1 )
        return SYMB_BOUNDARY_1;
      else
        return SYMB_BOUNDARY_2;
      break;

    case GV_CENTROID:
      int area = mProvider->centroidArea( line );
      if ( area == 0 )
        return SYMB_CENTROID_OUT;
      else if ( area > 0 )
        return SYMB_CENTROID_IN;
      else
        return SYMB_CENTROID_DUPL; /* area < 0 */
      break;
  }

  return 0; // Should not happen
}

QgsGrassEdit::~QgsGrassEdit()
{
  QgsDebugMsg( "entered." );

  // we can only call some methods if init was complete,
  // note that we cannot use mValid because it is disabled before
  // destructor is called
  if ( mInited )
  {
    // delete tool if exists
    delete mMapTool;

    eraseDynamic();
    mRubberBandLine->hide();
    mRubberBandIcon->hide();
    mRubberBandLine->reset();
    delete mRubberBandLine;
    delete mRubberBandIcon;

    delete mCanvasEdit;

    mCanvas->refresh();

    saveWindowLocation();
  }
  mRunning = false;
}

bool QgsGrassEdit::isRunning( void )
{
  return mRunning;
}

bool QgsGrassEdit::isValid( void )
{
  return mValid;
}

void QgsGrassEdit::closeEdit( void )
{
  QgsDebugMsg( "entered." );

  // Disconnect signals
  // Warning: it seems that slots (postRender) can be called even
  //          after disconnect (is it a queue?)
  disconnect( this, SLOT( postRender( QPainter * ) ) );

  mValid = false; // important for postRender

  if ( mAttributes )
  {
    delete mAttributes;
  }

  mProvider->closeEdit( mNewMap );

  hide();

  // Add new layers
  if ( mNewMap )
  {
    QString uri = QDir::cleanPath( mProvider->dataSourceUri() );
    QgsDebugMsg( QString( "uri = %1" ).arg( uri ) );
    // Note: QDir::cleanPath is using '/' also on Windows
    //QChar sep = QDir::separator();
    QChar sep = '/';

    QStringList split = uri.split( sep, QString::SkipEmptyParts );
    split.pop_back(); // layer
    QString map = split.last();
    split.pop_back(); // map
    QString mapset = split.last();

    QgsGrassUtils::addVectorLayers( mIface, QgsGrass::getDefaultGisdbase(),
                                    QgsGrass::getDefaultLocation(),
                                    mapset, map );
  }
  emit finished();
  delete this;
}

void QgsGrassEdit::closeEvent( QCloseEvent *e )
{
  QgsDebugMsg( "entered." );

  e->accept();

  closeEdit();
}

void QgsGrassEdit::catModeChanged( void )
{
  QgsDebugMsg( "entered." );
  int mode = mCatModeBox->currentIndex();

  int field = mFieldBox->currentText().toInt();

  if ( mode == CAT_MODE_NEXT )   // Find next not used
  {
    QString c = "1"; // Default for new field
    for ( unsigned int i = 0; i < mMaxCats.size(); i++ )
    {
      if ( mMaxCats[i].field == field )
      {
        c.sprintf( "%d", mMaxCats[i].maxCat + 1 );
        break;
      }
    }
    mCatEntry->setText( c );
    mCatEntry->setEnabled( false );
    mFieldBox->setDisabled( false );
  }
  else if ( mode == CAT_MODE_MANUAL )
  {
    mCatEntry->setEnabled( true );
    mFieldBox->setDisabled( false );
  }
  else   // CAT_MODE_NOCAT
  {
    mCatEntry->clear();
    mCatEntry->setEnabled( false );
    mFieldBox->setDisabled( true );
  }
}

void QgsGrassEdit::fieldChanged( void )
{
  QgsDebugMsg( "entered." );
  int mode = mCatModeBox->currentIndex();
  int field = mFieldBox->currentText().toInt();

  if ( mode == CAT_MODE_NEXT )   // Find next not used
  {
    QString c = "1"; // Default for new field
    for ( unsigned int i = 0; i < mMaxCats.size(); i++ )
    {
      if ( mMaxCats[i].field == field )
      {
        c.sprintf( "%d", mMaxCats[i].maxCat + 1 );
        break;
      }
    }
    mCatEntry->setText( c );
  }
}

int QgsGrassEdit::writeLine( int type, struct line_pnts *Points )
{
  int mode = mCatModeBox->currentIndex();
  int field = mFieldBox->currentText().toInt();
  int cat = mCatEntry->text().toInt();

  Vect_reset_cats( mCats );
  if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL )
  {
    Vect_cat_set( mCats, field, cat );

    // Insert new DB record if link is defined and the record for this cat does not exist
    QString key = mProvider->key( field );
    if ( !key.isEmpty() )   // Database link defined
    {
      QgsAttributeMap *atts = mProvider->attributes( field, cat );

      if ( atts->count() == 0 )   // Nothing selected
      {
        QString error = mProvider->insertAttributes( field, cat );
        if ( !error.isEmpty() )
        {
          QMessageBox::warning( 0, tr( "Warning" ), error );
        }
      }

      delete atts;
    }
  }
  Vect_line_prune( Points );
  int line = mProvider->writeLine( type, Points, mCats );

  increaseMaxCat();
  return line;
}

void QgsGrassEdit::increaseMaxCat( void )
{
  int mode = mCatModeBox->currentIndex();
  int field = mFieldBox->currentText().toInt();
  int cat = mCatEntry->text().toInt();

  if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL )
  {
    int found = 0;
    for ( unsigned int i = 0; i < mMaxCats.size(); i++ )
    {
      if ( mMaxCats[i].field == field )
      {
        if ( cat > mMaxCats[i].maxCat )
        {
          mMaxCats[i].maxCat = cat;
        }
        found = 1;
        break;
      }
    }
    if ( !found )
    {
      MaxCat mc;
      mc.field = field;
      mc.maxCat = cat;
      mMaxCats.push_back( mc );
    }

    if ( mode == CAT_MODE_NEXT )
    {
      QString c;
      c.sprintf( "%d", cat + 1 );
      mCatEntry->setText( c );
    }
  }

}

double QgsGrassEdit::threshold( void )
{
  int snapPixels = mSnapPixels->text().toInt();

  // Convert to map units (not nice)
  QgsPoint p1, p2;
  p1 = mTransform->toMapCoordinates( 0, 0 );
  p2 = mTransform->toMapCoordinates( snapPixels, 0 );

  if ( mProjectionEnabled )
  {
    try
    {
      p1 = mCanvas->mapSettings().mapToLayerCoordinates( mLayer, p1 );
      p2 = mCanvas->mapSettings().mapToLayerCoordinates( mLayer, p2 );
    }
    catch ( QgsCsException& cse )
    {
      Q_UNUSED( cse );
      //error
    }
  }

  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double thresh = sqrt( dx * dx + dy * dy );
  return thresh;
}

void QgsGrassEdit::snap( double *x, double *y )
{
  double thresh = threshold();

  int node = mProvider->findNode( *x, *y, thresh );

  if ( node > 0 )
  {
    mProvider->nodeCoor( node, x, y );
  }
}

void QgsGrassEdit::snap( QgsPoint & point )
{
  double x = point.x();
  double y = point.y();

  snap( &x, &y );

  point.setX( x );
  point.setY( y );
}

void QgsGrassEdit::snap( QgsPoint & point, double startX, double startY )
{
  double x = point.x();
  double y = point.y();

  double thresh = threshold();

  // Start
  double startDist = hypot( x - startX, y - startY );
  bool startIn = false;
  if ( startDist <= thresh )
    startIn = true;

  // Nearest node
  double nodeX = 0;
  double  nodeY = 0;
  double nodeDist = 0;
  bool nodeIn = false;
  int node = mProvider->findNode( x, y, thresh );

  if ( node > 0 )
  {
    mProvider->nodeCoor( node, &nodeX, &nodeY );
    nodeDist = hypot( x - nodeX, y - nodeY );
    nodeIn = true;
  }

  // Choose
  if (( startIn && !nodeIn ) || ( startIn && nodeIn && startDist < nodeDist ) )
  {
    x = startX; y = startY;
  }
  else if (( !startIn && nodeIn ) || ( startIn && nodeIn && startDist > nodeDist ) )
  {
    x = nodeX; y = nodeY;
  }

  point.setX( x );
  point.setY( y );
}

void QgsGrassEdit::newPoint( void )
{
  startTool( QgsGrassEdit::NEW_POINT );
}

void QgsGrassEdit::newLine( void )
{
  QgsDebugMsg( "entered." );
  startTool( QgsGrassEdit::NEW_LINE );
}

void QgsGrassEdit::newBoundary( void )
{
  QgsDebugMsg( "entered." );
  startTool( QgsGrassEdit::NEW_BOUNDARY );
}

void QgsGrassEdit::newCentroid( void )
{
  startTool( QgsGrassEdit::NEW_CENTROID );
}

void QgsGrassEdit::moveVertex( void )
{
  startTool( QgsGrassEdit::MOVE_VERTEX );
}

void QgsGrassEdit::addVertex( void )
{
  startTool( QgsGrassEdit::ADD_VERTEX );
}

void QgsGrassEdit::deleteVertex( void )
{
  startTool( QgsGrassEdit::DELETE_VERTEX );
}

void QgsGrassEdit::splitLine( void )
{
  startTool( QgsGrassEdit::SPLIT_LINE );
}

void QgsGrassEdit::moveLine( void )
{
  startTool( QgsGrassEdit::MOVE_LINE );
}

void QgsGrassEdit::deleteLine( void )
{
  startTool( QgsGrassEdit::DELETE_LINE );
}

void QgsGrassEdit::editCats( void )
{
  startTool( QgsGrassEdit::EDIT_CATS );
}

void QgsGrassEdit::editAttributes( void )
{
  startTool( QgsGrassEdit::EDIT_ATTRIBUTES );
}

void QgsGrassEdit::startTool( int tool )
{
  QgsDebugMsg( QString( "tool = %1" ).arg( tool ) );

  // Delete last dynamic drawing from canvas
  eraseDynamic();
  if ( mSelectedLine > 0 )
    displayElement( mSelectedLine, mSymb[mLineSymb[mSelectedLine]], mSize );

  // close old tool
  if ( mMapTool )
  {
    delete mMapTool;
    mMapTool = NULL;
  }

  // All necessary data were written -> reset mEditPoints etc.
  Vect_reset_line( mEditPoints );
  mSelectedLine = 0;

  // TODO: mTool != NEW_LINE is a hack for lines until more buttons can be received
  if ( mAttributes && mTool != QgsGrassEdit::NEW_LINE && mTool != QgsGrassEdit::NEW_BOUNDARY )
  {
    delete mAttributes;
    mAttributes = 0;
  }

  // Start new tool
  mTool = tool;

  switch ( mTool )
  {
    case NEW_POINT:
      mMapTool = new QgsGrassEditNewPoint( this, false );
      mMapTool->setAction( mNewPointAction );
      break;

    case NEW_CENTROID:
      mMapTool = new QgsGrassEditNewPoint( this, true );
      mMapTool->setAction( mNewCentroidAction );
      break;

    case NEW_LINE:
      mMapTool = new QgsGrassEditNewLine( this, false );
      mMapTool->setAction( mNewLineAction );
      break;

    case NEW_BOUNDARY:
      mMapTool = new QgsGrassEditNewLine( this, true );
      mMapTool->setAction( mNewBoundaryAction );
      break;

    case MOVE_VERTEX:
      mMapTool = new QgsGrassEditMoveVertex( this );
      mMapTool->setAction( mMoveVertexAction );
      break;

    case ADD_VERTEX:
      mMapTool = new QgsGrassEditAddVertex( this );
      mMapTool->setAction( mAddVertexAction );
      break;

    case DELETE_VERTEX:
      mMapTool = new QgsGrassEditDeleteVertex( this );
      mMapTool->setAction( mDeleteVertexAction );
      break;

    case MOVE_LINE:
      mMapTool = new QgsGrassEditMoveLine( this );
      mMapTool->setAction( mMoveLineAction );
      break;

    case DELETE_LINE:
      mMapTool = new QgsGrassEditDeleteLine( this );
      mMapTool->setAction( mDeleteLineAction );
      break;

    case SPLIT_LINE:
      mMapTool = new QgsGrassEditSplitLine( this );
      mMapTool->setAction( mSplitLineAction );
      break;

    case EDIT_ATTRIBUTES:
      mMapTool = new QgsGrassEditAttributes( this );
      mMapTool->setAction( mEditAttributesAction );
      break;

    case EDIT_CATS:
      mTool = NONE;
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Tool not yet implemented." ) );
      break;

    default:
      QgsDebugMsg( "Unknown tool" );
      break;
  }

  // assign newly created tool to map canvas
  mCanvas->setMapTool( mMapTool );
}

void QgsGrassEdit::checkOrphan( int field, int cat )
{
  QgsDebugMsg( QString( "field = %1 cat = %2" ).arg( field ).arg( cat ) );

  int orphan;
  QString error = mProvider->isOrphan( field, cat, orphan );

  if ( !error.isEmpty() )
  {
    QMessageBox::warning( 0, tr( "Warning" ),
                          tr( "Cannot check orphan record: %1" ).arg( error ) );
    return;
  }

  if ( !orphan )
    return;

  QMessageBox::StandardButton ret = QMessageBox::question( 0, tr( "Warning" ),
                                    tr( "Orphan record was left in attribute table. "
                                        "<br>Delete the record?" ),
                                    QMessageBox::Ok | QMessageBox::Cancel );

  if ( ret == QMessageBox::Cancel )
    return;

  // Delete record
  error = mProvider->deleteAttribute( field, cat );
  if ( !error.isEmpty() )
  {
    QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot delete orphan record: " )
                          + error );
    return;
  }
}

void QgsGrassEdit::addAttributes( int field, int cat )
{
  QString key = mProvider->key( field );

  QString lab;
  lab.sprintf( "%d:%d", field, cat );
  int tab = mAttributes->addTab( lab );
  mAttributes->setField( tab, field );

  QString catLabel;
  if ( key.isEmpty() )
  {
    catLabel = "Category";
  }
  else
  {
    catLabel = key;
  }
  mAttributes->setCat( tab, catLabel, cat );

  if ( !key.isEmpty() )   // Database link defined
  {
    QVector<QgsField> *cols = mProvider->columns( field );

    if ( cols->size() == 0 )
    {
      QString str;
      str.setNum( field );
      QMessageBox::warning( 0, tr( "Warning" ), tr( "Cannot describe table for field %1" ).arg( str ) );
    }
    else
    {
      QgsAttributeMap *atts = mProvider->attributes( field, cat );

      if ( atts->size() == 0 )   // cannot select attributes
      {
        mAttributes->addTextRow( tab, "WARNING: ATTRIBUTES MISSING" );
      }
      else
      {
        for ( int j = 0; j < cols->size(); j++ )
        {
          QgsField col = ( *cols )[j];
          QVariant att = ( *atts )[j];
          QgsDebugMsg( QString( " name = %1" ).arg( col.name() ) );

          if ( col.name() != key )
          {
            QgsDebugMsg( QString( " value = %1" ).arg( att.toString() ) );
            mAttributes->addAttribute( tab, col.name(), att.toString(), col.typeName() );
          }
        }
      }
      delete atts;
    }
    delete cols;
  }
}

void QgsGrassEdit::addCat( int line )
{
  int mode = mCatModeBox->currentIndex();
  int field = mFieldBox->currentText().toInt();
  int cat = mCatEntry->text().toInt();

  int type = mProvider->readLine( mPoints, mCats, line );
  if ( mode == CAT_MODE_NEXT || mode == CAT_MODE_MANUAL )
  {
    Vect_cat_set( mCats, field, cat );
  }

  line = mProvider->rewriteLine( line, type, mPoints, mCats );
  mSelectedLine = line;
  if ( mAttributes )
    mAttributes->setLine( line );
  updateSymb();
  increaseMaxCat();

  // Insert new DB record if link is defined and the record for this cat does not exist
  QString key = mProvider->key( field );
  if ( !key.isEmpty() )   // Database link defined
  {
    QgsAttributeMap *atts = mProvider->attributes( field, cat );

    if ( atts->size() == 0 )   // Nothing selected
    {
      QString error = mProvider->insertAttributes( field, cat );
      if ( !error.isEmpty() )
      {
        QMessageBox::warning( 0, tr( "Warning" ), error );
      }
    }

    delete atts;
  }

  if ( mAttributes )
    addAttributes( field, cat );
}

void QgsGrassEdit::deleteCat( int line, int field, int cat )
{
  QgsDebugMsg( "entered." );

  int type = mProvider->readLine( mPoints, mCats, line );
  Vect_field_cat_del( mCats, field, cat );

  line = mProvider->rewriteLine( line, type, mPoints, mCats );
  mSelectedLine = line;
  if ( mAttributes )
    mAttributes->setLine( line );

  // Check orphan record
  checkOrphan( field, cat );

  updateSymb();
}


void QgsGrassEdit::postRender( QPainter * )
{
  QgsDebugMsg( "entered." );

  // Warning: it seems that this slot can be called even
  //          after disconnect (is it a queue?)
  //          -> check mValid

  if ( !mValid )
    return;

  displayMap();

  // Redisplay highlighted
  if ( mSelectedLine )
  {
    displayElement( mSelectedLine, mSymb[SYMB_HIGHLIGHT], mSize );
  }
}

void QgsGrassEdit::displayMap()
{
  QgsDebugMsg( "entered." );

  mTransform = mCanvas->getCoordinateTransform();

  // re-create pixmap - it's transparent by default
  *mPixmap = QPixmap( mCanvas->size() );
  mPixmap->fill( QColor( 0, 0, 0, 0 ) );

  QPainter *painter = new QPainter();
  painter->begin( mPixmap );

  // Display lines
  int nlines = mProvider->numLines();

  QPen pen;

  // TODO?: 2 loops, first lines, then points
  for ( int line = 1; line <= nlines; line++ )
  {
    displayElement( line, mSymb[mLineSymb[line]], mSize, painter );
  }

  // Display nodes
  int nnodes = mProvider->numNodes();

  pen.setColor( QColor( 255, 0, 0 ) );

  if ( mSymbDisplay[SYMB_NODE_1] || mSymbDisplay[SYMB_NODE_2] )
  {
    for ( int node = 1; node <= nnodes; node++ )
    {
      if ( mNodeSymb[node] == SYMB_NODE_0 )
        continue; // do not display nodes with points only
      displayNode( node, mSymb[mNodeSymb[node]], mSize, painter );
    }
  }

  painter->end();
  delete painter;

  // porting mCanvas->update();
  mCanvasEdit->update();
  mRubberBandIcon->update();
  mRubberBandLine->update();
}

void QgsGrassEdit::displayUpdated( void )
{
  QgsDebugMsg( "entered." );

  mTransform = mCanvas->getCoordinateTransform();
  mProjectionEnabled = ( QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 ) != 0 );

  QPainter *painter = new QPainter();
  painter->begin( mPixmap );

  // Display lines
  int nlines = mProvider->numUpdatedLines();

  for ( int i = 0; i < nlines; i++ )
  {
    int line = mProvider->updatedLine( i );
    if ( !( mProvider->lineAlive( line ) ) )
      continue;

    displayElement( line, mSymb[mLineSymb[line]], mSize, painter );
  }

  // Display nodes
  int nnodes = mProvider->numUpdatedNodes();
  for ( int i = 0; i < nnodes; i++ )
  {
    int node = mProvider->updatedNode( i );
    if ( !( mProvider->nodeAlive( node ) ) )
      continue;
    if ( mNodeSymb[node] == SYMB_NODE_0 )
      continue; // do not display nodes with points only
    displayNode( node, mSymb[mNodeSymb[node]], mSize, painter );
  }

  painter->end();
  delete painter;

  // porting mCanvas->update();
  mCanvasEdit->update();
  mRubberBandIcon->update();
  mRubberBandLine->update();
}

void QgsGrassEdit::displayElement( int line, const QPen & pen, int size, QPainter *painter )
{
  QgsDebugMsg( QString( "line = %1" ).arg( line ) );

  // is it a valid line?
  if ( line == 0 )
    return;

  if ( !mSymbDisplay[mLineSymb[line]] )
    return;

  int type = mProvider->readLine( mPoints, NULL, line );
  if ( type < 0 )
    return;

  QPainter *myPainter;
  if ( !painter )
  {
    myPainter = new QPainter();
    myPainter->begin( mPixmap );
  }
  else
  {
    myPainter = painter;
  }

  if ( type & GV_POINTS )
  {
    displayIcon( mPoints->x[0], mPoints->y[0], pen, QgsVertexMarker::ICON_CROSS, size, myPainter );
  }
  else   // line
  {
    QgsPoint point;
    QPolygon pointArray( mPoints->n_points );

    for ( int i = 0; i < mPoints->n_points; i++ )
    {
      point.setX( mPoints->x[i] );
      point.setY( mPoints->y[i] );
      point = transformLayerToCanvas( point );
      pointArray.setPoint( i, qRound( point.x() ), qRound( point.y() ) );
    }

    myPainter->setPen( pen );
    myPainter->drawPolyline( pointArray );
  }

  if ( !painter )
  {
    myPainter->end();
    // porting mCanvas->update();
    mCanvasEdit->update();
    delete myPainter;
  }
}

void QgsGrassEdit::eraseElement( int line )
{
  QgsDebugMsg( QString( "line = %1" ).arg( line ) );

  int type = mProvider->readLine( NULL, NULL, line );
  if ( type < 0 )
    return;

  // Erase line
  displayElement( line, mSymb[SYMB_BACKGROUND], mSize );

  // Erase nodes
  if ( type & GV_LINES )
  {
    int node1, node2;
    mProvider->lineNodes( line, &node1, &node2 );

    double x, y;
    mProvider->nodeCoor( node1, &x, &y );
    displayIcon( x, y, mSymb[SYMB_BACKGROUND], QgsVertexMarker::ICON_X, mSize );

    mProvider->nodeCoor( node2, &x, &y );
    displayIcon( x, y, mSymb[SYMB_BACKGROUND], QgsVertexMarker::ICON_X, mSize );
  }
}

void QgsGrassEdit::eraseDynamic( void )
{
  displayDynamic( 0, 0.0, 0.0, QgsVertexMarker::ICON_NONE, 0 );
}

void QgsGrassEdit::displayDynamic( struct line_pnts *Points )
{
  displayDynamic( Points, 0.0, 0.0, QgsVertexMarker::ICON_NONE, 0 );
}

void QgsGrassEdit::displayDynamic( double x, double y, int type, int size )
{
  QgsDebugMsg( "entered." );

  displayDynamic( 0, x, y, type, size );
}

void QgsGrassEdit::displayDynamic( struct line_pnts *Points, double x, double y, int type, int size )
{
  QgsDebugMsg( QString( "Points = %1 type = %2" ).arg( QString::number(( qulonglong )Points, 16 ).toLocal8Bit().constData() ).arg( type ) );
  QgsPoint point;

  //mTransform = mCanvas->getCoordinateTransform();

  // Dynamic points are in layer coordinate system, we have to
  // reproject them to current coordinate system if necessary

  mRubberBandLine->reset();

  if ( Points )
  {
    for ( int i = 0; i < Points->n_points; i++ )
    {
      point.setX( Points->x[i] );
      point.setY( Points->y[i] );
      point = transformLayerToMap( point );
      mRubberBandLine->addPoint( point, false ); // false = don't update now
    }
    // Now add the last point again and force update of rubberband.
    // This should improve the performance as canvas is updated only once
    // and not with every added point to rubberband.
    mRubberBandLine->addPoint( point, true );
  }

  mRubberBandIcon->setIconType( type );
  mRubberBandIcon->setIconSize( size );
  point = transformLayerToMap( QgsPoint( x, y ) );
  mRubberBandIcon->setCenter( point );
}

void QgsGrassEdit::displayNode( int node, const QPen & pen, int size, QPainter *painter )
{
  if ( !mSymbDisplay[mNodeSymb[node]] )
    return;

  double x, y;

  if ( !( mProvider->nodeCoor( node, &x, &y ) ) )
    return;

  displayIcon( x, y, pen, QgsVertexMarker::ICON_X, size, painter );
}

QgsPoint QgsGrassEdit::transformLayerToCanvas( QgsPoint point )
{
  point = mCanvas->mapSettings().layerToMapCoordinates( mLayer, point );
  return mTransform->transform( point );
}

QgsPoint QgsGrassEdit::transformLayerToMap( QgsPoint point )
{
  return mCanvas->mapSettings().layerToMapCoordinates( mLayer, point );
}

void QgsGrassEdit::displayIcon( double x, double y, const QPen & pen,
                                int type, int size, QPainter *painter )
{
  QgsPoint point;
  QPolygon pointArray( 2 );

  point.setX( x );
  point.setY( y );

  point = transformLayerToCanvas( point );

  int px = qRound( point.x() );
  int py = qRound( point.y() );
  int m = ( size - 1 ) / 2;

  QPainter *myPainter;
  if ( !painter )
  {
    myPainter = new QPainter();
    myPainter->begin( mPixmap );
  }
  else
  {
    myPainter = painter;
  }

  myPainter->setPen( pen );

  switch ( type )
  {
    case QgsVertexMarker::ICON_CROSS :
      pointArray.setPoint( 0, px - m, py );
      pointArray.setPoint( 1, px + m, py );
      myPainter->drawPolyline( pointArray );

      pointArray.setPoint( 0, px, py + m );
      pointArray.setPoint( 1, px, py - m );
      myPainter->drawPolyline( pointArray );
      break;
    case QgsVertexMarker::ICON_X :
      pointArray.setPoint( 0, px - m, py + m );
      pointArray.setPoint( 1, px + m, py - m );
      myPainter->drawPolyline( pointArray );

      pointArray.setPoint( 0, px - m, py - m );
      pointArray.setPoint( 1, px + m, py + m );
      myPainter->drawPolyline( pointArray );
      break;
    case QgsVertexMarker::ICON_BOX :
      pointArray.resize( 5 );
      pointArray.setPoint( 0, px - m, py - m );
      pointArray.setPoint( 1, px + m, py - m );
      pointArray.setPoint( 2, px + m, py + m );
      pointArray.setPoint( 3, px - m, py + m );
      pointArray.setPoint( 4, px - m, py - m );
      myPainter->drawPolyline( pointArray );
      break;
  }

  if ( !painter )
  {
    myPainter->end();
    //mCanvas->update();
    mCanvasEdit->update();
    delete myPainter;
  }
}

void QgsGrassEdit::setCanvasPrompt( QString left, QString mid, QString right )
{
  QgsDebugMsg( "entered." );
  mCanvasPrompt = "";
  if ( left.length() > 0 )
    mCanvasPrompt.append( tr( "Left: %1" ).arg( left ) );
  if ( mid.length() > 0 )
    mCanvasPrompt.append( tr( " -- Middle: %1" ).arg( mid ) );
  if ( right.length() > 0 )
    mCanvasPrompt.append( tr( " -- Right: %1" ).arg( right ) );
}

void QgsGrassEdit::attributesClosed()
{
  mAttributes = 0;
}
