/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <limits>
#include <algorithm>
#include <string.h>

#include "mdal_ply.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"
#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "libplyxx.h"
#include "mdal_driver_manager.hpp"

#define DRIVER_NAME "PLY"

MDAL::DriverPly::DriverPly() :
  Driver( DRIVER_NAME,
          "Stanford PLY Ascii Mesh File",
          "*.ply",
          Capability::ReadMesh |
          Capability::SaveMesh |
          Capability::WriteDatasetsOnVertices |
          Capability::WriteDatasetsOnFaces |
          Capability::WriteDatasetsOnEdges |
          Capability::WriteDatasetsOnVolumes
        )
{
}

MDAL::DriverPly *MDAL::DriverPly::create()
{
  return new DriverPly();
}

MDAL::DriverPly::~DriverPly() = default;

size_t getIndex( std::vector<std::pair<std::string, bool>> v, std::string in )
{
  auto is_equal = [ in ]( std::pair<std::string, bool> s ) { return  s.first == in; };

  auto it = std::find_if( v.begin(), v.end(), is_equal );
  return ( size_t )std::distance( v.begin(), it );
}

// check for the magic number which in  a PLY file is "ply"
bool MDAL::DriverPly::canReadMesh( const std::string &uri )
{
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) || !startsWith( line, "ply" ) )
  {
    return false;
  }
  return true;
}

std::string MDAL::DriverPly::saveMeshOnFileSuffix() const
{
  return "ply";
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverPly::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();
  Vertices vertices( 0 );
  Faces faces( 0 );
  Edges edges( 0 );
  size_t maxSizeFace = 0;

  size_t vertexCount = 0;
  size_t faceCount = 0;
  size_t edgeCount = 0;

  //data structures that will contain all of the datasets, categorised by vertex, face and edge datasets
  std::vector<std::vector<double>> vertexDatasets; // contains the data
  std::vector<std::pair<std::string, bool>> vProp2Ds; // contains the dataset name and a flag for scalar / vector
  std::vector<std::vector<double>> faceDatasets;
  std::vector<std::pair<std::string, bool>> fProp2Ds;
  std::vector<std::vector<double>> edgeDatasets;
  std::vector<std::pair<std::string, bool>> eProp2Ds;
  std::unordered_map<std::string, std::pair<std::vector<double>, std::vector<int>>> listProps; // contains the list datasets as vector of values and vector indices for each face into the vector of values

  libply::File file( meshFile );
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) { return nullptr; }
  const libply::ElementsDefinition &definitions = file.definitions();
  const libply::Metadata &metadata = file.metadata();
  for ( const libply::Element &element : definitions )
  {
    if ( element.name == "vertex" )
    {
      vertexCount = element.size;
    }
    else if ( element.name == "face" )
    {
      faceCount = element.size;
    }
    else if ( element.name == "edge" )
    {
      edgeCount = element.size;
    }
    for ( const libply::Property &property : element.properties )
    {
      if ( element.name == "vertex" &&
           property.name != "X" &&
           property.name != "x" &&
           property.name != "Y" &&
           property.name != "y" &&
           property.name != "Z" &&
           property.name != "z"
         )
      {
        vProp2Ds.emplace_back( property.name, property.isList );
        vertexDatasets.push_back( std::vector<double>() );
        if ( property.isList ) listProps.emplace( property.name, std::make_pair( std::vector<double>(), std::vector<int>() ) );
      }
      else if ( element.name == "face" &&
                property.name != "vertex_indices"
              )
      {
        fProp2Ds.emplace_back( property.name, property.isList );
        faceDatasets.push_back( std::vector<double>() );
        if ( property.isList ) listProps.emplace( property.name, std::make_pair( std::vector<double>(), std::vector<int>() ) );
      }
      else if ( element.name == "edge" &&
                property.name != "vertex1" &&
                property.name != "vertex2"
              )
      {
        eProp2Ds.emplace_back( property.name, property.isList );
        edgeDatasets.push_back( std::vector<double>() );
        if ( property.isList ) listProps.emplace( property.name, std::make_pair( std::vector<double>(), std::vector<int>() ) );
      }
    }
  }



  for ( const libply::Element &el : definitions )
  {
    if ( el.name == "vertex" )
    {
      libply::ElementReadCallback vertexCallback = [&vertices, &el, &vProp2Ds, &vertexDatasets, &listProps]( libply::ElementBuffer & e )
      {
        Vertex vertex;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "X" || p.name == "x" )
          {
            vertex.x = e[i];
          }
          else if ( p.name == "Y" || p.name == "y" )
          {
            vertex.y = e[i];
          }
          else if ( p.name == "Z" || p.name == "z" )
          {
            vertex.z = e[i];
          }
          else
          {
            int dsIdx = getIndex( vProp2Ds, p.name );
            if ( vProp2Ds[ dsIdx ].second )
            {
              const std::string name = vProp2Ds[ dsIdx ].first;
              auto &vals = listProps.at( name );
              libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
              vals.second.push_back( lp->size() );
              for ( size_t j = 0; j < lp->size(); j++ )
              {
                vals.first.push_back( lp->value( j ) );
              }
            }
            else
            {
              std::vector<double> &ds = vertexDatasets[dsIdx];
              ds.push_back( e[i] );
            }
          }
        }
        vertices.push_back( vertex );
      };
      file.setElementReadCallback( "vertex", vertexCallback );
    }
    else if ( el.name == "face" )
    {
      libply::ElementReadCallback faceCallback = [&faces, &el, &maxSizeFace, &fProp2Ds, &faceDatasets, &listProps]( libply::ElementBuffer & e )
      {
        Face face;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex_indices" )
          {
            if ( !p.isList )
            {
              MDAL::Log::error( MDAL_Status::Err_InvalidData, "PLY: the triangles are not a list" );
            }
            else
            {
              libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
              if ( maxSizeFace < lp->size() ) maxSizeFace = lp->size();
              face.resize( lp->size() );
              for ( size_t j = 0; j < lp->size(); j++ )
              {
                face[j] = int( lp->value( j ) );
              }
            }
          }
          else
          {
            int dsIdx = getIndex( fProp2Ds, p.name );
            if ( fProp2Ds[ dsIdx ].second )
            {
              const std::string name = fProp2Ds[ dsIdx ].first;
              auto &vals = listProps.at( name );
              libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
              vals.second.push_back( lp->size() );
              for ( size_t j = 0; j < lp->size(); j++ )
              {
                vals.first.push_back( lp->value( j ) );
              }
            }
            else
            {
              std::vector<double> &ds = faceDatasets[dsIdx];
              ds.push_back( e[i] );
            }
          }
        }
        faces.push_back( face );
      };
      file.setElementReadCallback( "face", faceCallback );
    }
    else if ( el.name == "edge" )
    {
      libply::ElementReadCallback edgeCallback = [&edges, &el, &eProp2Ds, &edgeDatasets, &listProps]( libply::ElementBuffer & e )
      {
        Edge edge;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex1" )
          {
            edge.startVertex = int( e[i] );
          }
          else if ( p.name == "vertex2" )
          {
            edge.endVertex = int( e[i] );
          }
          else
          {
            int dsIdx = getIndex( eProp2Ds, p.name );
            if ( eProp2Ds[ dsIdx ].second )
            {
              const std::string name = eProp2Ds[ dsIdx ].first;
              auto &vals = listProps.at( name );
              libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
              vals.second.push_back( lp->size() );
              for ( size_t j = 0; j < lp->size(); j++ )
              {
                vals.first.push_back( lp->value( j ) );
              }
            }
            else
            {
              std::vector<double> &ds = edgeDatasets[dsIdx];
              ds.push_back( e[i] );
            }
          }
        }
        edges.push_back( edge );
      };
      file.setElementReadCallback( "edge", edgeCallback );
    }
  }

  file.read();
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) { return nullptr; }
  if ( vertices.size() != vertexCount ||
       faces.size() != faceCount ||
       edges.size() != edgeCount
     )
  {
    MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "Incomplete Mesh" );
    return nullptr;
  }

  std::unique_ptr< MemoryMesh > mesh(
    new MemoryMesh(
      DRIVER_NAME,
      maxSizeFace,
      meshFile
    )
  );
  mesh->setFaces( std::move( faces ) );
  mesh->setVertices( std::move( vertices ) );
  mesh->setEdges( std::move( edges ) );
  if ( metadata.find( "crs" ) != metadata.end() )
  {
    mesh->setSourceCrs( metadata.at( "crs" ) );
  }


  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );

  // Add Vertex Datasets
  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    if ( vProp2Ds[i].second )
    {
      auto &vals = listProps.at( vProp2Ds[i].first );
      vertexDatasets[i].clear();
      auto it = vals.second.begin();
      size_t idx = 0;
      while ( idx < vals.first.size() )
      {
        vertexDatasets[i].push_back( vals.first[idx] );
        vertexDatasets[i].push_back( vals.first[idx + 1] );
        idx += *it;
        it = std::next( it, 1 );
      }
      listProps.erase( vProp2Ds[i].first );
    }
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), vProp2Ds[i].first, DataOnVertices, !vProp2Ds[i].second );
    addDataset2D( group.get(), vertexDatasets[i] );
  }

  //Add face datasets and volume datasets
  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    if ( fProp2Ds[i].second )
    {
      std::string name = fProp2Ds[i].first;
      if ( name.find( "__vols" ) != std::string::npos ) break;
      auto &vals = listProps.at( name );
      faceDatasets[i].clear();
      auto it = vals.second.begin();
      size_t idx = 0;
      if ( listProps.find( name + "__vols" ) == listProps.end() )
      {
        while ( idx < vals.first.size() )
        {
          faceDatasets[i].push_back( vals.first[idx] );
          faceDatasets[i].push_back( vals.first[idx + 1] );
          idx += *it;
          it = std::next( it, 1 );
        }
        std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), name, DataOnFaces, false );
        addDataset2D( group.get(), faceDatasets[i] );
      }
      else
      {
        auto levels = listProps.at( name + "__vols" );
        std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), name, DataOnVolumes, true );
        addDataset3D( group.get(), vals.first, vals.second, levels.first, levels.second );
        listProps.erase( name + "__vols" );
      }
      listProps.erase( name );
    }
    else
    {
      std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), fProp2Ds[i].first, DataOnFaces, true );
      addDataset2D( group.get(), faceDatasets[i] );
    }
  }

  // Add Edge Datasets
  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    if ( eProp2Ds[i].second )
    {
      auto vals = listProps.at( eProp2Ds[i].first );
      edgeDatasets[i].clear();
      auto it = vals.second.begin();
      size_t idx = 0;
      while ( idx < vals.first.size() )
      {
        edgeDatasets[i].push_back( vals.first[idx] );
        edgeDatasets[i].push_back( vals.first[idx + 1] );
        idx += *it;
        it = std::next( it, 1 );
      }
      listProps.erase( eProp2Ds[i].first );
    }
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), eProp2Ds[i].first, DataOnEdges, !eProp2Ds[i].second );
    addDataset2D( group.get(), edgeDatasets[i] );
  }

  /*
  * Clean up
  */
  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    vertexDatasets.pop_back();
  };

  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    faceDatasets.pop_back();
  };

  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    edgeDatasets.pop_back();
  };

  return std::unique_ptr<Mesh>( mesh.release() );

}

std::shared_ptr< MDAL::DatasetGroup> MDAL::DriverPly::addDatasetGroup( MDAL::Mesh *mesh, const std::string &name, const MDAL_DataLocation location, bool isScalar )
{
  if ( !mesh )
    return NULL;

  if ( location == DataOnFaces && mesh->facesCount() == 0 )
    return NULL;

  if ( location == DataOnEdges && mesh->edgesCount() == 0 )
    return NULL;

  std::shared_ptr< DatasetGroup > group = std::make_shared< DatasetGroup >( mesh->driverName(), mesh, name, name );
  group->setDataLocation( location );
  group->setIsScalar( isScalar );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  return group;
}

void MDAL::DriverPly::addDataset2D( MDAL::DatasetGroup *group, const std::vector<double> &values )
{
  if ( !group )
    return;

  size_t mult = 1;
  if ( !group->isScalar() ) mult = 2;

  MDAL::Mesh *mesh = group->mesh();

  if ( values.empty() )
    return;

  if ( 0 == mesh->verticesCount() )
    return;

  if ( group->dataLocation() == DataOnVertices )
  {
    if ( values.size() != mesh->verticesCount()*mult )
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "PLY: Invalid Number of Data Values" );
      return;
    }
  }

  if ( group->dataLocation() == DataOnFaces )
  {
    if ( values.size() != mesh->facesCount()*mult )
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "PLY: Invalid Number of Data Values" );
      return;
    }
    if ( mesh->facesCount() == 0 )
      return;
  }

  if ( group->dataLocation() == DataOnEdges )
  {
    if ( values.size() != mesh->edgesCount()*mult )
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "PLY: Invalid Number of Data Values" );
      return;
    }
    if ( mesh->edgesCount() == 0 )
      return;
  }

  std::shared_ptr< MDAL::MemoryDataset2D > dataset = std::make_shared< MemoryDataset2D >( group );
  dataset->setTime( 0.0 );
  memcpy( dataset->values(), values.data(), sizeof( double ) * values.size() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
}

void MDAL::DriverPly::addDataset3D( MDAL::DatasetGroup *group,
                                    const std::vector<double> &values,
                                    const std::vector<int> &valueIndexes,
                                    const std::vector<double> &levels,
                                    const std::vector<int> &levelIndexes )
{
  if ( !group )
    return;


  MDAL::Mesh *mesh = group->mesh();

  if ( values.empty() )
    return;

  if ( 0 == mesh->facesCount() )
    return;

  if ( valueIndexes.size() != mesh->facesCount() ||
       levelIndexes.size() != mesh->facesCount() ||
       levels.size() != values.size() + mesh->facesCount()
     )
  {
    MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "Incomplete Volume Dataset" );
    return;
  }

  int maxVerticalLevelCount = * std::max_element( valueIndexes.begin(), valueIndexes.end() );

  std::shared_ptr< MDAL::MemoryDataset3D > dataset = std::make_shared< MemoryDataset3D >( group, values.size(), maxVerticalLevelCount, valueIndexes.data(), levels.data() );
  dataset->setTime( 0.0 );
  memcpy( dataset->values(), values.data(), sizeof( double ) * values.size() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
}

void MDAL::DriverPly::save( const std::string &fileName, const std::string &meshName, Mesh *mesh )
{
  MDAL_UNUSED( meshName );

  MDAL::Log::resetLastStatus();

  DatasetGroups groups = mesh->datasetGroups;

  // vectors to hold the different types of group
  DatasetGroups vgroups;
  DatasetGroups fgroups;
  DatasetGroups egroups;
  DatasetGroups volGroups;

  for ( std::shared_ptr<DatasetGroup> group : groups )
  {
    if ( group->dataLocation() == MDAL_DataLocation::DataOnVertices )
    {
      if ( group->name() != "Bed Elevation" ) vgroups.push_back( group );
    }
    else if ( group->dataLocation() == MDAL_DataLocation::DataOnFaces )
    {
      fgroups.push_back( group );
    }
    else if ( group->dataLocation() == MDAL_DataLocation::DataOnEdges )
    {
      egroups.push_back( group );
    }
    else if ( group->dataLocation() == MDAL_DataLocation::DataOnVolumes )
    {
      if ( group->isScalar() )
      {
        volGroups.push_back( group );
      }
      else
      {
        MDAL_SetStatus( MDAL_LogLevel::Warn, MDAL_Status::Err_IncompatibleDatasetGroup, "PLY: Vector Datasets on Volumes are not supported" );
      }
    }
  }


  libply::FileOut file( fileName, libply::File::Format::ASCII );
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) return;

  libply::ElementsDefinition definitions;
  std::vector<libply::Property> vproperties;
  vproperties.emplace_back( "X", libply::Type::COORDINATE, false );
  vproperties.emplace_back( "Y", libply::Type::COORDINATE, false );
  vproperties.emplace_back( "Z", libply::Type::COORDINATE, false );
  for ( std::shared_ptr<DatasetGroup> group : vgroups )
  {
    vproperties.emplace_back( group->name(), libply::Type::FLOAT64, ! group->isScalar() );
  }

  definitions.emplace_back( "vertex", mesh->verticesCount(), vproperties );
  if ( mesh->facesCount() > 0 )
  {
    std::vector<libply::Property> fproperties;
    fproperties.emplace_back( "vertex_indices", libply::Type::UINT32, true );
    for ( std::shared_ptr<DatasetGroup> group : fgroups )
    {
      fproperties.emplace_back( group->name(), libply::Type::FLOAT64, ! group->isScalar() );
    }
    for ( std::shared_ptr<DatasetGroup> group : volGroups )
    {
      fproperties.emplace_back( group->name(), libply::Type::FLOAT64, true );
      fproperties.emplace_back( group->name() + "__vols", libply::Type::FLOAT64, true );
    }
    definitions.emplace_back( "face", mesh->facesCount(), fproperties );
  }

  if ( mesh->edgesCount() > 0 )
  {
    std::vector<libply::Property> eproperties;
    eproperties.emplace_back( "vertex1", libply::Type::UINT32, false );
    eproperties.emplace_back( "vertex2", libply::Type::UINT32, false );
    for ( std::shared_ptr<DatasetGroup> group : egroups )
    {
      eproperties.emplace_back( group->name(), libply::Type::FLOAT64, ! group->isScalar() );
    }
    definitions.emplace_back( "edge", mesh->edgesCount(), eproperties );
  }
  file.setElementsDefinition( definitions );

  // write vertices
  std::unique_ptr<MDAL::MeshVertexIterator> vertices = mesh->readVertices();


  libply::ElementWriteCallback vertexCallback = [&vertices, &vgroups]( libply::ElementBuffer & e, size_t index )
  {
    double vertex[3];
    vertices->next( 1, vertex );
    e[0] = vertex[0];
    e[1] = vertex[1];
    e[2] = vertex[2];
    for ( size_t i = 0; i < vgroups.size(); i++ )
    {
      if ( vgroups[i]->isScalar() )
      {
        double val[1];
        vgroups[i]->datasets[0]->scalarData( index, 1, &val[0] );
        e[i + 3] = val[0];
      }
      else
      {
        double val[2];
        vgroups[i]->datasets[0]->vectorData( index, 1, &val[0] );
        libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i + 3] );
        lp->define( libply::Type::FLOAT64, 2 );
        lp->value( 0 ) = val[0];
        lp->value( 1 ) = val[1];
      };
    }
  };


  // write faces
  std::vector<int> vertexIndices( mesh->faceVerticesMaximumCount() );
  std::unique_ptr<MDAL::MeshFaceIterator> faces = mesh->readFaces();

  libply::ElementWriteCallback faceCallback = [&faces, &fgroups, &vertexIndices, &volGroups]( libply::ElementBuffer & e, size_t index )
  {
    int idx = 0;
    int faceOffsets[1];
    faces->next( 1, faceOffsets, vertexIndices.size(), vertexIndices.data() );
    libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[idx] );
    lp->define( libply::Type::UINT32, faceOffsets[0] );
    for ( int j = 0; j < faceOffsets[0]; ++j )
    {
      lp->value( j ) = vertexIndices[j];
    };
    idx++;
    for ( size_t i = 0; i < fgroups.size(); i++ )
    {
      if ( fgroups[i]->isScalar() )
      {
        double val[1];
        fgroups[i]->datasets[0]->scalarData( index, 1, &val[0] );
        e[idx] = val[0];
      }
      else
      {
        double val[2];
        fgroups[i]->datasets[0]->vectorData( index, 1, &val[0] );
        libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[idx] );
        lp->define( libply::Type::FLOAT64, 2 );
        lp->value( 0 ) = val[0];
        lp->value( 1 ) = val[1];
      };
      idx++;
    }
    int vCount[1];
    int f2v[1];
    for ( size_t i = 0; i < volGroups.size(); i++ )
    {
      std::shared_ptr<MDAL::MemoryDataset3D> ds = std::dynamic_pointer_cast<MDAL::MemoryDataset3D>( volGroups[i]->datasets[0] );
      ds->verticalLevelCountData( index, 1, &vCount[0] );
      const int count = vCount[0];
      ds->faceToVolumeData( index, 1, &f2v[0] );
      const int vindex = f2v[0];
      std::vector<double> val( count, 0 );
      ds->scalarVolumesData( vindex, count, val.data() );
      libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[idx] );
      lp->define( libply::Type::FLOAT64, count );

      for ( int j = 0; j < count; ++j )
      {
        lp->value( j ) = val[j];
      };
      idx++;
      std::vector<double> ex( count + 1, 0 );
      ds->verticalLevelData( vindex + index, count + 1, ex.data() );
      libply::ListProperty *lp1 = dynamic_cast<libply::ListProperty *>( &e[idx] );
      lp1->define( libply::Type::FLOAT64, count + 1 );
      for ( int j = 0; j < count + 1; ++j )
      {
        lp1->value( j ) = ex[j];
      };
      idx++;
    }
  };

  // write edges

  std::unique_ptr<MDAL::MeshEdgeIterator> edges = mesh->readEdges();

  libply::ElementWriteCallback edgeCallback = [&edges, &egroups]( libply::ElementBuffer & e, size_t index )
  {
    int startIndex;
    int endIndex;
    edges->next( 1, &startIndex, &endIndex );
    e[0] = startIndex;
    e[1] = endIndex;
    for ( size_t i = 0; i < egroups.size(); i++ )
    {
      if ( egroups[i]->isScalar() )
      {
        double val[1];
        egroups[i]->datasets[0]->scalarData( index, 1, &val[0] );
        e[i + 2] = val[0];
      }
      else
      {
        double val[2];
        egroups[i]->datasets[0]->vectorData( index, 1, &val[0] );
        libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i + 2] );
        lp->define( libply::Type::FLOAT64, 2 );
        lp->value( 0 ) = val[0];
        lp->value( 1 ) = val[1];
      };
    }
  };

  file.setElementWriteCallback( "vertex", vertexCallback );
  if ( mesh->facesCount() > 0 ) file.setElementWriteCallback( "face", faceCallback );
  if ( mesh->edgesCount() > 0 ) file.setElementWriteCallback( "edge", edgeCallback );

  file.write();


  /*
  * Clean up
  */
  for ( size_t i = 0; i < vgroups.size(); ++i )
  {
    vgroups.pop_back();
  };

  for ( size_t i = 0; i < fgroups.size(); ++i )
  {
    fgroups.pop_back();
  };

  for ( size_t i = 0; i < egroups.size(); ++i )
  {
    egroups.pop_back();
  };
}

bool MDAL::DriverPly::persist( DatasetGroup *group )
{
  save( group->uri(), "", group->mesh() );
  return false;
}
