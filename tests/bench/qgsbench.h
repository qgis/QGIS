/***************************************************************************
                 qgsbench.h  - Benchmark
                             -------------------
    begin                : 2011-11-15
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBENCH_H
#define QGSBENCH_H

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomNode>
#include <QMap>
#include <QPainter>
#include <QString>
#include <QTime>
#include <QVariant>
#include <QVector>

#include "qgsmaprenderer.h"

class QgsBench :  public QObject
{
    Q_OBJECT
  public:
    QgsBench( int theWidth, int theHeight, int theCycles );
    ~QgsBench();

    // start time counter
    void start();

    // save elapsed time since last start() in list and restarts counter
    void elapsed();

    void render();

    void printLog();

    bool openProject( const QString & fileName );

    void setExtent( const QgsRectangle & extent );

    void saveSnapsot( const QString & fileName );

    void saveLog( const QString & fileName );

    QString serialize( QMap<QString, QVariant> theMap, int level = 0 );

    void  setRenderHints( QPainter::RenderHints hints ) { mRendererHints = hints; }

  public slots:
    void readProject( const QDomDocument &doc );

  private:
    // snapshot image width
    int mWidth;

    // snapshot image height
    int mHeight;

    // Number of rendering cycles
    int mIterations;


    QgsRectangle mExtent;
    bool mSetExtent;

    QPainter::RenderHints mRendererHints;

    // log map
    QMap<QString, QVariant> mLogMap;

    double mUserStart;
    double mSysStart;
    QTime mWallTime; // 'wall clock' time

    // user, sys, total times
    QVector<double*> mTimes;

    QImage* mImage;

    QgsMapRenderer *mMapRenderer;
};

#endif // QGSBENCH_H

