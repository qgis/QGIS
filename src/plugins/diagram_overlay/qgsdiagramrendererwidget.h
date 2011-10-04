/***************************************************************************
                         qgsdiagramrendererwidget.h  -  description
                         --------------------------
    begin                : February 2007
    copyright            : (C) 2007 by Marco Hugentobler
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

#ifndef QGSDIAGRAMRENDERERWIDGET_H
#define QGSDIAGRAMRENDERERWIDGET_H

#include <QWidget>
#include "qgsdiagramfactory.h"
typedef QList<int> QgsAttributeList;

class QgsDiagramRenderer;
class QgsVectorLayer;

/**Interface for widgets that are specific to a cretain diagram renderer type. It returns a concrete diagram renderer
given the classification attribute, the attributes involved in diagram creation and a set of colors for the attributes*/
class QgsDiagramRendererWidget: public QWidget
{
  public:
    QgsDiagramRendererWidget( QgsVectorLayer* vl );
    virtual ~QgsDiagramRendererWidget();
    /**Returns a subclass of QgsDiagramRenderer or 0 in case of error
     @param type diagram type (e.g. Pie, Bar)
    @param classAttr index of the classification attribute
    @param attributes list of all attribute indices involved in diagram generation
    @param color color series for the attributes*/
    virtual QgsDiagramRenderer* createRenderer( int classAttr, const QgsAttributeList& attributes ) const = 0;
    /**Applies the settings of an existing renderer*/
    virtual void applySettings( const QgsDiagramRenderer* renderer ) = 0;
    /**Possibility to adapt the dialog to a new field*/
    virtual void changeClassificationField( int newField ) { Q_UNUSED( newField ); }

    virtual QgsDiagramFactory::SizeUnit sizeUnit() const { return QgsDiagramFactory::MM; }

  private:
    QgsDiagramRendererWidget();

  protected:
    QgsVectorLayer* mVectorLayer;
};

#endif
