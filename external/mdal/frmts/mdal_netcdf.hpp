/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_NETCDF_HPP
#define MDAL_NETCDF_HPP

#include <string>
#include <vector>

//! C++ Wrapper around netcdf C library
class NetCDFFile
{
  public:
    //! Create file with invalid handle
    NetCDFFile();
    //! Closes the file
    ~NetCDFFile();

    int handle() const;
    void openFile( const std::string &fileName );
    bool hasVariable( const std::string &name ) const;

    std::vector<int> readIntArr( const std::string &name, size_t dim ) const;
    std::vector<double> readDoubleArr( const std::string &name, size_t dim ) const;
    bool hasArr( const std::string &name ) const;
    std::vector<std::string> readArrNames() const;

    int getAttrInt( const std::string &name, const std::string &attr_name ) const;
    double getAttrDouble( int varid, const std::string &attr_name ) const;
    std::string getAttrStr( const std::string &name, const std::string &attr_name ) const;
    std::string getAttrStr( const std::string &name, int varid ) const;
    double getFillValue( int varid ) const;
    int getVarId( const std::string &name );
    void getDimension( const std::string &name, size_t *val, int *ncid_val ) const;
    void getDimensionOptional( const std::string &name, size_t *val, int *ncid_val ) const;
  private:
    int mNcid; // C handle to the file
};

#endif // MDAL_NETCDF_HPP
