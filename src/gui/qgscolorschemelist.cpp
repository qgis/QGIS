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
#include "qgssymbollayerv2utils.h"
#include "qgscolordialog.h"
#include <QPainter>
#include <QColorDialog>
#include <QMimeData>
#include <QClipboard>
#include <QKeyEvent>

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

QgsColorSchemeList::~QgsColorSchemeList()
{

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
  Q_FOREACH ( const QModelIndex &index, selectedIndexes() )
  {
    rows << index.row();
  }
  //remove duplicates
  QList<int> rowsToRemove =  QList<int>::fromSet( rows.toSet() );

  //remove rows in descending order
  qSort( rowsToRemove.begin(), rowsToRemove.end(), qGreater<int>() );
  Q_FOREACH ( int row, rowsToRemove )
  {
    mModel->removeRow( row );
  }
}

void QgsColorSchemeList::addColor( const QColor &color, const QString &label )
{
  mModel->addColor( color, label );
}

void QgsColorSchemeList::pasteColors()
{
  QgsNamedColorList pastedColors = QgsSymbolLayerV2Utils::colorListFromMimeData( QApplication::clipboard()->mimeData() );

  if ( pastedColors.length() == 0 )
  {
    //no pasted colors
    return;
  }

  //insert pasted colors
  QgsNamedColorList::const_iterator colorIt = pastedColors.constBegin();
  for ( ; colorIt != pastedColors.constEnd(); ++colorIt )
  {
    mModel->addColor(( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerV2Utils::colorToName(( *colorIt ).first ) );
  }
}

void QgsColorSchemeList::copyColors()
{
  QList<int> rows;
  Q_FOREACH ( const QModelIndex &index, selectedIndexes() )
  {
    rows << index.row();
  }
  //remove duplicates
  QList<int> rowsToCopy =  QList<int>::fromSet( rows.toSet() );

  QgsNamedColorList colorsToCopy;
  Q_FOREACH ( int row, rowsToCopy )
  {
    colorsToCopy << mModel->colors().at( row );
  }

  //copy colors
  QMimeData* mimeData = QgsSymbolLayerV2Utils::colorListToMimeData( colorsToCopy );
  QApplication::clipboard()->setMimeData( mimeData );
}

void QgsColorSchemeList::keyPressEvent( QKeyEvent *event )
{
  //listen out for delete/backspace presses and remove selected colors
  if (( event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete ) )
  {
    QList<int> rows;
    Q_FOREACH ( const QModelIndex &index, selectedIndexes() )
    {
      rows << index.row();
    }
    //remove duplicates
    QList<int> rowsToRemove =  QList<int>::fromSet( rows.toSet() );

    //remove rows in descending order
    qSort( rowsToRemove.begin(), rowsToRemove.end(), qGreater<int>() );
    Q_FOREACH ( int row, rowsToRemove )
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
  if (( event->button() == Qt::LeftButton ) &&
      ( event->pos() - mDragStartPosition ).manhattanLength() <= QApplication::startDragDistance() )
  {
    //just a click, not a drag

    //if only one item is selected, emit color changed signal
    //(if multiple are selected, user probably was interacting with color list rather than trying to pick a color)
    if ( selectedIndexes().length() == mModel->columnCount() )
    {
      QModelIndex selectedColor = selectedIndexes().at( 0 );
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
  importedColors = QgsSymbolLayerV2Utils::importColorsFromGpl( file, ok, name );
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
    mModel->addColor(( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerV2Utils::colorToName(( *colorIt ).first ) );
  }

  return true;
}

bool QgsColorSchemeList::exportColorsToGpl( QFile &file )
{
  return QgsSymbolLayerV2Utils::saveColorsToGpl( file, QString(), mModel->colors() );
}

bool QgsColorSchemeList::isDirty() const
{
  if ( !mModel )
  {
    return false;
  }

  return mModel->isDirty();
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

QgsColorSchemeModel::~QgsColorSchemeModel()
{

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
    //return an index for the composer item at this position
    return createIndex( row, column );
  }

  //only top level supported
  return QModelIndex();
}

QModelIndex QgsColorSchemeModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );

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
  Q_UNUSED( parent );
  return 2;
}

QVariant QgsColorSchemeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QPair< QColor, QString > namedColor = mColors.at( index.row() );
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
      return QVariant( Qt::AlignLeft | Qt::AlignVCenter );

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
  Q_UNUSED( role );

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
      break;
    }

    case Qt::TextAlignmentRole:
      switch ( section )
      {
        case ColorSwatch:
          return QVariant( Qt::AlignHCenter | Qt::AlignVCenter );
        case ColorLabel:
          return QVariant( Qt::AlignLeft | Qt::AlignVCenter );
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
  types << "text/xml";
  types << "text/plain";
  types << "application/x-color";
  types << "application/x-colorobject-list";
  return types;
}

QMimeData* QgsColorSchemeModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsNamedColorList colorList;

  QModelIndexList::const_iterator indexIt = indexes.constBegin();
  for ( ; indexIt != indexes.constEnd(); ++indexIt )
  {
    if (( *indexIt ).column() > 0 )
      continue;

    colorList << qMakePair( mColors[( *indexIt ).row()].first, mColors[( *indexIt ).row()].second );
  }

  QMimeData* mimeData = QgsSymbolLayerV2Utils::colorListToMimeData( colorList );
  return mimeData;
}

bool QgsColorSchemeModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  Q_UNUSED( column );

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
  QgsNamedColorList droppedColors = QgsSymbolLayerV2Utils::colorListFromMimeData( data );

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
    QPair< QColor, QString > color = qMakePair(( *colorIt ).first, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerV2Utils::colorToName(( *colorIt ).first ) );
    //if color already exists, remove it
    int existingIndex = mColors.indexOf( color );
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
    QModelIndex colorIdx = index( beginRow, 0, QModelIndex() );
    setData( colorIdx, QVariant(( *colorIt ).first ) );
    QModelIndex labelIdx = index( beginRow, 1, QModelIndex() );
    setData( labelIdx, !( *colorIt ).second.isEmpty() ? ( *colorIt ).second : QgsSymbolLayerV2Utils::colorToName(( *colorIt ).first ) );
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

bool QgsColorSchemeModel::insertRows( int row, int count, const QModelIndex& parent )
{
  Q_UNUSED( parent );

  if ( !mScheme || !mScheme->isEditable() )
  {
    return false;
  }

  beginInsertRows( QModelIndex(), row, row + count - 1 );
  for ( int i = row; i < row + count; ++i )
  {
    QPair< QColor, QString > newColor;
    mColors.insert( i, newColor );
  }
  endInsertRows();
  mIsDirty = true;
  return true;
}

void QgsColorSchemeModel::addColor( const QColor &color, const QString &label )
{
  if ( !mScheme || !mScheme->isEditable() )
  {
    return;
  }

  //matches existing color? if so, remove it first
  QPair< QColor, QString > newColor = qMakePair( color, !label.isEmpty() ? label : QgsSymbolLayerV2Utils::colorToName( color ) );
  //if color already exists, remove it
  int existingIndex = mColors.indexOf( newColor );
  if ( existingIndex >= 0 )
  {
    beginRemoveRows( QModelIndex(), existingIndex, existingIndex );
    mColors.removeAt( existingIndex );
    endRemoveRows();
  }

  int row = rowCount();
  insertRow( row );
  QModelIndex colorIdx = index( row, 0, QModelIndex() );
  setData( colorIdx, QVariant( color ) );
  QModelIndex labelIdx = index( row, 1, QModelIndex() );
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
      painter->setBrush( QBrush( QPalette().highlight() ) );
    }
    else
    {
      painter->setBrush( QBrush( QPalette().color( QPalette::Inactive,
                                 QPalette::Highlight ) ) );
    }
    painter->drawRect( option.rect );
  }

  QColor color = index.model()->data( index, Qt::DisplayRole ).value<QColor>();
  if ( !color.isValid() )
  {
    return;
  }

  QRect rect = option.rect;
  //center it
  rect.setLeft( option.rect.center().x() - 15 );
  rect.setSize( QSize( 30, 30 ) );
  rect.adjust( 0, 1, 0, 1 );
  //create an icon pixmap
  painter->save();
  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( Qt::NoPen );
  if ( color.alpha() < 255 )
  {
    //start with checkboard pattern
    QBrush checkBrush = QBrush( transparentBackground() );
    painter->setBrush( checkBrush );
    painter->drawRoundedRect( rect, 5, 5 );
  }

  //draw semi-transparent color on top
  painter->setBrush( color );
  painter->drawRoundedRect( rect, 5, 5 );
  painter->restore();
}

const QPixmap& QgsColorSwatchDelegate::transparentBackground() const
{
  static QPixmap transpBkgrd;

  if ( transpBkgrd.isNull() )
    transpBkgrd = QgsApplication::getThemePixmap( "/transp-background_8x8.png" );

  return transpBkgrd;
}

QSize QgsColorSwatchDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );
  return QSize( 30, 32 );
}

bool QgsColorSwatchDelegate::editorEvent( QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index )
{
  Q_UNUSED( option );
  if ( event->type() == QEvent::MouseButtonDblClick )
  {
    if ( !index.model()->flags( index ).testFlag( Qt::ItemIsEditable ) )
    {
      //item not editable
      return false;
    }
    QColor color = index.model()->data( index, Qt::DisplayRole ).value<QColor>();
    QColor newColor = QgsColorDialogV2::getColor( color, mParent, tr( "Select color" ), true );
    if ( !newColor.isValid() )
    {
      return false;
    }

    return model->setData( index, newColor, Qt::EditRole );
  }

  return false;
}
