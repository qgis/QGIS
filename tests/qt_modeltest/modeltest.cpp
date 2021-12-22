/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "modeltest.h"

#include <QStringList>
#include <QSize>
#include <QAbstractItemModel>
#include <QDebug>

/*!
    Connect to all of the models signals.  Whenever anything happens
    recheck everything.
*/
ModelTest::ModelTest( QAbstractItemModel *_model, QObject *parent ) : QObject( parent ), model( _model )
{
  Q_ASSERT( model );

  connect( model, &QAbstractItemModel::columnsAboutToBeInserted,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::columnsAboutToBeRemoved,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::columnsInserted,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::columnsRemoved,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::dataChanged,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::headerDataChanged,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::layoutAboutToBeChanged, this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::layoutChanged, this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::modelReset, this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::rowsAboutToBeInserted,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::rowsAboutToBeRemoved,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::rowsInserted,
           this, &ModelTest::runAllTests );
  connect( model, &QAbstractItemModel::rowsRemoved,
           this, &ModelTest::runAllTests );

  // Special checks for inserting/removing
  connect( model, &QAbstractItemModel::layoutAboutToBeChanged,
           this, &ModelTest::layoutAboutToBeChanged );
  connect( model, &QAbstractItemModel::layoutChanged,
           this, &ModelTest::layoutChanged );

  connect( model, &QAbstractItemModel::rowsAboutToBeInserted,
           this, &ModelTest::rowsAboutToBeInserted );
  connect( model, &QAbstractItemModel::rowsAboutToBeRemoved,
           this, &ModelTest::rowsAboutToBeRemoved );
  connect( model, &QAbstractItemModel::rowsInserted,
           this, &ModelTest::rowsInserted );
  connect( model, &QAbstractItemModel::rowsRemoved,
           this, &ModelTest::rowsRemoved );

  runAllTests();
}

void ModelTest::runAllTests()
{
  if ( fetchingMore )
    return;
  nonDestructiveBasicTest();
  rowCount();
  columnCount();
  hasIndex();
  index();
  parent();
  data();
}

/*!
    nonDestructiveBasicTest tries to call a number of the basic functions (not all)
    to make sure the model doesn't outright segfault, testing the functions that makes sense.
*/
void ModelTest::nonDestructiveBasicTest()
{
  Q_ASSERT( model->buddy( QModelIndex() ) == QModelIndex() );
  model->canFetchMore( QModelIndex() );
  Q_ASSERT( model->columnCount( QModelIndex() ) >= 0 );
  Q_ASSERT( model->data( QModelIndex() ) == QVariant() );
  fetchingMore = true;
  model->fetchMore( QModelIndex() );
  fetchingMore = false;
  Qt::ItemFlags flags = model->flags( QModelIndex() );
  Q_ASSERT( flags == Qt::ItemIsDropEnabled || flags == 0 );
  model->hasChildren( QModelIndex() );
  model->hasIndex( 0, 0 );
  model->headerData( 0, Qt::Horizontal );
  model->index( 0, 0 );
  model->itemData( QModelIndex() );
  QVariant cache;
  model->match( QModelIndex(), -1, cache );
  model->mimeTypes();
  QModelIndex m1 = model->parent( QModelIndex() );
  QModelIndex m2 = QModelIndex();
  Q_ASSERT( m1 == m2 );
  Q_ASSERT( model->parent( QModelIndex() ) == QModelIndex() );
  Q_ASSERT( model->rowCount() >= 0 );
  QVariant variant;
  model->setData( QModelIndex(), variant, -1 );
  model->setHeaderData( -1, Qt::Horizontal, QVariant() );
  model->setHeaderData( 999999, Qt::Horizontal, QVariant() );
  model->sibling( 0, 0, QModelIndex() );
  model->span( QModelIndex() );
  model->supportedDropActions();
}

/*!
    Tests model's implementation of QAbstractItemModel::rowCount() and hasChildren()

    Models that are dynamically populated are not as fully tested here.
 */
void ModelTest::rowCount()
{
  // check top row
  QModelIndex topIndex = model->index( 0, 0, QModelIndex() );
  int rows = model->rowCount( topIndex );
  Q_ASSERT( rows >= 0 );
  if ( rows > 0 )
    Q_ASSERT( model->hasChildren( topIndex ) == true );

  QModelIndex secondLevelIndex = model->index( 0, 0, topIndex );
  if ( secondLevelIndex.isValid() ) // not the top level
  {
    // check a row count where parent is valid
    rows = model->rowCount( secondLevelIndex );
    Q_ASSERT( rows >= 0 );
    if ( rows > 0 )
      Q_ASSERT( model->hasChildren( secondLevelIndex ) == true );
  }

  // The models rowCount() is tested more extensively in checkChildren(),
  // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::columnCount() and hasChildren()
 */
void ModelTest::columnCount()
{
  // check top row
  QModelIndex topIndex = model->index( 0, 0, QModelIndex() );
  Q_ASSERT( model->columnCount( topIndex ) >= 0 );

  // check a column count where parent is valid
  QModelIndex childIndex = model->index( 0, 0, topIndex );
  if ( childIndex.isValid() )
    Q_ASSERT( model->columnCount( childIndex ) >= 0 );

  // columnCount() is tested more extensively in checkChildren(),
  // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::hasIndex()
 */
void ModelTest::hasIndex()
{
  // Make sure that invalid values returns an invalid index
  Q_ASSERT( model->hasIndex( -2, -2 ) == false );
  Q_ASSERT( model->hasIndex( -2, 0 ) == false );
  Q_ASSERT( model->hasIndex( 0, -2 ) == false );

  int rows = model->rowCount();
  int columns = model->columnCount();

  // check out of bounds
  Q_ASSERT( model->hasIndex( rows, columns ) == false );
  Q_ASSERT( model->hasIndex( rows + 1, columns + 1 ) == false );

  if ( rows > 0 )
    Q_ASSERT( model->hasIndex( 0, 0 ) == true );

  // hasIndex() is tested more extensively in checkChildren(),
  // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::index()
 */
void ModelTest::index()
{
  // Make sure that invalid values returns an invalid index
  Q_ASSERT( model->index( -2, -2 ) == QModelIndex() );
  Q_ASSERT( model->index( -2, 0 ) == QModelIndex() );
  Q_ASSERT( model->index( 0, -2 ) == QModelIndex() );

  int rows = model->rowCount();
  int columns = model->columnCount();

  if ( rows == 0 )
    return;

  // Catch off by one errors
  QModelIndex tmp;
  tmp = model->index( rows, columns );
  Q_ASSERT( tmp == QModelIndex() );
  tmp = model->index( 0, 0 );
  Q_ASSERT( tmp.isValid() == true );

  // Make sure that the same index is *always* returned
  QModelIndex a = model->index( 0, 0 );
  QModelIndex b = model->index( 0, 0 );
  Q_ASSERT( a == b );

  // index() is tested more extensively in checkChildren(),
  // but this catches the big mistakes
}

/*!
    Tests model's implementation of QAbstractItemModel::parent()
 */
void ModelTest::parent()
{
  // Make sure the model won't crash and will return an invalid QModelIndex
  // when asked for the parent of an invalid index.
  Q_ASSERT( model->parent( QModelIndex() ) == QModelIndex() );

  if ( model->rowCount() == 0 )
    return;

  QModelIndex tmp;

  // Column 0                | Column 1    |
  // QModelIndex()           |             |
  //    \- topIndex          | topIndex1   |
  //         \- childIndex   | childIndex1 |

  // Common error test #1, make sure that a top level index has a parent
  // that is a invalid QModelIndex.
  QModelIndex topIndex = model->index( 0, 0, QModelIndex() );
  tmp = model->parent( topIndex );
  Q_ASSERT( tmp == QModelIndex() );

  // Common error test #2, make sure that a second level index has a parent
  // that is the first level index.
  if ( model->rowCount( topIndex ) > 0 )
  {
    QModelIndex childIndex = model->index( 0, 0, topIndex );
    tmp = model->parent( childIndex );
    Q_ASSERT( tmp == topIndex );
  }

  // Common error test #3, the second column should NOT have the same children
  // as the first column in a row.
  // Usually the second column shouldn't have children.
  QModelIndex topIndex1 = model->index( 0, 1, QModelIndex() );
  if ( model->rowCount( topIndex1 ) > 0 )
  {
    QModelIndex childIndex = model->index( 0, 0, topIndex );
    QModelIndex childIndex1 = model->index( 0, 0, topIndex1 );
    Q_ASSERT( childIndex != childIndex1 );
  }

  // Full test, walk n levels deep through the model making sure that all
  // parent's children correctly specify their parent.
  checkChildren( QModelIndex() );
}

/*!
    Called from the parent() test.

    A model that returns an index of parent X should also return X when asking
    for the parent of the index.

    This recursive function does pretty extensive testing on the whole model in an
    effort to catch edge cases.

    This function assumes that rowCount(), columnCount() and index() already work.
    If they have a bug it will point it out, but the above tests should have already
    found the basic bugs because it is easier to figure out the problem in
    those tests then this one.
 */
void ModelTest::checkChildren( const QModelIndex &parent, int currentDepth )
{
  QModelIndex tmp;

  // First just try walking back up the tree.
  QModelIndex p = parent;
  while ( p.isValid() )
    p = p.parent();

  // For models that are dynamically populated
  if ( model->canFetchMore( parent ) )
  {
    fetchingMore = true;
    model->fetchMore( parent );
    fetchingMore = false;
  }

  int rows = model->rowCount( parent );
  int columns = model->columnCount( parent );

  if ( rows > 0 )
    Q_ASSERT( model->hasChildren( parent ) );

  // Some further testing against rows(), columns(), and hasChildren()
  Q_ASSERT( rows >= 0 );
  Q_ASSERT( columns >= 0 );
  if ( rows > 0 )
    Q_ASSERT( model->hasChildren( parent ) == true );

  //qDebug() << "parent:" << model->data(parent).toString() << "rows:" << rows
  //         << "columns:" << columns << "parent column:" << parent.column();

  Q_ASSERT( model->hasIndex( rows + 1, 0, parent ) == false );
  for ( int r = 0; r < rows; ++r )
  {
    if ( model->canFetchMore( parent ) )
    {
      fetchingMore = true;
      model->fetchMore( parent );
      fetchingMore = false;
    }
    Q_ASSERT( model->hasIndex( r, columns + 1, parent ) == false );
    for ( int c = 0; c < columns; ++c )
    {
      Q_ASSERT( model->hasIndex( r, c, parent ) == true );
      QModelIndex index = model->index( r, c, parent );
      // rowCount() and columnCount() said that it existed...
      Q_ASSERT( index.isValid() == true );

      // index() should always return the same index when called twice in a row
      QModelIndex modifiedIndex = model->index( r, c, parent );
      Q_ASSERT( index == modifiedIndex );

      // Make sure we get the same index if we request it twice in a row
      QModelIndex a = model->index( r, c, parent );
      QModelIndex b = model->index( r, c, parent );
      Q_ASSERT( a == b );

      // Some basic checking on the index that is returned
      Q_ASSERT( index.model() == model );
      Q_ASSERT( index.row() == r );
      Q_ASSERT( index.column() == c );
      // While you can technically return a QVariant usually this is a sign
      // of an bug in data()  Disable if this really is ok in your model.
      //Q_ASSERT(model->data(index, Qt::DisplayRole).isValid() == true);

      // If the next test fails here is some somewhat useful debug you play with.
      /*
      if (model->parent(index) != parent) {
          qDebug() << r << c << currentDepth << model->data(index).toString()
                   << model->data(parent).toString();
          qDebug() << index << parent << model->parent(index);
          // And a view that you can even use to show the model.
          //QTreeView view;
          //view.setModel(model);
          //view.show();
      }*/

      // Check that we can get back our real parent.
      //qDebug() << "TTT 1: " << model->parent(index);
      //qDebug() << "TTT 2: " << parent;
      //qDebug() << "TTT 3: " << index;
      tmp = model->parent( index );
      Q_ASSERT( tmp == parent );

      // recursively go down the children
      if ( model->hasChildren( index ) && currentDepth < 10 )
      {
        //qDebug() << r << c << "has children" << model->rowCount(index);
        checkChildren( index, ++currentDepth );
      }/* else { if (currentDepth >= 10) qDebug() << "checked 10 deep"; };*/

      // make sure that after testing the children that the index doesn't change.
      QModelIndex newerIndex = model->index( r, c, parent );
      Q_ASSERT( index == newerIndex );
    }
  }
}

/*!
    Tests model's implementation of QAbstractItemModel::data()
 */
void ModelTest::data()
{
  // Invalid index should return an invalid qvariant
  Q_ASSERT( !model->data( QModelIndex() ).isValid() );

  if ( model->rowCount() == 0 )
    return;

  // A valid index should have a valid QVariant data
  Q_ASSERT( model->index( 0, 0 ).isValid() );

  // shouldn't be able to set data on an invalid index
  Q_ASSERT( model->setData( QModelIndex(), QLatin1String( "foo" ), Qt::DisplayRole ) == false );

  // General Purpose roles that should return a QString
  QVariant variant = model->data( model->index( 0, 0 ), Qt::ToolTipRole );
  if ( variant.isValid() )
    Q_ASSERT( variant.canConvert( QVariant::String ) );
  variant = model->data( model->index( 0, 0 ), Qt::StatusTipRole );
  if ( variant.isValid() )
    Q_ASSERT( variant.canConvert( QVariant::String ) );
  variant = model->data( model->index( 0, 0 ), Qt::WhatsThisRole );
  if ( variant.isValid() )
    Q_ASSERT( variant.canConvert( QVariant::String ) );

  // General Purpose roles that should return a QSize
  variant = model->data( model->index( 0, 0 ), Qt::SizeHintRole );
  if ( variant.isValid() )
    Q_ASSERT( variant.canConvert( QVariant::Size ) );

  // General Purpose roles that should return a QFont
  QVariant fontVariant = model->data( model->index( 0, 0 ), Qt::FontRole );
  if ( fontVariant.isValid() )
    Q_ASSERT( fontVariant.canConvert( QVariant::Font ) );

  // Check that the alignment is one we know about
  QVariant textAlignmentVariant = model->data( model->index( 0, 0 ), Qt::TextAlignmentRole );
  if ( textAlignmentVariant.isValid() )
  {
    uint alignment = textAlignmentVariant.toUInt();
    Q_ASSERT( alignment == ( alignment & ( Qt::AlignHorizontal_Mask | Qt::AlignVertical_Mask ) ) );
  }

  // General Purpose roles that should return a QColor
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  QVariant colorVariant = model->data( model->index( 0, 0 ), Qt::BackgroundColorRole );
#else
  QVariant colorVariant = model->data( model->index( 0, 0 ), Qt::BackgroundRole );
#endif
  if ( colorVariant.isValid() )
    Q_ASSERT( colorVariant.canConvert( QVariant::Color ) );

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
  colorVariant = model->data( model->index( 0, 0 ), Qt::TextColorRole );
#else
  colorVariant = model->data( model->index( 0, 0 ), Qt::ForegroundRole );
#endif
  if ( colorVariant.isValid() )
    Q_ASSERT( colorVariant.canConvert( QVariant::Color ) );

  // Check that the "check state" is one we know about.
  QVariant checkStateVariant = model->data( model->index( 0, 0 ), Qt::CheckStateRole );
  if ( checkStateVariant.isValid() )
  {
    int state = checkStateVariant.toInt();
    Q_ASSERT( state == Qt::Unchecked ||
              state == Qt::PartiallyChecked ||
              state == Qt::Checked );
  }
}

/*!
    Store what is about to be inserted to make sure it actually happens

    \sa rowsInserted()
 */
void ModelTest::rowsAboutToBeInserted( const QModelIndex &parent, int start, int end )
{
  Q_UNUSED( end )
  Changing c;
  c.parent = parent;
  c.oldSize = model->rowCount( parent );
  c.last = model->data( model->index( start - 1, 0, parent ) );
  c.next = model->data( model->index( start, 0, parent ) );
  insert.push( c );
}

/*!
    Confirm that what was said was going to happen actually did

    \sa rowsAboutToBeInserted()
 */
void ModelTest::rowsInserted( const QModelIndex &parent, int start, int end )
{
  Changing c = insert.pop();
  //*
  if ( c.next != model->data( model->index( end + 1, 0, c.parent ) ) )
  {
    qDebug() << start << end;
    for ( int i = 0; i < model->rowCount(); ++i )
      qDebug() << model->index( i, 0 ).data().toString();
    qDebug() << c.next << model->data( model->index( end + 1, 0, c.parent ) );
  }
  //*/
  Q_ASSERT( c.parent == parent );
  Q_ASSERT_X( c.oldSize + ( end - start + 1 ) == model->rowCount( parent ), "Rows inserted", QStringLiteral( "%1 != %2" ).arg( c.oldSize + ( end - start + 1 ) ).arg( model->rowCount( parent ) ).toStdString().c_str() );
  Q_ASSERT( c.last == model->data( model->index( start - 1, 0, c.parent ) ) );
  Q_ASSERT( c.next == model->data( model->index( end + 1, 0, c.parent ) ) );
}

void ModelTest::layoutAboutToBeChanged()
{
  for ( int i = 0; i < std::clamp( model->rowCount(), 0, 100 ); ++i )
    changing.append( QPersistentModelIndex( model->index( i, 0 ) ) );
}

void ModelTest::layoutChanged()
{
  for ( int i = 0; i < changing.count(); ++i )
  {
    QPersistentModelIndex p = changing[i];
    Q_ASSERT( p == model->index( p.row(), p.column(), p.parent() ) );
  }
  changing.clear();
}

/*!
    Store what is about to be inserted to make sure it actually happens

    \sa rowsRemoved()
 */
void ModelTest::rowsAboutToBeRemoved( const QModelIndex &parent, int start, int end )
{
  Changing c;
  c.parent = parent;
  c.oldSize = model->rowCount( parent );
  c.last = model->data( model->index( start - 1, 0, parent ) );
  c.next = model->data( model->index( end + 1, 0, parent ) );
  remove.push( c );
}

/*!
    Confirm that what was said was going to happen actually did

    \sa rowsAboutToBeRemoved()
 */
void ModelTest::rowsRemoved( const QModelIndex &parent, int start, int end )
{
  Changing c = remove.pop();
  Q_ASSERT( c.parent == parent );
  Q_ASSERT_X( c.oldSize - ( end - start + 1 ) == model->rowCount( parent ), "Rows removed", QStringLiteral( "%1 != %2" ).arg( c.oldSize + ( end - start + 1 ) ).arg( model->rowCount( parent ) ).toStdString().c_str() );
  Q_ASSERT( c.last == model->data( model->index( start - 1, 0, c.parent ) ) );
  Q_ASSERT( c.next == model->data( model->index( start, 0, c.parent ) ) );
}
