/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DEFINES_HPP
#define MDAL_DEFINES_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>

namespace MDAL
{
  class DatasetGroup;

  struct BBox
  {
    BBox() {}
    BBox( double lx, double ux, double ly, double uy ): minX( lx ), maxX( ux ), minY( ly ), maxY( uy ) {}

    double minX;
    double maxX;
    double minY;
    double maxY;
  };


  typedef struct
  {
    double x;
    double y;
    double z; // Bed elevation
  } Vertex;

  typedef std::vector<size_t> Face;

  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Face> Faces;

  typedef struct
  {
    double x;
    double y;

    bool noData = false;
  } Value; //Dataset Value

  typedef std::vector< std::pair< std::string, std::string > > Metadata;

  class Dataset
  {
    public:
      double time;

      /**
       * size - face count if !isOnVertices
       * size - vertex count if isOnVertices
       */
      std::vector<Value> values;
      std::vector<bool> active; // size - face count. Whether the output for this is active...

      bool isValid = true;
      DatasetGroup *parent = nullptr;

      bool isActive( size_t faceIndex );
  };

  typedef std::vector<std::shared_ptr<Dataset>> Datasets;

  class DatasetGroup
  {
    public:
      std::string getMetadata( const std::string &key );

      void setMetadata( const std::string &key, const std::string &val );

      std::string name();
      void setName( const std::string &name );

      Metadata metadata;

      bool isScalar = true;
      bool isOnVertices = true;
      Datasets datasets;
      std::string uri; // file/uri from where it came
  };

  typedef std::vector<std::shared_ptr<DatasetGroup>> DatasetGroups;

  struct Mesh
  {
    std::string uri; // file/uri from where it came
    std::string crs;

    Vertices vertices;
    std::map<size_t, size_t> vertexIDtoIndex; // only for 2DM and DAT files

    Faces faces;
    std::map<size_t, size_t> faceIDtoIndex; // only for 2DM and DAT files

    DatasetGroups datasetGroups;

    void setSourceCrs( const std::string &str );
    void setSourceCrsFromWKT( const std::string &wkt );
    void setSourceCrsFromEPSG( int code );

    void addBedElevationDataset();
  };

} // namespace MDAL
#endif //MDAL_DEFINES_HPP

