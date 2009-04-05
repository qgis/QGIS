/***************************************************************************
     BeataView.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QModelIndex>
#include <QItemDelegate>
#include <QHeaderView>
#include <QSettings>
#include <QLineEdit>
#include <QPainter>
#include <QKeyEvent>

#include "BeataView.h"
#include "BeataModel.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"


class BeataDelegate : public QItemDelegate
{
  public:
    BeataDelegate( QObject* parent = NULL ) : QItemDelegate( parent ) {}

    QWidget * createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
    {
      QWidget *editor = QItemDelegate::createEditor( parent, option, index );

      QLineEdit *le = dynamic_cast<QLineEdit*>( editor );
      if ( !le ) return editor;

      const BeataModel* m = dynamic_cast<const BeataModel*>( index.model() );
      if ( !m ) return editor;

      int col = index.column();
      QVariant::Type type = m->layer()->dataProvider()->fields()[col].type();

      if ( type == QVariant::Int )
      {
        le->setValidator( new QIntValidator( le ) );
      }
      else if ( type == QVariant::Double )
      {
        le->setValidator( new QDoubleValidator( le ) );
      }

      return editor;
    }


    void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
    {
      QItemDelegate::paint( painter, option, index );

      if ( option.state & QStyle::State_HasFocus )
      {
        QRect r = option.rect.adjusted( 1, 1, -1, -1 );
        QPen p( QBrush( QColor( 0, 255, 127 ) ), 2 );
        painter->save();
        painter->setPen( p );
        painter->drawRect( r );
        painter->restore();
      }
    }

};

BeataView::BeataView( QWidget* parent )
    : QTableView( parent )
{
  QSettings settings;
  restoreGeometry( settings.value( "/BetterTable/geometry" ).toByteArray() );

  verticalHeader()->setDefaultSectionSize( 20 );
  horizontalHeader()->setHighlightSections( false );

  setItemDelegate( new BeataDelegate( this ) );

  setSelectionBehavior( QAbstractItemView::SelectRows );
  setSelectionMode( QAbstractItemView::NoSelection );
  setSortingEnabled( true );

  shiftPressed = false;
  ctrlPressed = false;
}

void BeataView::setLayer( QgsVectorLayer* layer )
{
  BeataModel *bModel;

  if ( layer->dataProvider()->capabilities() & QgsVectorDataProvider::RandomSelectGeometryAtId )
    bModel = new BeataModel( layer );
  else
    bModel = new BeataMemModel( layer );

  BeataFilterModel* bfModel = new BeataFilterModel( layer );
  bfModel->setSourceModel( bModel );

  setModel( bfModel );
}

BeataView::~BeataView()
{
}

void BeataView::closeEvent( QCloseEvent *event )
{
  QSettings settings;
  settings.setValue( "/BetterAttributeTable/geometry", QVariant( saveGeometry() ) );
}

void BeataView::keyPressEvent( QKeyEvent *event )
{
  // shift pressed
  if ( event->key() == Qt::Key_Shift )// && event->modifiers() & Qt::ShiftModifier)
    shiftPressed = true;
  else if ( event->key() == Qt::Key_Control )
    ctrlPressed = true;
  else
    QTableView::keyPressEvent( event );
}

void BeataView::keyReleaseEvent( QKeyEvent *event )
{
  // workaround for some Qt bug
  if ( event->key() == Qt::Key_Shift || event->key() == -1 )
    shiftPressed = false;
  else if ( event->key() == Qt::Key_Control )
    ctrlPressed = false;
  else
    QTableView::keyReleaseEvent( event );
}
