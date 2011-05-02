/***************************************************************************
                         qgsdiagramfactorywidget.h  -  description
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

#ifndef QGSDIAGRAMFACTORYWIDGET_H
#define QGSDIAGRAMFACTORYWIDGET_H

#include <QWidget>

class QgsDiagramFactory;

/**Abstract factory for dialogs that display options to create a diagram factory*/
class QgsDiagramFactoryWidget: public QWidget
{
  public:
    QgsDiagramFactoryWidget();
    virtual ~QgsDiagramFactoryWidget();
    /**Creates a diagram factory object with the settings specified by the user*/
    virtual QgsDiagramFactory* createFactory() = 0;
    /**Sets the GUI element to the state of an existing diagram factory*/
    virtual void setExistingFactory( const QgsDiagramFactory* f ) = 0;
};

#endif
