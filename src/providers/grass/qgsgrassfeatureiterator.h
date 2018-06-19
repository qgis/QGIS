/***************************************************************************
    qgsgrassfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSFEATUREITERATOR_H
#define QGSGRASSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QMutex>
#include <QBitArray>

#include "qgsgrassprovider.h"
//class QgsGrassProvider;
class QgsGrassVectorMapLayer;

class GRASS_LIB_EXPORT QgsGrassFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsGrassFeatureSource( const QgsGrassProvider *provider );
    ~QgsGrassFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
#if 0
    enum Selection
    {

      NotSelected = 0, //!< Not selected
      Selected = 1, //!< Line/area selected
      Used = 2 /*!< the line was already used to create feature read in this cycle.
                * The codes Used must be reset to Selected if getFirstFeature() or select() is called.
                * Distinction between Selected and Used is used if attribute table exists, in which case
                * attributes are read from the table and line geometry is attached to attributes and selection
                * for that line is set to Used. In the end the selection is scanned for Selected (attributes missing)
                * and the geometry is returned without attributes. */
    };
#endif


    struct Map_info *map();
    QgsGrassVectorMapLayer *mLayer = nullptr;
    int mLayerType;     // layer type POINT, LINE, ...
    int mGrassType;     // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA,

    QgsWkbTypes::Type mQgisType; // WKBPoint, WKBLineString, ...

    QgsFields mFields;
    QTextCodec *mEncoding = nullptr;

    bool mEditing; // Standard QGIS editing mode

    int mSymbolAttributeIndex;

    friend class QgsGrassFeatureIterator;
};


class GRASS_LIB_EXPORT QgsGrassFeatureIterator : public QObject, public QgsAbstractFeatureIteratorFromSource<QgsGrassFeatureSource>
{
    Q_OBJECT
  public:
    QgsGrassFeatureIterator( QgsGrassFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsGrassFeatureIterator() override;

    bool fetchFeature( QgsFeature &feature ) override;
    bool rewind() override;
    bool close() override;

    /**
     * Gets GRASS line id from a QGIS \a fid.
     */
    static int lidFromFid( QgsFeatureId fid );

    /**
     * Gets GRASS cat from QGIS \a fid.
     */
    static int catFromFid( QgsFeatureId fid );

    /**
     * Gets layer number from QGIS \a fid.
     */
    static int layerFromFid( QgsFeatureId fid );

    /**
     * Gets attribute value to be used in different layer when it is edited.
     */
    static QVariant nonEditableValue( int layerNumber );

  public slots:

    /**
     * Cancel iterator, iterator will be closed on next occasion, probably when next getFeature() gets called.
     * This function can be called directly from other threads (setting bool is atomic) */
    void cancel();

    void doClose();

  private:

    // create QgsFeatureId from GRASS geometry object id, cat and layer number (editing)
    static QgsFeatureId makeFeatureId( int grassId, int cat, int layer = 0 );

    //void lock();
    //void unlock();

    //! Reset selection
    void resetSelection( bool value );

    void setSelectionRect( const QgsRectangle &rect, bool useIntersect );

    void setFeatureGeometry( QgsFeature &feature, int id, int type );

    /**
     * Set feature attributes.
     *  \param feature
     *  \param cat category number
     */
    void setFeatureAttributes( int cat, QgsFeature *feature, QgsGrassVectorMap::TopoSymbol symbol );

    /**
     * Set feature attributes.
     *  \param feature
     *  \param cat category number
     *  \param attlist a list containing the index number of the fields to set
     */
    void setFeatureAttributes( int cat, QgsFeature *feature, const QgsAttributeList &attlist, QgsGrassVectorMap::TopoSymbol symbol );

    //! Canceled -> close when possible
    bool mCanceled = false;

    //! Selection array
    QBitArray mSelection; // !UPDATE!

    // Edit mode is using mNextLid + mNextCidx
    // Next index in cidxFieldIndex to be read in standard mode or next index of line Cats in editing mode
    int mNextCidx = 0;
    // Next topology line/node id to be read in topo mode or next line id in edit mode, starts from 1
    int mNextLid = 1;

    static QMutex sMutex;
};

#endif // QGSGRASSFEATUREITERATOR_H
