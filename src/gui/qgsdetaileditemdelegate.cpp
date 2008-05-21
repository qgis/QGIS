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
  if (qVariantCanConvert<QgsDetailedItemData>(theIndex.data(Qt::UserRole))) 
  {
    QgsDetailedItemData myData = 
      qVariantValue<QgsDetailedItemData>(theIndex.data(Qt::UserRole));
    bool myCheckState = theIndex.model()->data(theIndex, Qt::CheckStateRole).toBool();
    mpWidget->setChecked(myCheckState);
    mpWidget->setData(myData);
    mpWidget->resize(theOption.rect.width(),mpWidget->height());
    mpWidget->setAutoFillBackground(false);
    mpWidget->repaint();

    if (theOption.state & QStyle::State_Selected)
    {
      QColor myColor1 = theOption.palette.highlight();
      QColor myColor2 = myColor1;
      myColor2 = myColor2.lighter();
      QLinearGradient myGradient(QPointF(0,theOption.rect.y()),
          QPointF(0,theOption.rect.y() + mpWidget->height()));
      myGradient.setColorAt(0, myColor1);
      myGradient.setColorAt(0.1, myColor2);
      myGradient.setColorAt(0.5, myColor1);
      myGradient.setColorAt(0.9, myColor2);
      myGradient.setColorAt(1, myColor2);
      thepPainter->fillRect(theOption.rect, QBrush(myGradient));
    }
    QPixmap myPixmap = QPixmap::grabWidget(mpWidget);
    thepPainter->drawPixmap(theOption.rect.x(),
        theOption.rect.y(), 
        myPixmap);
  } 
  else 
  {
    // After painting we need to restore the painter to its original state
    thepPainter->save();
    //
    // Get the strings and check box properties
    //
    QString myString = theIndex.model()->data(theIndex, Qt::DisplayRole).toString();
    QString myDetailString = theIndex.model()->data(theIndex, Qt::UserRole).toString();
    bool myCheckState = theIndex.model()->data(theIndex, Qt::CheckStateRole).toBool();
    mpCheckBox->setChecked(myCheckState);
    QPixmap myPixmap(mpCheckBox->size());
    mpCheckBox->render(&myPixmap); //we will draw this onto the widget further down

    //
    // Calculate the widget height and other metrics
    //
    QFont myFont = theOption.font;
    QFont myBoldFont = myFont;
    myBoldFont.setBold(true);
    myBoldFont.setPointSize(myFont.pointSize() + 3);
    QFontMetrics myMetrics(myBoldFont);
    int myVerticalSpacer = 3; //spacing between title and description
    int myHorizontalSpacer = 5; //spacing between checkbox / icon and description
    int myTextStartX = theOption.rect.x() + myPixmap.width() + myHorizontalSpacer;
    int myHeight = myMetrics.height() + myVerticalSpacer;

    //
    // Draw the item background with a gradient if its highlighted
    //
    if (theOption.state & QStyle::State_Selected)
    {
      QColor myColor1 = theOption.palette.highlight();
      QColor myColor2 = myColor1;
      myColor2 = myColor2.lighter();
      int myHeight = myMetrics.height() + myVerticalSpacer;
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
    thepPainter->drawPixmap(theOption.rect.x(),
        theOption.rect.y() + mpCheckBox->height(), 
        myPixmap);

    //
    // Draw the title and description
    //
    thepPainter->setFont(myBoldFont);
    thepPainter->drawText( myTextStartX ,theOption.rect.y() + myHeight, myString);
    thepPainter->setFont(myFont); //return to original font set by client
    thepPainter->drawText( myTextStartX, 
        theOption.rect.y() + (myHeight *2) - myVerticalSpacer, 
        myDetailString);
    thepPainter->restore();
  }
}

QSize QgsDetailedItemDelegate::sizeHint( 
       const QStyleOptionViewItem & theOption, 
       const QModelIndex & theIndex ) const
{
  if (qVariantCanConvert<QgsDetailedItemData>(theIndex.data(Qt::UserRole))) 
  {
    return QSize(378,mpWidget->height());
  }
  else // fall back to hand calculated & hand drawn item
  {
    QFont myFont = theOption.font;
    QFont myBoldFont = myFont;
    myBoldFont.setBold(true);
    myBoldFont.setPointSize(myFont.pointSize() + 3);
    QFontMetrics myMetrics(myBoldFont);
    int myVerticalSpacer = 3; //spacing between title and description
    int myHorizontalSpacer = 5; //spacing between checkbox / icon and description
    int myHeight = myMetrics.height() + myVerticalSpacer;
    return QSize(50,
        myHeight *2 + myVerticalSpacer);
  }
}
