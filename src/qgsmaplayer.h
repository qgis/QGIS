/***************************************************************************
                          qgsmaplayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include <qwidget.h>
#include "qgsdatasource.h"
#include "qgsrect.h"
#include "qgscoordinatetransform.h"
#include "qgssymbol.h"
//Class QgsSymbol;

/** \class QgsMapLayer
 * \brief Base class for all map layer types.
 * This class is the base class for all map layer types (shapefile,
 * raster, database). 
 */
class QgsMapLayer : public QgsDataSource  {

 public: 
    /*! Constructor
     * @param type Type of layer as defined in LAYERS enum
     * @param lyrname Display Name of the layer
     */
    QgsMapLayer(int type=0, QString lyrname=QString::null );
    //! Destructor
    virtual ~QgsMapLayer();
    /*! Get the type of the layer
     * @return Integer matching a value in the LAYERS enum
     */
    const int type();
    /*! Set the name of the layer
      # @param name New name for the layer
    */
    void setlayerName( const QString& name);
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
    virtual void draw(QPainter *, QgsRect *, QgsCoordinateTransform *cXf);
    /*! Return the extent of the layer as a QRect
     */
    const QgsRect extent();
    /*! Returns the status of the layer. An invalid layer is one which has a bad datasource
     * or other problem. Child classes set this flag when intialized
     *@return True if the layer is valid and can be accessed
     */
    bool isValid();
  /** Write property of QgsSymbol * symbol. */
  virtual void setSymbol( QgsSymbol * _newVal);
  /** Read property of QgsSymbol * symbol. */
  virtual const QgsSymbol * symbol();

 public: // Public attributes
    //! Layers enum defining the types of layers that can be added to a map
    enum LAYERS {
	VECTOR,
	RASTER,
	DATABASE
    }  ;
 protected:
    //! Extent of the layer
    QgsRect layerExtent; 
    //! Position in the map stack 
    int zpos;
    //! Indicates if the layer is valid and can be drawn
    bool valid;
 private: // Private attributes
    /** Name of the layer - used for display  */
    QString layerName;
    /** Type of the layer (eg. vector, raster, database  */
    int layerType;

    //! Tag for embedding additional information
    QString tag;
  /**  */
  QgsSymbol * m_symbol;
 
};

#endif
