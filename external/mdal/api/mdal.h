/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_H
#define MDAL_H

/**********************************************************************/
/**********************************************************************/
/* API is considered EXPERIMENTAL and can be changed without a notice */
/**********************************************************************/
/**********************************************************************/

#ifdef MDAL_STATIC
#  define MDAL_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef mdal_EXPORTS
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllexport))
#      else
#        define MDAL_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    else
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllimport))
#      else
#        define MDAL_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define MDAL_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define MDAL_EXPORT
#    endif
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Statuses */
enum MDAL_Status
{
  None,
  // Errors
  Err_NotEnoughMemory,
  Err_FileNotFound,
  Err_UnknownFormat,
  Err_IncompatibleMesh,
  Err_InvalidData,
  Err_IncompatibleDataset,
  Err_IncompatibleDatasetGroup,
  Err_MissingDriver,
  Err_MissingDriverCapability,
  Err_FailToWriteToDisk,
  // Warnings
  Warn_UnsupportedElement,
  Warn_InvalidElements,
  Warn_ElementWithInvalidNode,
  Warn_ElementNotUnique,
  Warn_NodeNotUnique
};

typedef void *MeshH;
typedef void *MeshVertexIteratorH;
typedef void *MeshFaceIteratorH;
typedef void *DatasetGroupH;
typedef void *DatasetH;
typedef void *DriverH;

//! Returns MDAL version
MDAL_EXPORT const char *MDAL_Version();

//! Returns last status message
MDAL_EXPORT MDAL_Status MDAL_LastStatus();

///////////////////////////////////////////////////////////////////////////////////////
/// DRIVERS
///////////////////////////////////////////////////////////////////////////////////////

//! Returns count of registed MDAL drivers
MDAL_EXPORT int MDAL_driverCount();

/**
 * Returns driver handle by index
 * Do not free the returned pointer
 */
MDAL_EXPORT DriverH MDAL_driverFromIndex( int index );

/**
 * Returns driver handle by name
 * Do not free the returned pointer
 */
MDAL_EXPORT DriverH MDAL_driverFromName( const char *name );

/**
 * Returns whether driver can be used to mesh
 * if false, driver can be only used to load datasets to existing mesh
 */
MDAL_EXPORT bool MDAL_DR_meshLoadCapability( DriverH driver );

//! Returns whether driver has capability to write/edit dataset (groups)
MDAL_EXPORT bool MDAL_DR_writeDatasetsCapability( DriverH driver );

//! Returns whether driver has capability to save mesh
MDAL_EXPORT bool MDAL_DR_SaveMeshCapability( DriverH driver );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_name( DriverH driver );

/**
 * Returns long name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_longName( DriverH driver );

/**
 * Returns file filters that MDAL driver recognizes
 * Filters are separated by ;;, e.g. *.abc;;*.def
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_filters( DriverH driver );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Loads mesh file. On error see MDAL_LastStatus for error type
 * This may effectively load whole mesh in-memory for some providers
 * Caller must free memory with MDAL_CloseMesh() afterwards
 */
MDAL_EXPORT MeshH MDAL_LoadMesh( const char *meshFile );

//! Closes mesh, frees the memory
MDAL_EXPORT void MDAL_CloseMesh( MeshH mesh );

//! Saves mesh (only mesh structure) on a file with the specified driver. On error see MDAL_LastStatus for error type.
MDAL_EXPORT void MDAL_SaveMesh( MeshH mesh, const char *meshFile, const char *driver );

/**
 * Returns mesh projection
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_M_projection( MeshH mesh );

/**
 * Returns mesh extent in native projection
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_M_extent( MeshH mesh, double *minX, double *maxX, double *minY, double *maxY );
//! Returns vertex count for the mesh
MDAL_EXPORT int MDAL_M_vertexCount( MeshH mesh );
//! Returns face count for the mesh
MDAL_EXPORT int MDAL_M_faceCount( MeshH mesh );
//! Returns maximum number of vertices face can consist of, e.g. 4 for regular quad mesh
MDAL_EXPORT int MDAL_M_faceVerticesMaximumCount( MeshH mesh );

/**
 * Loads dataset file. On error see MDAL_LastStatus for error type.
 * This may effectively load whole dataset in-memory for some providers
 * Datasets will be closed automatically on mesh destruction or memory
 * can be freed manually with MDAL_CloseDataset if needed
 */
MDAL_EXPORT void MDAL_M_LoadDatasets( MeshH mesh, const char *datasetFile );
//! Returns dataset groups count
MDAL_EXPORT int MDAL_M_datasetGroupCount( MeshH mesh );
//! Returns dataset group handle
MDAL_EXPORT DatasetGroupH MDAL_M_datasetGroup( MeshH mesh, int index );

/**
 * Adds empty (new) dataset group to the mesh
 * This increases dataset group count MDAL_M_datasetGroupCount() by 1
 *
 * The Dataset Group is opened in edit mode.
 * To persist dataset group, call MDAL_G_closeEditMode();
 *
 * It is not possible to read and write to the same group
 * at the same time. Finalize edits before reading.
 *
 * \param mesh mesh handle
 * \param driver the driver to use for storing the data
 * \param name dataset group name
 * \param isOnVertices whether data is defined on vertices
 * \param hasScalarData whether data is scalar (false = vector data)
 * \param datasetGroupFile file to store the new dataset group
 * \returns empty pointer if not possible to create group, otherwise handle to new group
 */
MDAL_EXPORT DatasetGroupH MDAL_M_addDatasetGroup( MeshH mesh,
    const char *name,
    bool isOnVertices,
    bool hasScalarData,
    DriverH driver,
    const char *datasetGroupFile );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_M_driverName( MeshH mesh );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH VERTICES
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns iterator to the mesh vertices
 * For some formats this may effectively load all vertices in-memory until iterator is closed
 */
MDAL_EXPORT MeshVertexIteratorH MDAL_M_vertexIterator( MeshH mesh );

/**
 * Returns vertices from iterator for the mesh
 * \param iterator mesh data iterator
 * \param verticesCount maximum number or vertices to be written to buffer
 * \param coordinates must be allocated to 3* verticesCount items to store x1, y1, z1, ..., xN, yN, zN coordinates
 * \returns number of vertices written in the buffer
 */
MDAL_EXPORT int MDAL_VI_next( MeshVertexIteratorH iterator, int verticesCount, double *coordinates );

//! Closes mesh data iterator, frees the memory
MDAL_EXPORT void MDAL_VI_close( MeshVertexIteratorH iterator );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH FACES
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns iterator to the mesh faces
 * For some formats this may effectively load all faces in-memory until iterator is closed
 */
MDAL_EXPORT MeshFaceIteratorH MDAL_M_faceIterator( MeshH mesh );

/**
 * Returns next faces from iterator for the mesh
 *
 * Reading stops when vertexIndicesBuffer capacity is full / faceOffsetsBuffer
 * capacity is full / end of faces is reached, whatever comes first
 *
 * \param iterator mesh data iterator
 * \param faceOffsetsBufferLen size of faceOffsetsBuffer, minimum 1
 * \param faceOffsetsBuffer allocated array to store face offset in vertexIndicesBuffer for given face.
 * To find number of vertices of face i, calculate faceOffsetsBuffer[i] - faceOffsetsBuffer[i-1]
 * \param vertexIndicesBufferLen size of vertexIndicesBuffer, minimum is MDAL_M_faceVerticesMaximumCount()
 * \param vertexIndicesBuffer writes vertex indexes for faces
 * faceOffsetsBuffer[i-1] is index where the vertices for face i begins,
 * \returns number of faces written in the buffer
 */
MDAL_EXPORT int MDAL_FI_next( MeshFaceIteratorH iterator,
                              int faceOffsetsBufferLen,
                              int *faceOffsetsBuffer,
                              int vertexIndicesBufferLen,
                              int *vertexIndicesBuffer );

//! Closes mesh data iterator, frees the memory
MDAL_EXPORT void MDAL_FI_close( MeshFaceIteratorH iterator );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASET GROUPS
///////////////////////////////////////////////////////////////////////////////////////

//! Returns dataset parent mesh
MDAL_EXPORT MeshH MDAL_G_mesh( DatasetGroupH group );

//! Returns dataset count in group
MDAL_EXPORT int MDAL_G_datasetCount( DatasetGroupH group );

//! Returns dataset handle
MDAL_EXPORT DatasetH MDAL_G_dataset( DatasetGroupH group, int index );

//! Returns number of metadata values
MDAL_EXPORT int MDAL_G_metadataCount( DatasetGroupH group );

/**
 * Returns dataset metadata key
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_metadataKey( DatasetGroupH group, int index );

/**
 * Returns dataset metadata value
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_metadataValue( DatasetGroupH group, int index );

/**
 * Adds new metadata to the group
 * Group must be in edit mode MDAL_G_isInEditMode()
 */
MDAL_EXPORT void MDAL_G_setMetadata( DatasetGroupH group, const char *key, const char *val );

/**
 * Returns dataset group name
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_name( DatasetGroupH group );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_driverName( DatasetGroupH group );

//! Whether dataset has scalar data associated
MDAL_EXPORT bool MDAL_G_hasScalarData( DatasetGroupH group );

//! Whether dataset is on vertices
MDAL_EXPORT bool MDAL_G_isOnVertices( DatasetGroupH group );

/**
 * Returns the minimum and maximum values of the group
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_G_minimumMaximum( DatasetGroupH group, double *min, double *max );

/**
 * Adds empty (new) dataset to the group
 * This increases dataset group count MDAL_G_datasetCount() by 1
 *
 * The dataset is opened in edit mode.
 * To persist dataset, call MDAL_G_closeEditMode() on parent group
 *
 * Minimum and maximum dataset values are automatically calculated
 *
 * \param group parent group handle
 * \param time time for dataset
 * \param values For scalar data on vertices, the size must be vertex count
 * For scalar data on faces, the size must be faces count
 * For vector data on vertices, the size must be vertex count * 2 (x1, y1, x2, y2, ..., xN, yN)
 * For vector data on faces, the size must be faces count * 2 (x1, y1, x2, y2, ..., xN, yN)
 * \param active if null pointer, all faces are active. Otherwise size must be equal to face count.
 * \returns empty pointer if not possible to create dataset (e.g. group opened in read mode), otherwise handle to new dataset
 */
MDAL_EXPORT DatasetH MDAL_G_addDataset( DatasetGroupH group,
                                        double time,
                                        const double *values,
                                        const int *active
                                      );

//! Returns whether dataset group is in edit mode
MDAL_EXPORT bool MDAL_G_isInEditMode( DatasetGroupH group );

/**
 * Close edit mode for group and all its datasets.
 * This may effectively write the data to the files and/or
 * reopen the file in read-only mode
 *
 * When closed, minimum and maximum dataset group values are automatically calculated
 */
MDAL_EXPORT void MDAL_G_closeEditMode( DatasetGroupH group );

/**
 * Returns reference time for dataset group
 * If returned value begins with word JULIAN, following number represents date in Julian format
 */
MDAL_EXPORT const char *MDAL_G_referenceTime( DatasetGroupH group );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASETS
///////////////////////////////////////////////////////////////////////////////////////

//! Returns dataset parent group
MDAL_EXPORT DatasetGroupH MDAL_D_group( DatasetH dataset );

//! Returns dataset time
MDAL_EXPORT double MDAL_D_time( DatasetH dataset );

//! Returns number of values
MDAL_EXPORT int MDAL_D_valueCount( DatasetH dataset );

//! Returns whether dataset is valid
MDAL_EXPORT bool MDAL_D_isValid( DatasetH dataset );

//! Data type to be returned by MDAL_D_data
enum MDAL_DataType
{
  SCALAR_DOUBLE, //!< Double value for scalar datasets
  VECTOR_2D_DOUBLE, //!< Double, double value for vector datasets
  ACTIVE_INTEGER //!< Integer, active flag for dataset faces. Some formats support switching off the element for particular timestep.
};

/**
 * Populates buffer with values from the dataset
 * for nodata, returned is numeric_limits<double>::quiet_NaN
 *
 * \param dataset handle to dataset
 * \param indexStart index of face/vertex to start reading of values to the buffer
 * \param count number of values to be written to the buffer
 * \param dataType type of values to be written to the buffer
 * \param buffer output array to be populated with the values. must be already allocated
 * For SCALAR_DOUBLE, the minimum size must be valuesCount * size_of(double)
 * For VECTOR_2D_DOUBLE, the minimum size must be valuesCount * 2 * size_of(double).
 * Values are returned as x1, y1, x2, y2, ..., xN, yN
 * For ACTIVE_INTEGER, the minimum size must be valuesCount * size_of(int)
 * \returns number of values written to buffer. If return value != count requested, see MDAL_LastStatus() for error type
 */
MDAL_EXPORT int MDAL_D_data( DatasetH dataset, int indexStart, int count, MDAL_DataType dataType, void *buffer );

/**
 * Returns the minimum and maximum values of the dataset
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_D_minimumMaximum( DatasetH dataset, double *min, double *max );

#ifdef __cplusplus
}
#endif

#endif //MDAL_H
