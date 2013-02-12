/***************************************************************************
  qgsoraclefeatureiterator.h -  QGIS data provider for Oracle layers
                             -------------------
    begin                : December 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLEFEATUREITERATOR_H
#define QGSORACLEFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QSqlQuery>


class QgsOracleProvider;

class QgsOracleFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsOracleFeatureIterator( QgsOracleProvider *p, const QgsFeatureRequest &request );

    ~QgsOracleFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsOracleProvider *P;

    bool openQuery( QString whereClause );

    bool getFeature( QgsFeature &feature );

    QSqlQuery mQry;
    bool mRewind;
    QgsAttributeList mAttributeList;
};

#endif // QGSORACLEFEATUREITERATOR_H
