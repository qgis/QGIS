/***************************************************************************
    qgssvgselectorwidget.cpp - group and preview selector for SVG files
                               built off of work in qgssymbollayerwidget

    ---------------------
    begin                : April 2, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssvgselectorwidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgssymbollayerutils.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsfieldexpressionwidget.h"
#include "qgssymbollayerwidget.h"
#include "qgsvectorlayer.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QModelIndex>
#include <QPixmapCache>
#include <QStyle>
#include <QTime>
#include <QMenu>

// QgsSvgSelectorLoader

///@cond PRIVATE
QgsSvgSelectorLoader::QgsSvgSelectorLoader( QObject *parent )
  : QThread( parent )
{
}

QgsSvgSelectorLoader::~QgsSvgSelectorLoader()
{
  stop();
}

void QgsSvgSelectorLoader::run()
{
  mCanceled = false;
  mQueuedSvgs.clear();
  mTraversedPaths.clear();

  // start with a small initial timeout (ms)
  mTimerThreshold = 10;
  mTimer.start();

  loadPath( mPath );

  if ( !mQueuedSvgs.isEmpty() )
  {
    // make sure we notify model of any remaining queued svgs (ie svgs added since last foundSvgs() signal was emitted)
    emit foundSvgs( mQueuedSvgs );
  }
  mQueuedSvgs.clear();
}

void QgsSvgSelectorLoader::stop()
{
  mCanceled = true;
  while ( isRunning() ) {}
}

void QgsSvgSelectorLoader::loadPath( const QString &path )
{
  if ( mCanceled )
    return;

  // QgsDebugMsg( QStringLiteral( "loading path: %1" ).arg( path ) );

  if ( path.isEmpty() )
  {
    QStringList svgPaths = QgsApplication::svgPaths();
    const auto constSvgPaths = svgPaths;
    for ( const QString &svgPath : constSvgPaths )
    {
      if ( mCanceled )
        return;

      if ( !svgPath.isEmpty() )
      {
        loadPath( svgPath );
      }
    }
  }
  else
  {
    QDir dir( path );

    //guard against circular symbolic links
    QString canonicalPath = dir.canonicalPath();
    if ( mTraversedPaths.contains( canonicalPath ) )
      return;

    mTraversedPaths.insert( canonicalPath );

    loadImages( path );

    const auto constEntryList = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
    for ( const QString &item : constEntryList )
    {
      if ( mCanceled )
        return;

      QString newPath = dir.path() + '/' + item;
      loadPath( newPath );
      // QgsDebugMsg( QStringLiteral( "added path: %1" ).arg( newPath ) );
    }
  }
}

void QgsSvgSelectorLoader::loadImages( const QString &path )
{
  QDir dir( path );
  const auto constEntryList = dir.entryList( QStringList( "*.svg" ), QDir::Files );
  for ( const QString &item : constEntryList )
  {
    if ( mCanceled )
      return;

    // TODO test if it is correct SVG
    QString svgPath = dir.path() + '/' + item;
    // QgsDebugMsg( QStringLiteral( "adding svg: %1" ).arg( svgPath ) );

    // add it to the list of queued SVGs
    mQueuedSvgs << svgPath;

    // we need to avoid spamming the model with notifications about new svgs, so foundSvgs
    // is only emitted for blocks of SVGs (otherwise the view goes all flickery)
    if ( mTimer.elapsed() > mTimerThreshold && !mQueuedSvgs.isEmpty() )
    {
      emit foundSvgs( mQueuedSvgs );
      mQueuedSvgs.clear();

      // increase the timer threshold - this ensures that the first lots of svgs loaded are added
      // to the view quickly, but as the list grows new svgs are added at a slower rate.
      // ie, good for initial responsiveness but avoid being spammy as the list grows.
      if ( mTimerThreshold < 1000 )
        mTimerThreshold *= 2;
      mTimer.restart();
    }
  }
}


//
// QgsSvgGroupLoader
//

QgsSvgGroupLoader::QgsSvgGroupLoader( QObject *parent )
  : QThread( parent )
{

}

QgsSvgGroupLoader::~QgsSvgGroupLoader()
{
  stop();
}

void QgsSvgGroupLoader::run()
{
  mCanceled = false;
  mTraversedPaths.clear();

  while ( !mCanceled && !mParentPaths.isEmpty() )
  {
    QString parentPath = mParentPaths.takeFirst();
    loadGroup( parentPath );
  }
}

void QgsSvgGroupLoader::stop()
{
  mCanceled = true;
  while ( isRunning() ) {}
}

void QgsSvgGroupLoader::loadGroup( const QString &parentPath )
{
  QDir parentDir( parentPath );

  //guard against circular symbolic links
  QString canonicalPath = parentDir.canonicalPath();
  if ( mTraversedPaths.contains( canonicalPath ) )
    return;

  mTraversedPaths.insert( canonicalPath );

  const auto constEntryList = parentDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot );
  for ( const QString &item : constEntryList )
  {
    if ( mCanceled )
      return;

    emit foundPath( parentPath, item );
    mParentPaths.append( parentDir.path() + '/' + item );
  }
}

///@endcond




QgsSvgSelectorFilterModel::QgsSvgSelectorFilterModel( QObject *parent, const QString &path, int iconSize )
  : QSortFilterProxyModel( parent )
{
  mModel = new QgsSvgSelectorListModel( parent, path, iconSize );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  setSourceModel( mModel );
  setFilterRole( Qt::UserRole );
}

//,
// QgsSvgSelectorListModel
//

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject *parent, int iconSize )
  : QgsSvgSelectorListModel( parent, QString(), iconSize )
{}

QgsSvgSelectorListModel::QgsSvgSelectorListModel( QObject *parent, const QString &path, int iconSize )
  : QAbstractListModel( parent )
  , mSvgLoader( new QgsSvgSelectorLoader( this ) )
  , mIconSize( iconSize )
{
  mSvgLoader->setPath( path );
  connect( mSvgLoader, &QgsSvgSelectorLoader::foundSvgs, this, &QgsSvgSelectorListModel::addSvgs );
  mSvgLoader->start();
}

int QgsSvgSelectorListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mSvgFiles.count();
}

QPixmap QgsSvgSelectorListModel::createPreview( const QString &entry ) const
{
  // render SVG file
  QColor fill, stroke;
  double strokeWidth, fillOpacity, strokeOpacity;
  bool fillParam, fillOpacityParam, strokeParam, strokeWidthParam, strokeOpacityParam;
  bool hasDefaultFillColor = false, hasDefaultFillOpacity = false, hasDefaultStrokeColor = false,
       hasDefaultStrokeWidth = false, hasDefaultStrokeOpacity = false;
  QgsApplication::svgCache()->containsParams( entry, fillParam, hasDefaultFillColor, fill,
      fillOpacityParam, hasDefaultFillOpacity, fillOpacity,
      strokeParam, hasDefaultStrokeColor, stroke,
      strokeWidthParam, hasDefaultStrokeWidth, strokeWidth,
      strokeOpacityParam, hasDefaultStrokeOpacity, strokeOpacity );

  //if defaults not set in symbol, use these values
  if ( !hasDefaultFillColor )
    fill = QColor( 200, 200, 200 );
  fill.setAlphaF( hasDefaultFillOpacity ? fillOpacity : 1.0 );
  if ( !hasDefaultStrokeColor )
    stroke = Qt::black;
  stroke.setAlphaF( hasDefaultStrokeOpacity ? strokeOpacity : 1.0 );
  if ( !hasDefaultStrokeWidth )
    strokeWidth = 0.2;

  bool fitsInCache; // should always fit in cache at these sizes (i.e. under 559 px ^ 2, or half cache size)
  QImage img = QgsApplication::svgCache()->svgAsImage( entry, mIconSize, fill, stroke, strokeWidth, 3.5 /*appr. 88 dpi*/, fitsInCache );
  return QPixmap::fromImage( img );
}

QVariant QgsSvgSelectorListModel::data( const QModelIndex &index, int role ) const
{
  QString entry = mSvgFiles.at( index.row() );

  if ( role == Qt::DecorationRole ) // icon
  {
    QPixmap *pixmap = nullptr;
    if ( !QPixmapCache::find( entry, pixmap ) || !pixmap )
    {
      QPixmap newPixmap = createPreview( entry );
      QPixmapCache::insert( entry, newPixmap );
      return newPixmap;
    }
    else
    {
      return *pixmap;
    }
  }
  else if ( role == Qt::UserRole || role == Qt::ToolTipRole )
  {
    return entry;
  }

  return QVariant();
}

void QgsSvgSelectorListModel::addSvgs( const QStringList &svgs )
{
  beginInsertRows( QModelIndex(), mSvgFiles.count(), mSvgFiles.count() + svgs.size() - 1 );
  mSvgFiles.append( svgs );
  endInsertRows();
}





//--- QgsSvgSelectorGroupsModel

QgsSvgSelectorGroupsModel::QgsSvgSelectorGroupsModel( QObject *parent )
  : QStandardItemModel( parent )
  , mLoader( new QgsSvgGroupLoader( this ) )
{
  QStringList svgPaths = QgsApplication::svgPaths();
  QStandardItem *parentItem = invisibleRootItem();
  QStringList parentPaths;
  parentPaths.reserve( svgPaths.size() );

  for ( int i = 0; i < svgPaths.size(); i++ )
  {
    QDir dir( svgPaths.at( i ) );
    QStandardItem *baseGroup = nullptr;

    if ( dir.path().contains( QgsApplication::pkgDataPath() ) )
    {
      baseGroup = new QStandardItem( tr( "App Symbols" ) );
    }
    else if ( dir.path().contains( QgsApplication::qgisSettingsDirPath() ) )
    {
      baseGroup = new QStandardItem( tr( "User Symbols" ) );
    }
    else
    {
      baseGroup = new QStandardItem( dir.dirName() );
    }
    baseGroup->setData( QVariant( svgPaths.at( i ) ) );
    baseGroup->setEditable( false );
    baseGroup->setCheckable( false );
    baseGroup->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ) );
    baseGroup->setToolTip( dir.path() );
    parentItem->appendRow( baseGroup );
    parentPaths << svgPaths.at( i );
    mPathItemHash.insert( svgPaths.at( i ), baseGroup );
    QgsDebugMsgLevel( QStringLiteral( "SVG base path %1: %2" ).arg( i ).arg( baseGroup->data().toString() ), 2 );
  }
  mLoader->setParentPaths( parentPaths );
  connect( mLoader, &QgsSvgGroupLoader::foundPath, this, &QgsSvgSelectorGroupsModel::addPath );
  mLoader->start();
}

QgsSvgSelectorGroupsModel::~QgsSvgSelectorGroupsModel()
{
  mLoader->stop();
}

void QgsSvgSelectorGroupsModel::addPath( const QString &parentPath, const QString &item )
{
  QStandardItem *parentGroup = mPathItemHash.value( parentPath );
  if ( !parentGroup )
    return;

  QString fullPath = parentPath + '/' + item;
  QStandardItem *group = new QStandardItem( item );
  group->setData( QVariant( fullPath ) );
  group->setEditable( false );
  group->setCheckable( false );
  group->setToolTip( fullPath );
  group->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ) );
  parentGroup->appendRow( group );
  mPathItemHash.insert( fullPath, group );
}


//-- QgsSvgSelectorWidget

QgsSvgSelectorWidget::QgsSvgSelectorWidget( QWidget *parent )
  : QWidget( parent )
{
  // TODO: in-code gui setup with option to vertically or horizontally stack SVG groups/images widgets
  setupUi( this );

  mIconSize = std::max( 30, static_cast< int >( std::round( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 3 ) ) );
  mImagesListView->setGridSize( QSize( mIconSize * 1.2, mIconSize * 1.2 ) );
  mImagesListView->setUniformItemSizes( false );

  mGroupsTreeView->setHeaderHidden( true );
  populateList();

  connect( mSvgFilterLineEdit, &QgsFilterLineEdit::textChanged, this, [ = ]( const QString & filterText )
  {
    if ( !mImagesListView->selectionModel()->selectedIndexes().isEmpty() )
    {
      disconnect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgSelectorWidget::svgSelectionChanged );
      mImagesListView->selectionModel()->clearSelection();
      connect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgSelectorWidget::svgSelectionChanged );
    }
    qobject_cast<QgsSvgSelectorFilterModel *>( mImagesListView->model() )->setFilterFixedString( filterText );
  } );


  mParametersModel = new QgsSvgParametersModel( this );
  mParametersTreeView->setModel( mParametersModel );
  mParametersGroupBox->setVisible( mAllowParameters );

  mParametersTreeView->setItemDelegateForColumn( static_cast<int>( QgsSvgParametersModel::Column::ExpressionColumn ), new QgsSvgParameterValueDelegate( this ) );
  mParametersTreeView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
  mParametersTreeView->header()->setStretchLastSection( true );
  mParametersTreeView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mParametersTreeView->setSelectionMode( QAbstractItemView::MultiSelection );
  mParametersTreeView->setEditTriggers( QAbstractItemView::DoubleClicked );

  connect( mParametersModel, &QgsSvgParametersModel::parametersChanged, this, &QgsSvgSelectorWidget::svgParametersChanged );
  connect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgSelectorWidget::svgSelectionChanged );
  connect( mGroupsTreeView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSvgSelectorWidget::populateIcons );
  connect( mAddParameterButton, &QToolButton::clicked, mParametersModel, &QgsSvgParametersModel::addParameter );
  connect( mRemoveParameterButton, &QToolButton::clicked, this, [ = ]()
  {
    const QModelIndexList selectedRows = mParametersTreeView->selectionModel()->selectedRows();
    if ( selectedRows.count() > 0 )
      mParametersModel->removeParameters( selectedRows );
  } );

  connect( mSourceLineEdit, &QgsPictureSourceLineEditBase::sourceChanged, this, &QgsSvgSelectorWidget::svgSelected );
}

void QgsSvgSelectorWidget::initParametersModel( const QgsExpressionContextGenerator *generator, QgsVectorLayer *layer )
{
  mParametersModel->setExpressionContextGenerator( generator );
  mParametersModel->setLayer( layer );
}

void QgsSvgSelectorWidget::setSvgPath( const QString &svgPath )
{
  mCurrentSvgPath = svgPath;

  whileBlocking( mSourceLineEdit )->setSource( svgPath );

  mImagesListView->selectionModel()->blockSignals( true );
  QAbstractItemModel *m = mImagesListView->model();
  QItemSelectionModel *selModel = mImagesListView->selectionModel();
  for ( int i = 0; i < m->rowCount(); i++ )
  {
    QModelIndex idx( m->index( i, 0 ) );
    if ( m->data( idx ).toString() == svgPath )
    {
      selModel->select( idx, QItemSelectionModel::SelectCurrent );
      selModel->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
      mImagesListView->scrollTo( idx );
      break;
    }
  }
  mImagesListView->selectionModel()->blockSignals( false );
}

void QgsSvgSelectorWidget::setSvgParameters( const QMap<QString, QgsProperty> &parameters )
{
  mParametersModel->setParameters( parameters );
}

QString QgsSvgSelectorWidget::currentSvgPath() const
{
  return mCurrentSvgPath;
}

void QgsSvgSelectorWidget::setAllowParameters( bool allow )
{
  if ( mAllowParameters == allow )
    return;

  mAllowParameters = allow;
  mParametersGroupBox->setVisible( allow );
}

void QgsSvgSelectorWidget::setBrowserVisible( bool visible )
{
  if ( mBrowserVisible == visible )
    return;

  mBrowserVisible = visible;
  mSvgBrowserGroupBox->setVisible( visible );
}

QgsPropertyOverrideButton *QgsSvgSelectorWidget::propertyOverrideToolButton() const
{
  return mSourceLineEdit->propertyOverrideToolButton();
}

void QgsSvgSelectorWidget::updateCurrentSvgPath( const QString &svgPath )
{
  mCurrentSvgPath = svgPath;
  emit svgSelected( currentSvgPath() );
}

void QgsSvgSelectorWidget::svgSelectionChanged( const QModelIndex &idx )
{
  QString filePath = idx.data( Qt::UserRole ).toString();
  whileBlocking( mSourceLineEdit )->setSource( filePath );
  updateCurrentSvgPath( filePath );
}

void QgsSvgSelectorWidget::populateIcons( const QModelIndex &idx )
{
  QString path = idx.data( Qt::UserRole + 1 ).toString();

  QAbstractItemModel *oldModel = mImagesListView->model();
  QgsSvgSelectorFilterModel *m = new QgsSvgSelectorFilterModel( mImagesListView, path, mIconSize );
  mImagesListView->setModel( m );
  connect( mSvgFilterLineEdit, &QgsFilterLineEdit::textChanged, m, &QSortFilterProxyModel::setFilterFixedString );
  delete oldModel; //explicitly delete old model to force any background threads to stop

  connect( mImagesListView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsSvgSelectorWidget::svgSelectionChanged );
}

void QgsSvgSelectorWidget::svgSourceChanged( const QString &text )
{
  QString resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( text, QgsProject::instance()->pathResolver() );
  bool validSVG = !resolvedPath.isNull();

  updateCurrentSvgPath( validSVG ? resolvedPath : text );
}

void QgsSvgSelectorWidget::populateList()
{
  QgsSvgSelectorGroupsModel *g = new QgsSvgSelectorGroupsModel( mGroupsTreeView );
  mGroupsTreeView->setModel( g );
  // Set the tree expanded at the first level
  int rows = g->rowCount( g->indexFromItem( g->invisibleRootItem() ) );
  for ( int i = 0; i < rows; i++ )
  {
    mGroupsTreeView->setExpanded( g->indexFromItem( g->item( i ) ), true );
  }

  // Initially load the icons in the List view without any grouping
  QAbstractItemModel *oldModel = mImagesListView->model();
  QgsSvgSelectorFilterModel *m = new QgsSvgSelectorFilterModel( mImagesListView );
  mImagesListView->setModel( m );
  delete oldModel; //explicitly delete old model to force any background threads to stop
}

//-- QgsSvgSelectorDialog

QgsSvgSelectorDialog::QgsSvgSelectorDialog( QWidget *parent, Qt::WindowFlags fl,
    QDialogButtonBox::StandardButtons buttons,
    Qt::Orientation orientation )
  : QDialog( parent, fl )
{
  // TODO: pass 'orientation' to QgsSvgSelectorWidget for customizing its layout, once implemented
  Q_UNUSED( orientation )

  // create buttonbox
  mButtonBox = new QDialogButtonBox( buttons, orientation, this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  setMinimumSize( 480, 320 );

  // dialog's layout
  mLayout = new QVBoxLayout();
  mSvgSelector = new QgsSvgSelectorWidget( this );
  mLayout->addWidget( mSvgSelector );

  mLayout->addWidget( mButtonBox );
  setLayout( mLayout );
}


///@cond PRIVATE


QgsSvgParametersModel::QgsSvgParametersModel( QObject *parent )
  : QAbstractTableModel( parent )
{
  connect( this, &QAbstractTableModel::rowsInserted, this, [ = ]() {emit parametersChanged( parameters() );} );
  connect( this, &QAbstractTableModel::rowsRemoved, this, [ = ]() {emit parametersChanged( parameters() );} );
  connect( this, &QAbstractTableModel::dataChanged, this, [ = ]() {emit parametersChanged( parameters() );} );
}

void QgsSvgParametersModel::setParameters( const QMap<QString, QgsProperty> &parameters )
{
  beginResetModel();
  mParameters.clear();
  QMap<QString, QgsProperty>::const_iterator paramIt = parameters.constBegin();
  for ( ; paramIt != parameters.constEnd(); ++paramIt )
  {
    mParameters << Parameter( paramIt.key(), paramIt.value() );
  }
  endResetModel();
}

QMap<QString, QgsProperty> QgsSvgParametersModel::parameters() const
{
  QMap<QString, QgsProperty> params;
  for ( const Parameter &param : std::as_const( mParameters ) )
  {
    if ( !param.name.isEmpty() )
      params.insert( param.name, param.property );
  }
  return params;
}

void QgsSvgParametersModel::removeParameters( const QModelIndexList &indexList )
{
  if ( indexList.isEmpty() )
    return;

  auto mm = std::minmax_element( indexList.constBegin(), indexList.constEnd(), []( const QModelIndex & i1, const QModelIndex & i2 ) {return i1.row() < i2.row();} );

  beginRemoveRows( QModelIndex(), ( *mm.first ).row(), ( *mm.second ).row() );
  for ( const QModelIndex &index : indexList )
    mParameters.removeAt( index.row() );
  endRemoveRows();
}

void QgsSvgParametersModel::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsSvgParametersModel::setExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

int QgsSvgParametersModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mParameters.count();
}

int QgsSvgParametersModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 2;
}

QVariant QgsSvgParametersModel::data( const QModelIndex &index, int role ) const
{
  QgsSvgParametersModel::Column col = static_cast<QgsSvgParametersModel::Column>( index.column() );
  if ( role == Qt::DisplayRole )
  {
    switch ( col )
    {
      case QgsSvgParametersModel::Column::NameColumn:
        return mParameters.at( index.row() ).name;
      case QgsSvgParametersModel::Column::ExpressionColumn:
        return mParameters.at( index.row() ).property.expressionString();
    }
  }

  return QVariant();
}

bool QgsSvgParametersModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || role != Qt::EditRole )
    return false;

  QgsSvgParametersModel::Column col = static_cast<QgsSvgParametersModel::Column>( index.column() );
  switch ( col )
  {
    case QgsSvgParametersModel::Column::NameColumn:
    {
      QString oldName = mParameters.at( index.row() ).name;
      QString newName = value.toString();
      for ( const Parameter &param : std::as_const( mParameters ) )
      {
        if ( param.name == newName && param.name != oldName )
        {
          // names must be unique!
          return false;
        }
      }
      mParameters[index.row()].name = newName;
      emit dataChanged( index, index );
      return true;
    }

    case QgsSvgParametersModel::Column::ExpressionColumn:
      mParameters[index.row()].property = QgsProperty::fromExpression( value.toString() );
      emit dataChanged( index, index );
      return true;
  }

  return false;
}

QVariant QgsSvgParametersModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    QgsSvgParametersModel::Column col = static_cast<QgsSvgParametersModel::Column>( section );
    switch ( col )
    {
      case QgsSvgParametersModel::Column::NameColumn:
        return tr( "Name" );
      case QgsSvgParametersModel::Column::ExpressionColumn:
        return tr( "Expression" );
    }
  }

  return QVariant();
}

void QgsSvgParametersModel::addParameter()
{
  int c = rowCount( QModelIndex() );
  beginInsertRows( QModelIndex(), c, c );
  int i = 1;
  QStringList currentNames;
  std::transform( mParameters.begin(), mParameters.end(), std::back_inserter( currentNames ), []( const Parameter & parameter ) {return parameter.name;} );
  while ( currentNames.contains( QStringLiteral( "param%1" ).arg( i ) ) )
    i++;
  mParameters.append( Parameter( QStringLiteral( "param%1" ).arg( i ), QgsProperty() ) );
  endResetModel();
}


Qt::ItemFlags QgsSvgParametersModel::flags( const QModelIndex &index ) const
{
  Q_UNUSED( index )
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}


QWidget *QgsSvgParameterValueDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QgsFieldExpressionWidget *w = new QgsFieldExpressionWidget( parent );
  const QgsSvgParametersModel *model = qobject_cast<const QgsSvgParametersModel *>( index.model() );
  w->registerExpressionContextGenerator( model->expressionContextGenerator() );
  w->setLayer( model->layer() );
  return w;
}

void QgsSvgParameterValueDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *w = qobject_cast<QgsFieldExpressionWidget *>( editor );
  if ( !w )
    return;

  w->setExpression( index.model()->data( index ).toString() );
}

void QgsSvgParameterValueDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsFieldExpressionWidget *w = qobject_cast<QgsFieldExpressionWidget *>( editor );
  if ( !w )
    return;
  model->setData( index, w->currentField() );
}

void QgsSvgParameterValueDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  editor->setGeometry( option.rect );
}

///@endcond


