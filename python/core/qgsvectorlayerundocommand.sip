class QgsVectorLayerUndoCommand : QUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommand( QgsVectorLayerEditBuffer *buffer /Transfer/ );
    QgsVectorLayer *layer();
    QgsGeometryCache *cache();

    virtual int id() const;
    virtual bool mergeWith( QUndoCommand * );
};


class QgsVectorLayerUndoCommandAddFeature : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeature& f );

    virtual void undo();
    virtual void redo();
};


class QgsVectorLayerUndoCommandDeleteFeature : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid );

    virtual void undo();
    virtual void redo();
};


class QgsVectorLayerUndoCommandChangeGeometry : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid, QgsGeometry* newGeom /Transfer/ );
    ~QgsVectorLayerUndoCommandChangeGeometry();

    virtual void undo();
    virtual void redo();
    virtual int id() const;
    virtual bool mergeWith( const QUndoCommand * );
};


class QgsVectorLayerUndoCommandChangeAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid, int fieldIndex, const QVariant &newValue, const QVariant &oldValue );
    virtual void undo();
    virtual void redo();
};


class QgsVectorLayerUndoCommandAddAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, const QgsField& field );

    virtual void undo();
    virtual void redo();
};


class QgsVectorLayerUndoCommandDeleteAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:
    QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, int fieldIndex );

    virtual void undo();
    virtual void redo();

};
