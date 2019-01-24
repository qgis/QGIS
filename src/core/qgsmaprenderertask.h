/***************************************************************************
                          qgsmaprenderertask.h
                          -------------------------
    begin                : Apr 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPRENDERERTASK_H
#define QGSMAPRENDERERTASK_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsannotation.h"
#include "qgsannotationmanager.h"
#include "qgsmapsettings.h"
#include "qgsmapdecoration.h"
#include "qgstaskmanager.h"
#include "qgsmaprenderercustompainterjob.h"

#include <QPainter>
class QgsMapRendererCustomPainterJob;

/**
 * \class QgsMapRendererTask
 * \ingroup core
 * QgsTask task which draws a map to an image file or a painter as a background
 * task. This can be used to draw maps without blocking the QGIS interface.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMapRendererTask : public QgsTask
{
    Q_OBJECT

  public:

    //! \brief Error type
    enum ErrorType
    {
      ImageAllocationFail = 1, //!< Image allocation failure
      ImageSaveFail, //!< Image save failure
      ImageUnsupportedFormat //!< Format is unsupported on the platform \since QGIS 3.4
    };

    /**
     * Constructor for QgsMapRendererTask to render a map to an image file.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ),
                        bool forceRaster = false );

    /**
     * Constructor for QgsMapRendererTask to render a map to a QPainter object.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        QPainter *p );

    /**
     * Adds \a annotations to be rendered on the map.
     */
    void addAnnotations( QList< QgsAnnotation * > annotations );

    /**
     * Adds \a decorations to be rendered on the map.
     */
    void addDecorations( const QList<QgsMapDecoration *> &decorations );

    /**
     * Sets whether a world file will be created alongside an image file.
     */
    void setSaveWorldFile( bool save ) { mSaveWorldFile = save; }

    void cancel() override;

  signals:

    /**
     * Emitted when the map rendering is successfully completed.
     */
    void renderingComplete();

    /**
     * Emitted when map rendering failed.
     */
    void errorOccurred( int error );

  protected:

    bool run() override;
    void finished( bool result ) override;

  private:

    QgsMapSettings mMapSettings;

    QMutex mJobMutex;
    std::unique_ptr< QgsMapRendererCustomPainterJob > mJob;

    QPainter *mPainter = nullptr;

    QString mFileName;
    QString mFileFormat;
    bool mForceRaster = false;
    bool mSaveWorldFile = false;

    QList< QgsAnnotation * > mAnnotations;
    QList< QgsMapDecoration * > mDecorations;

    int mError = 0;
};

// clazy:excludeall=qstring-allocations

#endif
