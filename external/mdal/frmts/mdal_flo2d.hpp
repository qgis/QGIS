/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_FLO2D_HPP
#define MDAL_FLO2D_HPP

#include <string>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

class HdfGroup;
class HdfFile;

namespace MDAL
{
  /**
   *
   * This driver can be used to read FLO-2D mesh (1D, 2D)
   * The required file for this format are :
   *  - CADPTS.DAT file that contains the cells centre position
   *  - for 2D mesh, FPLAIN.DAT that includes the 2D mesh topology
   *  - for 1D mesh, CHAN.DAT that includes the 1D mesh topology and CHANBANK.DAT that includes the left/right bank relation
   *
   * The 2D mesh is composed of cells which centres are contained in CADPTS.DAT files. FPLAIN.DAT defines the neighboring of the cells.
   * 2D mesh datasets file are not required but can be :
   *  - TIMDEP.hdf5, hdf5 format files
   *  - TIMDEP.OUT
   *  - DEPTH.OUT
   *  - VELFP.OUT
   *
   * hdf5 format extra datasets can also be read, and this driver supports persisting dataset with this format.
   *
   * For 1D mesh, the cell centres are also defined in CADPTS.DAT file.
   * Each vertex is linked to a cell from CADPTS.DAT that represents the position of the left bank, and each cell can be linked only with one left bank.
   * So the topology of the 1D mesh is defined by the id of the left bank of each vertex.
   * Vertices can also have (but not required) a right bank represented by another cell. For the right bank, one cell can be linked to several right banks of vertex, so to several vertices.
   * When a vertex hasn't a right bank, its position is defined by the cell link to the left bank.
   * When vertex has a right bank, the vertex position is the middle of the segment between the right and the left bank.
   *
   * 1D mesh datasets file is not required but is HYCHAN.OUT
   *
   */
  class DriverFlo2D: public Driver
  {
    public:
      DriverFlo2D();
      ~DriverFlo2D( ) override = default;
      DriverFlo2D *create() override;

      bool canReadMesh( const std::string &uri ) override;
      bool canReadDatasets( const std::string &uri ) override;
      std::string buildUri( const std::string &meshFile ) override;

      std::unique_ptr< Mesh > load( const std::string &resultsFile, const std::string &meshName = "" ) override;
      void load( const std::string &uri, Mesh *mesh ) override;
      bool persist( DatasetGroup *group ) override;

    private:
      struct CellCenter
      {
        size_t id;
        double x;
        double y;
      };

      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mDatFileName;

      void createMesh2d( const std::vector<CellCenter> &cells, const MDAL::BBox &cellCenterExtent, double cell_size );
      void createMesh1d( const std::string &datFileName, const std::vector<CellCenter> &cells, std::map<size_t, size_t> &cellsIdToVertex );

      void parseOUTDatasets( const std::string &datFileName, const std::vector<double> &elevations );
      bool parseHDF5Datasets( MDAL::MemoryMesh *mesh, const std::string &timedepFileName );
      void parseVELFPVELOCFile( const std::string &datFileName );
      void parseDEPTHFile( const std::string &datFileName, const std::vector<double> &elevations );
      void parseTIMDEPFile( const std::string &datFileName, const std::vector<double> &elevations );
      void parseFPLAINFile( std::vector<double> &elevations, const std::string &datFileName, std::vector<CellCenter> &cells, double &cellZize );
      void parseCADPTSFile( const std::string &datFileName, std::vector<CellCenter> &cells, BBox &cellCenterExtent );
      void parseCHANBANKFile( const std::string &datFileName, std::map<size_t, size_t> &cellIdToVertices, std::map<size_t, std::vector<size_t> > &duplicatedRightBankToVertex, size_t &verticesCount );
      void parseCHANFile( const std::string &datFileName, const std::map<size_t, size_t> &cellIdToVertices, std::vector<Edge> &edges );
      void parseHYCHANFile( const std::string &datFileName, const std::map<size_t, size_t> &cellIdToVertices );
      void addStaticDataset( std::vector<double> &vals, const std::string &groupName, const std::string &datFileName );
      static MDAL::Vertex createVertex( size_t position, double half_cell_size, const CellCenter &cell );

      std::unique_ptr< Mesh > loadMesh2d();
      std::unique_ptr<Mesh> loadMesh1d();

      // Write API
      bool addToHDF5File( DatasetGroup *group );
      bool saveNewHDF5File( DatasetGroup *group );
      bool appendGroup( HdfFile &file, DatasetGroup *dsGroup, HdfGroup &groupTNOR );
  };

} // namespace MDAL
#endif //MDAL_FLO2D_HPP
