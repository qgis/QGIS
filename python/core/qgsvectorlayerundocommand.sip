/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerundocommand.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsVectorLayerUndoCommand : QUndoCommand
{
%Docstring
 Base class for undo commands within a QgsVectorLayerEditBuffer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommand( QgsVectorLayerEditBuffer *buffer /Transfer/ );
%Docstring
 Constructor for QgsVectorLayerUndoCommand
 \param buffer associated edit buffer
%End

    QgsVectorLayer *layer();
%Docstring
Returns the layer associated with the undo command
 :rtype: QgsVectorLayer
%End

    virtual int id() const;
    virtual bool mergeWith( const QUndoCommand * );

  protected:
};



class QgsVectorLayerUndoCommandAddFeature : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for adding a feature to a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer *buffer /Transfer/, QgsFeature &f );
%Docstring
 Constructor for QgsVectorLayerUndoCommandAddFeature
 \param buffer associated edit buffer
 \param f feature to add to layer
%End

    virtual void undo();
    virtual void redo();

};



class QgsVectorLayerUndoCommandDeleteFeature : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for deleting a feature from a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer *buffer /Transfer/, QgsFeatureId fid );
%Docstring
 Constructor for QgsVectorLayerUndoCommandDeleteFeature
 \param buffer associated edit buffer
 \param fid feature ID of feature to delete from layer
%End

    virtual void undo();
    virtual void redo();

};


class QgsVectorLayerUndoCommandChangeGeometry : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for modifying the geometry of a feature from a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer *buffer /Transfer/, QgsFeatureId fid, const QgsGeometry &newGeom );
%Docstring
 Constructor for QgsVectorLayerUndoCommandChangeGeometry
 \param buffer associated edit buffer
 \param fid feature ID of feature to modify geometry of
 \param newGeom new geometry for feature
%End

    virtual void undo();
    virtual void redo();
    virtual int id() const;
    virtual bool mergeWith( const QUndoCommand * );

};



class QgsVectorLayerUndoCommandChangeAttribute : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for modifying an attribute of a feature from a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer *buffer /Transfer/, QgsFeatureId fid, int fieldIndex, const QVariant &newValue, const QVariant &oldValue );
%Docstring
 Constructor for QgsVectorLayerUndoCommandChangeAttribute
 \param buffer associated edit buffer
 \param fid feature ID of feature to modify
 \param fieldIndex index of field to modify
 \param newValue new value of attribute
 \param oldValue previous value of attribute
%End
    virtual void undo();
    virtual void redo();

};


class QgsVectorLayerUndoCommandAddAttribute : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for adding a new attribute to a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer *buffer /Transfer/, const QgsField &field );
%Docstring
 Constructor for QgsVectorLayerUndoCommandAddAttribute
 \param buffer associated edit buffer
 \param field definition of new field to add
%End

    virtual void undo();
    virtual void redo();

};


class QgsVectorLayerUndoCommandDeleteAttribute : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for removing an existing attribute from a vector layer.
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer *buffer /Transfer/, int fieldIndex );
%Docstring
 Constructor for QgsVectorLayerUndoCommandDeleteAttribute
 \param buffer associated edit buffer
 \param fieldIndex index of field to delete
%End

    virtual void undo();
    virtual void redo();

};



class QgsVectorLayerUndoCommandRenameAttribute : QgsVectorLayerUndoCommand
{
%Docstring
 Undo command for renaming an existing attribute of a vector layer.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    QgsVectorLayerUndoCommandRenameAttribute( QgsVectorLayerEditBuffer *buffer /Transfer/, int fieldIndex, const QString &newName );
%Docstring
 Constructor for QgsVectorLayerUndoCommandRenameAttribute
 \param buffer associated edit buffer
 \param fieldIndex index of field to rename
 \param newName new name for field
%End

    virtual void undo();
    virtual void redo();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectorlayerundocommand.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
