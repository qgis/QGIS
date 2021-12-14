/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SELAFIN_HPP
#define MDAL_SELAFIN_HPP

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{
  /**
   * This class is used to read the selafin file format.
   * The file is opened with initialize() and stay opened until this object is destroyed
   *
   * \note SelafinFile object is shared between different datasets, with the mesh and its iterators.
   *       As SelafinFile is not thread safe, it has to be shared in the same thread.
   *
   * This class can be used to create a mesh with all the dataset contained in a file with the static method createMessh()
   * It is also pôssible to add all the dataset of a file in a separate existing mesh with the static method populateDataset()
   *
   * All the method to access with lazy loading to the mesh data or datasets are encapsulted and only accessible by the friend classes :
   *    MeshSelafin
   *    MeshSelafinVertexIterator
   *    MeshSelafinFaceIterator
   *    DatasetSelafin
   *
   * This ecapsulation protects these lazy loading access methods because they can't be used before the instance of SelafinFile has been initialized and parsed.
   *
  */
  class SelafinFile
  {
    public:
      //! Constructor
      SelafinFile( const std::string &fileName );

      //! Returns a mesh created with the file
      static std::unique_ptr<Mesh> createMesh( const std::string &fileName );
      //! Populates the mesh with dataset from the file
      static void populateDataset( Mesh *mesh, const std::string &fileName );

      //! Extracts data related to the mesh frame for the file
      void parseMeshFrame();

      //! Add the dataset group to the file (persist), replace dataset in the new group by Selafindataset with lazy loading
      bool addDatasetGroup( DatasetGroup *datasetGroup );

    private:

      //! Initializes and open the file file with the \a fileName
      void initialize();

      //! Reads the header of the file and return the project name
      std::string readHeader();

      //! Extracts data from the file
      void parseFile();

      //! Returns the vertices count in the mesh stored in the file
      size_t verticesCount();
      //! Returns the faces count in the mesh stored in the file
      size_t facesCount();
      //! Returns the vertices count per face for the mesh stored in the file
      size_t verticesPerFace();

      //! Returns \a count values at \a timeStepIndex and \a variableIndex, and an \a offset from the start
      std::vector<double> datasetValues( size_t timeStepIndex, size_t variableIndex, size_t offset, size_t count );
      //! Returns \a count vertex indexex in face with an \a offset from the start
      std::vector<int> connectivityIndex( size_t offset, size_t count );
      //! Returns \a count vertices with an \a offset from the start
      std::vector<double> vertices( size_t offset, size_t count );

      //! Reads a string record with a size \a len from current position in the stream, throws an exception if the size in not compaitble
      std::string readString( size_t len );

      /**
       * Reads a double array record with a size \a len from current position in the stream,
       * throws an exception if the size in not compatible
       */
      std::vector<double> readDoubleArr( size_t len );

      /**
       * Reads a int array record with a size \a len from current position in the stream,
       * throws an exception if the size in not compatible
       */
      std::vector<int> readIntArr( size_t len );

      /**
       * Reads some values in a double array record. The values count is \a len,
       * the reading begin at the stream \a position with the \a offset
       */
      std::vector<double> readDoubleArr( const std::streampos &position, size_t offset, size_t len );

      /**
       * Reads some values in a int array record. The values count is \a len,
       * the reading begin at the stream \a position with the \a offset
       */
      std::vector<int> readIntArr( const std::streampos &position, size_t offset, size_t len );

      //! Returns whether there is a int array with size \a len at the current position in the stream
      bool checkIntArraySize( size_t len );

      //! Returns whether there is a double array with size \a len at the current position in the stream
      bool checkDoubleArraySize( size_t len );

      //! Returns the remaining bytes in the stream from current position until the end
      size_t remainingBytes();

      /**
       * Set the position in the stream just after the int array with \a size, returns position of the beginning of the array
       * The presence of int array can be check with checkIntArraySize()
       */
      std::streampos passThroughIntArray( size_t size );

      /**
       * Set the position in the stream just after the double array with \a size, returns position of the beginning of the array
       * The presence of double array can be check with checkDoubleArraySize()
       */
      std::streampos passThroughDoubleArray( size_t size );

      double readDouble( );
      int readInt( );
      size_t readSizeT( );

      void ignoreArrayLength( );
      std::string readStringWithoutLength( size_t len );
      void ignore( int len );

      static void populateDataset( Mesh *mesh, std::shared_ptr<SelafinFile> reader );

      // ///////
      //  attribute updated by parseFile() method
      // //////
      std::vector<int> mParameters;
      // Dataset
      DateTime mReferenceTime;
      std::vector<std::vector<std::streampos>> mVariableStreamPosition; //! [variableIndex][timeStep]
      std::vector<RelativeTimestamp> mTimeSteps;
      std::vector<std::string> mVariableNames;
      // Mesh
      size_t mVerticesCount;
      size_t mFacesCount;
      size_t mVerticesPerFace;
      std::streampos mXStreamPosition;
      std::streampos mYStreamPosition;
      std::streampos mConnectivityStreamPosition;
      std::streampos mIPOBOStreamPosition;
      double mXOrigin;
      double mYOrigin;

      std::string mFileName;
      bool mStreamInFloatPrecision = true;
      bool mChangeEndianness = true;
      long long mFileSize = -1;

      std::ifstream mIn;
      bool mParsed = false;


      friend class MeshSelafin;
      friend class MeshSelafinVertexIterator;
      friend class MeshSelafinFaceIterator;
      friend class DatasetSelafin;
  };

  class DatasetSelafin : public Dataset2D
  {
    public:
      /**
       * Contructs a dataset with a SelafinFile object and the index of the time step
       *
       * \note SelafinFile object is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
       *
       * Position of array(s) in the stream has to be set after construction (default = begin of the stream),
       * see setXStreamPosition() and setYStreamPosition()  (X for scalar dataset, X and Y for vector dataset)
      */
      DatasetSelafin( DatasetGroup *parent,
                      std::shared_ptr<SelafinFile> reader,
                      size_t timeStepIndex );

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

      //! Sets the position of the X array in the stream
      void setXVariableIndex( size_t index );
      //! Sets the position of the Y array in the stream
      void setYVariableIndex( size_t index );

    private:
      std::shared_ptr<SelafinFile> mReader;

      size_t mXVariableIndex = 0;
      size_t mYVariableIndex = 0;
      size_t mTimeStepIndex = 0;
  };

  class MeshSelafinVertexIterator: public MeshVertexIterator
  {
    public:
      /**
       * Contructs a vertex iterator with a SerafinFile instance
       *
       * \note SerafinFile instance is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
       */
      MeshSelafinVertexIterator( std::shared_ptr<SelafinFile> reader );

      size_t next( size_t vertexCount, double *coordinates ) override;

    private:
      std::shared_ptr<SelafinFile> mReader;
      size_t mPosition = 0;
  };

  class MeshSelafinFaceIterator: public MeshFaceIterator
  {
    public:
      /**
       * Contructs a face iterator with a SerafinFile instance
       *
       * \note SerafinFile instance is shared between different dataset, with the mesh and its iterators.
       *       As SerafinFile is not thread safe, it has to be shared in the same thread.
       */
      MeshSelafinFaceIterator( std::shared_ptr<SelafinFile> reader );

      size_t next( size_t faceOffsetsBufferLen, int *faceOffsetsBuffer, size_t vertexIndicesBufferLen, int *vertexIndicesBuffer ) override;

    private:
      std::shared_ptr<SelafinFile> mReader;
      size_t mPosition = 0;
  };

  class MeshSelafin: public Mesh
  {
    public:
      /**
       * Contructs a dataset with a SerafinFile instance \a reader
       *
       * \note SerafinFile instance is shared between different dataset, with the mesh and its iterators.
       *       As SerafinFile is not thread safe, it has to be shared in the same thread.
      */
      MeshSelafin( const std::string &uri,
                   std::shared_ptr<SelafinFile> reader );

      std::unique_ptr<MeshVertexIterator> readVertices() override;

      //! Selafin format doesn't support edges in MDAL, returns a void unique_ptr
      std::unique_ptr<MeshEdgeIterator> readEdges() override;

      std::unique_ptr<MeshFaceIterator> readFaces() override;

      size_t verticesCount() const override {return mReader->verticesCount();}
      size_t edgesCount() const override {return 0;}
      size_t facesCount() const override {return mReader->facesCount();}
      BBox extent() const override;

      void closeSource() override;

    private:
      mutable bool mIsExtentUpToDate = false;
      mutable BBox mExtent;

      std::shared_ptr<SelafinFile> mReader;

      void calculateExtent() const;
  };

  /**
   * Serafin format (also called Selafin)
   *
   * Binary format for triangular mesh with datasets defined on vertices
   * Source of this doc come from :
   * http://www.opentelemac.org/downloads/MANUALS/TELEMAC-2D/telemac-2d_user_manual_en_v7p0.pdf Appendix 3
   * https://www.gdal.org/drv_selafin.html
   *
   * The Selafin file records are listed below:
   * - 1 record containing the title of the study (72 characters) and a 8 characters string indicating the type
   *    of format (SERAFIN or SERAFIND)
   * - record containing the two integers NBV(1)and NBV(2)(number of linear and quadratic variables, NBV(2)with the value of 0 for Telemac,
   * cas quadratic values are not saved so far),
   * - NBV(1)records containing the names and units of each variable (over 32 characters),
   * - 1 record containing the integers table IPARAM(10 integers, of which only the 6are currently being used),
   *        - if IPARAM (3)!=0: the value corresponds to the x-coordinate of the origin of the mesh,
   *        - if IPARAM (4)!=0: the value corresponds to the y-coordinate of the origin of the mesh,
   *        - if IPARAM (7): the value corresponds to the number of  planes on the vertical (3D computation),
   *        - if IPARAM (8)!=0: the value corresponds to the number of boundary points (in parallel),
   *        - if IPARAM (9)!=0: the value corresponds to the number of interface points (in parallel),
   *        - if IPARAM(8 ) or IPARAM(9) !=0: the array IPOBO below is replaced by the array KNOLG(total initial number of points).
   *            All the other numbers are local to the sub-domain, including IKLE.
   *
   * - if IPARAM(10)= 1: a record containing the computation starting date,
   * - 1 record containing the integers NELEM,NPOIN,NDP,1(number of elements, number of points, number of points per element and the value 1),
   * - 1 record containing table IKLE(integer array of dimension (NDP,NELEM) which is the connectivity table.
   *    N.B.: in TELEMAC-2D, the dimensions of this array are (NELEM,NDP)),
   * - 1 record containing table IPOBO(integer array of dimension NPOIN);
   *    the value of one element is 0 for an internal point, and gives the numbering of boundary points for the others,
   * - 1 record containing table X(real array of dimension NPOINcontaining the abscissae of the points),
   * - 1 record containing table Y(real array of dimension NPOINcontaining the ordinates of the points),
   *
   * Next, for each time step, the following are found:
   * - 1 record containing time T(real),
   * - NBV(1)+NBV(2)records containing the results tables for each variable at time T.
   */
  class DriverSelafin: public Driver
  {
    public:
      DriverSelafin();
      ~DriverSelafin() override;
      DriverSelafin *create() override;

      bool canReadMesh( const std::string &uri ) override;
      bool canReadDatasets( const std::string &uri ) override;

      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;
      void load( const std::string &datFile, Mesh *mesh ) override;

      bool persist( DatasetGroup *group ) override;

      int faceVerticesMaximumCount() const override {return 3;}
      void save( const std::string &fileName,  const std::string &meshName, Mesh *mesh ) override;

      std::string writeDatasetOnFileSuffix() const override;
      std::string saveMeshOnFileSuffix() const override;

    private:
      bool saveDatasetGroupOnFile( DatasetGroup *datasetGroup );
  };

} // namespace MDAL
#endif //MDAL_SELAFIN_HPP
