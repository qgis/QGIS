/***************************************************************************
      qgsafsfeatureiterator.h
      -----------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAFSFEATUREITERATOR_H
#define QGSAFSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

class QgsAfsProvider;
class QgsSpatialIndex;


class QgsAfsFeatureSource : public QObject, public QgsAbstractFeatureSource
{
    Q_OBJECT

  public:
    QgsAfsFeatureSource( const QgsAfsProvider* provider );
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;
    QgsAfsProvider* provider() const;

  signals:
    void extentRequested( const QgsRectangle & );

  protected:
    QgsAfsProvider* mProvider;

    friend class QgsAfsFeatureIterator;
};

class QgsAfsFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsAfsFeatureSource>
{
  public:
    QgsAfsFeatureIterator( QgsAfsFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );
    ~QgsAfsFeatureIterator();
    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature& f ) override;

  private:
    QgsFeatureId mFeatureIterator;
};

#endif // QGSAFSFEATUREITERATOR_H
