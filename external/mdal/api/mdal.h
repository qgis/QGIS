/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_H
#define MDAL_H

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

/**
 * Statuses
 */
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
  Err_UnsupportedElement,

  // Warnings
  Warn_InvalidElements,
  Warn_ElementWithInvalidNode,
  Warn_ElementNotUnique,
  Warn_NodeNotUnique,
  Warn_MultipleMeshesInFile
};

/**
 * Log levels
 */
enum MDAL_LogLevel
{
  Error,
  Warn,
  Info,
  Debug
};

/**
 * Specifies where the data is defined
 */
enum MDAL_DataLocation
{
  //! Unknown/Invalid location
  DataInvalidLocation = 0,
  //! Data is defined on vertices of 1D or 2D mesh
  DataOnVertices,
  //! Data is defined on face centres of 2D mesh
  DataOnFaces,
  //! Data is defined on volume centres of 3D mesh
  DataOnVolumes,
  //! Data is defined on edges of 1D mesh \since MDAL 0.6.0
  DataOnEdges
};

typedef void *MDAL_MeshH;
typedef void *MDAL_MeshVertexIteratorH;
typedef void *MDAL_MeshEdgeIteratorH;
typedef void *MDAL_MeshFaceIteratorH;
typedef void *MDAL_DatasetGroupH;
typedef void *MDAL_DatasetH;
typedef void *MDAL_DriverH;

typedef void ( *MDAL_LoggerCallback )( MDAL_LogLevel logLevel, MDAL_Status status, const char *message );

/**
 * Returns MDAL version
 */
MDAL_EXPORT const char *MDAL_Version();

/**
 * Returns last status message
 */
MDAL_EXPORT MDAL_Status MDAL_LastStatus();

/**
 * Resets the last status message
 * \since MDAL 0.9.0
 */
MDAL_EXPORT void MDAL_ResetStatus();

/**
 * Sets the last status message
 * \since MDAL 0.9.0
 */
MDAL_EXPORT void MDAL_SetStatus( MDAL_LogLevel Level, MDAL_Status status, const char *message );

/**
 * Sets custom callback for logging output
 *
 * By default standard stdout is used as output.
 * Calling this method with nullptr dissables logger ( logs will not be shown anywhere ).
 * MDAL_LoggerCallback is a function accepting MDAL_LogLevel, MDAL_Status and const char* string
 * \since MDAL 0.6.0
 */
MDAL_EXPORT void MDAL_SetLoggerCallback( MDAL_LoggerCallback callback );

/**
 * Sets maximum log level (verbosity)
 *
 * By default logger outputs errors.
 * Log levels (low to high): Error, Warn, Info, Debug
 * For example, if LogVerbosity is set to Warn, logger outputs errors and warnings.
 * \since MDAL 0.6.0
 */
MDAL_EXPORT void MDAL_SetLogVerbosity( MDAL_LogLevel verbosity );

///////////////////////////////////////////////////////////////////////////////////////
/// DRIVERS
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns count of registed MDAL drivers
 */
MDAL_EXPORT int MDAL_driverCount();

/**
 * Returns driver handle by index
 * Do not free the returned pointer
 */
MDAL_EXPORT MDAL_DriverH MDAL_driverFromIndex( int index );

/**
 * Returns driver handle by name
 * Do not free the returned pointer
 */
MDAL_EXPORT MDAL_DriverH MDAL_driverFromName( const char *name );

/**
 * Returns whether driver can be used to mesh
 * if false, driver can be only used to load datasets to existing mesh
 */
MDAL_EXPORT bool MDAL_DR_meshLoadCapability( MDAL_DriverH driver );

/**
 * Returns whether driver has capability to write/edit dataset (groups)
 */
MDAL_EXPORT bool MDAL_DR_writeDatasetsCapability( MDAL_DriverH driver, MDAL_DataLocation location );

/**
 * Returns the file suffix used to write datasets on file
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.7.0
 */
MDAL_EXPORT const char *MDAL_DR_writeDatasetsSuffix( MDAL_DriverH driver );

/**
 * Returns the file suffix used to save mesh frame on file
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT const char *MDAL_DR_saveMeshSuffix( MDAL_DriverH driver );

/**
 * Returns whether driver has capability to save mesh
 */
MDAL_EXPORT bool MDAL_DR_saveMeshCapability( MDAL_DriverH driver );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_name( MDAL_DriverH driver );

/**
 * Returns long name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_longName( MDAL_DriverH driver );

/**
 * Returns file filters that MDAL driver recognizes
 * Filters are separated by ;;, e.g. *.abc;;*.def
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_DR_filters( MDAL_DriverH driver );

/**
 * Returns the maximum number of vertices per face supported by the driver
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT int MDAL_DR_faceVerticesMaximumCount( MDAL_DriverH driver );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Loads mesh file. On error see MDAL_LastStatus for error type
 * This may effectively load whole mesh in-memory for some providers
 * Caller must free memory with MDAL_CloseMesh() afterwards
 *
 * since MDAL 0.6.0 const char parameter is renamed to uri and might contain
 * following:  <DriverName>:"<MeshFilePath>":<SpecificMeshName or Id>
 * examples: Ugrid:"mesh.nc":0, Ugrid:"mesh.nc":mesh1d, "mesh.nc":mesh1d, Ugrid:"mesh.nc", "mesh.nc", mesh.nc
 */
MDAL_EXPORT MDAL_MeshH MDAL_LoadMesh( const char *uri );

/**
 * Returns uris that the resource contains (mesh names)
 * Uris are separated by ;; and have form <DriverName>:"<MeshFilePath>"[:<SpecificMeshName>]
 * not thread-safe and valid only till next call
 *
 * Parameter uri can be in format:
 *
 * - <drivername>:"meshfile" - function then returns uris with provided driver and meshfile
 * - "meshfile" or meshfile  - function then finds proper driver and returns uris with it
 *
 * The uris can be used directly in MDAL_LoadMesh to load particular meshes
 *
 * \since MDAL 0.6.0
 */
MDAL_EXPORT const char *MDAL_MeshNames( const char *uri );

/**
 * Closes mesh, frees the memory
 */
MDAL_EXPORT void MDAL_CloseMesh( MDAL_MeshH mesh );

/**
 * Creates a empty mesh in memory
 *
 * \note the mesh is editable (vertices and faces can be added, see MDAL_M_addVertices() and MDAL_M_addFaces()),
 * and can be saved with MDAL_SaveMesh()
 *
 * \since MDAL 0.7
 */
MDAL_EXPORT MDAL_MeshH MDAL_CreateMesh( MDAL_DriverH driver );

/**
 * Saves mesh (only mesh structure) on a file with the specified driver. On error see MDAL_LastStatus for error type.
 */
MDAL_EXPORT void MDAL_SaveMesh( MDAL_MeshH mesh, const char *meshFile, const char *driver );

/**
 * Saves mesh (only mesh structure) on a file with an uri. On error see MDAL_LastStatus for error type.
 *
 * uri has form <DriverName>:"<MeshFilePath>"[:<SpecificMeshName>]
 * examples: Ugrid:"mesh.nc":0, Ugrid:"mesh.nc":mesh2d, Ugrid:"mesh.nc"
 *
 * since MDAL 0.9
 */
MDAL_EXPORT void MDAL_SaveMeshWithUri( MDAL_MeshH mesh, const char *uri );

/**
 * Returns mesh projection
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_M_projection( MDAL_MeshH mesh );

/**
 * Sets mesh projection
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.7
 */
MDAL_EXPORT void MDAL_M_setProjection( MDAL_MeshH mesh, const char *projection );

/**
 * Returns mesh extent in native projection
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_M_extent( MDAL_MeshH mesh, double *minX, double *maxX, double *minY, double *maxY );

/**
 * Adds vertices to the mesh
 * \param mesh the mesh which the vertices are added
 * \param vertexCount the count of vertices
 * \param coordinates coordinates of vertices (x0,y0,z0,x1,y1,z1,...,xn,yn,zn)
 *
 * \note to avoid incompatible datasets, adding faces removes all the existing dataset group
 *
 * \since MDAL 0.7
 */
MDAL_EXPORT void MDAL_M_addVertices( MDAL_MeshH mesh, int vertexCount, double *coordinates );

/**
 * Adds faces to the mesh
 * \param mesh the mesh which the faces are added
 * \param faceCount the count of faces
 * \param faceSizes a pointer to an array of integer containing the number of vertices per each faces
 * \param vertexIndices a pointer to an array of integer containing the indices of vertices of each faces
 *
 * \note to avoid incompatible datasets, adding faces removes all the existing dataset group
 *
 * \since MDAL 0.7
 */
MDAL_EXPORT void MDAL_M_addFaces( MDAL_MeshH mesh,
                                  int faceCount,
                                  int *faceSizes,
                                  int *vertexIndices );

/**
 * Adds edges to the mesh
 * \param mesh the mesh which the faces are added
 * \param edgeCount the count of edges
 * \param startVertexIndices must be allocated to edgesCount items to store start vertex indices for edges
 * \param endVertexIndices must be allocated to edgesCount items to store end vertex indices for edges
 *
 * \note to avoid incompatible datasets, adding edges removes all the existing dataset group
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT void MDAL_M_addEdges( MDAL_MeshH mesh,
                                  int edgesCount,
                                  int *startVertexIndices,
                                  int *endVertexIndices );

/**
 * Returns vertex count for the mesh
 */
MDAL_EXPORT int MDAL_M_vertexCount( MDAL_MeshH mesh );

/**
 * Returns edge count for the mesh
 * \since MDAL 0.6.0
 */
MDAL_EXPORT int MDAL_M_edgeCount( MDAL_MeshH mesh );

/**
 * Returns face count for the mesh
 */
MDAL_EXPORT int MDAL_M_faceCount( MDAL_MeshH mesh );

/**
 * Returns maximum number of vertices per face for this mesh, can consist of, e.g. 4 for regular quad mesh.
 */
MDAL_EXPORT int MDAL_M_faceVerticesMaximumCount( MDAL_MeshH mesh );

/**
 * Loads dataset file. On error see MDAL_LastStatus for error type.
 * This may effectively load whole dataset in-memory for some providers
 * Datasets will be closed automatically on mesh destruction or memory
 * can be freed manually with MDAL_CloseDataset if needed
 */
MDAL_EXPORT void MDAL_M_LoadDatasets( MDAL_MeshH mesh, const char *datasetFile );

/**
 * Returns number of metadata values
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT int MDAL_M_metadataCount( MDAL_MeshH mesh );

/**
 * Returns mesh metadata key
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT const char *MDAL_M_metadataKey( MDAL_MeshH mesh, int index );

/**
 * Returns mesh metadata value
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT const char *MDAL_M_metadataValue( MDAL_MeshH mesh, int index );

/**
 * Adds new metadata to the mesh
 *
 * \since MDAL 0.9.0
 */
MDAL_EXPORT void MDAL_M_setMetadata( MDAL_MeshH mesh, const char *key, const char *val );

/**
 * Returns dataset groups count
 */
MDAL_EXPORT int MDAL_M_datasetGroupCount( MDAL_MeshH mesh );

/**
 * Returns dataset group handle
 */
MDAL_EXPORT MDAL_DatasetGroupH MDAL_M_datasetGroup( MDAL_MeshH mesh, int index );

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
 * \param dataLocation location of data (face, vertex, volume)
 * \param hasScalarData whether data is scalar (false = vector data)
 * \param datasetGroupFile file to store the new dataset group
 * \returns empty pointer if not possible to create group, otherwise handle to new group
 */
MDAL_EXPORT MDAL_DatasetGroupH MDAL_M_addDatasetGroup(
  MDAL_MeshH mesh,
  const char *name,
  MDAL_DataLocation dataLocation,
  bool hasScalarData,
  MDAL_DriverH driver,
  const char *datasetGroupFile );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_M_driverName( MDAL_MeshH mesh );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH VERTICES
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns iterator to the mesh vertices
 * For some formats this may effectively load all vertices in-memory until iterator is closed
 */
MDAL_EXPORT MDAL_MeshVertexIteratorH MDAL_M_vertexIterator( MDAL_MeshH mesh );

/**
 * Returns vertices from iterator for the mesh
 * \param iterator mesh data iterator
 * \param verticesCount maximum number or vertices to be written to buffer
 * \param coordinates must be allocated to 3* verticesCount items to store x1, y1, z1, ..., xN, yN, zN coordinates
 * \returns number of vertices written in the buffer
 */
MDAL_EXPORT int MDAL_VI_next( MDAL_MeshVertexIteratorH iterator, int verticesCount, double *coordinates );

/**
 * Closes mesh data iterator, frees the memory
 */
MDAL_EXPORT void MDAL_VI_close( MDAL_MeshVertexIteratorH iterator );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH EDGES
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns iterator to the mesh edges
 * For some formats this may effectively load all edges in-memory until iterator is closed
 *
 * \since MDAL 0.6.0
 */
MDAL_EXPORT MDAL_MeshEdgeIteratorH MDAL_M_edgeIterator( MDAL_MeshH mesh );

/**
 * Returns edges from iterator for the mesh
 * \param iterator mesh data iterator
 * \param edgesCount maximum number or edges to be written to buffer
 * \param startVertexIndices must be allocated to edgesCount items to store start vertex indices for edges
 * \param endVertexIndices must be allocated to edgesCount items to store end vertex indices for edges
 * \returns number of vertices written in the buffer
 *
 * \since MDAL 0.6.0
 */
MDAL_EXPORT int MDAL_EI_next( MDAL_MeshEdgeIteratorH iterator, int edgesCount, int *startVertexIndices, int *endVertexIndices );

/**
 * Closes mesh data iterator, frees the memory
 *
 * \since MDAL 0.6.0
 */
MDAL_EXPORT void MDAL_EI_close( MDAL_MeshEdgeIteratorH iterator );

///////////////////////////////////////////////////////////////////////////////////////
/// MESH FACES
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns iterator to the mesh faces
 * For some formats this may effectively load all faces in-memory until iterator is closed
 */
MDAL_EXPORT MDAL_MeshFaceIteratorH MDAL_M_faceIterator( MDAL_MeshH mesh );

/**
 * Returns next faces from iterator for the mesh
 *
 * Reading stops when vertexIndicesBuffer capacity is full / faceOffsetsBuffer
 * capacity is full / end of faces is reached, whatever comes first
 *
 * \param iterator mesh data iterator
 * \param faceOffsetsBufferLen size of faceOffsetsBuffer, minimum 1
 * \param faceOffsetsBuffer allocated array to store face offset in vertexIndicesBuffer for given face.
 *                          To find number of vertices of face i, calculate faceOffsetsBuffer[i] - faceOffsetsBuffer[i-1]
 * \param vertexIndicesBufferLen size of vertexIndicesBuffer, minimum is MDAL_M_faceVerticesMaximumCount()
 * \param vertexIndicesBuffer writes vertex indexes for faces
 *                            faceOffsetsBuffer[i-1] is index where the vertices for face i begins,
 * \returns number of faces written in the buffer
 */
MDAL_EXPORT int MDAL_FI_next( MDAL_MeshFaceIteratorH iterator,
                              int faceOffsetsBufferLen,
                              int *faceOffsetsBuffer,
                              int vertexIndicesBufferLen,
                              int *vertexIndicesBuffer );

/**
 * Closes mesh data iterator, frees the memory
 */
MDAL_EXPORT void MDAL_FI_close( MDAL_MeshFaceIteratorH iterator );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASET GROUPS
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns dataset parent mesh
 */
MDAL_EXPORT MDAL_MeshH MDAL_G_mesh( MDAL_DatasetGroupH group );

/**
 * Returns dataset count in group
 */
MDAL_EXPORT int MDAL_G_datasetCount( MDAL_DatasetGroupH group );

/**
 * Returns dataset handle
 */
MDAL_EXPORT MDAL_DatasetH MDAL_G_dataset( MDAL_DatasetGroupH group, int index );

/**
 * Returns number of metadata values
 */
MDAL_EXPORT int MDAL_G_metadataCount( MDAL_DatasetGroupH group );

/**
 * Returns dataset metadata key
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_metadataKey( MDAL_DatasetGroupH group, int index );

/**
 * Returns dataset metadata value
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_metadataValue( MDAL_DatasetGroupH group, int index );

/**
 * Adds new metadata to the group
 * Group must be in edit mode MDAL_G_isInEditMode()
 */
MDAL_EXPORT void MDAL_G_setMetadata( MDAL_DatasetGroupH group, const char *key, const char *val );

/**
 * Returns dataset group name
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_name( MDAL_DatasetGroupH group );

/**
 * Returns name of MDAL driver
 * not thread-safe and valid only till next call
 */
MDAL_EXPORT const char *MDAL_G_driverName( MDAL_DatasetGroupH group );

/**
 * Whether dataset has scalar data associated
 */
MDAL_EXPORT bool MDAL_G_hasScalarData( MDAL_DatasetGroupH group );

/**
 * Whether dataset is on vertices
 */
MDAL_EXPORT MDAL_DataLocation MDAL_G_dataLocation( MDAL_DatasetGroupH group );

/**
 * Returns maximum number of vertical levels (for 3D meshes)
 */
MDAL_EXPORT int MDAL_G_maximumVerticalLevelCount( MDAL_DatasetGroupH group );

/**
 * Returns the minimum and maximum values of the group
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_G_minimumMaximum( MDAL_DatasetGroupH group, double *min, double *max );

/**
 * Adds empty (new) dataset to the group
 * This increases dataset group count MDAL_G_datasetCount() by 1
 *
 * The dataset is opened in edit mode.
 * To persist dataset, call MDAL_G_closeEditMode() on parent group
 *
 * Minimum and maximum dataset values are automatically calculated
 *
 * Only for 2D datasets
 *
 * \param group parent group handle
 * \param time time for dataset (hours)
 * \param values For scalar data on vertices, the size must be vertex count
 *               For scalar data on faces, the size must be faces count
 *               For scalar data on edges, the size must be edges count
 *               For vector data on vertices, the size must be vertex count * 2 (x1, y1, x2, y2, ..., xN, yN)
 *               For vector data on faces, the size must be faces count * 2 (x1, y1, x2, y2, ..., xN, yN)
 *               For vector data on edges, the size must be edges count * 2 (x1, y1, x2, y2, ..., xN, yN)
 * \param active if null pointer, MDAL_D_hasActiveFlagCapability returns false. Otherwise size must be equal to face count.
 * \returns empty pointer if not possible to create dataset (e.g. group opened in read mode), otherwise handle to new dataset
 */
MDAL_EXPORT MDAL_DatasetH MDAL_G_addDataset(
  MDAL_DatasetGroupH group,
  double time,
  const double *values,
  const int *active
);

/**
 * Adds empty (new) 3D dataset to the group
 * This increases dataset group count MDAL_G_datasetCount() by 1
 *
 * The dataset is opened in edit mode.
 * To persist dataset, call MDAL_G_closeEditMode() on parent group
 *
 * Minimum and maximum dataset values are automatically calculated
 *
 * Only for 3D datasets
 *
 * \param group parent group handle
 * \param time time for dataset (hours)
 * \param values For scalar data, the size must be volume count
 *               For vector data , the size must be volume count * 2 (x1, y1, x2, y2, ..., xN, yN)
 * \param verticalLevelCount Int Array holding the number of vertical levels for each face.
 *               Size must be the face count
 * \param verticalExtrusion Double Array holding the vertical level values for the voxels
 *               Size must be Face count + Volume count
 * \returns empty pointer if not possible to create dataset (e.g. group opened in read mode), otherwise handle to new dataset
 *
 * \since MDAL 0.9.0
 */

MDAL_EXPORT MDAL_DatasetH MDAL_G_addDataset3D(
  MDAL_DatasetGroupH group,
  double time,
  const double *values,
  const int *verticalLevelCount,
  const double *verticalExtrusions
);

/**
 * Returns whether dataset group is in edit mode
 */
MDAL_EXPORT bool MDAL_G_isInEditMode( MDAL_DatasetGroupH group );

/**
 * Close edit mode for group and all its datasets.
 * This may effectively write the data to the files and/or
 * reopen the file in read-only mode
 *
 * When closed, minimum and maximum dataset group values are automatically calculated
 */
MDAL_EXPORT void MDAL_G_closeEditMode( MDAL_DatasetGroupH group );

/**
 * Returns reference time for dataset group expressed in date with ISO8601 format, return "" if reference time is not defined
 */
MDAL_EXPORT const char *MDAL_G_referenceTime( MDAL_DatasetGroupH group );

/**
 * Sets reference time for dataset group expressed in date with ISO8601 format
 */
MDAL_EXPORT void MDAL_G_setReferenceTime( MDAL_DatasetGroupH group, const char *referenceTimeISO8601 );

/**
 * Returns whether the dataset group is temporal, i.e. has time-related datasets
 *
 * \since MDAL 0.6.0
 */
MDAL_EXPORT bool MDAL_G_isTemporal( MDAL_DatasetGroupH group );

/**
 * Returns dataset group uri
 * not thread-safe and valid only till next call
 *
 * \since MDAL 0.7.0
 */
MDAL_EXPORT const char *MDAL_G_uri( MDAL_DatasetGroupH group );

///////////////////////////////////////////////////////////////////////////////////////
/// DATASETS
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns dataset parent group
 */
MDAL_EXPORT MDAL_DatasetGroupH MDAL_D_group( MDAL_DatasetH dataset );

/**
 * Returns dataset time (hours)
 */
MDAL_EXPORT double MDAL_D_time( MDAL_DatasetH dataset );

/**
 * Returns volumes count for the mesh (for 3D meshes)
 */
MDAL_EXPORT int MDAL_D_volumesCount( MDAL_DatasetH dataset );

/**
 * Returns maximum number of vertical levels (for 3D meshes)
 */
MDAL_EXPORT int MDAL_D_maximumVerticalLevelCount( MDAL_DatasetH dataset );

/**
 * Returns number of values
 * For dataset with data location DataOnVertices returns vertex count
 * For dataset with data location DataOnFaces returns face count
 * For dataset with data location DataOnVolumes returns volumes count
 * For dataset with data location DataOnEdges returns edges count
 */
MDAL_EXPORT int MDAL_D_valueCount( MDAL_DatasetH dataset );

/**
 * Returns whether dataset is valid
 */
MDAL_EXPORT bool MDAL_D_isValid( MDAL_DatasetH dataset );

/**
 * Returns whether dataset supports active flag for dataset faces
 */
MDAL_EXPORT bool MDAL_D_hasActiveFlagCapability( MDAL_DatasetH dataset );

/**
 * Data type to be returned by MDAL_D_data
 */
enum MDAL_DataType
{
  SCALAR_DOUBLE = 0, //!< Double value for scalar datasets (DataOnVertices or DataOnFaces or DataOnEdges)
  VECTOR_2D_DOUBLE, //!< Double, double value for vector datasets (DataOnVertices or DataOnFaces or DataOnEdges)
  ACTIVE_INTEGER, //!< Integer, active flag for dataset faces. Some formats support switching off the element for particular timestep (see MDAL_D_hasActiveFlagCapability)
  VERTICAL_LEVEL_COUNT_INTEGER, //!< Number of vertical level for particular mesh's face in 3D Stacked Meshes (DataOnVolumes)
  VERTICAL_LEVEL_DOUBLE, //!< Vertical level extrusion for particular mesh's face in 3D Stacked Meshes (DataOnVolumes)
  FACE_INDEX_TO_VOLUME_INDEX_INTEGER, //!< The first index of 3D volume for particular mesh's face in 3D Stacked Meshes (DataOnVolumes)
  SCALAR_VOLUMES_DOUBLE, //!< Double scalar values for volumes in 3D Stacked Meshes (DataOnVolumes)
  VECTOR_2D_VOLUMES_DOUBLE, //!< Double, double value for volumes in 3D Stacked Meshes (DataOnVolumes)
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
 *               For SCALAR_DOUBLE, the minimum size must be valuesCount * size_of(double)
 *               For VECTOR_2D_DOUBLE, the minimum size must be valuesCount * 2 * size_of(double).
 *                                     Values are returned as x1, y1, x2, y2, ..., xN, yN
 *               For ACTIVE_INTEGER, the minimum size must be valuesCount * size_of(int)
 *               For VERTICAL_LEVEL_COUNT_INTEGER, the minimum size must be faceCount * size_of(int)
 *               For VERTICAL_LEVEL_DOUBLE, the minimum size must be (faceCount + volumesCount) * size_of(double)
 *               For FACE_INDEX_TO_VOLUME_INDEX_INTEGER, the minimum size must be faceCount * size_of(int)
 *               For SCALAR_VOLUMES_DOUBLE, the minimum size must be volumesCount * size_of(double)
 *               For VECTOR_2D_VOLUMES_DOUBLE, the minimum size must be 2 * volumesCount * size_of(double)
 * \returns number of values written to buffer. If return value != count requested, see MDAL_LastStatus() for error type
 */
MDAL_EXPORT int MDAL_D_data( MDAL_DatasetH dataset, int indexStart, int count, MDAL_DataType dataType, void *buffer );

/**
 * Returns the minimum and maximum values of the dataset
 * Returns NaN on error
 */
MDAL_EXPORT void MDAL_D_minimumMaximum( MDAL_DatasetH dataset, double *min, double *max );

#ifdef __cplusplus
}
#endif

#endif //MDAL_H
