/***************************************************************************
                         qgsgrasyextensionwidget.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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

#ifndef QGSGRASYEXTENSIONWIDGET_H
#define QGSGRASYEXTENSIONWIDGET_H

#include <qscrollview.h>
#include "qgsgrasydialog.h"
#include <vector>

class QGridLayout;
class QgsVectorLayer;

/**This widget can be used as extension to QgsGraSyDialog. QgsGraSyDialog creates a new instance every time the number of classes or the mode changes*/
class QgsGraSyExtensionWidget: public QScrollView
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
    QgsGraSyExtensionWidget(QWidget* parent, int classfield, QgsGraSyDialog::mode mode, int nofclasses, QgsVectorLayer* vlayer);
    /**Destructor*/
    ~QgsGraSyExtensionWidget();
    /**Returns the number of the field to classify*/
    int classfield();
    /**Access to the widget objects. In QgsGraSyDialog, the widgets have to be casted to the proper subclasses to retrieve their information*/
    QWidget* getWidget(int column, int row);
 protected:
    /**Number of the field to classify*/
    int m_classfield;
    /**Layout object for the widget*/
    QGridLayout* mGridLayout;
    /**Stores the classification mode*/
    QgsGraSyDialog::mode mMode;
    /**Stores the number of classes*/
    int mNumberOfClasses;
    /**Pointer to the vector layer*/
    QgsVectorLayer* mVectorLayer;
    /**Pointers to the widgets are stored so that they are accessible for other classes*/
    std::vector<QWidget*> m_widgetvector;
    virtual void resizeEvent (QResizeEvent* e);
 protected slots:
     void selectColor();
     void selectFillPattern();
     void selectOutlineStyle();   
 private:
     /**Do not use the default constructor*/
     QgsGraSyExtensionWidget();

};

inline int QgsGraSyExtensionWidget::classfield()
{
    return m_classfield;
}

#endif
