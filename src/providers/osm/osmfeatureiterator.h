/***************************************************************************
    osmfeatureiterator.h
    ---------------------
    begin                : Januar 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef OSMFEATUREITERATOR_H
#define OSMFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <sqlite3.h>


class QgsOSMDataProvider;

class QgsOSMFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsOSMFeatureIterator( QgsOSMDataProvider* p, const QgsFeatureRequest& request );

    ~QgsOSMFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:

    /**
     * Function fetches one node from current sqlite3 statement.
     * @param feature output; feature representing fetched node
     * @return success of failure flag (true/false)
     */
    bool fetchNode( QgsFeature& feature );

    /**
     * Function fetches one way from current sqlite3 statement.
     * @param feature output; feature representing fetched way
     * @return success of failure flag (true/false)
     */
    bool fetchWay( QgsFeature& feature );


    /**
     * Function returns string of concatenated tags of specified feature.
     * @param isNode true for node, false for way
     * @param id feature identifier
     * @return string of tags concatenation
     */
    QString tagsForObject( bool isNode, int id );

    /**
     * Function returns one tag value of specified feature and specified key.
     * @param isNode true for node, false for way
     * @param id feature identifier
     * @param tagKey tag key
     * @return tag value
     */
    QString tagForObject( bool isNode, int id, QString tagKey );

    /**
     * Add attributes to the feature from the current row
     * @param feature
     * @param isNode true for node, false for way
     */
    void fetchAttributes( QgsFeature& feature, bool isNode );

  protected:
    QgsOSMDataProvider* P;

    //! sqlite3 database statement for selection
    sqlite3_stmt *mSelectStmt;

    //! sqlite3 database statement ready to select all feature tags
    sqlite3_stmt *mTagsStmt;

    //! sqlite3 database statement ready to select concrete feature tag
    sqlite3_stmt *mCustomTagsStmt;

    //! geometry for exact intersection test
    QgsGeometry* mRectGeom;

};



#endif // OSMFEATUREITERATOR_H
