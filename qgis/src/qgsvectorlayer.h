/***************************************************************************
                          qgsvectorlayer.h  -  description
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
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

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H
class QPainter;
class QgsRect;
class QgsCoordinateTransform;
class OGRLayer;
class OGRDataSource;
class QgsDataProvider;
class QgsRenderer;
class QgsLegendItem;
class QgsVectorLayerProperties;
class QgisApp;

#include <map>
#include "qgsmaplayer.h"
#include "qvaluevector.h"
#include "qgsattributetabledisplay.h"
#include <map>

/*! \class QgsVectorLayer
 * \brief Vector layer backed by a data source provider
 */

class QgsVectorLayer:public QgsMapLayer
{
  Q_OBJECT public:

	//! Constructor
	  QgsVectorLayer(QString baseName = 0, QString path = 0, QString providerLib=0);
	//! Destructor
	 ~QgsVectorLayer();
	//! Identify feature found within the search rectangle
	void identify(QgsRect *);
	//! Select features found within the search rectangle
	void select(QgsRect * rect, bool lock);
	//! Display the attribute table
	void table();
  //! Set the primary display field to be used in the identify results dialog 
  void setDisplayField();
  //! Initialize the context menu
  void initContextMenu(QgisApp *app);
	enum SHAPETYPE
	{
		Point,
		Line,
		Polygon
	};
	void setDataProvider(QgsDataProvider * dp);
	QgsDataProvider* getDataProvider();
	public slots:
	 /**Sets the 'tabledisplay' to 0 again*/
	void invalidateTableDisplay();
	void select(int number);
	void removeSelection();
	void triggerRepaint();
	/**Shows the properties dialog*/
	virtual void showLayerProperties();
	/**Returns a pointer to the renderer*/
	QgsRenderer* renderer();
	/**Returns a pointer to the renderer dialog*/
	QDialog* rendererDialog();
	/**Sets the renderer. If a renderer is already present, it is deleted*/
	void setRenderer(QgsRenderer* r);
	/**Sets the renderer dialog. If a renderer dialog is already present, it is deleted*/
	void setRendererDialog(QDialog* dialog);
	/**Returns point, line or polygon*/
	QGis::VectorType vectorType();
	/**Returns a pointer to the properties dialog*/
	QgsVectorLayerProperties* propertiesDialog();
  /** Return the context menu for the layer */
  QPopupMenu *contextMenu();
  protected:
	/**Pointer to the table display object if there is one, else a pointer to 0*/
	QgsAttributeTableDisplay* tabledisplay;
	/**Vector holding the information which features are activated*/
	std::map<int,bool> selected;
	/**Color to draw and fill the selected features*/
	QColor selectionColor;
	/**Renderer object which holds the information about how to display the features*/
	QgsRenderer* m_renderer;
	/**Dialog to set the properties*/
	QgsVectorLayerProperties* m_propertiesDialog;
	/**Widget to set the symbology properties*/
	QDialog* m_rendererDialog;
  private:						// Private attributes
//! Draws the layer using coordinate transformation
	void draw_old(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf);
	void draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf);
	//! Pointer to data provider derived from the abastract base class QgsDataProvider
	QgsDataProvider *dataProvider;
  //  OGRDataSource *ogrDataSource;
   // OGRLayer *ogrLayer;
  /**  */
  // index of the primary label field
  QString fieldIndex;
	bool registered;
  QPopupMenu *popMenu;
	enum ENDIAN
	{
		NDR = 1,
		XDR = 0
	};
	enum WKBTYPE
	{
		WKBPoint = 1,
		WKBLineString,
		WKBPolygon,
		WKBMultiPoint,
		WKBMultiLineString,
		WKBMultiPolygon
	};
  private:						// Private methods
  /** No descriptions */
	void registerFormats();
	int endian();
	signals:void repaintRequested();

};

#endif
