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
#include "mdal_datetime.hpp"

namespace MDAL
{
  class DatasetGroup;
  class Mesh;

  struct BBox
  {
    BBox() {}
    BBox( double lx, double ux, double ly, double uy ): minX( lx ), maxX( ux ), minY( ly ), maxY( uy ) {}

    double minX = std::numeric_limits<double>::max();
    double maxX = -std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxY = -std::numeric_limits<double>::max();
  };

  typedef struct StatisticsType
  {
    double minimum = std::numeric_limits<double>::quiet_NaN();
    double maximum = std::numeric_limits<double>::quiet_NaN();
  } Statistics;

  typedef std::vector< std::pair< std::string, std::string > > Metadata;
  typedef std::vector<std::pair<double, double>> Classification;

  class Dataset
  {
    public:
      Dataset( DatasetGroup *parent );
      virtual ~Dataset();

      size_t valuesCount() const;

      //! For DataOnVertices or DataOnFaces
      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) = 0;
      //! For DataOnVertices or DataOnFaces
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) = 0;
      //! For drivers that supports it, see supportsActiveFlag()
      virtual size_t activeData( size_t indexStart, size_t count, int *buffer );

      //! For DataOnVolumes
      virtual size_t verticalLevelCountData( size_t indexStart, size_t count, int *buffer ) = 0;
      //! For DataOnVolumes
      virtual size_t verticalLevelData( size_t indexStart, size_t count, double *buffer ) = 0;
      //! For DataOnVolumes
      virtual size_t faceToVolumeData( size_t indexStart, size_t count, int *buffer ) = 0;
      //! For DataOnVolumes
      virtual size_t scalarVolumesData( size_t indexStart, size_t count, double *buffer ) = 0;
      //! For DataOnVolumes
      virtual size_t vectorVolumesData( size_t indexStart, size_t count, double *buffer ) = 0;

      virtual size_t volumesCount() const = 0;
      virtual size_t maximumVerticalLevelsCount() const = 0;

      Statistics statistics() const;
      void setStatistics( const Statistics &statistics );

      bool isValid() const;

      DatasetGroup *group() const;
      Mesh *mesh() const;

      double time( RelativeTimestamp::Unit unit ) const;
      RelativeTimestamp timestamp() const;
      void setTime( double time, RelativeTimestamp::Unit unit = RelativeTimestamp::hours );
      void setTime( const RelativeTimestamp &time );

      bool supportsActiveFlag() const;
      void setSupportsActiveFlag( bool value );

    private:
      RelativeTimestamp mTime;
      bool mIsValid = true;
      bool mSupportsActiveFlag = false;
      DatasetGroup *mParent = nullptr;
      Statistics mStatistics;
  };

  class Dataset2D: public Dataset
  {
    public:
      Dataset2D( DatasetGroup *parent );
      virtual ~Dataset2D() override;

      size_t verticalLevelCountData( size_t indexStart, size_t count, int *buffer ) override;
      size_t verticalLevelData( size_t indexStart, size_t count, double *buffer ) override;
      size_t faceToVolumeData( size_t indexStart, size_t count, int *buffer ) override;
      size_t scalarVolumesData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorVolumesData( size_t indexStart, size_t count, double *buffer ) override;

      size_t volumesCount() const override;
      size_t maximumVerticalLevelsCount() const override;
  };

  class Dataset3D: public Dataset
  {
    public:
      Dataset3D(
        DatasetGroup *parent,
        size_t volumes,
        size_t maxVerticalLevelCount
      );
      virtual ~Dataset3D() override;

      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

      size_t volumesCount() const override;
      size_t maximumVerticalLevelsCount() const override;

    private:
      size_t mVolumesCount = 0;
      size_t mMaximumVerticalLevelsCount = 0;
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
      void setMetadata( const Metadata &new_metadata );

      std::string name();
      void setName( const std::string &name );

      Metadata metadata;
      Datasets datasets;

      bool isScalar() const;
      void setIsScalar( bool isScalar );

      MDAL_DataLocation dataLocation() const;
      void setDataLocation( MDAL_DataLocation dataLocation );

      std::string uri() const;
      void replaceUri( std::string uri );

      Statistics statistics() const;
      void setStatistics( const Statistics &statistics );

      DateTime referenceTime() const;
      void setReferenceTime( const DateTime &referenceTime );

      Mesh *mesh() const;

      size_t maximumVerticalLevelsCount() const;

      bool isInEditMode() const;
      void startEditing();
      void stopEditing();

      //! First value is the angle for full rotation and second value is the start angle
      void setReferenceAngles( const std::pair<double, double> &referenceAngle );
      std::pair<double, double> referenceAngles() const;

      bool isPolar() const;
      void setIsPolar( bool isPolar );
    private:
      bool mInEditMode = false;

      const std::string mDriverName;
      Mesh *mParent = nullptr;
      bool mIsScalar = true;
      bool mIsPolar = false;
      std::pair<double, double> mReferenceAngles = { -360, 0}; //default full rotation is negative to be consistent with usual geographical clockwise
      MDAL_DataLocation mDataLocation = MDAL_DataLocation::DataOnVertices;
      std::string mUri; // file/uri from where it came
      Statistics mStatistics;
      DateTime mReferenceTime;
  };

  typedef std::vector<std::shared_ptr<DatasetGroup>> DatasetGroups;

  class MeshVertexIterator
  {
    public:
      virtual ~MeshVertexIterator();

      virtual size_t next( size_t vertexCount, double *coordinates ) = 0;
  };

  class MeshEdgeIterator
  {
    public:
      virtual ~MeshEdgeIterator();

      virtual size_t next( size_t edgeCount,
                           int *startVertexIndices,
                           int *endVertexIndices ) = 0;
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
            size_t faceVerticesMaximumCount,
            const std::string &uri );

      virtual ~Mesh();

      std::string driverName() const;

      void setSourceCrs( const std::string &str );
      void setSourceCrsFromWKT( const std::string &wkt );
      void setSourceCrsFromEPSG( int code );
      void setSourceCrsFromPrjFile( const std::string &filename );

      virtual std::unique_ptr<MDAL::MeshVertexIterator> readVertices() = 0;
      virtual std::unique_ptr<MDAL::MeshEdgeIterator> readEdges() = 0;
      virtual std::unique_ptr<MDAL::MeshFaceIterator> readFaces() = 0;

      DatasetGroups datasetGroups;

      //! Find a dataset group by name
      std::shared_ptr<DatasetGroup> group( const std::string &name );

      virtual size_t verticesCount() const = 0;
      virtual size_t edgesCount() const = 0;
      virtual size_t facesCount() const = 0;
      virtual BBox extent() const = 0;
      std::string uri() const;
      std::string crs() const;
      size_t faceVerticesMaximumCount() const;

      std::string getMetadata( const std::string &key );
      void setMetadata( const std::string &key, const std::string &val );
      void setMetadata( const Metadata &new_metadata );

      Metadata metadata;

      virtual void closeSource() {};

      virtual bool isEditable() const {return false;}

      virtual void addVertices( size_t vertexCount, double *coordinates );
      virtual void addFaces( size_t faceCount, size_t driverMaxVerticesPerFace, int *faceSizes, int *vertexIndices );
      virtual void addEdges( size_t edgeCount, int *startVertexIndices, int *endVertexIndices );


    protected:
      void setFaceVerticesMaximumCount( const size_t &faceVerticesMaximumCount );

    private:
      const std::string mDriverName;
      size_t mFaceVerticesMaximumCount = 0; //typically 3 or 4, sometimes up to 9
      const std::string mUri; // file/uri from where it came
      std::string mCrs;
  };
} // namespace MDAL
#endif //MDAL_DATA_MODEL_HPP

