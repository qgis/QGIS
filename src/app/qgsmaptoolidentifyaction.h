/***************************************************************************
    qgsmaptoolidentify.h  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLIDENTIFYACTION_H
#define QGSMAPTOOLIDENTIFYACTION_H

#include "qgis.h"
#include "qgsmaptoolidentify.h"
#include "qgspoint.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfield.h"
#include "qgsdistancearea.h"

#include <QObject>
#include <QPointer>

class QgsIdentifyResultsDialog;
class QgsMapLayer;
class QgsRasterLayer;
class QgsRubberBand;
class QgsVectorLayer;

/**
  \brief Map tool for identifying features layers and showing results

  after selecting a point shows dialog with identification results
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows to edit values when vector layer is in editing mode)
*/
class QgsMapToolIdentifyAction : public QgsMapToolIdentify
{
    Q_OBJECT

  public:
    QgsMapToolIdentifyAction( QgsMapCanvas * canvas );

    ~QgsMapToolIdentifyAction();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    virtual void activate();

    virtual void deactivate();

  public slots:
    void handleCopyToClipboard( QgsFeatureStore & );
    void handleChangedRasterResults( QList<IdentifyResult>& results );

  signals:
    void identifyProgress( int, int );
    void identifyMessage( QString );
    void copyToClipboard( QgsFeatureStore & );

  private:
    //! Pointer to the identify results dialog for name/value pairs
    QPointer<QgsIdentifyResultsDialog> mResultsDialog;

    QgsIdentifyResultsDialog *resultsDialog();

    virtual QGis::UnitType displayUnits();
};

#endif
