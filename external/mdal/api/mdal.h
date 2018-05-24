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
  Err_MissingDriver,
  // Warnings
  Warn_UnsupportedElement,
  Warn_InvalidElements,
  Warn_ElementWithInvalidNode,
  Warn_ElementNotUnique,
  Warn_NodeNotUnique
};

//! Mesh
typedef void *MeshH;
typedef void *DatasetH;

//! Return MDAL version
MDAL_EXPORT const char *MDAL_Version();

//! Return last status message
MDAL_EXPORT MDAL_Status MDAL_LastStatus();

//! Load mesh file. On error see MDAL_LastStatus for error type This effectively loads whole mesh in-memory
MDAL_EXPORT MeshH MDAL_LoadMesh( const char *meshFile );
//! Close mesh, free the memory
MDAL_EXPORT void MDAL_CloseMesh( MeshH mesh );
//! Return vertex count for the mesh
MDAL_EXPORT int MDAL_M_vertexCount( MeshH mesh );
//! Return vertex X coord for the mesh
MDAL_EXPORT double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index );
//! Return vertex Y coord for the mesh
MDAL_EXPORT double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index );
//! Return face count for the mesh
MDAL_EXPORT int MDAL_M_faceCount( MeshH mesh );
//! Return number of vertices face consist of, e.g. 3 for triangle
MDAL_EXPORT int MDAL_M_faceVerticesCountAt( MeshH mesh, int index );
//! Return vertex index for face
MDAL_EXPORT int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index );

/**
 * Load dataset file. On error see MDAL_LastStatus for error type.
 * This may effectively load whole dataset in-memory for some providers
 * Datasets will be closed automatically on mesh destruction or memory
 * can be freed manually with MDAL_CloseDataset if needed
 */
MDAL_EXPORT void MDAL_M_LoadDatasets( MeshH mesh, const char *datasetFile );

//! Free the memory used to get dataset values
MDAL_EXPORT void MDAL_M_CloseDataset( DatasetH dataset );

//! Return dataset count
MDAL_EXPORT int MDAL_M_datasetCount( MeshH mesh );

//! Return dataset handle
MDAL_EXPORT DatasetH MDAL_M_dataset( MeshH mesh, int index );

//! Whether dataset has scalar data associated
MDAL_EXPORT bool MDAL_D_hasScalarData( DatasetH dataset );

//! Whether dataset is on vertices
MDAL_EXPORT bool MDAL_D_isOnVertices( DatasetH dataset );

//! Return number of metadata values
MDAL_EXPORT int MDAL_D_metadataCount( DatasetH dataset );

//! Return dataset metadata key
MDAL_EXPORT const char *MDAL_D_metadataKey( DatasetH dataset, int index );

//! Return dataset metadata value
MDAL_EXPORT const char *MDAL_D_metadataValue( DatasetH dataset, int index );

//! Return number of values
MDAL_EXPORT int MDAL_D_valueCount( DatasetH dataset );

/**
 * Return scalar value associated with the index from the dataset
 * for nodata return numeric_limits<double>:quiet_NaN
 */
MDAL_EXPORT double MDAL_D_value( DatasetH dataset, int valueIndex );

/**
 * Return X value associated with the index from the vector dataset
 * for nodata return numeric_limits<double>:quiet_NaN
 */
MDAL_EXPORT double MDAL_D_valueX( DatasetH dataset, int valueIndex );

/**
 * Return Y value associated with the index from the vector dataset
 * for nodata return numeric_limits<double>:quiet_NaN
 */
MDAL_EXPORT double MDAL_D_valueY( DatasetH dataset, int valueIndex );

/**
 * Whether element is active - should be taken into account
 * Some formats support switching off the element for particular timestep
 */
MDAL_EXPORT bool MDAL_D_active( DatasetH dataset, int faceIndex );

//! Return whether dataset is valid
MDAL_EXPORT bool MDAL_D_isValid( DatasetH dataset );

#ifdef __cplusplus
}
#endif

#endif //MDAL_H
