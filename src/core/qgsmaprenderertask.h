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

#include "qgis.h"
#include "qgis_core.h"
#include "qgsannotation.h"
#include "qgsannotationmanager.h"
#include "qgsmapsettings.h"
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
      ImageAllocationFail = 1,  // Image allocation failure
      ImageSaveFail             // Image save failure
    };

    /**
     * Constructor for QgsMapRendererTask to render a map to an image file.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        const QString &fileName,
                        const QString &fileFormat = QString( "PNG" ) );

    /**
     * Constructor for QgsMapRendererTask to render a map to a painter object.
     */
    QgsMapRendererTask( const QgsMapSettings &ms,
                        QPainter *p );

    /**
     * Adds \a annotations to be rendered on the map.
     */
    void addAnnotations( QList< QgsAnnotation * > annotations );

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

    virtual bool run() override;
    virtual void finished( bool result ) override;

  private:

    QgsMapSettings mMapSettings;

    QMutex mJobMutex;
    std::unique_ptr< QgsMapRendererCustomPainterJob > mJob;

    QPainter *mPainter = nullptr;

    QString mFileName;
    QString mFileFormat;

    QList< QgsAnnotation * > mAnnotations;

    int mError = 0;
};

#endif
