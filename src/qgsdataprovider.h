/***************************************************************************
                qgsdataprovider.h - DataProvider Interface class
                     --------------------------------------
    Date                 : 09-Sep-2003
    Copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QQGSDATAPROVIDER_H
#define QQGSDATAPROVIDER_H

#include <vector>
#include <list>

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class QgsRect;
class QgsFeature;
class QgsField;
class QgsDataSourceURI;
class QgsSpatialRefSys;
/** \class QgsDataProvider
 * \brief Abstract base class for spatial data provider implementations
 * @author Gary E.Sherman
 * 
 * This object needs to inherit from QObject to enable event 
 * processing in the Postgres/PostGIS provider (QgsPostgresProvider).
 * It is called *here* so that this vtable and the vtable for 
 * QgsPostgresProvider don't get misaligned - 
 * the QgsVectorLayer class factory (which refers
 * to generic QgsVectorDataProvider's) depends on it.
 */


class QgsDataProvider : public QObject {

  Q_OBJECT

    public: 
      /**
       * We need this so the subclass destructors get called
       */
      virtual ~QgsDataProvider() {};

      /*! Set the QgsSpatialReferenceSystem for this layer..
       * @note Must be reimplemented by each provider. 
       * @param theSRS QgsSpatialRefSys to be assigned to this layer
       *        A complete copy of the passed in SRS will be made.
       */
      virtual void setSRS(QgsSpatialRefSys * theSRS){};

      /*! Get the QgsSpatialRefSys for this layer
       * @note Must be reimplemented by each provider. 
       * If the provider isn't capable of returning
       * its projection an empty srs will be return, ti will return 0
       */
      virtual QgsSpatialRefSys * getSRS(){return 0;};


      /** Used to ask the layer for its projection as a WKT string. 
       *
       * Must be reimplemented by each provider. 
       *
       * @note XXXXX WARNING THIS METHOD WILL BE DEPRECATED
       *       XXXXX in favour of SpatialRefSys accessors
       *       XXXXX and mutators!
       *
       */
      virtual QString getProjectionWKT()  = 0 ;

      /** 
       * Set the data source specification. This may be a path or database
       * connection string
       * @param data source specification
       */
      virtual void setDataSourceUri(QString uri) = 0;

      /** 
       * Get the data source specification. This may be a path or database
       * connection string
       * @return data source specification
       */
      virtual QString getDataSourceUri() = 0;

      virtual QgsDataSourceURI * getURI()=0;
      /**
       * Get the extent of the layer
       * @return QgsRect containing the extent of the layer
       */
      virtual QgsRect * extent() = 0;

      /**
       * Identify features within the search radius specified by rect
       * @param rect Bounding rectangle of search radius
       * @return std::vector containing QgsFeature objects that intersect rect
       */
      //virtual std::vector<QgsFeature>& QgsDataProvider::identify(QgsRect *rect)=0;

      /** type for byte order

        XDR is for network byte order, or big-endian
        NDR is for little-endian systems

        @note that default values were taken from similarly named WKB types
        */
      typedef enum
      {
        XDR = 0,                    // network byte order (big-endian)
        NDR = 1                     // little endian
      } endian_t;

      /**
        Return the endian of this layer.

        XDR for network, or big-endian, byte order
        NDR for little-endian byte order

        @note 

        By default this returns the endian-ness of the current platform.
        Sub-classes are free to over-ride this to perhaps return endian-ness of
        data as stored persistently instead of local hardware architecture
        endian-ness.
        */
      virtual endian_t endian()
      {
        return (htonl(1) == 1) ? XDR : NDR;
      }

      /**
       * Returns true if this is a valid layer. It is up to individual providers
       * to determine what constitutes a valid layer
       */
      virtual bool isValid()=0;

      /* Reset the layer - for an OGRLayer, this means clearing the
       * spatial filter and calling ResetReading
       */
      virtual void reset()
      { 
        // NOP by default 
      }

      /**
       * Update the extents of the layer. Not implemented by default
       */
      virtual void updateExtents()
      {
        // NOP by default
      }
      /**
       * Set the subset string used to create a subset of features in
       * the layer. This may be a sql where clause or any other string
       * that can be used by the data provider to create a subset.
       * Must be implemented in the dataprovider.
       */
      virtual void setSubsetString(QString subset)
      {
        // NOP by default
      }
      /**
       * Returns the subset definition string (typically sql) currently in
       * use by the layer and used by the provider to limit the feature set.
       * Must be overridden in the dataprovider, otherwise returns a null
       * QString.
       */
      virtual QString subsetString()
      {
        return QString::null;
      }
      
      /**
       * Sub-layers handled by this provider, in order from bottom to top
       *
       * Sub-layers are used when the provider's source can combine layers
       * it knows about in some way before it hands them off to the provider.
       */
      virtual QStringList subLayers()
      {
        return QStringList();  // Empty
      }
    
    
      /**
       * Reorder the list of layer names to be rendered by this provider
       * (in order from bottom to top)
       * \note   layers must have been previously added.
       */
      virtual void setLayerOrder(QStringList layers)
      {
        // NOOP
      }
    
      /**
       * Set the visibility of the given sublayer name
       */
      virtual void setSubLayerVisibility(QString name, bool vis)
      {
        // NOOP
      }


signals:

      /** 
       *   This is emitted whenever the worker thread has fully calculated the
       *   PostGIS extents for this layer, and its event has been received by this
       *   provider.
       */  
      virtual void fullExtentCalculated();

};


#endif

