/***************************************************************************
                          qgsmaplayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
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
#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include <qwidget.h>
#include <qobject.h>
/* #include "qgsdatasource.h" */
#include "qgsrect.h"
#include "qgis.h"
#include "qgscoordinatetransform.h"
#include "qgssymbol.h"
#include <qpixmap.h>
class QPopupMenu;
class QgsLegendItem;


/** \class QgsMapLayer
 * \brief Base class for all map layer types.
 * This class is the base class for all map layer types (shapefile,
 * raster, database). 
 */
class QgsMapLayer:public QObject
{
  Q_OBJECT public:
	/*! Constructor
	 * @param type Type of layer as defined in LAYERS enum
	 * @param lyrname Display Name of the layer
	 */
	QgsMapLayer(int type = 0, QString lyrname = QString::null, QString source = QString::null);
	//! Destructor
	  virtual ~ QgsMapLayer();
	/*! Get the type of the layer
	 * @return Integer matching a value in the LAYERS enum
	 */
	const int type();
	/*! Get this layer's unique ID */
	QString getLayerID();
	/*! Set the name of the layer
	   # @param name New name for the layer
	 */
	void setlayerName(const QString & name);
	/*! Get the name of the layer
	 * @return the layer name
	 */
	const QString name();
	/*! Virtual function to calculate the extent of the current layer.
	 * This function must be overridden in all child classes and implemented
	 * based on the layer type
	 */
	virtual QgsRect calculateExtent();
	virtual void draw(QPainter *, QgsRect *, int);
	virtual void draw(QPainter *, QgsRect *, QgsCoordinateTransform * cXf);
	/*! Identify the feature(s) in this layer that are contained in the search rectangle
	 */
	virtual void identify(QgsRect *){};
	/*!Select features on the map canvas by dragging a rectangle*/
	virtual void select(QgsRect *, bool lock){};
	/*! Display the attribute table for the layer
	 */
	virtual void table(){};
	/*! Return the extent of the layer as a QRect
	 */
	const QgsRect extent();
	/*! Returns the status of the layer. An invalid layer is one which has a bad datasource
	 * or other problem. Child classes set this flag when intialized
	 *@return True if the layer is valid and can be accessed
	 */
	bool isValid();
  /** Write property of QgsSymbol * symbol. */
	virtual void setSymbol(QgsSymbol * _newVal);
  /** Read property of QgsSymbol * symbol. */
	virtual QgsSymbol *symbol();
  /** Write property of QString labelField. */
	virtual void setlabelField(const QString & _newVal);
  /** Read property of QString labelField. */
	virtual const QString & labelField();
	//! Visibility of the layer
	bool visible();
	//! set visibility
	void setVisible(bool vis);
	//! Returns the source for the layer
	QString source();
  /** Write property of int featureType. */
	virtual void setFeatureType(const int &_newVal);
  /** Read property of int featureType. */
	virtual const int &featureType();
  /** Return the context menu for the layer */
  virtual QPopupMenu *contextMenu(); //TODO make this pure virtual after implemented in raster
  public:						// Public attributes
	//! Layers enum defining the types of layers that can be added to a map
	enum LAYERS
	{
		VECTOR,
		RASTER,
		DATABASE
	};
	/**Shows the properties dialog for the map layer*/
	virtual void showLayerProperties()=0;
	/**Returns a pointer to the legend pixmap*/
	virtual QPixmap* legendPixmap();
	/**Returns a pointer to the legend item*/
	QgsLegendItem* legendItem();
	/**Sets the pointer to the legend item*/
	void setLegendItem(QgsLegendItem* li);

	  signals:void visibilityChanged(void);
  protected:
	//! Extent of the layer
	  QgsRect layerExtent;
	//! Indicates if the layer is valid and can be drawn
	bool valid;
	QString dataSource;
	//! Geometry type as defined in enum WKBTYPE (qgis.h)
	int geometryType;
	/**Pointer to the legend item for this layer*/
	QgsLegendItem* m_legendItem;
	/**Pixmap used in the legend item*/
	QPixmap m_legendPixmap;
	/** Name of the layer - used for display  */
	QString layerName;
  private:						// Private attributes
	/** Unique ID of this layer - used to refer to this layer  in QGIS code */
	  QString ID;
	/** Type of the layer (eg. vector, raster, database  */
	int layerType;

	//! Tag for embedding additional information
	QString tag;
  /**  */
	QgsSymbol *m_symbol;

	bool m_visible;
  /**  */

  public:						// Public attributes
  /**  */
	  QString m_labelField;

  signals:
	  /**This signal should be connected with the slot QgsMapCanvas::refresh()*/
	  void repaintRequested();
};

#endif
