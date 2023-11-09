/***************************************************************************
    qgscolorschemelist.cpp
    ----------------------
    Date                 : August 2014
    Copyright            : (C) 2014 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorschemelist.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssymbollayerutils.h"
#include "qgscolordialog.h"
#include "qgssettings.h"

#include <QPainter>
#include <QColorDialog>
#include <QMimeData>
#include <QClipboard>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMessageBox>

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

QgsColorSchemeList::QgsColorSchemeList( QWidget *parent, QgsColorScheme *scheme, const QString &context, const QColor &baseColor )
  : QTreeView( parent )
  , mScheme( scheme )
{
  mModel = new QgsColorSchemeModel( scheme, context, baseColor, this );
#ifdef ENABLE_MODELTEST
  new ModelTest( mModel, this );
#endif
  setModel( mModel );

  mSwatchDelegate = new QgsColorSwatchDelegate( this );
  setItemDelegateForColumn( 0, mSwatchDelegate );

  setRootIsDecorated( false );
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setSelectionBehavior( QAbstractItemView::SelectRows );
  setDragEnabled( true );
  setAcceptDrops( true );
  setDragDropMode( QTreeView::DragDrop );
  setDropIndicatorShown( true );
  setDefaultDropAction( Qt::CopyAction );
}

void QgsColorSchemeList::setScheme( QgsColorScheme *scheme, const QString &context, const QColor &baseColor )
{
  mScheme = scheme;
  mModel->setScheme( scheme, context, baseColor );
}

bool QgsColorSchemeList::saveColorsToScheme()
{
  if ( !mScheme || !mScheme->isEditable() )
  {
    return false;
  }

  mScheme->setColors( mModel->colors(), mModel->context(), mModel->baseColor() );
  return true;
}

void QgsColorSchemeList::removeSelection()
{
  QList<int> rows;
  const auto constSelectedIndexes = selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    rows << index.row();
  }
  //remove duplicates
  QList<int> rowsToRemove = qgis::setToList( qgis::listToSet( rows ) );

  //remove rows in descending order
  std::sort( rowsToRemove.begin(), rowsToRemove.end(), std::greater<int>() );
  const auto constRowsToRemove = rowsToRemove;
  for ( const int row : constRowsToRemove )
  {
    mModel->removeRow( row );
  }
}

void QgsColorSchemeList::addColor( const QColor &color, const QString &label, bool allowDuplicate )
{
  mModel->addColor( color, label, allowDuplicate );
}

void QgsColorSchemeList::pasteColors()
{
  const QgsNamedColorList pastedColors = QgsSymbolLayerUtils::colorListFromMimeData( QApplication::clipboard()->mimeData() );

  if ( pastedColors.length() == 0 )
  {
    //no pasted colors
    return;
  }

  //insert pasted colors
  QgsNamedColorList::const_iterator colorIt = pastedColors.constBegin();
  for ( ; colorIt != pastedColors.constEnd(); ++colorIt )
  {
    mModel->addColor( ( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerUtils::colorToName( ( *colorIt ).first ) );
  }
}

void QgsColorSchemeList::copyColors()
{
  QList<int> rows;
  const auto constSelectedIndexes = selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    rows << index.row();
  }
  //remove duplicates
  const QList<int> rowsToCopy = qgis::setToList( qgis::listToSet( rows ) );

  QgsNamedColorList colorsToCopy;
  const auto constRowsToCopy = rowsToCopy;
  for ( const int row : constRowsToCopy )
  {
    colorsToCopy << mModel->colors().at( row );
  }

  //copy colors
  QMimeData *mimeData = QgsSymbolLayerUtils::colorListToMimeData( colorsToCopy );
  QApplication::clipboard()->setMimeData( mimeData );
}

void QgsColorSchemeList::showImportColorsDialog()
{
  QgsSettings s;
  const QString lastDir = s.value( QStringLiteral( "/UI/lastGplPaletteDir" ), QDir::homePath() ).toString();
  const QString filePath = QFileDialog::getOpenFileName( this, tr( "Select Palette File" ), lastDir, QStringLiteral( "GPL (*.gpl);;All files (*.*)" ) );
  activateWindow();
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  const QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( nullptr, tr( "Import Colors" ), tr( "Error, file does not exist or is not readable." ) );
    return;
  }

  s.setValue( QStringLiteral( "/UI/lastGplPaletteDir" ), fileInfo.absolutePath() );
  QFile file( filePath );
  const bool importOk = importColorsFromGpl( file );
  if ( !importOk )
  {
    QMessageBox::critical( nullptr, tr( "Import Colors" ), tr( "Error, no colors found in palette file." ) );
    return;
  }
}

void QgsColorSchemeList::showExportColorsDialog()
{
  QgsSettings s;
  const QString lastDir = s.value( QStringLiteral( "/UI/lastGplPaletteDir" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Palette file" ), lastDir, QStringLiteral( "GPL (*.gpl)" ) );
  activateWindow();
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure filename contains extension
  if ( !fileName.endsWith( QLatin1String( ".gpl" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".gpl" );
  }

  const QFileInfo fileInfo( fileName );
  s.setValue( QStringLiteral( "/UI/lastGplPaletteDir" ), fileInfo.absolutePath() );

  QFile file( fileName );
  const bool exportOk = exportColorsToGpl( file );
  if ( !exportOk )
  {
    QMessageBox::critical( nullptr, tr( "Export Colors" ), tr( "Error writing palette file." ) );
    return;
  }
}

void QgsColorSchemeList::keyPressEvent( QKeyEvent *event )
{
  //listen out for delete/backspace presses and remove selected colors
  if ( ( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) )
  {
    QList<int> rows;
    const auto constSelectedIndexes = selectedIndexes();
    for ( const QModelIndex &index : constSelectedIndexes )
    {
      rows << index.row();
    }
    //remove duplicates
    QList<int> rowsToRemove = qgis::setToList( qgis::listToSet( rows ) );

    //remove rows in descending order
    std::sort( rowsToRemove.begin(), rowsToRemove.end(), std::greater<int>() );
    const auto constRowsToRemove = rowsToRemove;
    for ( const int row : constRowsToRemove )
    {
      mModel->removeRow( row );
    }
    return;
  }

  QTreeView::keyPressEvent( event );
}

void QgsColorSchemeList::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    //record press start position
    mDragStartPosition = event->pos();
  }
  QTreeView::mousePressEvent( event );
}

void QgsColorSchemeList::mouseReleaseEvent( QMouseEvent *event )
{
  if ( ( event->button() == Qt::LeftButton ) &&
       ( event->pos() - mDragStartPosition ).manhattanLength() <= QApplication::startDragDistance() )
  {
    //just a click, not a drag

    //if only one item is selected, emit color changed signal
    //(if multiple are selected, user probably was interacting with color list rather than trying to pick a color)
    if ( selectedIndexes().length() == mModel->columnCount() )
    {
      const QModelIndex selectedColor = selectedIndexes().at( 0 );
      emit colorSelected( mModel->colors().at( selectedColor.row() ).first );
    }
  }

  QTreeView::mouseReleaseEvent( event );
}

bool QgsColorSchemeList::importColorsFromGpl( QFile &file )
{
  QgsNamedColorList importedColors;
  bool ok = false;
  QString name;
  importedColors = QgsSymbolLayerUtils::importColorsFromGpl( file, ok, name );
  if ( !ok )
  {
    return false;
  }

  if ( importedColors.length() == 0 )
  {
    //no imported colors
    return false;
  }

  //insert imported colors
  QgsNamedColorList::const_iterator colorIt = importedColors.constBegin();
  for ( ; colorIt != importedColors.constEnd(); ++colorIt )
  {
    mModel->addColor( ( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerUtils::colorToName( ( *colorIt ).first ) );
  }

  return true;
}

bool QgsColorSchemeList::exportColorsToGpl( QFile &file )
{
  return QgsSymbolLayerUtils::saveColorsToGpl( file, QString(), mModel->colors() );
}

bool QgsColorSchemeList::isDirty() const
{
  if ( !mModel )
  {
    return false;
  }

  return mModel->isDirty();
}

QgsColorScheme *QgsColorSchemeList::scheme()
{
  return mScheme;
}

//
// QgsColorSchemeModel
//

QgsColorSchemeModel::QgsColorSchemeModel( QgsColorScheme *scheme, const QString &context, const QColor &baseColor, QObject *parent )
  : QAbstractItemModel( parent )
  , mScheme( scheme )
  , mContext( context )
  , mBaseColor( baseColor )
  , mIsDirty( false )
{
  if ( scheme )
  {
    mColors = scheme->fetchColors( context, baseColor );
  }
}

QModelIndex QgsColorSchemeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row >= 0 && row < mColors.size() )
  {
    //return an index for the color item at this position
    return createIndex( row, column );
  }

  //only top level supported
  return QModelIndex();
}

QModelIndex QgsColorSchemeModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )

  //all items are top level
  return QModelIndex();
}

int QgsColorSchemeModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mColors.size();
  }
  else
  {
    //no children
    return 0;
  }
}

int QgsColorSchemeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 2;
}

QVariant QgsColorSchemeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const QPair< QColor, QString > namedColor = mColors.at( index.row() );
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch ( index.column() )
      {
        case ColorSwatch:
          return namedColor.first;
        case ColorLabel:
          return namedColor.second;
        default:
          return QVariant();
      }

    case Qt::TextAlignmentRole:
      return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );

    default:
      return QVariant();
  }
}

Qt::ItemFlags QgsColorSchemeModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( ! index.isValid() )
  {
    return flags | Qt::ItemIsDropEnabled;
  }

  switch ( index.column() )
  {
    case ColorSwatch:
    case ColorLabel:
      if ( mScheme && mScheme->isEditable() )
      {
        flags = flags | Qt::ItemIsEditable;
      }
      return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    default:
      return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

bool QgsColorSchemeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_UNUSED( role )

  if ( !mScheme || !mScheme->isEditable() )
    return false;

  if ( !index.isValid() )
    return false;

  if ( index.row() >= mColors.length() )
    return false;

  switch ( index.column() )
  {
    case ColorSwatch:
      mColors[ index.row()].first = value.value<QColor>();
      emit dataChanged( index, index );
      mIsDirty = true;
      return true;

    case ColorLabel:
      mColors[ index.row()].second = value.toString();
      emit dataChanged( index, index );
      mIsDirty = true;
      return true;

    default:
      return false;
  }
}

QVariant QgsColorSchemeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    {
      switch ( section )
      {
        case ColorSwatch:
          return tr( "Color" );
        case ColorLabel:
          return tr( "Label" );
        default:
          return QVariant();
      }
    }

    case Qt::TextAlignmentRole:
      switch ( section )
      {
        case ColorSwatch:
          return static_cast<Qt::Alignment::Int>( Qt::AlignHCenter | Qt::AlignVCenter );
        case ColorLabel:
          return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );
        default:
          return QVariant();
      }
    default:
      return QAbstractItemModel::headerData( section, orientation, role );
  }
}

Qt::DropActions QgsColorSchemeModel::supportedDropActions() const
{
  if ( mScheme && mScheme->isEditable() )
  {
    return Qt::CopyAction | Qt::MoveAction;
  }
  else
  {
    return Qt::CopyAction;
  }
}

QStringList QgsColorSchemeModel::mimeTypes() const
{
  if ( !mScheme || !mScheme->isEditable() )
  {
    return QStringList();
  }

  QStringList types;
  types << QStringLiteral( "text/xml" );
  types << QStringLiteral( "text/plain" );
  types << QStringLiteral( "application/x-color" );
  types << QStringLiteral( "application/x-colorobject-list" );
  return types;
}

QMimeData *QgsColorSchemeModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsNamedColorList colorList;

  QModelIndexList::const_iterator indexIt = indexes.constBegin();
  for ( ; indexIt != indexes.constEnd(); ++indexIt )
  {
    if ( ( *indexIt ).column() > 0 )
      continue;

    colorList << qMakePair( mColors[( *indexIt ).row()].first, mColors[( *indexIt ).row()].second );
  }

  QMimeData *mimeData = QgsSymbolLayerUtils::colorListToMimeData( colorList );
  return mimeData;
}

bool QgsColorSchemeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column )

  if ( !mScheme || !mScheme->isEditable() )
  {
    return false;
  }

  if ( action == Qt::IgnoreAction )
  {
    return true;
  }

  if ( parent.isValid() )
  {
    return false;
  }

  int beginRow = row != -1 ? row : rowCount( QModelIndex() );
  const QgsNamedColorList droppedColors = QgsSymbolLayerUtils::colorListFromMimeData( data );

  if ( droppedColors.length() == 0 )
  {
    //no dropped colors
    return false;
  }

  //any existing colors? if so, remove them first
  QgsNamedColorList::const_iterator colorIt = droppedColors.constBegin();
  for ( ; colorIt != droppedColors.constEnd(); ++colorIt )
  {
    //dest color
    const QPair< QColor, QString > color = qMakePair( ( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerUtils::colorToName( ( *colorIt ).first ) );
    //if color already exists, remove it
    const int existingIndex = mColors.indexOf( color );
    if ( existingIndex >= 0 )
    {
      if ( existingIndex < beginRow )
      {
        //color is before destination row, so decrease destination row to account for removal
        beginRow--;
      }

      beginRemoveRows( parent, existingIndex, existingIndex );
      mColors.removeAt( existingIndex );
      endRemoveRows();
    }
  }

  //insert dropped colors
  insertRows( beginRow, droppedColors.length(), QModelIndex() );
  colorIt = droppedColors.constBegin();
  for ( ; colorIt != droppedColors.constEnd(); ++colorIt )
  {
    const QModelIndex colorIdx = index( beginRow, 0, QModelIndex() );
    setData( colorIdx, QVariant( ( *colorIt ).first ) );
    const QModelIndex labelIdx = index( beginRow, 1, QModelIndex() );
    setData( labelIdx, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerUtils::colorToName( ( *colorIt ).first ) );
    beginRow++;
  }
  mIsDirty = true;

  return true;
}

void QgsColorSchemeModel::setScheme( QgsColorScheme *scheme, const QString &context, const QColor &baseColor )
{
  mScheme = scheme;
  mContext = context;
  mBaseColor = baseColor;
  mIsDirty = false;
  beginResetModel();
  mColors = scheme->fetchColors( mContext, mBaseColor );
  endResetModel();
}

bool QgsColorSchemeModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( !mScheme || !mScheme->isEditable() )
  {
    return false;
  }

  if ( parent.isValid() )
  {
    return false;
  }

  if ( row >= mColors.count() )
  {
    return false;
  }

  for ( int i = row + count - 1; i >= row; --i )
  {
    beginRemoveRows( parent, i, i );
    mColors.removeAt( i );
    endRemoveRows();
  }

  mIsDirty = true;
  return true;
}

bool QgsColorSchemeModel::insertRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( parent )

  if ( !mScheme || !mScheme->isEditable() )
  {
    return false;
  }

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i < row + count; ++i )
  {
    const QPair< QColor, QString > newColor;
    mColors.insert( i, newColor );
  }
  endInsertRows();
  mIsDirty = true;
  return true;
}

void QgsColorSchemeModel::addColor( const QColor &color, const QString &label, bool allowDuplicate )
{
  if ( !mScheme || !mScheme->isEditable() )
  {
    return;
  }

  if ( !allowDuplicate )
  {
    //matches existing color? if so, remove it first
    const QPair< QColor, QString > newColor = qMakePair( color, !label.isEmpty() ? label : QgsSymbolLayerUtils::colorToName( color ) );
    //if color already exists, remove it
    const int existingIndex = mColors.indexOf( newColor );
    if ( existingIndex >= 0 )
    {
      beginRemoveRows( QModelIndex(), existingIndex, existingIndex );
      mColors.removeAt( existingIndex );
      endRemoveRows();
    }
  }

  const int row = rowCount();
  insertRow( row );
  const QModelIndex colorIdx = index( row, 0, QModelIndex() );
  setData( colorIdx, QVariant( color ) );
  const QModelIndex labelIdx = index( row, 1, QModelIndex() );
  setData( labelIdx, QVariant( label ) );
  mIsDirty = true;
}


//
// QgsColorSwatchDelegate
//
QgsColorSwatchDelegate::QgsColorSwatchDelegate( QWidget *parent )
  : QAbstractItemDelegate( parent )
  , mParent( parent )
{

}

void QgsColorSwatchDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( option.state & QStyle::State_Selected )
  {
    painter->setPen( QPen( Qt::NoPen ) );
    if ( option.state & QStyle::State_Active )
    {
      painter->setBrush( QBrush( option.widget->palette().highlight() ) );
    }
    else
    {
      painter->setBrush( QBrush( option.widget->palette().color( QPalette::Inactive,
                                 QPalette::Highlight ) ) );
    }
    painter->drawRect( option.rect );
  }

  const QColor color = index.model()->data( index, Qt::DisplayRole ).value<QColor>();
  if ( !color.isValid() )
  {
    return;
  }

  QRect rect = option.rect;
  const int iconSize = Qgis::UI_SCALE_FACTOR * option.fontMetrics.horizontalAdvance( 'X' ) * 4;

  const int cornerSize = iconSize / 6;
  //center it
  rect.setLeft( option.rect.center().x() - iconSize / 2 );

  rect.setSize( QSize( iconSize, iconSize ) );
  rect.adjust( 0, 1, 0, 1 );
  //create an icon pixmap
  const QgsScopedQPainterState painterState( painter );
  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( Qt::NoPen );
  if ( color.alpha() < 255 )
  {
    //start with checkboard pattern
    const QBrush checkBrush = QBrush( transparentBackground() );
    painter->setBrush( checkBrush );
    painter->drawRoundedRect( rect, cornerSize, cornerSize );
  }

  //draw semi-transparent color on top
  painter->setBrush( color );
  painter->drawRoundedRect( rect, cornerSize, cornerSize );
}

QPixmap QgsColorSwatchDelegate::transparentBackground() const
{
  static QPixmap sTranspBkgrd;

  if ( sTranspBkgrd.isNull() )
    sTranspBkgrd = QgsApplication::getThemePixmap( QStringLiteral( "/transp-background_8x8.png" ) );

  return sTranspBkgrd;
}

QSize QgsColorSwatchDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )

  const int iconSize = Qgis::UI_SCALE_FACTOR * option.fontMetrics.horizontalAdvance( 'X' ) * 4;
  return QSize( iconSize, iconSize * 32 / 30.0 );
}

bool QgsColorSwatchDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  Q_UNUSED( option )
  if ( event->type() == QEvent::MouseButtonDblClick )
  {
    if ( !index.model()->flags( index ).testFlag( Qt::ItemIsEditable ) )
    {
      //item not editable
      return false;
    }

    const QColor color = index.model()->data( index, Qt::DisplayRole ).value<QColor>();

    QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( qobject_cast< QWidget * >( parent() ) );
    if ( panel && panel->dockMode() )
    {
      QgsCompoundColorWidget *colorWidget = new QgsCompoundColorWidget( panel, color, QgsCompoundColorWidget::LayoutVertical );
      colorWidget->setPanelTitle( tr( "Select Color" ) );
      colorWidget->setAllowOpacity( true );
      colorWidget->setProperty( "index", index );
      connect( colorWidget, &QgsCompoundColorWidget::currentColorChanged, this, &QgsColorSwatchDelegate::colorChanged );
      panel->openPanel( colorWidget );
      return true;
    }

    const QColor newColor = QgsColorDialog::getColor( color, mParent, tr( "Select color" ), true );
    if ( !newColor.isValid() )
    {
      return false;
    }

    return model->setData( index, newColor, Qt::EditRole );
  }

  return false;
}

void QgsColorSwatchDelegate::colorChanged()
{
  if ( QgsCompoundColorWidget *colorWidget = qobject_cast< QgsCompoundColorWidget * >( sender() ) )
  {
    const QModelIndex index = colorWidget->property( "index" ).toModelIndex();
    const_cast< QAbstractItemModel * >( index.model() )->setData( index, colorWidget->color(), Qt::EditRole );
  }
}
