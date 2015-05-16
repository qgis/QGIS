/***************************************************************************
    qgsgrassimport.h  -  Import to GRASS mapset
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSIMPORT_H
#define QGSGRASSIMPORT_H

#include <QFutureWatcher>
#include <QObject>

#include "qgslogger.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterpipe.h"

#include "qgsgrass.h"

class GRASS_LIB_EXPORT QgsGrassImport : public QObject
{
    Q_OBJECT
  public:
    QgsGrassImport( QgsGrassObject grassObject );
    virtual ~QgsGrassImport() {}
    QgsGrassObject grassObject() const { return mGrassObject; }
    virtual QString uri() const = 0;
    // get error if import failed
    QString error();
    virtual QStringList names() const = 0;

  signals:
    // sent when process finished
    void finished( QgsGrassImport *import );

  protected:
    void setError( QString error );
    QgsGrassObject mGrassObject;
    QString mError;
};

class GRASS_LIB_EXPORT QgsGrassRasterImport : public QgsGrassImport
{
    Q_OBJECT
  public:
    // takes pipe ownership
    //QgsGrassRasterImport( QgsRasterDataProvider* provider, const QgsGrassObject& grassObject );
    QgsGrassRasterImport( QgsRasterPipe* pipe, const QgsGrassObject& grassObject,
                          const QgsRectangle &extent, int xSize, int ySize );
    ~QgsGrassRasterImport();
    bool import();
    void importInThread();
    QString uri() const override;
    // get list of extensions (for bands)
    static QStringList extensions( QgsRasterDataProvider* provider );
    // get list of all output names (basename + extension for each band)
    QStringList names() const override;
  public slots:
    void onFinished();
  private:
    static bool run( QgsGrassRasterImport *imp );
    //QgsRasterDataProvider* mProvider;
    QgsRasterPipe* mPipe;
    QgsRectangle mExtent;
    int mXSize;
    int mYSize;
    QFutureWatcher<bool>* mFutureWatcher;
};

#endif // QGSGRASSIMPORT_H
