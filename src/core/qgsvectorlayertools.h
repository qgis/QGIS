/***************************************************************************
    qgsvectorlayertools.h
     --------------------------------------
    Date                 : 29.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERTOOLS_H
#define QGSVECTORLAYERTOOLS_H

#include "qgis_core.h"
#include "qgis.h"
#include <QObject>

#include "qgsfeature.h"
#include "qgsgeometry.h"

class QgsFeatureRequest;
class QgsVectorLayer;

/**
 * \ingroup core
 * Methods in this class are used to handle basic operations on vector layers.
 * With an implementation of this class, parts of the application can ask for
 * an operation to be done and the implementation will then take care of it.
 *
 * Reimplement this class, if you need to have custom checks or GUI elements
 * in your application.
 *
 */
class CORE_EXPORT QgsVectorLayerTools : public QObject
{
    Q_OBJECT

  public:
    QgsVectorLayerTools();

    /**
     * This method should/will be called, whenever a new feature will be added to the layer
     *
     * \param layer           The layer to which the feature should be added
     * \param defaultValues   Default values for the feature to add
     * \param defaultGeometry A default geometry to add to the feature
     * \param feature         Updated feature after adding will be written back to this
     * \returns                True in case of success, False if the operation failed/was aborted
     *
     * TODO QGIS 3: remove const qualifier
     */
    virtual bool addFeature( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues = QgsAttributeMap(), const QgsGeometry &defaultGeometry = QgsGeometry(), QgsFeature *feature SIP_OUT = nullptr ) const = 0;

    /**
     * This will be called, whenever a vector layer should be switched to edit mode. Check the providers
     * capability to edit in here.
     * If successful layer->startEditing() will be called and true returned.
     *
     * \param layer  The layer on which to start an edit session
     *
     * \returns       True, if the editing session was started
     *
     * TODO QGIS 3: remove const qualifier
     */
    virtual bool startEditing( QgsVectorLayer *layer ) const = 0;

    /**
     * Will be called, when an editing session is ended and the features should be committed.
     * Appropriate dialogs should be shown like
     *
     * \param layer       The layer to commit
     * \param allowCancel True if a cancel button should be offered
     * \returns            True if successful
     *
     * TODO QGIS 3: remove const qualifier
     */
    virtual bool stopEditing( QgsVectorLayer *layer, bool allowCancel = true ) const = 0;

    /**
     * Should be called, when the features should be committed but the editing session is not ended.
     *
     * \param layer       The layer to commit
     * \returns            True if successful
     *
     * TODO QGIS 3: remove const qualifier
     */
    virtual bool saveEdits( QgsVectorLayer *layer ) const = 0;

    /**
     * Copy and move features with defined translation.
     *
     * \param layer The layer
     * \param request The request for the features to be moved. It will be assigned to a new feature request with the newly copied features.
     * \param dx The translation on x
     * \param dy The translation on y
     * \param errorMsg If given, it will contain the error message
     * \returns True if all features could be copied.
     *
     * TODO QGIS 3: remove const qualifier
     */
    virtual bool copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request SIP_INOUT, double dx = 0, double dy = 0, QString *errorMsg SIP_OUT = nullptr ) const;

};

#endif // QGSVECTORLAYERTOOLS_H
