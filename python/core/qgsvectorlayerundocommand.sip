/** \ingroup core
 * \class QgsVectorLayerUndoCommand
 * \brief Base class for undo commands within a QgsVectorLayerEditBuffer.
 */

class QgsVectorLayerUndoCommand : QUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommand
     * @param buffer associated edit buffer
     */
    QgsVectorLayerUndoCommand( QgsVectorLayerEditBuffer *buffer /Transfer/ );

    //! Returns the layer associated with the undo command
    QgsVectorLayer *layer();
    QgsGeometryCache *cache();

    virtual int id() const;
    virtual bool mergeWith( QUndoCommand * );
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandAddFeature
 * \brief Undo command for adding a feature to a vector layer.
 */

class QgsVectorLayerUndoCommandAddFeature : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandAddFeature
     * @param buffer associated edit buffer
     * @param f feature to add to layer
     */
    QgsVectorLayerUndoCommandAddFeature( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeature& f );

    virtual void undo();
    virtual void redo();
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandDeleteFeature
 * \brief Undo command for deleting a feature from a vector layer.
 */

class QgsVectorLayerUndoCommandDeleteFeature : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandDeleteFeature
     * @param buffer associated edit buffer
     * @param fid feature ID of feature to delete from layer
     */
    QgsVectorLayerUndoCommandDeleteFeature( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid );

    virtual void undo();
    virtual void redo();
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandChangeGeometry
 * \brief Undo command for modifying the geometry of a feature from a vector layer.
 */

class QgsVectorLayerUndoCommandChangeGeometry : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandChangeGeometry
     * @param buffer associated edit buffer
     * @param fid feature ID of feature to modify geometry of
     * @param newGeom new geometry for feature
     */
    QgsVectorLayerUndoCommandChangeGeometry( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid, QgsGeometry* newGeom /Transfer/ );
    ~QgsVectorLayerUndoCommandChangeGeometry();

    virtual void undo();
    virtual void redo();
    virtual int id() const;
    virtual bool mergeWith( const QUndoCommand * );
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandChangeAttribute
 * \brief Undo command for modifying an attribute of a feature from a vector layer.
 */

class QgsVectorLayerUndoCommandChangeAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandChangeAttribute
     * @param buffer associated edit buffer
     * @param fid feature ID of feature to modify
     * @param fieldIndex index of field to modify
     * @param newValue new value of attribute
     * @param oldValue previous value of attribute
     */
    QgsVectorLayerUndoCommandChangeAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, QgsFeatureId fid, int fieldIndex, const QVariant &newValue, const QVariant &oldValue );
    virtual void undo();
    virtual void redo();
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandAddAttribute
 * \brief Undo command for adding a new attribute to a vector layer.
 */

class QgsVectorLayerUndoCommandAddAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandAddAttribute
     * @param buffer associated edit buffer
     * @param field definition of new field to add
     */
    QgsVectorLayerUndoCommandAddAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, const QgsField& field );

    virtual void undo();
    virtual void redo();
};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandDeleteAttribute
 * \brief Undo command for removing an existing attribute from a vector layer.
 */

class QgsVectorLayerUndoCommandDeleteAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandDeleteAttribute
     * @param buffer associated edit buffer
     * @param fieldIndex index of field to delete
     */
    QgsVectorLayerUndoCommandDeleteAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, int fieldIndex );

    virtual void undo();
    virtual void redo();

};


/** \ingroup core
 * \class QgsVectorLayerUndoCommandRenameAttribute
 * \brief Undo command for renaming an existing attribute of a vector layer.
 * \note added in QGIS 2.16
 */

class QgsVectorLayerUndoCommandRenameAttribute : QgsVectorLayerUndoCommand
{
%TypeHeaderCode
#include "qgsvectorlayerundocommand.h"
%End
  public:

    /** Constructor for QgsVectorLayerUndoCommandRenameAttribute
     * @param buffer associated edit buffer
     * @param fieldIndex index of field to rename
     * @param newName new name for field
     */
    QgsVectorLayerUndoCommandRenameAttribute( QgsVectorLayerEditBuffer* buffer /Transfer/, int fieldIndex, const QString& newName );

    virtual void undo();
    virtual void redo();

};
