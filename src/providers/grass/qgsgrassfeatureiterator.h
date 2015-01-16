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

class QgsGrassProvider;


class QgsGrassFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsGrassFeatureSource( const QgsGrassProvider* provider );
    ~QgsGrassFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    struct Map_info* mMap;
    int     mLayerType;     // layer type POINT, LINE, ...
    int     mGrassType;     // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA,
    int     mLayerId;       // ID used in layers
    QGis::WkbType mQgisType;// WKBPoint, WKBLineString, ...

    int    mCidxFieldIndex;    // !UPDATE! Index for layerField in category index or -1 if no such field
    int    mCidxFieldNumCats;  // !UPDATE! Number of records in field index

    QgsFields mFields;
    QTextCodec* mEncoding;

    friend class QgsGrassFeatureIterator;
};


class QgsGrassFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsGrassFeatureSource>
{
  public:
    QgsGrassFeatureIterator( QgsGrassFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsGrassFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:

    // create QgsFeatureId from GRASS geometry object id and cat
    static QgsFeatureId makeFeatureId( int grassId, int cat );

    void setSelectionRect( const QgsRectangle& rect, bool useIntersect );

    void setFeatureGeometry( QgsFeature& feature, int id, int type );


    /*! Set feature attributes.
     *  @param feature
     *  @param cat category number
     */
    void setFeatureAttributes( int cat, QgsFeature *feature );

    /*! Set feature attributes.
     *  @param feature
     *  @param cat category number
     *  @param attlist a list containing the index number of the fields to set
     */
    void setFeatureAttributes( int cat, QgsFeature *feature, const QgsAttributeList & attlist );

    struct line_pnts *mPoints; // points structure
    struct line_cats *mCats;   // cats structure
    struct ilist     *mList;

    // selection: array of size nlines or nareas + 1, set to 1 - selected or 0 - not selected, 2 - read
    // Code 2 means that the line was already read in this cycle, all 2 must be reset to 1
    // if getFirstFeature() or select() is calles.
    // Distinction between 1 and 2 is used if attribute table exists, in that case attributes are
    // read from the table and geometry is append and selection set to 2.
    // In the end the selection array is scanned for 1 (attributes missing), and the geometry
    // is returned without attributes
    char    *mSelection;           // !UPDATE!
    int     mSelectionSize;        // !UPDATE! Size of selection array

    // Either mNextCidx or mNextTopoId is used according to type
    int    mNextCidx;          // !UPDATE! Next index in cidxFieldIndex to be read, used to find nextFeature
    int    mNextTopoId;          // !UPDATE! Next topology id to be read, used to find nextFeature, starts from 1

    /*! reset selection */
    void resetSelection( bool sel );

    /*! Allocate sellection array for given map id. The array is large enough for lines or areas
     *  (bigger from num lines and num areas)
     *  @param map pointer to map structure
     */
    void allocateSelection( struct Map_info *map );

    //! Mutex that protects GRASS library from parallel access from multiple iterators at once.
    //! The library uses static/global variables in various places when accessing vector data,
    //! making it non-reentrant and thus impossible to use from multiple threads.
    //! (e.g. static LocList in Vect_select_lines_by_box, global BranchBuf in RTreeGetBranches)
    static QMutex sMutex;
};

#endif // QGSGRASSFEATUREITERATOR_H
