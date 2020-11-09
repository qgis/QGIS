/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd
*/

#ifndef MDAL_PLY_HPP
#define MDAL_PLY_HPP

#include <string>
#include <memory>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"


namespace MDAL
{
  /**
   * PLY format specification : http://gamma.cs.unc.edu/POWERPLANT/papers/ply.pdf
   */
  class DriverPly : public Driver
  {
    public:
      DriverPly();
      ~DriverPly() override;
      DriverPly *create() override;

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;

    private:
      std::shared_ptr<DatasetGroup> addDatasetGroup( MDAL::Mesh *mesh, const std::string &name, const MDAL_DataLocation location, bool isScalar );
      void addDataset( MDAL::DatasetGroup *group, const std::vector<double> &values );

      /*
      * Element specification. holds the name, size and arbitraily long vector of properties
      */
      struct element
      {
        public:
          std::string name; // element name
          std::vector<std::string> properties; // the name of each property
          std::vector<std::string> types; // the type of each property
          std::vector<bool> list; // is the property a list
          size_t size; // element size

          bool operator==( const std::string &rhs ) const
          {
            return name == rhs;
          }
      };

      size_t getIndex( std::vector<std::string> v, std::string in );
  };

} // namespace MDAL
#endif //MDAL_PLY_HPP
