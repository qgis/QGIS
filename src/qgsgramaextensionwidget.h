/***************************************************************************
                          qgsgramaextensionwidget.h  -  description
                             -------------------
    begin                : April 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id */

#ifndef QGSGRAMAEXTENSIONWIDGET
#define QGSGRAMAEXTENSIONWIDGET

#include <qscrollview.h>
#include <vector>
#include "qgsgrasydialog.h"

class QgsVectorLayer;

class QgsGraMaExtensionWidget: public QScrollView
{
    Q_OBJECT
 public:
    /**Constructor
       @param parent the parent widget
       @param the index of the field, which is used to classify
       @param mode the mode used for classification, e.g. QgsGraSyDialog::EQUAL_INTERVAL or QgsGraSyDialog::EMPTY
       @param nofclasses the number of classes
       @param vlayer a pointer to the vector layer
    */
    QgsGraMaExtensionWidget(QWidget* parent, int classfield, QgsGraSyDialog::mode mode, int nofclasses, QgsVectorLayer* vlayer);
    /**Destructor*/
    ~QgsGraMaExtensionWidget();
    /**Returns the number of the field to classify*/
    int classfield();
    /**Access to the widget objects. In QgsGraMaDialog, the widgets have to be casted to the proper subclasses to retrieve their information*/
    QWidget* getWidget(int column, int row);
    /**Resizes all marker images (in case the scale factors may have changed)*/
    void adjustMarkers();
    /**Adjusts the marker size in one row*/
    void adjustMarker(int row);

 protected:
    /**Number of the field to classify*/
    int mClassField;
    /**Resizing works properly if the layout is put into a widget*/
    QWidget* mWidget;
    /**Layout object for the widget*/
    QGridLayout* mGridLayout;
    /**Stores the classification mode*/
    QgsGraSyDialog::mode mMode;
    /**Stores the number of classes*/
    int mNumberOfClasses;
    /**Pointer to the vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Pointers to the widgets are stored so that they are accessible for other classes*/
    std::vector<QWidget*> mWidgetVector;

protected slots:
    void selectMarker();
    void handleReturnPressed(); 

 private:
    /**Do not use the default constructor*/
    QgsGraMaExtensionWidget();
};

inline QWidget* QgsGraMaExtensionWidget::getWidget(int column, int row)
{
    return mWidgetVector[column+row*5];
}


#endif
