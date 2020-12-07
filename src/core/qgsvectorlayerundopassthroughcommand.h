/***************************************************************************
    qgsvectorlayerundopassthroughcommand.h
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Vincent Mora
    email                : vincent dot mora at osalndia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERUNDOPASSTHROUGHCOMMAND_H
#define QGSVECTORLAYERUNDOPASSTHROUGHCOMMAND_H

#include "qgsvectorlayerundocommand.h"

#include "qgsvectorlayereditbuffer.h"

class QgsTransaction;

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommand
 * \brief Undo command for vector layer in transaction group mode.
 * \since QGIS 3.0
 */


class CORE_EXPORT QgsVectorLayerUndoPassthroughCommand : public QgsVectorLayerUndoCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommand
     * \param buffer associated edit buffer
     * \param text text associated with command
     * \param autocreate flag allowing to automatically create a savepoint if necessary
     */
    QgsVectorLayerUndoPassthroughCommand( QgsVectorLayerEditBuffer *buffer, const QString &text, bool autocreate = true );

    /**
     * Returns error status
     */
    bool hasError() const { return mHasError; }

  protected:

    /**
     * Rollback command, release savepoint or set error status
     * save point must be set prior to call
     * error satus should be FALSE prior to call
     */
    bool rollBackToSavePoint();

    /**
     * Set the command savepoint or set error status.
     * Error satus should be FALSE prior to call. If the savepoint given in
     * parameter is empty, then a new one is created if none is currently
     * available in the transaction.
     */
    bool setSavePoint( const QString &savePointId = QString() );

    /**
     * Set error flag and append "failed" to text
     */
    void setError();

    /**
     * Sets the error message.
     *
     * \since QGIS 3.0
     */
    void setErrorMessage( const QString &errorMessage );

    /**
     * Returns the error message or an empty string if there's none.
     *
     * \since QGIS 3.0
     */
    QString errorMessage() const;

  private:
    QString mError;
    QString mSavePointId;
    bool mHasError;
    bool mRecreateSavePoint;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandAddFeatures
 * \brief Undo command for adding a feature to a vector layer in transaction group mode.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandAddFeatures : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandAddFeatures
     * \param buffer associated edit buffer
     * \param features features to add to layer
     */
    QgsVectorLayerUndoPassthroughCommandAddFeatures( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureList &features );

    void undo() override;
    void redo() override;

    /**
     * List of features (added feaures can be modified by default values from database)
     */
    QgsFeatureList features() const { return mFeatures; }

  private:
    QgsFeatureList mFeatures;
    QgsFeatureList mInitialFeatures;
};


/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandDeleteFeatures
 * \brief Undo command for deleting features from a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandDeleteFeatures : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandDeleteFeatures
     * \param buffer associated edit buffer
     * \param fids feature IDs of features to delete from layer
     */
    QgsVectorLayerUndoPassthroughCommandDeleteFeatures( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, const QgsFeatureIds &fids );

    void undo() override;
    void redo() override;

  private:
    const QgsFeatureIds mFids;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandChangeGeometry
 * \brief Undo command for changing feature geometry from a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandChangeGeometry : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandChangeGeometry
     * \param buffer associated edit buffer
     * \param fid feature ID of feature to change
     * \param geom new geometru
     */
    QgsVectorLayerUndoPassthroughCommandChangeGeometry( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid, const QgsGeometry &geom );

    void undo() override;
    void redo() override;

  private:
    QgsFeatureId mFid;
    const QgsGeometry mNewGeom;
    const QgsGeometry mOldGeom;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandChangeAttribute
 * \brief Undo command for changing attr value from a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandChangeAttribute: public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandChangeAttribute
     * \param buffer associated edit buffer
     * \param fid feature ID of feature
     * \param field
     * \param newValue
     */
    QgsVectorLayerUndoPassthroughCommandChangeAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid, int field, const QVariant &newValue );

    void undo() override;
    void redo() override;

  private:
    QgsFeatureId mFid;
    const int mField;
    const QVariant mNewValue;
    const QVariant mOldValue;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandChangeAttributes
 * \brief Undo command for changing attributes' values from a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandChangeAttributes: public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandChangeAttributes
     * \param buffer associated edit buffer
     * \param fid feature ID of feature
     * \param newValues New values for attributes
     * \param oldValues Old values for attributes
     */
    QgsVectorLayerUndoPassthroughCommandChangeAttributes( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsFeatureId fid, const QgsAttributeMap &newValues, const QgsAttributeMap &oldValues = QgsAttributeMap() );

    void undo() override;
    void redo() override;

  private:
    QgsFeatureId mFid;
    const QgsAttributeMap mNewValues;
    const QgsAttributeMap mOldValues;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandAddAttribute
 * \brief Undo command for adding attri to a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandAddAttribute : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoPassthroughCommandAddAttribute
     * \param buffer associated edit buffer
     * \param field
     */
    QgsVectorLayerUndoPassthroughCommandAddAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, const QgsField &field );

    void undo() override;
    void redo() override;

  private:
    const QgsField mField;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandDeleteAttribute
 * \brief Undo command for deleting attri of a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandDeleteAttribute : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandDeleteAttribute
     * \param buffer associated edit buffer
     * \param attr
     */
    QgsVectorLayerUndoPassthroughCommandDeleteAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, int attr );

    void undo() override;
    void redo() override;

  private:
    const QgsField mField;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandRenameAttribute
 * \brief Undo command for deleting attri of a vector layer in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandRenameAttribute : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandRenameAttribute
     * \param buffer associated edit buffer
     * \param attr
     * \param newName
     */
    QgsVectorLayerUndoPassthroughCommandRenameAttribute( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, int attr, const QString &newName );

    void undo() override;
    void redo() override;

  private:
    const int mAttr;
    const QString mNewName;
    const QString mOldName;
};

/**
 * \ingroup core
 * \class QgsVectorLayerUndoPassthroughCommandUpdate
 * \brief Undo command for running a specific sql query in transaction group.
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsVectorLayerUndoPassthroughCommandUpdate : public QgsVectorLayerUndoPassthroughCommand
{
  public:

    /**
     * Constructor for QgsVectorLayerUndoCommandUpdate
     * \param buffer associated edit buffer
     * \param transaction transaction running the sql query
     * \param sql the query
     * \param name The name of the command
     */
    QgsVectorLayerUndoPassthroughCommandUpdate( QgsVectorLayerEditBuffer *buffer SIP_TRANSFER, QgsTransaction *transaction, const QString &sql, const QString &name );

    void undo() override;
    void redo() override;

  private:
    QgsTransaction *mTransaction = nullptr;
    QString mSql;
    bool mUndone = false;
};

#endif
