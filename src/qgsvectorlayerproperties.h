/***************************************************************************
                          qgsvectorlayerproperties.h  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
        Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSVECTORLAYERPROPERTIES_H
#define QGSVECTORLAYERPROPERTIES_H
class QgsVectorLayer;
#include "qgssymbol.h"
class QString;
#include "qgsvectorlayerpropertiesbase.uic.h"
#include "qgsrenderer.h"
#include "qpixmap.h"


/**Property sheet for a map layer
  *@author Gary E.Sherman
  */

class QgsVectorLayerProperties : public QgsVectorLayerPropertiesBase  {

Q_OBJECT

public:
/*! Constructor
* @param ml Map layer for which properties will be displayed
*/
    QgsVectorLayerProperties(QgsVectorLayer* ml);
	~QgsVectorLayerProperties();
	//! Name to display in legend
	QgsSymbol* getSymbol();
	/**Sets the legend type to "single symbol", "graduated symbol" or "continuous color"*/
	void setLegendType(QString type);
	/**Sets the dirty flag to false*/
	void unsetRendererDirty();
	/**Returns the value of rendererDirty*/
	bool getRendererDirty() const;
	/**Returns a pointer to the bufferDialog*/
	QDialog* getBufferDialog();
	/**Sets the buffer dialog*/
	void setBufferDialog(QDialog* dialog);
	QgsRenderer* getBufferRenderer();
	QPixmap* getBufferPixmap();
private:
	QgsVectorLayer* layer;
	/**Flag indicating if the render type still has to be changed (true) or not (false)*/
	bool rendererDirty;
	/**Renderer dialog. If the legend type has changed, it is assigned to the vectorlayer if apply or ok are pressed*/
	QDialog* bufferDialog;
	QgsRenderer* bufferRenderer;
	QPixmap bufferPixmap;
public slots:
    void alterLayerDialog(const QString& string);
    /**Sets the dirty flag to true*/ 
    void setRendererDirty();
    void apply();
    void cancel();
protected slots:
    void showSymbolSettings(); 
};

inline bool QgsVectorLayerProperties::getRendererDirty() const
{
    return rendererDirty;
}

inline void QgsVectorLayerProperties::setRendererDirty()
{
    rendererDirty=true; 
}

inline void QgsVectorLayerProperties::unsetRendererDirty()
{
    rendererDirty=false;
}

inline QDialog* QgsVectorLayerProperties::getBufferDialog()
{
    return bufferDialog;
}

inline void QgsVectorLayerProperties::setBufferDialog(QDialog* dialog)
{
    bufferDialog=dialog; 
}

inline QgsRenderer* QgsVectorLayerProperties::getBufferRenderer()
{
    return bufferRenderer;
}

inline QPixmap* QgsVectorLayerProperties::getBufferPixmap()
{
    return &bufferPixmap;
}

#endif
