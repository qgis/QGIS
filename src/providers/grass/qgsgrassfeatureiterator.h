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
    QgsGrassFeatureSource( const QgsGrassProvider* provider );
    ~QgsGrassFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
#if 0
    enum Selection
    {

      NotSelected = 0, /*!< not selected */
      Selected = 1, /*!< line/area selected */
      Used = 2 /*!< the line was already used to create feature read in this cycle.
                * The codes Used must be reset to Selected if getFirstFeature() or select() is called.
                * Distinction between Selected and Used is used if attribute table exists, in which case
                * attributes are read from the table and line geometry is attached to attributes and selection
                * for that line is set to Used. In the end the selection is scanned for Selected (attributes missing)
                * and the geometry is returned without attributes. */
    };
#endif


    struct Map_info* map();
    QgsGrassVectorMapLayer * mLayer;
    int mLayerType;     // layer type POINT, LINE, ...
    int mGrassType;     // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA,

    QGis::WkbType mQgisType; // WKBPoint, WKBLineString, ...

    QgsFields mFields;
    QTextCodec* mEncoding;

    bool mEditing; // Standard QGIS editing mode

    int mSymbolAttributeIndex;

    friend class QgsGrassFeatureIterator;
};


class GRASS_LIB_EXPORT QgsGrassFeatureIterator : public QObject, public QgsAbstractFeatureIteratorFromSource<QgsGrassFeatureSource>
{
    Q_OBJECT
  public:
    QgsGrassFeatureIterator( QgsGrassFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsGrassFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

    // create QgsFeatureId from GRASS geometry object id, cat and layer number (editing)
    static QgsFeatureId makeFeatureId( int grassId, int cat, int layer = 0 );

    // Get layer number from QGIS fid
    static int layerFromFid( QgsFeatureId fid );

    // Get GRASS line id from QGIS fid
    static int lidFromFid( QgsFeatureId fid );

    // Get GRASS cat from QGIS fid
    static int catFromFid( QgsFeatureId fid );

    // get attribute value to be used in different layer when it is edited
    static QVariant nonEditableValue( int layerNumber );

  public slots:
    /** Cancel iterator, iterator will be closed on next occasion, probably when next getFeature() gets called.
     * This function can be called directly from other threads (setting bool is atomic) */
    void cancel();

    void doClose();

  protected:
    //void lock();
    //void unlock();

    /** Reset selection */
    void resetSelection( bool value );

    void setSelectionRect( const QgsRectangle& rect, bool useIntersect );

    void setFeatureGeometry( QgsFeature& feature, int id, int type );

    /** Set feature attributes.
     *  @param feature
     *  @param cat category number
     */
    void setFeatureAttributes( int cat, QgsFeature *feature, QgsGrassVectorMap::TopoSymbol symbol );

    /** Set feature attributes.
     *  @param feature
     *  @param cat category number
     *  @param attlist a list containing the index number of the fields to set
     */
    void setFeatureAttributes( int cat, QgsFeature *feature, const QgsAttributeList & attlist, QgsGrassVectorMap::TopoSymbol symbol );

    /** Canceled -> close when possible */
    bool mCanceled;

    /** Selection array */
    QBitArray mSelection; // !UPDATE!

    // Edit mode is using mNextLid + mNextCidx
    // Next index in cidxFieldIndex to be read in standard mode or next index of line Cats in editing mode
    int mNextCidx;
    // Next topology line/node id to be read in topo mode or next line id in edit mode, starts from 1
    int mNextLid;



    static QMutex sMutex;
};

#endif // QGSGRASSFEATUREITERATOR_H
