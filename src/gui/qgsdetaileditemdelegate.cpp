/***************************************************************************
     qgsdetailedlistwidget.cpp  -  A rich QItemDelegate subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdetaileditemdelegate.h"
#include "qgsdetaileditemwidget.h"
#include "qgsdetaileditemdata.h"
#include "qgsrendercontext.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QCheckBox>
#include <QLinearGradient>
QgsDetailedItemDelegate::QgsDetailedItemDelegate( QObject *parent )
  : QAbstractItemDelegate( parent )
  , mpWidget( new QgsDetailedItemWidget() )
  , mpCheckBox( new QCheckBox() )

{
  //mpWidget->setFixedHeight(80);
  mpCheckBox->resize( mpCheckBox->sizeHint().height(), mpCheckBox->sizeHint().height() );
  setVerticalSpacing( 3 );
  setHorizontalSpacing( 5 );
}

QgsDetailedItemDelegate::~QgsDetailedItemDelegate()
{
  delete mpCheckBox;
  delete mpWidget;
}

void QgsDetailedItemDelegate::paint( QPainter *thepPainter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index ) const
{
  // After painting we need to restore the painter to its original state
  const QgsScopedQPainterState painterState( thepPainter );
  if ( index.data( Qt::UserRole ).userType() == QMetaType::type( "QgsDetailedItemData" ) )
  {
    const QgsDetailedItemData myData =
      index.data( Qt::UserRole ).value<QgsDetailedItemData>();
    if ( myData.isRenderedAsWidget() )
    {
      paintAsWidget( thepPainter, option, myData );
    }
    else //render by manually painting
    {
      paintManually( thepPainter, option, myData );
    }
  } //can convert item data
}



QSize QgsDetailedItemDelegate::sizeHint(
  const QStyleOptionViewItem &option,
  const QModelIndex &index ) const
{
  if ( index.data( Qt::UserRole ).userType() == QMetaType::type( "QgsDetailedItemData" ) )
  {
    const QgsDetailedItemData myData =
      index.data( Qt::UserRole ).value<QgsDetailedItemData>();
    if ( myData.isRenderedAsWidget() )
    {
      return QSize( 378, mpWidget->height() );
    }
    else // fall back to hand calculated & hand drawn item
    {
      //for some reason itmes are non selectable if using rect.width() on osx and win
      return QSize( 50, height( option, myData ) );
      //return QSize(theOption.rect.width(), myHeight + myVerticalSpacer);
    }
  }
  else //can't convert to qgsdetaileditemdata
  {
    return QSize( 50, 50 ); //fallback
  }
}

void QgsDetailedItemDelegate::paintManually( QPainter *thepPainter,
    const QStyleOptionViewItem &option,
    const QgsDetailedItemData &data ) const
{
  //
  // Get the strings and checkbox properties
  //
  //bool myCheckState = index.model()->data(theIndex, Qt::CheckStateRole).toBool();
  mpCheckBox->setChecked( data.isChecked() );
  mpCheckBox->setEnabled( data.isEnabled() );
  QPixmap myCbxPixmap( mpCheckBox->size() );
  mpCheckBox->render( &myCbxPixmap ); //we will draw this onto the widget further down

  //
  // Calculate the widget height and other metrics
  //

  const QFontMetrics myTitleMetrics( titleFont( option ) );
  const QFontMetrics myDetailMetrics( detailFont( option ) );
  int myTextStartX = option.rect.x() + horizontalSpacing();
  int myTextStartY = option.rect.y() + verticalSpacing();
  const int myHeight = myTitleMetrics.height() + verticalSpacing();

  //
  // Draw the item background with a gradient if its highlighted
  //
  if ( option.state & QStyle::State_Selected )
  {
    drawHighlight( option, thepPainter, height( option, data ) );
    thepPainter->setPen( option.palette.highlightedText().color() );
  }
  else
  {
    thepPainter->setPen( option.palette.text().color() );
  }


  //
  // Draw the checkbox
  //
  if ( data.isCheckable() )
  {
    thepPainter->drawPixmap( option.rect.x(),
                             option.rect.y() + mpCheckBox->height(),
                             myCbxPixmap );
    myTextStartX = option.rect.x() + myCbxPixmap.width() + horizontalSpacing();
  }
  //
  // Draw the decoration (pixmap)
  //
  bool myIconFlag = false;
  const QPixmap myDecoPixmap = data.icon();
  if ( !myDecoPixmap.isNull() )
  {
    myIconFlag = true;
    int iconWidth = 32, iconHeight = 32;

    if ( myDecoPixmap.width() <= iconWidth && myDecoPixmap.height() <= iconHeight )
    {
      // the pixmap has reasonable size
      int offsetX = 0, offsetY = 0;
      if ( myDecoPixmap.width() < iconWidth )
        offsetX = ( iconWidth - myDecoPixmap.width() ) / 2;
      if ( myDecoPixmap.height() < iconHeight )
        offsetY = ( iconHeight - myDecoPixmap.height() ) / 2;

      thepPainter->drawPixmap( myTextStartX + offsetX,
                               myTextStartY + offsetY,
                               myDecoPixmap );
    }
    else
    {
      // shrink the pixmap, it's too big
      thepPainter->drawPixmap( myTextStartX, myTextStartY, iconWidth, iconHeight, myDecoPixmap );
    }

    myTextStartX += iconWidth + horizontalSpacing();
  }
  //
  // Draw the title
  //
  myTextStartY += myHeight / 2;
  thepPainter->setFont( titleFont( option ) );
  thepPainter->drawText( myTextStartX,
                         myTextStartY,
                         data.title() );
  //
  // Draw the description with word wrapping if needed
  //
  thepPainter->setFont( detailFont( option ) ); //return to original font set by client
  if ( myIconFlag )
  {
    myTextStartY += verticalSpacing();
  }
  else
  {
    myTextStartY += myDetailMetrics.height() + verticalSpacing();
  }
  const QStringList myList =
    wordWrap( data.detail(), myDetailMetrics, option.rect.width() - myTextStartX );
  QStringListIterator myLineWrapIterator( myList );
  while ( myLineWrapIterator.hasNext() )
  {
    const QString myLine = myLineWrapIterator.next();
    thepPainter->drawText( myTextStartX,
                           myTextStartY,
                           myLine );
    myTextStartY += myDetailMetrics.height() - verticalSpacing();
  }

  //
  // Draw the category. Not sure if we need word wrapping for it.
  //
  thepPainter->setFont( categoryFont( option ) ); //return to original font set by client
  thepPainter->drawText( myTextStartX,
                         myTextStartY,
                         data.category() );

  //
  // Draw the category with word wrapping if needed
  //
  /*
  myTextStartY += verticalSpacing();
  if ( myIconFlag )
  {
    myTextStartY += verticalSpacing();
  }
  else
  {
    myTextStartY += myCategoryMetrics.height() + verticalSpacing();
  }
  myList =
    wordWrap( data.category(), myCategoryMetrics, option.rect.width() - myTextStartX );
  QStringListIterator myLineWrapIter( myList );
  while ( myLineWrapIter.hasNext() )
  {
    QString myLine = myLineWrapIter.next();
    thepPainter->drawText( myTextStartX,
                           myTextStartY,
                           myLine );
    myTextStartY += myCategoryMetrics.height() - verticalSpacing();
  }
  */
} //render by manual painting


void QgsDetailedItemDelegate::paintAsWidget( QPainter *thepPainter,
    const QStyleOptionViewItem &option,
    const QgsDetailedItemData &data ) const
{

  mpWidget->setChecked( data.isChecked() );
  mpWidget->setData( data );
  mpWidget->resize( option.rect.width(), mpWidget->height() );
  mpWidget->setAutoFillBackground( true );
  //mpWidget->setAttribute(Qt::WA_OpaquePaintEvent);
  mpWidget->repaint();
  if ( option.state & QStyle::State_Selected )
  {
    drawHighlight( option, thepPainter, height( option, data ) );
  }
  const QPixmap myPixmap = mpWidget->grab();
  thepPainter->drawPixmap( option.rect.x(),
                           option.rect.y(),
                           myPixmap );
}//render as widget

void QgsDetailedItemDelegate::drawHighlight( const QStyleOptionViewItem &option,
    QPainter *thepPainter,
    int height ) const
{
  const QColor myColor1 = option.palette.highlight().color();
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QLinearGradient myGradient( QPointF( 0, option.rect.y() ),
                              QPointF( 0, option.rect.y() + height ) );
  myGradient.setColorAt( 0, myColor1 );
  myGradient.setColorAt( 0.1, myColor2 );
  myGradient.setColorAt( 0.5, myColor1 );
  myGradient.setColorAt( 0.9, myColor2 );
  myGradient.setColorAt( 1, myColor2 );
  thepPainter->fillRect( option.rect, QBrush( myGradient ) );
}

int QgsDetailedItemDelegate::height( const QStyleOptionViewItem &option,
                                     const QgsDetailedItemData &data ) const
{
  const QFontMetrics myTitleMetrics( titleFont( option ) );
  const QFontMetrics myDetailMetrics( detailFont( option ) );
  const QFontMetrics myCategoryMetrics( categoryFont( option ) );
  //we don't word wrap the title so its easy to measure
  int myHeight = myTitleMetrics.height() + verticalSpacing();
  //the detail needs to be measured though
  const QStringList myList = wordWrap( data.detail(),
                                       myDetailMetrics,
                                       option.rect.width() - ( mpCheckBox->width() + horizontalSpacing() ) );
  myHeight += ( myList.count() + 1 ) * ( myDetailMetrics.height() - verticalSpacing() );
  //we don't word wrap the category so its easy to measure
  myHeight += myCategoryMetrics.height() + verticalSpacing();
#if 0
  // if category should be wrapped use this code
  myList = wordWrap( data.category(),
                     myCategoryMetrics,
                     option.rect.width() - ( mpCheckBox->width() + horizontalSpacing() ) );
  myHeight += ( myList.count() + 1 ) * ( myCategoryMetrics.height() - verticalSpacing() );
#endif
  return myHeight;
}


QFont QgsDetailedItemDelegate::detailFont( const QStyleOptionViewItem &option ) const
{
  const QFont myFont = option.font;
  return myFont;
}

QFont QgsDetailedItemDelegate::categoryFont( const QStyleOptionViewItem &option ) const
{
  QFont myFont = option.font;
  myFont.setBold( true );
  return myFont;
}

QFont QgsDetailedItemDelegate::titleFont( const QStyleOptionViewItem &option ) const
{
  QFont myTitleFont = detailFont( option );
  myTitleFont.setBold( true );
  myTitleFont.setPointSize( myTitleFont.pointSize() );
  return myTitleFont;
}


QStringList QgsDetailedItemDelegate::wordWrap( const QString &string,
    const QFontMetrics &metrics,
    int width ) const
{
  if ( string.isEmpty() )
    return QStringList();
  if ( 50 >= width )
    return QStringList() << string;
  //QString myDebug = QString("Word wrapping: %1 into %2 pixels").arg(theString).arg(theWidth);
  //qDebug(myDebug.toLocal8Bit());
  //iterate the string
  QStringList myList;
  QString myCumulativeLine;
  QString myStringToPreviousSpace;
  int myPreviousSpacePos = 0;
  for ( int i = 0; i < string.count(); ++i )
  {
    const QChar myChar = string.at( i );
    if ( myChar == QChar( ' ' ) )
    {
      myStringToPreviousSpace = myCumulativeLine;
      myPreviousSpacePos = i;
    }
    myCumulativeLine += myChar;
    if ( metrics.boundingRect( myCumulativeLine ).width() >= width )
    {
      //time to wrap
      //TODO deal with long strings that have no spaces
      //forcing a break at current pos...
      myList << myStringToPreviousSpace.trimmed();
      i = myPreviousSpacePos;
      myStringToPreviousSpace.clear();
      myCumulativeLine.clear();
    }
  }//end of i loop
  //add whatever is left in the string to the list
  if ( !myCumulativeLine.trimmed().isEmpty() )
  {
    myList << myCumulativeLine.trimmed();
  }

  //qDebug("Wrapped legend entry:");
  //qDebug(theString);
  //qDebug(myList.join("\n").toLocal8Bit());
  return myList;

}



int QgsDetailedItemDelegate::verticalSpacing() const
{
  return mVerticalSpacing;
}


void QgsDetailedItemDelegate::setVerticalSpacing( int value )
{
  mVerticalSpacing = value;
}


int QgsDetailedItemDelegate::horizontalSpacing() const
{
  return mHorizontalSpacing;
}


void QgsDetailedItemDelegate::setHorizontalSpacing( int value )
{
  mHorizontalSpacing = value;
}
