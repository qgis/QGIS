/***************************************************************************

               ----------------------------------------------------
              date                 : 17.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswelcomepageitemdelegate.h"
#include "qgsmessagelog.h"

#include <QApplication>
#include <QPainter>
#include <QTextDocument>

QgsWelcomePageItemDelegate::QgsWelcomePageItemDelegate( QObject * parent ) : QStyledItemDelegate( parent ) {}

void QgsWelcomePageItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    painter->save();
    
    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, NULL );
    
    QTextDocument doc;
    doc.setHtml( index.data( Qt::DisplayRole ).toString() );
    doc.setTextWidth( 800 );

    painter->translate( option.rect.left(), option.rect.top() );
    QRect clip( 0, 0, option.rect.width(), option.rect.height() );
    doc.drawContents( painter, clip );

    painter->restore();
}

QSize QgsWelcomePageItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{    
    QgsMessageLog::logMessage( QString( "width: %1" ).arg( index.data( Qt::DisplayRole ).toString() ) );
    
    QTextDocument doc;
    doc.setHtml( index.data( Qt::DisplayRole ).toString() );
    doc.setTextWidth( 800 );
    return QSize( option.rect.width(), doc.size().height() );
}

