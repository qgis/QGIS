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

#ifndef QGSGRASYEXTENSIONWIDGET_H
#define QGSGRASYEXTENSIONWIDGET_H

#include <qwidget.h>
#include "qgsgrasydialog.h"
#include <vector>

class QGridLayout;
class QgsVectorLayer;

/**This widget can be used as extension to QgsGraSyDialog. QgsGraSyDialog creates a new instance every time the number of classes or the mode changes*/
class QgsGraSyExtensionWidget: public QWidget
{
    Q_OBJECT
 public:
    QgsGraSyExtensionWidget(QWidget* parent, int classfield, QgsGraSyDialog::mode mode, int nofclasses, QgsVectorLayer* vlayer);
    ~QgsGraSyExtensionWidget();
    /**Returns the number of the field to classify*/
    int classfield();
    /**Access to the widget objects. In QgsGraSyDialog, the widgets have to be casted to the proper subclasses to retrieve their information*/
    QWidget* getWidget(int column, int row);
 protected:
    /**Number of the field to classify*/
    int m_classfield;
    QGridLayout* m_gridlayout;
    QgsGraSyDialog::mode m_mode;
    int m_numberofclasses;
    QgsVectorLayer* m_vectorlayer;
    /**Pointers to the widgets are stored so that they are accessible for other classes*/
    std::vector<QWidget*> m_widgetvector;
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
