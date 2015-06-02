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
#include "qgsrasterpipe.h"
#include "qgsvectordataprovider.h"

#include "qgsgrass.h"

class GRASS_LIB_EXPORT QgsGrassImport : public QObject
{
    Q_OBJECT
  public:
    QgsGrassImport( QgsGrassObject grassObject );
    virtual ~QgsGrassImport();
    QgsGrassObject grassObject() const { return mGrassObject; }
    virtual void importInThread();
    virtual bool import() = 0;
    // source description for error message purposes (maybe uri or something similar)
    virtual QString srcDescription() const = 0;
    // get error if import failed
    QString error();
    virtual QStringList names() const;
    bool isCanceled() const;
  public slots:
    void onFinished();
    // TODO: this is not completely kosher, because QgsGrassImport exist on the main thread
    // but import is running in another thread, to do it right, we should have an import object
    // created on another thread, send cancel signal to that object which regularly processes events
    // and thus recieves the signal.
    // Most probably however, it will work correctly, even if read/write the bool wasn't atomic
    void cancel();

  signals:
    // sent when process finished
    void finished( QgsGrassImport *import );

  protected:
    static bool run( QgsGrassImport *imp );
    void setError( QString error );
    QgsGrassObject mGrassObject;
    QString mError;
    bool mCanceled;
    QFutureWatcher<bool>* mFutureWatcher;
};

class GRASS_LIB_EXPORT QgsGrassRasterImport : public QgsGrassImport
{
    Q_OBJECT
  public:
    // takes pipe ownership
    QgsGrassRasterImport( QgsRasterPipe* pipe, const QgsGrassObject& grassObject,
                          const QgsRectangle &extent, int xSize, int ySize );
    ~QgsGrassRasterImport();
    bool import() override;
    QString srcDescription() const override;
    // get list of extensions (for bands)
    static QStringList extensions( QgsRasterDataProvider* provider );
    // get list of all output names (basename + extension for each band)
    QStringList names() const override;

  private:
    QgsRasterPipe* mPipe;
    QgsRectangle mExtent;
    int mXSize;
    int mYSize;
};

class GRASS_LIB_EXPORT QgsGrassVectorImport : public QgsGrassImport
{
    Q_OBJECT
  public:
    // takes provider ownership
    QgsGrassVectorImport( QgsVectorDataProvider* provider, const QgsGrassObject& grassObject );
    ~QgsGrassVectorImport();
    bool import() override;
    QString srcDescription() const override;

  private:
    QgsVectorDataProvider* mProvider;
};

class GRASS_LIB_EXPORT QgsGrassCopy : public QgsGrassImport
{
    Q_OBJECT
  public:
    // takes provider ownership
    QgsGrassCopy( const QgsGrassObject& srcObject, const QgsGrassObject& destObject );
    ~QgsGrassCopy();
    bool import() override;
    QString srcDescription() const override;

  private:
    QgsGrassObject mSrcObject;

};

// Creates link to GDAL data source with r.external
class GRASS_LIB_EXPORT QgsGrassExternal : public QgsGrassImport
{
    Q_OBJECT
  public:
    // takes provider ownership
    QgsGrassExternal( const QString& gdalSource, const QgsGrassObject& destObject );
    ~QgsGrassExternal();
    bool import() override;
    QString srcDescription() const override;

  private:
    QString mSource;

};

#endif // QGSGRASSIMPORT_H
