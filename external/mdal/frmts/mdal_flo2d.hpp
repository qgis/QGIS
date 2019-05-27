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

namespace MDAL
{
  class DriverFlo2D: public Driver
  {
    public:
      DriverFlo2D();
      ~DriverFlo2D( ) override = default;
      DriverFlo2D *create() override;

      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &resultsFile, MDAL_Status *status ) override;

    private:
      struct CellCenter
      {
        size_t id;
        double x;
        double y;
        std::vector<int> conn; // north, east, south, west cell center index, -1 boundary Vertex
      };

      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mDatFileName;

      void createMesh( const std::vector<CellCenter> &cells, double half_cell_size );
      void parseOUTDatasets( const std::string &datFileName, const std::vector<double> &elevations );
      bool parseHDF5Datasets( const std::string &datFileName );
      void parseVELFPVELOCFile( const std::string &datFileName );
      void parseDEPTHFile( const std::string &datFileName, const std::vector<double> &elevations );
      void parseTIMDEPFile( const std::string &datFileName, const std::vector<double> &elevations );
      void parseFPLAINFile( std::vector<double> &elevations, const std::string &datFileName, std::vector<CellCenter> &cells );
      void parseCADPTSFile( const std::string &datFileName, std::vector<CellCenter> &cells );
      void addStaticDataset( std::vector<double> &vals, const std::string &groupName, const std::string &datFileName );
      static MDAL::Vertex createVertex( size_t position, double half_cell_size, const CellCenter &cell );
      static double calcCellSize( const std::vector<CellCenter> &cells );
  };

} // namespace MDAL
#endif //MDAL_FLO2D_HPP
