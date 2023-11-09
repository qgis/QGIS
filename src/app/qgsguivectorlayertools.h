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
     * \param feat            A pointer to the feature
     * \param parentWidget    The widget calling this function to be passed to the used dialog
     * \param showModal       If the used dialog should be modal or not
     * \param hideParent      If the parent widget should be hidden, when the used dialog is opened
     *
     * \returns                TRUE in case of success, FALSE if the operation failed/was aborted
     */
    bool addFeature( QgsVectorLayer *layer, const QgsAttributeMap &defaultValues, const QgsGeometry &defaultGeometry, QgsFeature *feat = nullptr, QWidget *parentWidget = nullptr, bool showModal = true, bool hideParent = false ) const override;

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

  private:
    void commitError( QgsVectorLayer *vlayer ) const;

};

#endif // QGSGUIVECTORLAYERTOOLS_H
