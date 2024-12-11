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

#include <QObject>
#include <QPointer>
#include "qgis_app.h"

class QgisInterface;
class QgsIdentifyResultsDialog;
class QgsMapLayer;
class QgsMapToolSelectionHandler;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsFeatureStore;
class QgsMapLayerActionContext;

/**
 * \brief Map tool for identifying features layers and showing results
 *
 * after selecting a point shows dialog with identification results
 *
 * - for raster layers shows value of underlying pixel
 * - for vector layers shows feature attributes within search radius
 *   (allows editing values when vector layer is in editing mode)
*/
class APP_EXPORT QgsMapToolIdentifyAction : public QgsMapToolIdentify
{
    Q_OBJECT

  public:
    QgsMapToolIdentifyAction( QgsMapCanvas *canvas );

    ~QgsMapToolIdentifyAction() override;

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;
    void activate() override;

    void deactivate() override;

    /**
     * Triggers map identification at the given location and outputs results in GUI
     * \param geom The geometry to use for identification
     * \param properties Sets overridden properties for this identification, like search radius
     */
    void identifyAndShowResults( const QgsGeometry &geom, IdentifyProperties properties );
    //! Clears any previous results from the GUI
    void clearResults();
    //! Looks up feature by its ID and outputs the result in GUI
    void showResultsForFeature( QgsVectorLayer *vlayer, QgsFeatureId fid, const QgsPoint &pt );

    /**
     * Shows identification results in the GUI
     * \since QGIS 3.18
     */
    void showIdentifyResults( const QList<IdentifyResult> &identifyResults );

    /**
     * Returns a pointer to the identify results dialog for name/value pairs
     * \since QGIS 3.42
     */
    QgsIdentifyResultsDialog *resultsDialog();

  public slots:
    void handleCopyToClipboard( QgsFeatureStore & );
    void handleChangedRasterResults( QList<QgsMapToolIdentify::IdentifyResult> &results );

  signals:

    void copyToClipboard( QgsFeatureStore & );

  private slots:
    void showAttributeTable( QgsMapLayer *layer, const QList<QgsFeature> &featureList, const QgsMapLayerActionContext &context );

    void identifyFromGeometry();

  private:
    //! Pointer to the identify results dialog for name/value pairs
    QPointer<QgsIdentifyResultsDialog> mResultsDialog;

    QgsMapToolSelectionHandler *mSelectionHandler = nullptr;
    bool mShowExtendedMenu = false;


    Qgis::DistanceUnit displayDistanceUnits() const override;
    Qgis::AreaUnit displayAreaUnits() const override;
    void setClickContextScope( const QgsPointXY &point );

    friend class TestQgsIdentify;
};

#endif
