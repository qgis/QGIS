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

#include "qgsmaprendererjob.h"
#include "qgsmapsettings.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QDomNode>
#include <QElapsedTimer>
#include <QMap>
#include <QPainter>
#include <QString>
#include <QTime>
#include <QVariant>
#include <QVector>

class QgsBench : public QObject
{
    Q_OBJECT
  public:
    QgsBench( int width, int height, int cycles );

    // start time counter
    void start();

    // save elapsed time since last start() in list and restarts counter
    void elapsed();

    void render();

    void printLog( const QString &printTime );

    bool openProject( const QString &fileName );

    void setExtent( const QgsRectangle &extent );

    void saveSnapsot( const QString &fileName );

    void saveLog( const QString &fileName );

    QString serialize( const QMap<QString, QVariant> &map, int level = 0 );

    void setRenderHints( const QPainter::RenderHints &hints ) { mRendererHints = hints; }

    void setParallel( bool enabled ) { mParallel = enabled; }

  signals:
    void renderingComplete();

  public slots:
    void readProject( const QDomDocument &doc );

  private slots:
    void onRenderingFinished();

  private:
    void startNextRender();
    void finalizeRendering();
    // snapshot image width
    int mWidth;

    // snapshot image height
    int mHeight;

    // Number of rendering cycles
    int mIterations;


    QgsRectangle mExtent;
    bool mSetExtent = false;

    QPainter::RenderHints mRendererHints;

    // log map
    QMap<QString, QVariant> mLogMap;

    double mUserStart = 0.0;
    double mSysStart = 0.0;
    QElapsedTimer mWallTime; // 'wall clock' time

    // user, sys, total times
    QVector<double *> mTimes;

    QImage mImage;

    QgsMapSettings mMapSettings;

    bool mParallel = false;

    // async rendering state
    int mCurrentIteration = 0;
    QgsMapRendererQImageJob *mCurrentJob = nullptr;
};

#endif // QGSBENCH_H
