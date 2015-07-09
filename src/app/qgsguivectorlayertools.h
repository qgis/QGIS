/***************************************************************************
    qgsguivectorlayertools.h
     --------------------------------------
    Date                 : 30.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
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

class QgsGuiVectorLayerTools : public QgsVectorLayerTools, public QObject
{
  public:
    QgsGuiVectorLayerTools();

    /**
     * This method should be called, whenever a new feature is added to a layer
     *
     * @param layer           The layer to which the feature should be added
     * @param defaultValues   Default values for the feature to add
     * @param defaultGeometry A default geometry to add to the feature
     *
     * @return                True in case of success, False if the operation failed/was aborted
     */
    bool addFeature( QgsVectorLayer *layer, QgsAttributeMap defaultValues, const QgsGeometry &defaultGeometry ) const override;

    /**
     * This should be called, whenever a vector layer should be switched to edit mode. If successful
     * the layer is switched to editable and an edit sessions started.
     *
     * @param layer  The layer on which to start an edit session
     *
     * @return       True, if the editing session was started
     */
    bool startEditing( QgsVectorLayer* layer ) const override;

    /**
     * Should be called, when an editing session is ended and the features should be commited.
     * An appropriate dialog asking the user if he wants to save the edits will be shown if
     * allowCancel is set to true.
     *
     * @param layer       The layer to commit
     * @param allowCancel True if a cancel button should be offered
     *
     * @return            True if successful
     */
    bool stopEditing( QgsVectorLayer* layer, bool allowCancel = true ) const override;

    /**
     * Should be called, when the features should be commited but the editing session is not ended.
     *
     * @param layer       The layer to commit
     * @return            True if successful
     */
    bool saveEdits( QgsVectorLayer* layer ) const override;

  private:
    void commitError( QgsVectorLayer* vlayer ) const;
};

#endif // QGSGUIVECTORLAYERTOOLS_H
