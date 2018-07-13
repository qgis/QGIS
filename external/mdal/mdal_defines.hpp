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

  class Dataset
  {
    public:

      std::string getMetadata( const std::string &key )
      {
        for ( auto &pair : metadata )
        {
          if ( pair.first == key )
          {
            return pair.second;
          }
        }
        return std::string();
      }

      void setMetadata( const std::string &key, const std::string &val )
      {
        bool found = false;
        for ( auto &pair : metadata )
        {
          if ( pair.first == key )
          {
            found = true;
            pair.second = val;
          }
        }
        if ( !found )
          metadata.push_back( std::make_pair( key, val ) );
      }

      std::string name()
      {
        return getMetadata( "name" );
      }

      void setName( const std::string &name )
      {
        setMetadata( "name", name );
      }

      std::vector< std::pair< std::string, std::string > > metadata;
      std::vector<Value> values; // size - vertex count if isOnVertices
      // size - face count if !isOnVertices
      std::vector<bool> active; // size - face count. Whether the output for this is active...

      bool isScalar = true;
      bool isOnVertices = true;
      bool isValid = true;

      std::string uri; // file/uri from where it came


      void free()
      {
        values.clear();
        metadata.clear();
        isValid = false;
      }

      bool isActive( size_t faceIndex )
      {
        if ( isOnVertices )
        {
          if ( active.size() > faceIndex )
            return active[faceIndex];
          else
            return false;
        }
        else
        {
          return true;
        }
      }
  };

  typedef std::vector<std::shared_ptr<Dataset>> Datasets;

  struct Mesh
  {
    std::string uri; // file/uri from where it came
    std::string crs;

    Vertices vertices;
    std::map<size_t, size_t> vertexIDtoIndex; // only for 2DM and DAT files

    Faces faces;
    std::map<size_t, size_t> faceIDtoIndex; // only for 2DM and DAT files

    Datasets datasets;

    void setSourceCrs( const std::string &str ) {crs = str;} //TODO
    void setSourceCrsFromWKT( const std::string &str ) {crs = str;} //TODO
  };

} // namespace MDAL
#endif //MDAL_DEFINES_HPP

