/***************************************************************************
                         qgswkndiagramfactorywidget.h  -  description
                         --------------------------
    begin                : December 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWKNDIAGRAMFACTORYWIDGET_H
#define QGSWKNDIAGRAMFACTORYWIDGET_H

#include "qgsdiagramfactorywidget.h"
#include "ui_qgswkndiagramfactorywidgetbase.h"

class QgsVectorLayer;

/**A class that creates diagram factories for well-known-name diagram types
   (bar, pie, etc.)*/
class QgsWKNDiagramFactoryWidget: public QgsDiagramFactoryWidget, private Ui::QgsWKNDiagramFactoryWidgetBase
{
    Q_OBJECT
  public:
    /**Creates a */
    QgsWKNDiagramFactoryWidget( QgsVectorLayer* vl, const QString& wellKnownName );
    ~QgsWKNDiagramFactoryWidget();

    QgsDiagramFactory* createFactory();

    void setExistingFactory( const QgsDiagramFactory* f );

  private:
    QgsWKNDiagramFactoryWidget();

    QgsVectorLayer* mVectorLayer;
    QString mDiagramTypeName;

  private slots:
    /**Adds name of the attribute combo box into the tree widget*/
    void addAttribute();
    /**Removes the current attribute from the tree widget*/
    void removeAttribute();
    /**Calls the color dialog if column == 1*/
    void handleItemDoubleClick( QTreeWidgetItem * item, int column );
};

#endif
