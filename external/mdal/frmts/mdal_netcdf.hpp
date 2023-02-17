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
    void openFile( const std::string &fileName, bool write = false );

    std::vector<int> readIntArr( const std::string &name, size_t dim ) const;
    /** Reads hyperslap from double variable - 2D array */
    std::vector<int> readIntArr( int arr_id,
                                 size_t start_dim1,
                                 size_t start_dim2,
                                 size_t count_dim1,
                                 size_t count_dim2
                               ) const;

    /** Reads hyperslap from double variable - 1D array */
    std::vector<int> readIntArr( int arr_id,
                                 size_t start_dim,
                                 size_t count_dim
                               ) const;

    std::vector<double> readDoubleArr( const std::string &name, size_t dim ) const;

    /** Reads hyperslap from double variable - 2D array */
    std::vector<double> readDoubleArr( int arr_id,
                                       size_t start_dim1,
                                       size_t start_dim2,
                                       size_t count_dim1,
                                       size_t count_dim2
                                     ) const;

    /** Reads hyperslap from double variable - 1D array */
    std::vector<double> readDoubleArr( int arr_id,
                                       size_t start_dim,
                                       size_t count_dim
                                     ) const;

    bool hasArr( const std::string &name ) const;
    int arrId( const std::string &name ) const;

    std::vector<std::string> readArrNames() const;

    bool hasAttrInt( const std::string &name, const std::string &attr_name ) const;
    int getAttrInt( const std::string &name, const std::string &attr_name ) const;
    bool hasAttrDouble( int varid, const std::string &attr_name ) const;
    double getAttrDouble( int varid, const std::string &attr_name ) const;
    /**
     * Get string attribute
     * \param name name of the variable
     * \param attr_name name of the attribute of the variable
     * \returns empty string if attribute is missing, else attribute value
     */
    std::string getAttrStr( const std::string &name, const std::string &attr_name ) const;
    std::string getAttrStr( const std::string &attr_name, int varid ) const;

    double getFillValue( int varid ) const;
    void setFillValue( int varid, double fillValue );
    int getVarId( const std::string &name );
    void getDimension( const std::string &name, size_t *val, int *ncid_val ) const;
    void getDimensions( const std::string &variableName, std::vector<size_t> &dimensionsId, std::vector<int> &dimensionIds );
    bool hasDimension( const std::string &name ) const;

    void createFile( const std::string &fileName );
    int defineDimension( const std::string &name, size_t size );
    int defineVar( const std::string &varName, int ncType, int dimensionCount, const int *dimensions );
    void putAttrStr( int varId, const std::string &attrName, const std::string &value );
    void putAttrInt( int varId, const std::string &attrName, int value );
    void putAttrDouble( int varId, const std::string &attrName, double value );
    void putDataDouble( int varId, const size_t index, const double value );
    void putDataArrayDouble( int varId, const size_t index, const std::vector<double> &values );
    void putDataArrayInt( int varId, size_t line, size_t faceVerticesMax, int *values );

    std::string getFileName() const;

  private:
    int mNcid; // C handle to the file
    std::string mFileName;
};

#endif // MDAL_NETCDF_HPP
