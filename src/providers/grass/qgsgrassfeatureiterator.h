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


class QgsGrassProvider;

class QgsGrassFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsGrassFeatureIterator( QgsGrassProvider* p, const QgsFeatureRequest& request );

    ~QgsGrassFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsGrassProvider* P;

    // create QgsFeatureId from GRASS geometry object id and cat
    static QgsFeatureId makeFeatureId( int grassId, int cat );

    void setSelectionRect( const QgsRectangle& rect, bool useIntersect );

    void setFeatureGeometry( QgsFeature& feature, int id, int type );

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
};

#endif // QGSGRASSFEATUREITERATOR_H
