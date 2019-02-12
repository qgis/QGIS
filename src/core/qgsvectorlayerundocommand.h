/***************************************************************************
    qgsvectorlayerundocommand.h
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERUNDOCOMMAND_H
#define QGSVECTORLAYERUNDOCOMMAND_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QUndoCommand>

#include <QVariant>
#include <QSet>
#include <QList>

#include "qgsfields.h"
#include "qgsfeature.h"

class QgsGeometry;

#include "qgsvectorlayereditbuffer.h"

/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommand
 * \brief Base class for undo commands within a QgsVectorLayerEditBuffer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommand : public QUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommand
     * \param buffer associated edit buffer
     */
    QgsVectorLayerUndoCommand( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER )
      : mBuffer( buffer )
    {}

    //! Returns the layer associated with the undo command
    inline QgsVectorLayer *layer() { return mBuffer->L; }

    int id() const override { return -1; }
    bool mergeWith( const QUndoCommand * ) override { return false; }

  protected:
    //! Associated edit buffer
    QgsVectorLayerEditBuffer *mBuffer = nullptr;
};


/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandAddFeature
 * \brief Undo command for adding a feature to a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandAddFeature : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandAddFeature
     * \param buffer associated edit buffer
     * \param f feature to add to layer
     */
    QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeature &f );

    void undo() override;
    void redo() override;

  private:
    QgsFeature mFeature;
};


/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandDeleteFeature
 * \brief Undo command for deleting a feature from a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandDeleteFeature : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandDeleteFeature
     * \param buffer associated edit buffer
     * \param fid feature ID of feature to delete from layer
     */
    QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid );

    void undo() override;
    void redo() override;

  private:
    QgsFeatureId mFid;
    QgsFeature mOldAddedFeature;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandChangeGeometry
 * \brief Undo command for modifying the geometry of a feature from a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandChangeGeometry : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandChangeGeometry
     * \param buffer associated edit buffer
     * \param fid feature ID of feature to modify geometry of
     * \param newGeom new geometry for feature
     */
    QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid, const QgsGeometry &newGeom );

    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith( const QUndoCommand * ) override;

  private:
    QgsFeatureId mFid;
    QgsGeometry mOldGeom;
    mutable QgsGeometry mNewGeom;
};


/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandChangeAttribute
 * \brief Undo command for modifying an attribute of a feature from a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandChangeAttribute : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandChangeAttribute
     * \param buffer associated edit buffer
     * \param fid feature ID of feature to modify
     * \param fieldIndex index of field to modify
     * \param newValue new value of attribute
     * \param oldValue previous value of attribute
     */
    QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid, int fieldIndex, const QVariant &newValue, const QVariant &oldValue );
    void undo() override;
    void redo() override;

  private:
    QgsFeatureId mFid;
    int mFieldIndex;
    QVariant mOldValue;
    QVariant mNewValue;
    bool mFirstChange;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandAddAttribute
 * \brief Undo command for adding a new attribute to a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandAddAttribute : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandAddAttribute
     * \param buffer associated edit buffer
     * \param field definition of new field to add
     */
    QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, const QgsField &field );

    void undo() override;
    void redo() override;

  private:
    QgsField mField;
    int mFieldIndex;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandDeleteAttribute
 * \brief Undo command for removing an existing attribute from a vector layer.
 */

class CORE_EXPORT QgsVectorLayerUndoCommandDeleteAttribute : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandDeleteAttribute
     * \param buffer associated edit buffer
     * \param fieldIndex index of field to delete
     */
    QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, int fieldIndex );

    void undo() override;
    void redo() override;

  private:
    int mFieldIndex;
    QString mFieldName;
    bool mProviderField;
    int mOriginIndex;
    QgsField mOldField;
    QVariantMap mOldEditorWidgetConfig;

    QMap<QgsFeatureId, QVariant> mDeletedValues;
    QString mOldName;
};


/**
 * \ingroup core
 * \class QgsVectorLayerUndoCommandRenameAttribute
 * \brief Undo command for renaming an existing attribute of a vector layer.
 * \since QGIS 2.16
 */

class CORE_EXPORT QgsVectorLayerUndoCommandRenameAttribute : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandRenameAttribute
     * \param buffer associated edit buffer
     * \param fieldIndex index of field to rename
     * \param newName new name for field
     */
    QgsVectorLayerUndoCommandRenameAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, int fieldIndex, const QString &newName );

    void undo() override;
    void redo() override;

  private:
    int mFieldIndex;
    bool mProviderField;
    int mOriginIndex;
    QString mOldName;
    QString mNewName;
};

#endif
