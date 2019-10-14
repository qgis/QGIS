/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DATA_MODEL_HPP
#define MDAL_DATA_MODEL_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <limits>
#include "mdal.h"

namespace MDAL
{
  class DatasetGroup;
  class Mesh;

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
    double minimum = std::numeric_limits<double>::quiet_NaN();
    double maximum = std::numeric_limits<double>::quiet_NaN();
  } Statistics;

  typedef std::vector< std::pair< std::string, std::string > > Metadata;

  class Dataset
  {
    public:
      Dataset( DatasetGroup *parent );
      virtual ~Dataset();

      std::string driverName() const;

      size_t valuesCount() const;
      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) = 0;
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) = 0;
      virtual size_t activeData( size_t indexStart, size_t count, int *buffer ) = 0;

      Statistics statistics() const;
      void setStatistics( const Statistics &statistics );

      bool isValid() const;
      void setIsValid( bool isValid );

      DatasetGroup *group() const;
      Mesh *mesh() const;

      double time() const;
      void setTime( double time );

    private:
      double mTime = std::numeric_limits<double>::quiet_NaN();
      bool mIsValid = true;
      DatasetGroup *mParent = nullptr;
      Statistics mStatistics;
  };

  typedef std::vector<std::shared_ptr<Dataset>> Datasets;

  class DatasetGroup
  {
    public:
      DatasetGroup( const std::string &driverName,
                    Mesh *parent,
                    const std::string &uri
                  );

      DatasetGroup( const std::string &driverName,
                    Mesh *parent,
                    const std::string &uri,
                    const std::string &name );

      ~DatasetGroup();

      std::string driverName() const;

      std::string getMetadata( const std::string &key );
      void setMetadata( const std::string &key, const std::string &val );

      std::string name();
      void setName( const std::string &name );

      Metadata metadata;
      Datasets datasets;

      bool isScalar() const;
      void setIsScalar( bool isScalar );

      bool isOnVertices() const;
      void setIsOnVertices( bool isOnVertices );

      std::string uri() const;

      Statistics statistics() const;
      void setStatistics( const Statistics &statistics );

      std::string referenceTime() const;
      void setReferenceTime( const std::string &referenceTime );

      Mesh *mesh() const;

      bool isInEditMode() const;
      void startEditing();
      void stopEditing();

    private:
      bool mInEditMode = false;

      const std::string mDriverName;
      Mesh *mParent = nullptr;
      bool mIsScalar = true;
      bool mIsOnVertices = true;
      std::string mUri; // file/uri from where it came
      Statistics mStatistics;
      std::string mReferenceTime;
  };

  typedef std::vector<std::shared_ptr<DatasetGroup>> DatasetGroups;

  class MeshVertexIterator
  {
    public:
      virtual ~MeshVertexIterator();

      virtual size_t next( size_t vertexCount, double *coordinates ) = 0;
  };

  class MeshFaceIterator
  {
    public:
      virtual ~MeshFaceIterator();

      virtual size_t next( size_t faceOffsetsBufferLen,
                           int *faceOffsetsBuffer,
                           size_t vertexIndicesBufferLen,
                           int *vertexIndicesBuffer ) = 0;
  };

  class Mesh
  {
    public:
      Mesh( const std::string &driverName,
            size_t verticesCount,
            size_t facesCount,
            size_t faceVerticesMaximumCount,
            BBox extent,
            const std::string &uri );
      virtual ~Mesh();

      std::string driverName() const;

      void setSourceCrs( const std::string &str );
      void setSourceCrsFromWKT( const std::string &wkt );
      void setSourceCrsFromEPSG( int code );

      virtual std::unique_ptr<MDAL::MeshVertexIterator> readVertices() = 0;
      virtual std::unique_ptr<MDAL::MeshFaceIterator> readFaces() = 0;

      DatasetGroups datasetGroups;

      //! Find a dataset group by name
      std::shared_ptr<DatasetGroup> group( const std::string &name );

      size_t verticesCount() const;
      size_t facesCount() const;
      std::string uri() const;
      BBox extent() const;
      std::string crs() const;
      size_t faceVerticesMaximumCount() const;

    private:
      const std::string mDriverName;
      size_t mVerticesCount = 0;
      size_t mFacesCount = 0;
      size_t mFaceVerticesMaximumCount = 0; //typically 3 or 4, sometimes up to 9
      BBox mExtent;
      const std::string mUri; // file/uri from where it came
      std::string mCrs;
  };
} // namespace MDAL
#endif //MDAL_DATA_MODEL_HPP

