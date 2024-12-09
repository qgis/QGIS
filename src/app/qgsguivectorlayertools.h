/***************************************************************************
    qgsguivectorlayertools.h
     --------------------------------------
    Date                 : 30.5.2013
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

#ifndef QGSGUIVECTORLAYERTOOLS_H
#define QGSGUIVECTORLAYERTOOLS_H

#include "qgsvectorlayertools.h"

/**
 * Implements all the dialogs and actions when editing on a vector layer is toggled
 * or a feature is added.
 */

class QgsGuiVectorLayerTools : public QgsVectorLayerTools
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsGuiVectorLayerTools.
     */
    QgsGuiVectorLayerTools() = default;

    /**
     * This method should be called, whenever a new feature is added to a layer
     *
     * \param layer           The layer to which the feature should be added
     * \param defaultValues   Default values for the feature to add
     * \param defaultGeometry A default geometry to add to the feature
     * \param feature         A pointer to the feature
     * \param context         A context object to be used for e.g. to calculate feature expression-based values (since QGIS 3.38)
     *
     * \returns               TRUE in case of success, FALSE if the operation failed/was aborted
     */
    bool addFeatureV2( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feature, const QgsVectorLayerToolsContext &context ) const override;

    /**
     * This should be called, whenever a vector layer should be switched to edit mode. If successful
     * the layer is switched to editable and an edit sessions started.
     *
     * \param layer  The layer on which to start an edit session
     *
     * \returns       TRUE, if the editing session was started
     */
    bool startEditing( QgsVectorLayer *layer ) const override;

    /**
     * Should be called, when an editing session is ended and the features should be committed.
     * An appropriate dialog asking the user if he wants to save the edits will be shown if
     * allowCancel is set to TRUE.
     *
     * \param layer       The layer to commit
     * \param allowCancel TRUE if a cancel button should be offered
     *
     * \returns            TRUE if successful
     */
    bool stopEditing( QgsVectorLayer *layer, bool allowCancel = true ) const override;

    /**
     * Should be called, when the features should be committed but the editing session is not ended.
     *
     * \param layer       The layer to commit
     * \returns            TRUE if successful
     */
    bool saveEdits( QgsVectorLayer *layer ) const override;

    /**
     * Copy and move features with defined translation.
     *
     * \param layer The layer
     * \param request The request for the features to be moved. It will be assigned to a new feature request with the newly copied features.
     * \param dx The translation on x
     * \param dy The translation on y
     * \param errorMsg If given, it will contain the error message
     * \param topologicalEditing If TRUE, the function will perform topological
     * editing of the vertices of \a layer on \a layer and \a topologicalLayer
     * \param topologicalLayer The layer where vertices from the moved features of \a layer will be added
     * \param childrenInfoMsg If given, it will contain messages related to the creation of child features
     * \returns TRUE if all features could be copied.
     *
     */
    bool copyMoveFeatures( QgsVectorLayer *layer, QgsFeatureRequest &request SIP_INOUT, double dx = 0, double dy = 0, QString *errorMsg SIP_OUT = nullptr, const bool topologicalEditing = false, QgsVectorLayer *topologicalLayer = nullptr, QString *childrenInfoMsg = nullptr ) const override;

  private:
    void commitError( QgsVectorLayer *vlayer ) const;
    bool avoidIntersection( QgsVectorLayer *layer, QgsFeatureRequest &request, QString *errorMsg = nullptr ) const;
};

#endif // QGSGUIVECTORLAYERTOOLS_H
