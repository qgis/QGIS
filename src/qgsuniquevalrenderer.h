/***************************************************************************
                         qgsuniquevalrenderer.h  -  description
                             -------------------
    begin                : July 2004
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

#ifndef QGSUNIQUEVALRENDERER_H
#define QGSUNIQUEVALRENDERER_H

#include "qgsrenderer.h"
#include <qcolor.h>
#include <map>

class QgsRenderItem;

class QgsUniqueValRenderer: public QgsRenderer
{
 public:
    QgsUniqueValRenderer();
    ~QgsUniqueValRenderer();
    void initializeSymbology(QgsVectorLayer* layer, QgsDlgVectorLayerProperties* pr=0);
    void renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, double* scalefactor, bool selected);
    /**Reads the renderer configuration from an XML file
     @param rnode the DOM node to read 
     @param vl the vector layer which will be associated with the renderer*/
    void readXML(const QDomNode& rnode, QgsVectorLayer& vl);
    /**Writes the contents of the renderer to a configuration file*/
    void writeXML(std::ofstream& xml);
    /** Returns true, if attribute values are used by the renderer and false otherwise*/
    bool needsAttributes();
    /**Returns a list with indexes of classification attributes*/
    std::list<int> classificationAttributes();
    /**Returns the renderers name*/
    QString name();
    /**Inserts an entry into mEntries. The render items have to be created with the new operator and are automatically destroyed if not needed anymore*/
    void insertValue(QString name, QgsRenderItem* item);
    /**Removes all entries from mEntries*/
    void clearValues();
    /**Sets the Field index used for classification*/
    void setClassificationField(int field);
    /**Returns the index of the classification field*/
    int classificationField();
 protected:
    /**Field index used for classification*/
    int mClassificationField;
    /**Entries for the unique values*/
    std::map<QString,QgsRenderItem*> mEntries;
    /**Colour used to render selected features*/
    QColor mSelectionColor;
};

inline bool QgsUniqueValRenderer::needsAttributes()
{
    return true;
}

inline void QgsUniqueValRenderer::insertValue(QString name, QgsRenderItem* item)
{
    mEntries.insert(std::make_pair(name,item));
}

inline void QgsUniqueValRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

inline int QgsUniqueValRenderer::classificationField()
{
    return mClassificationField;
}

#endif
