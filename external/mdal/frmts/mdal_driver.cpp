/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include "mdal_driver.hpp"
#include "mdal_utils.hpp"

MDAL::Driver::Driver( const std::string &name,
                      const std::string &longName,
                      const std::string &filters,
                      DriverType type )
  : mName( name )
  , mLongName( longName )
  , mFilters( filters )
  , mType( type )
{

}

MDAL::Driver::~Driver() = default;

std::string MDAL::Driver::name() const
{
  return mName;
}

std::string MDAL::Driver::longName() const
{
  return mLongName;
}

std::string MDAL::Driver::filters() const
{
  return mFilters;
}

MDAL::DriverType MDAL::Driver::type() const
{
  return mType;
}

std::unique_ptr< MDAL::Mesh > MDAL::Driver::load( const std::string &uri, MDAL_Status *status )
{
  MDAL_UNUSED( uri );
  MDAL_UNUSED( status );
  return std::unique_ptr< MDAL::Mesh >();
}

void MDAL::Driver::load( const std::string &uri, Mesh *mesh, MDAL_Status *status )
{
  MDAL_UNUSED( uri );
  MDAL_UNUSED( mesh );
  MDAL_UNUSED( status );
  return;
}
