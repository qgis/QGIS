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
    QgsPointCloudLayerUndoCommand( QgsPointCloudIndex index );
    QgsPointCloudIndex mIndex;
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
     * \param index associated point cloud index
     * \param n the node id whose points will be modified
     * \param points the list of points to be modified
     * \param attribute the attribute whose value will be modified
     * \param value the new value for the modified attribure
     */
    QgsPointCloudLayerUndoCommandChangeAttribute( QgsPointCloudIndex index, const QgsPointCloudNodeId &n, const QVector<int> &points, const QgsPointCloudAttribute &attribute, double value );

    void undo() override;
    void redo() override;

  private:
    void undoRedoPrivate( bool isUndo );

    QgsPointCloudNodeId mNode;
    QHash< int, double > mPointValues; // contains pairs of (point number, old value)
    QgsPointCloudAttribute mAttribute;
    int mAttributeOffset;
    double mNewValue;
};
#endif // QGSPOINTCLOUDLAYERUNDOCOMMAND_H
