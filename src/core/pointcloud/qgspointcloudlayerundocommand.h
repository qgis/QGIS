/***************************************************************************
    qgspointcloudlayerundocommand.h
    ---------------------
    begin                : January 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYERUNDOCOMMAND_H
#define QGSPOINTCLOUDLAYERUNDOCOMMAND_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudattribute.h"

#include <QUndoCommand>

class QgsPointCloudLayer;

/**
 * \ingroup core
 *
 * \brief Base class for undo/redo command for point cloud editing
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudLayerUndoCommand : public QUndoCommand
{
  protected:
    //! Ctor
    QgsPointCloudLayerUndoCommand( QgsPointCloudLayer *layer );
    QgsPointCloudLayer *mLayer;
};

/**
 * \ingroup core
 *
 * \brief An undo command subclass for changing point attribute values in a point cloud index
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudLayerUndoCommandChangeAttribute : public QgsPointCloudLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsPointCloudLayerUndoCommandChangeAttribute
     * \param layer associated point cloud layer
     * \param nodesAndPoints affected nodes, each with a list of points to be modified
     * \param attribute the attribute whose value will be modified
     * \param value the new value for the modified attribure
     */
    QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudLayer *layer, const QHash<QgsPointCloudNodeId, QVector<int>> &nodesAndPoints, const QgsPointCloudAttribute &attribute, double value );

    void undo() override;
    void redo() override;

  private:
    struct PerNodeData
    {
      QHash<int, double> oldPointValues;
      bool firstEdit = false;
      int attributeOffset = 0;
    };

    void undoRedoPrivate( bool isUndo );

    QHash<QgsPointCloudNodeId, PerNodeData> mPerNodeData;
    QgsPointCloudAttribute mAttribute;
    double mNewValue = 0;
};
#endif // QGSPOINTCLOUDLAYERUNDOCOMMAND_H
