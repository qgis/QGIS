/***************************************************************************
                         qgswkndiagramfactory.h  -  description
                         ----------------------
    begin                : January 2007
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


#ifndef QGSWKNDIAGRAMFACTORY_H
#define QGSWKNDIAGRAMFACTORY_H

#include <QBrush>
#include <QList>
#include <QPen>
#include <QString>
#include "qgsdiagramcategory.h"
#include "qgsdiagramfactory.h"
#include "qgsvectorlayer.h" //for QgsAttributeList

class QImage;
class QgsDiagramItem;
class QgsFeature;

/**A class that renders diagrams for WellKnownNames diagram types
 (pie, bar)*/
class QgsWKNDiagramFactory: public QgsDiagramFactory
{
  public:
    QgsWKNDiagramFactory();
    virtual ~QgsWKNDiagramFactory();

    /**Writes the wkn diagram information into a <factory> tag*/
    bool writeXML( QDomNode& overlay_node, QDomDocument& doc ) const;
    /**Possibility for wkn subclasses to write specific information into the XML document*/
    virtual bool _writeXML( QDomNode& factory_node, QDomDocument& doc ) const
    { Q_UNUSED( factory_node ); Q_UNUSED( doc ); return true; }
    /**Returns the property described by the size (e.g. diameter or height). This can be important to
     know if e.g. size has to be calculated proportional to pie area*/
    virtual QgsDiagramFactory::SizeType sizeType() const = 0;

    //setters and getters for diagram type
    QString diagramType() const {return mDiagramType;}
    void setDiagramType( const QString& name ) {mDiagramType = name;}

    QList<QgsDiagramCategory> categories() const {return mCategories;}
    /**Adds a new category (attribute together with symbolisation)*/
    void addCategory( QgsDiagramCategory c );

    /**Returns the attribute indexes represented in the bars/pie slices*/
    QgsAttributeList categoryAttributes() const;

    /**Returns the supported well known names in a list*/
    static void supportedWellKnownNames( std::list<QString>& names );

    /**Read wkn settings from project file*/
    bool readXML( const QDomNode& factoryNode );

  protected:

    /**Well known diagram name (e.g. pie, bar, line)*/
    QString mDiagramType;
    /**List of attributes that are represented as slices, pillars, etc.*/
    QList<QgsDiagramCategory> mCategories;
    /**Maximum line width. Needs to be considered for the size of the generated image*/
    int mMaximumPenWidth;
    /**Maximum Gap. Needs to be considered for the size of the generated image*/
    int mMaximumGap;
};

#endif
