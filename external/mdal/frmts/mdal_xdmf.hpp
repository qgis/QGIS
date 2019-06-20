/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_XDMF_HPP
#define MDAL_XDMF_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <utility>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_hdf5.hpp"
#include "mdal_driver.hpp"
#include "mdal_xml.hpp"

namespace MDAL
{

  /**
   * The XdmfDataset reads the data directly from HDF5 file
   * by usage of hyperslabs retrieval
   *
   * http://xdmf.org/index.php/XDMF_Model_and_Format#HyperSlab
   * HyperSlab consists of 3 rows: start, stride, and count
   * - Currently we do not support stride other than 1 (every element)
   * - Assumes BASEMENT 3.x format where the array is nFaces x 1
   */
  struct HyperSlab
  {
    size_t startX = 0; // offset X
    size_t startY = 0; // offset Y
    size_t count = 0; // number of cells/vertices
    bool countInFirstColumn = true;
    bool isScalar;
  };

  /**
   * The XdmfDataset is simple vector or scalar dataset
   * where values are stored in one HD5 variable
   * and are lazy loaded on demand. Active flag is always ON.
   *
   * The definition is stored in XML file in format:
   *
   * <Attribute Name="water_surface" AttributeType="Scalar" Center="Cell">
   *   <DataItem ItemType="HyperSlab" Dimensions="9 1" Type="HyperSlab">
   *     <DataItem Dimensions="3 2" Format="XML"> 0 0 1 1 9 1 </DataItem>
   *     <DataItem Dimensions="9 3" Format="HDF"> test.h5:/RESULTS/CellsAll/HydState/0000002 </DataItem>
   *   </DataItem>
   * </Attribute>
   */
  class XdmfDataset: public Dataset
  {
    public:
      XdmfDataset( DatasetGroup *grp,
                   const HyperSlab &slab,
                   const HdfDataset &valuesDs,
                   double time
                 );
      ~XdmfDataset() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

    private:
      std::vector<hsize_t> offsets( size_t indexStart );
      std::vector<hsize_t> selections( size_t copyValues );

      HdfDataset mHdf5DatasetValues;
      HyperSlab mHyperSlab;
  };

  /**
   * The XdmfFunctionDataset is a function that
   * references two or three scalar XdmfDatasets
   * to create a vector or scalar dataset based on
   * referenced datasets. Active flag is always ON.
   *
   * Currently we do not use any fancy bison/flex based
   * expression parsing, just supporting few types of
   * most common function types:
   *   - subtraction (A-B)
   *   - join ( [A, B] vector)
   *   - magnitude
   *
   * The definition is stored in XML file in format:
   *
   * <Attribute Name="..." AttributeType="Scalar" Center="Cell">
   *    <DataItem ItemType="Function" Function="$0 - $1" Dimensions="9" >
   *     <DataItem ItemType="HyperSlab" Type="HyperSlab">...</DataItem>
   *     <DataItem ItemType="HyperSlab" Type="HyperSlab">...</DataItem>
   *   </DataItem>
   * </Attribute>
   */
  class XdmfFunctionDataset: public Dataset
  {
    public:
      enum FunctionType
      {
        Join = 1, //!< vector: [$0, $1] from 2 scalars
        Subtract, //!< scalar: $1 - $0, e.g. calculate relative depth
        Flow, //!< scalar: flow velocity (abs) = sqrt($0/($2-$3)*$0/($2-$3) + $1/($2-$3)*$1/($2-$3))
      };

      XdmfFunctionDataset(
        DatasetGroup *grp,
        FunctionType type,
        double time
      );
      ~XdmfFunctionDataset() override;

      //! Adds reference XMDF dataset
      void addReferenceDataset( const HyperSlab &slab, const HdfDataset &hdfDataset, double time );
      //! Swaps first and second reference dataset
      void swap();

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

    private:
      size_t subtractFunction( size_t indexStart, size_t count, double *buffer );
      size_t flowFunction( size_t indexStart, size_t count, double *buffer );
      size_t joinFunction( size_t indexStart, size_t count, double *buffer );
      size_t extractRawData( size_t indexStart, size_t count, size_t nDatasets, std::vector<double> &buf );

      const FunctionType mType;
      std::vector<std::shared_ptr<XdmfDataset>> mReferenceDatasets;
      /**
       * "fake" base group for reference datasets.
       * This group is not exposed to public API and
       * it is just an implementation detail.
       */
      DatasetGroup mBaseReferenceGroup;
  };

  class DriverXdmf: public Driver
  {
    public:
      /**
       * Driver for XDMF Files
       *
       * XDMF is combination of XML file with dataset metadata and
       * HDF5 file with actual data for the datasets
       *
       * full file specification http://xdmf.org/index.php/XDMF_Model_and_Format
       *
       * XDMF file can have data (vectors) stored in different ways. Currently we
       * only support format for BASEMENET 3.x solver
       */
      DriverXdmf();
      ~DriverXdmf( ) override;
      DriverXdmf *create() override;

      bool canRead( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh, MDAL_Status *status ) override;

    private:
      /**
       Parses XML File with this structure, where data is specified as pointers to HDF in Attribute tags

       <?xml version="1.0" ?>
       <!DOCTYPE Xdmf SYSTEM "Xdmf.dtd" []>
       <Xdmf Version="2.0">
        <Domain>
            <Topology> ... </Topology>
            <Geometry> ... </Geometry>
            <Grid GridType="Collection" Name="..." CollectionType="Temporal">
                <Grid GridType="Uniform" Name="Timestep">
                    <Time TimeType="Single" Name="time = 0.000000" Value="0.000000"> </Time>
                    <Topology></Topology>
                    <Geometry GeometryType="XY" Reference="/Xdmf/Domain/Geometry[1]"></Geometry>
                <Attribute> ... </Attribute>
                </Grid>
            </Grid>
        <Domain>
       </Xdmf>
      */
      DatasetGroups parseXdmfXml( );

      //! Finds a group with a name or creates a new group if does not exists
      std::shared_ptr<MDAL::DatasetGroup> findGroup( std::map< std::string, std::shared_ptr<MDAL::DatasetGroup> > &groups,
          const std::string &groupName,
          bool isScalar );

      /**
       * Parses scalar/vector definition for XDMF dataset, e.g.
       *
       *  <DataItem ItemType="HyperSlab" Dimensions="9 1" Type="HyperSlab">
       *    <DataItem Dimensions="3 2" Format="XML"> 0 1 1 1 9 1 </DataItem>
       *    <DataItem Dimensions="9 3" Format="HDF"> test.h5:/RESULTS/CellsAll/HydState/0000002 </DataItem>
       *  </DataItem>
       */
      std::pair<HdfDataset, HyperSlab > parseXdmfDataset( const XMLFile &xmfFile, xmlNodePtr itemNod );

      //! Parses hypeslab specification from string
      HyperSlab parseHyperSlab( const std::string &str, size_t dimB );

      /**
       * Parses hyperslab specification from matrix in DataItem [Dimension] tag

         <DataItem ItemType="Uniform" Dimensions="3 3" Format="XML">
           0 0 0 1 1 1 18497 1 1
         </DataItem>
       */
      HyperSlab parseHyperSlabNode( const XMLFile &xmfFile, xmlNodePtr node );

      /**
       * Parses hdf5 dataset from node
         <DataItem Dimensions="9 3" Format="HDF"> test.h5:/RESULTS/CellsAll/HydState/0000002 </DataItem>
       */
      HdfDataset parseHdf5Node( const XMLFile &xmfFile, xmlNodePtr node );

      //! Extracts HDF5 filename and HDF5 Path from XML file dirname and fileName:hdfPath syntax
      void hdf5NamePath( const std::string &dataItemPath, std::string &filePath, std::string &hdf5Path );

      //! Parses 2d matrix from text, e.g. 3 2 -> [3, 2]. Verifies that it has 2 items
      std::vector<size_t> parseDimensions2D( const std::string &data );

      MDAL::Mesh *mMesh = nullptr;
      std::string mDatFile;
      std::map< std::string, std::shared_ptr<HdfFile> > mHdfFiles;

  };

} // namespace MDAL
#endif //MDAL_XDMF_HPP
