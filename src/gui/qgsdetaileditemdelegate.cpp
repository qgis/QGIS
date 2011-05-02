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
/* $Id:$ */

#include "qgsdetaileditemdelegate.h"
#include "qgsdetaileditemwidget.h"
#include "qgsdetaileditemdata.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QCheckBox>
#include <QLinearGradient>
QgsDetailedItemDelegate::QgsDetailedItemDelegate( QObject * parent ) :
    QAbstractItemDelegate( parent ),
    mpWidget( new QgsDetailedItemWidget() ),
    mpCheckBox( new QCheckBox() )

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

void QgsDetailedItemDelegate::paint( QPainter * thepPainter,
                                     const QStyleOptionViewItem & theOption,
                                     const QModelIndex & theIndex ) const
{
  // After painting we need to restore the painter to its original state
  thepPainter->save();
  if ( qVariantCanConvert<QgsDetailedItemData>( theIndex.data( Qt::UserRole ) ) )
  {
    QgsDetailedItemData myData =
      qVariantValue<QgsDetailedItemData>( theIndex.data( Qt::UserRole ) );
    if ( myData.isRenderedAsWidget() )
    {
      paintAsWidget( thepPainter, theOption, myData );
    }
    else //render by manually painting
    {
      paintManually( thepPainter, theOption, myData );
    }
  } //can convert item data
  thepPainter->restore();
}



QSize QgsDetailedItemDelegate::sizeHint(
  const QStyleOptionViewItem & theOption,
  const QModelIndex & theIndex ) const
{
  if ( qVariantCanConvert<QgsDetailedItemData>( theIndex.data( Qt::UserRole ) ) )
  {
    QgsDetailedItemData myData =
      qVariantValue<QgsDetailedItemData>( theIndex.data( Qt::UserRole ) );
    if ( myData.isRenderedAsWidget() )
    {
      return QSize( 378, mpWidget->height() );
    }
    else // fall back to hand calculated & hand drawn item
    {
      //for some reason itmes are non selectable if using rect.width() on osx and win
      return QSize( 50, height( theOption, myData ) );
      //return QSize(theOption.rect.width(), myHeight + myVerticalSpacer);
    }
  }
  else //cant convert to qgsdetaileditemdata
  {
    return QSize( 50, 50 ); //fallback
  }
}

void QgsDetailedItemDelegate::paintManually( QPainter * thepPainter,
    const QStyleOptionViewItem & theOption,
    const QgsDetailedItemData theData ) const
{
  //
  // Get the strings and check box properties
  //
  //bool myCheckState = theIndex.model()->data(theIndex, Qt::CheckStateRole).toBool();
  mpCheckBox->setChecked( theData.isChecked() );
  mpCheckBox->setEnabled( theData.isEnabled() );
  QPixmap myCbxPixmap( mpCheckBox->size() );
  mpCheckBox->render( &myCbxPixmap ); //we will draw this onto the widget further down

  //
  // Calculate the widget height and other metrics
  //

  QFontMetrics myTitleMetrics( titleFont( theOption ) );
  QFontMetrics myDetailMetrics( detailFont( theOption ) );
  int myTextStartX = theOption.rect.x() + horizontalSpacing();
  int myTextStartY = theOption.rect.y() + verticalSpacing();
  int myHeight = myTitleMetrics.height() + verticalSpacing();

  //
  // Draw the item background with a gradient if its highlighted
  //
  if ( theOption.state & QStyle::State_Selected )
  {
    drawHighlight( theOption, thepPainter, height( theOption, theData ) );
    thepPainter->setPen( theOption.palette.highlightedText().color() );
  }
  else
  {
    thepPainter->setPen( theOption.palette.text().color() );
  }


  //
  // Draw the checkbox
  //
  if ( theData.isCheckable() )
  {
    thepPainter->drawPixmap( theOption.rect.x(),
                             theOption.rect.y() + mpCheckBox->height(),
                             myCbxPixmap );
    myTextStartX = theOption.rect.x() + myCbxPixmap.width() + horizontalSpacing();
  }
  //
  // Draw the decoration (pixmap)
  //
  bool myIconFlag = false;
  QPixmap myDecoPixmap = theData.icon();
  if ( !myDecoPixmap.isNull() )
  {
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
  thepPainter->setFont( titleFont( theOption ) );
  thepPainter->drawText( myTextStartX,
                         myTextStartY,
                         theData.title() );
  //
  // Draw the description with word wrapping if needed
  //
  thepPainter->setFont( detailFont( theOption ) ); //return to original font set by client
  if ( myIconFlag )
  {
    myTextStartY += verticalSpacing();
  }
  else
  {
    myTextStartY +=  myDetailMetrics.height() + verticalSpacing();
  }
  QStringList myList =
    wordWrap( theData.detail(), myDetailMetrics, theOption.rect.width() - myTextStartX );
  QStringListIterator myLineWrapIterator( myList );
  while ( myLineWrapIterator.hasNext() )
  {
    QString myLine = myLineWrapIterator.next();
    thepPainter->drawText( myTextStartX,
                           myTextStartY,
                           myLine );
    myTextStartY += myDetailMetrics.height() - verticalSpacing();
  }
} //render by manual painting


void QgsDetailedItemDelegate::paintAsWidget( QPainter * thepPainter,
    const QStyleOptionViewItem & theOption,
    const QgsDetailedItemData theData ) const
{

  mpWidget->setChecked( theData.isChecked() );
  mpWidget->setData( theData );
  mpWidget->resize( theOption.rect.width(), mpWidget->height() );
  mpWidget->setAutoFillBackground( true );
  //mpWidget->setAttribute(Qt::WA_OpaquePaintEvent);
  mpWidget->repaint();
  if ( theOption.state & QStyle::State_Selected )
  {
    drawHighlight( theOption, thepPainter, height( theOption, theData ) );
  }
  QPixmap myPixmap = QPixmap::grabWidget( mpWidget );
  thepPainter->drawPixmap( theOption.rect.x(),
                           theOption.rect.y(),
                           myPixmap );
}//render as widget

void QgsDetailedItemDelegate::drawHighlight( const QStyleOptionViewItem &theOption,
    QPainter * thepPainter,
    int theHeight ) const
{
  QColor myColor1 = theOption.palette.highlight().color();
  QColor myColor2 = myColor1;
  myColor2 = myColor2.lighter( 110 ); //10% lighter
  QLinearGradient myGradient( QPointF( 0, theOption.rect.y() ),
                              QPointF( 0, theOption.rect.y() + theHeight ) );
  myGradient.setColorAt( 0, myColor1 );
  myGradient.setColorAt( 0.1, myColor2 );
  myGradient.setColorAt( 0.5, myColor1 );
  myGradient.setColorAt( 0.9, myColor2 );
  myGradient.setColorAt( 1, myColor2 );
  thepPainter->fillRect( theOption.rect, QBrush( myGradient ) );
}

int QgsDetailedItemDelegate::height( const QStyleOptionViewItem & theOption,
                                     const QgsDetailedItemData theData ) const
{
  QFontMetrics myTitleMetrics( titleFont( theOption ) );
  QFontMetrics myDetailMetrics( detailFont( theOption ) );
  //we don't word wrap the title so its easy to measure
  int myHeight = myTitleMetrics.height() + verticalSpacing();
  //the detail needs to be measured though
  QStringList myList = wordWrap( theData.detail(),
                                 myDetailMetrics,
                                 theOption.rect.width() - ( mpCheckBox->width() + horizontalSpacing() ) );
  myHeight += ( myList.count() + 1 ) * ( myDetailMetrics.height() - verticalSpacing() );
  return myHeight;
}


QFont QgsDetailedItemDelegate::detailFont( const QStyleOptionViewItem &theOption ) const
{
  QFont myFont = theOption.font;
  return myFont;
}

QFont QgsDetailedItemDelegate::titleFont( const QStyleOptionViewItem &theOption ) const
{
  QFont myTitleFont = detailFont( theOption );
  myTitleFont.setBold( true );
  myTitleFont.setPointSize( myTitleFont.pointSize() );
  return myTitleFont;
}


QStringList QgsDetailedItemDelegate::wordWrap( QString theString,
    QFontMetrics theMetrics,
    int theWidth ) const
{
  if ( theString.isEmpty() ) return QStringList();
  if ( 50 >= theWidth ) return QStringList() << theString;
  //QString myDebug = QString("Word wrapping: %1 into %2 pixels").arg(theString).arg(theWidth);
  //qDebug(myDebug.toLocal8Bit());
  //iterate the string
  QStringList myList;
  QString myCumulativeLine = "";
  QString myStringToPreviousSpace = "";
  int myPreviousSpacePos = 0;
  for ( int i = 0; i < theString.count(); ++i )
  {
    QChar myChar = theString.at( i );
    if ( myChar == QChar( ' ' ) )
    {
      myStringToPreviousSpace = myCumulativeLine;
      myPreviousSpacePos = i;
    }
    myCumulativeLine += myChar;
    if ( theMetrics.width( myCumulativeLine ) >= theWidth )
    {
      //time to wrap
      //@todo deal with long strings that have no spaces
      //forcing a break at current pos...
      myList << myStringToPreviousSpace.trimmed();
      i = myPreviousSpacePos;
      myStringToPreviousSpace = "";
      myCumulativeLine = "";
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


void QgsDetailedItemDelegate::setVerticalSpacing( int theValue )
{
  mVerticalSpacing = theValue;
}


int QgsDetailedItemDelegate::horizontalSpacing() const
{
  return mHorizontalSpacing;
}


void QgsDetailedItemDelegate::setHorizontalSpacing( int theValue )
{
  mHorizontalSpacing = theValue;
}
