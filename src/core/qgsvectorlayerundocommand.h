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

#include <QUndoCommand>

#include <QVariant>
#include <QSet>
#include <QList>

#include "qgsfield.h"
#include "qgsfeature.h"

class QgsGeometry;
class QgsGeometryCache;

#include "qgsvectorlayer.h"
#include "qgsvectorlayereditbuffer.h"


class QgsVectorLayerUndoCommand : public QUndoCommand
{
  public:
    QgsVectorLayerUndoCommand( QgsVectorLayerEditBuffer *buffer )
        : QUndoCommand()
        , mBuffer( buffer )
    {}
    inline QgsVectorLayer *layer() { return mBuffer->L; }
    inline QgsGeometryCache *cache() { return mBuffer->L->cache(); }

    virtual int id() const { return -1; }
    virtual bool mergeWith( QUndoCommand * ) { return -1; }

  protected:
    QgsVectorLayerEditBuffer* mBuffer;
};


class QgsVectorLayerUndoCommandAddFeature : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer* buffer, QgsFeature& f );

    virtual void undo();
    virtual void redo();

  private:
    QgsFeature mFeature;
};


class QgsVectorLayerUndoCommandDeleteFeature : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid );

    virtual void undo();
    virtual void redo();

  private:
    QgsFeatureId mFid;
    QgsFeature mOldAddedFeature;
};


class QgsVectorLayerUndoCommandChangeGeometry : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid, QgsGeometry* newGeom );
    ~QgsVectorLayerUndoCommandChangeGeometry();

    virtual void undo();
    virtual void redo();
    virtual int id() const;
    virtual bool mergeWith( const QUndoCommand * );

  private:
    QgsFeatureId mFid;
    QgsGeometry* mOldGeom;
    mutable QgsGeometry* mNewGeom;
};


class QgsVectorLayerUndoCommandChangeAttribute : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer* buffer, QgsFeatureId fid, int fieldIndex, const QVariant& newValue );
    virtual void undo();
    virtual void redo();

  private:
    QgsFeatureId mFid;
    int mFieldIndex;
    QVariant mOldValue;
    QVariant mNewValue;
    bool mFirstChange;
};


class QgsVectorLayerUndoCommandAddAttribute : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer* buffer, const QgsField& field );

    virtual void undo();
    virtual void redo();

  private:
    QgsField mField;
    int mFieldIndex;
};


class QgsVectorLayerUndoCommandDeleteAttribute : public QgsVectorLayerUndoCommand
{
  public:
    QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer* buffer, int fieldIndex );

    virtual void undo();
    virtual void redo();

  private:
    int mFieldIndex;
    bool mProviderField;
    int mOriginIndex;
    QgsField mOldField;

    QMap<QgsFeatureId, QVariant> mDeletedValues;
};


#endif
