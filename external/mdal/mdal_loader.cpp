/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_loader.hpp"
#include "frmts/mdal_2dm.hpp"

MDAL::Mesh *MDAL::Loader::load( const std::string &meshFile, Status *status )
{
  MDAL::Loader2dm loader( meshFile );
  return loader.load( status );
}
