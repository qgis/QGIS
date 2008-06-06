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
QgsDetailedItemDelegate::QgsDetailedItemDelegate(QObject * parent) : 
       QAbstractItemDelegate(parent),
       mpWidget(new QgsDetailedItemWidget()),
       mpCheckBox(new QCheckBox())
       
{
  //mpWidget->setFixedHeight(80);
  mpCheckBox->resize(16,16);
}

QgsDetailedItemDelegate::~QgsDetailedItemDelegate()
{
  delete mpCheckBox;
  delete mpWidget;
}

void QgsDetailedItemDelegate::paint(QPainter * thepPainter,
      const QStyleOptionViewItem & theOption,
      const QModelIndex & theIndex) const
{
  // After painting we need to restore the painter to its original state
  thepPainter->save();
  if (qVariantCanConvert<QgsDetailedItemData>(theIndex.data(Qt::UserRole))) 
  {
    QgsDetailedItemData myData = 
      qVariantValue<QgsDetailedItemData>(theIndex.data(Qt::UserRole));
    bool myCheckState = theIndex.model()->data(theIndex, Qt::CheckStateRole).toBool();
    if (myData.isRenderedAsWidget())
    {
      mpWidget->setChecked(myCheckState);
      mpWidget->setData(myData);
      mpWidget->resize(theOption.rect.width(),mpWidget->height());
      mpWidget->setAutoFillBackground(false);
      mpWidget->repaint();

      if (theOption.state & QStyle::State_Selected)
      {
        QColor myColor1 = theOption.palette.highlight();
        QColor myColor2 = myColor1;
        myColor2 = myColor2.lighter(110); //10% lighter
        QLinearGradient myGradient(QPointF(0,theOption.rect.y()),
            QPointF(0,theOption.rect.y() + mpWidget->height()));
        myGradient.setColorAt(0, myColor1);
        myGradient.setColorAt(0.1, myColor2);
        myGradient.setColorAt(0.5, myColor1);
        myGradient.setColorAt(0.9, myColor2);
        myGradient.setColorAt(1, myColor1);
        thepPainter->fillRect(theOption.rect, QBrush(myGradient));
      }
      QPixmap myPixmap = QPixmap::grabWidget(mpWidget);
      thepPainter->drawPixmap(theOption.rect.x(),
          theOption.rect.y(), 
          myPixmap);
    } //render as widget 
    else //render by manually painting
    {
      //
      // Get the strings and check box properties
      //
      bool myCheckState = theIndex.model()->data(theIndex, Qt::CheckStateRole).toBool();
      mpCheckBox->setChecked(myCheckState);
      QPixmap myCbxPixmap(mpCheckBox->size());
      mpCheckBox->render(&myCbxPixmap); //we will draw this onto the widget further down

      //
      // Calculate the widget height and other metrics
      //
      QFont myFont = theOption.font;
      QFont myTitleFont = myFont;
      myTitleFont.setBold(true);
      myTitleFont.setPointSize(myFont.pointSize() + 3);
      QFontMetrics myTitleMetrics(myTitleFont);
      QFontMetrics myDetailMetrics(myFont);
      int myVerticalSpacer = 3; //spacing between title and description
      int myHorizontalSpacer = 5; //spacing between checkbox / icon and description
      int myTextStartX = theOption.rect.x() + myHorizontalSpacer;
      int myTextStartY= theOption.rect.y() + myVerticalSpacer;
      int myHeight = myTitleMetrics.height() + myVerticalSpacer;

      //
      // Draw the item background with a gradient if its highlighted
      //
      if (theOption.state & QStyle::State_Selected)
      {
        QColor myColor1 = theOption.palette.highlight();
        QColor myColor2 = myColor1;
        myColor2 = myColor2.lighter(110); //10% lighter
        int myHeight = myTitleMetrics.height() + myVerticalSpacer;
        QLinearGradient myGradient(QPointF(0,theOption.rect.y()),
            QPointF(0,theOption.rect.y() + myHeight*2));
        myGradient.setColorAt(0, myColor1);
        myGradient.setColorAt(0.1, myColor2);
        myGradient.setColorAt(0.5, myColor1);
        myGradient.setColorAt(0.9, myColor2);
        myGradient.setColorAt(1, myColor2);
        thepPainter->fillRect(theOption.rect, QBrush(myGradient));
      }

      //
      // Draw the checkbox
      //
      bool myCheckableFlag = true;
      if (theIndex.flags() == Qt::ItemIsUserCheckable)
      {
        myCheckableFlag = false;
      }
      if (myCheckableFlag)
      {
        thepPainter->drawPixmap(theOption.rect.x(),
            theOption.rect.y() + mpCheckBox->height(), 
            myCbxPixmap);
        myTextStartX = theOption.rect.x() + myCbxPixmap.width() + myHorizontalSpacer;
      }
      //
      // Draw the decoration (pixmap)
      //
      bool myIconFlag = false;
      QPixmap myDecoPixmap = myData.icon();
      if (!myDecoPixmap.isNull())
      {
        thepPainter->drawPixmap(myTextStartX,
            myTextStartY + (myDecoPixmap.height() / 2), 
            myDecoPixmap);
        myTextStartX += myDecoPixmap.width() + myHorizontalSpacer;
      }
      //
      // Draw the title 
      //
      myTextStartY += myHeight/2;
      thepPainter->setFont(myTitleFont);
      thepPainter->drawText( myTextStartX ,
          myTextStartY , 
          myData.title());
      //
      // Draw the description with word wrapping if needed
      //
      thepPainter->setFont(myFont); //return to original font set by client
      if (myIconFlag)
      {
        myTextStartY += myVerticalSpacer;
      }
      else
      {
        myTextStartY +=  myDetailMetrics.height() + myVerticalSpacer;
      }
      QStringList myList = 
        wordWrap( myData.detail(), myDetailMetrics, theOption.rect.width() - myTextStartX );
      QStringListIterator myLineWrapIterator(myList);
      while (myLineWrapIterator.hasNext())
      {
        QString myLine = myLineWrapIterator.next();
        thepPainter->drawText( myTextStartX, 
            myTextStartY,
            myLine);
        myTextStartY += myDetailMetrics.height() - myVerticalSpacer;
      }
    } //render by manual painting
  } //can convert item data
  thepPainter->restore();
}

QSize QgsDetailedItemDelegate::sizeHint( 
       const QStyleOptionViewItem & theOption, 
       const QModelIndex & theIndex ) const
{
  if (qVariantCanConvert<QgsDetailedItemData>(theIndex.data(Qt::UserRole))) 
  {
    QgsDetailedItemData myData = 
      qVariantValue<QgsDetailedItemData>(theIndex.data(Qt::UserRole));
    if (myData.isRenderedAsWidget())
    {
      return QSize(378,mpWidget->height());
    }
    else // fall back to hand calculated & hand drawn item
    {
      QFont myFont = theOption.font;
      QFont myTitleFont = myFont;
      myTitleFont.setBold(true);
      myTitleFont.setPointSize(myFont.pointSize() + 3);
      QFontMetrics myTitleMetrics(myTitleFont);
      QFontMetrics myDetailMetrics(myFont);
      int myVerticalSpacer = 3; //spacing between title and description
      int myHorizontalSpacer = 5; //spacing between checkbox / icon and description
      int myHeight = myTitleMetrics.height() + myVerticalSpacer;
      QString myDetailString = theIndex.model()->data(theIndex, Qt::UserRole).toString();
      QStringList myList = wordWrap( myDetailString, 
          myDetailMetrics, 
          theOption.rect.width() - (mpCheckBox->width() + myHorizontalSpacer));
      myHeight += (myList.count() + 1) * (myDetailMetrics.height() - myVerticalSpacer);
      //for some reason itmes are non selectable if using rect.width() on osx and win
      return QSize(50, myHeight + myVerticalSpacer);
      //return QSize(theOption.rect.width(), myHeight + myVerticalSpacer);
    }
  }
  else //cant convert to qgsdetaileditemdata
  {
    return QSize(50,50); //fallback
  }
}

QStringList QgsDetailedItemDelegate::wordWrap(QString theString, 
                                    QFontMetrics theMetrics, 
                                    int theWidth) const
{
  if ( theString.isEmpty() ) return QStringList();
  if ( 50 >= theWidth ) return QStringList() << theString;
  //QString myDebug = QString("Word wrapping: %1 into %2 pixels").arg(theString).arg(theWidth);
  //qDebug(myDebug.toLocal8Bit());
  //iterate the string 
  QStringList myList;
  QString myCumulativeLine="";
  QString myStringToPreviousSpace="";
  int myPreviousSpacePos=0;
  for (int i=0; i < theString.count(); ++i)
  {
    QChar myChar = theString.at(i);
    if (myChar == QChar(' '))
    {
      myStringToPreviousSpace = myCumulativeLine;
      myPreviousSpacePos=i;
    }
    myCumulativeLine += myChar;
    if (theMetrics.width(myCumulativeLine) >= theWidth)
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
  if (!myCumulativeLine.trimmed().isEmpty())
  {
      myList << myCumulativeLine.trimmed();
  }

  //qDebug("Wrapped legend entry:");
  //qDebug(theString);
  //qDebug(myList.join("\n").toLocal8Bit());
  return myList;

}

