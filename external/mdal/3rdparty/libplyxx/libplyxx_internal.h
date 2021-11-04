/*
This file forked from https://github.com/srajotte/libplyxx

Copyright (c) 2016 Simon Rajotte

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Updated (c) 2021 Runette Software Ltd to make multiplatform, to complete the typemaps and add to voxel types.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include "libplyxx.h"
#include "mdal_utils.hpp"

#include <sstream>
#include <iostream>


// a custom specialisation (and yes, you are allowed (and have to) put this in std)
// from http://www.cplusplus.com/forum/general/238538/
namespace std
{
  template<>
  struct hash<libply::Type>
  {
    using argument_type = libply::Type;
    using result_type = int;

    result_type operator()( argument_type a ) const
    {
      return static_cast<result_type>( a );
    }
  };
}

namespace libply
{
  typedef std::unordered_map<std::string, Type> TypeMap;
  const TypeMap TYPE_MAP =
  {
    { "char", Type::INT8 },
    { "uchar", Type::UINT8 },
    { "short", Type::INT16 },
    { "ushort", Type::UINT16},
    { "int", Type::INT32 },
    { "uint", Type::UINT32},
    { "float", Type::FLOAT32 },
    { "double", Type::FLOAT64 },
    { "int8", Type::INT8 },
    { "uint8", Type::UINT8 },
    { "int16", Type::INT16 },
    { "uint16", Type::UINT16},
    { "int32", Type::INT32 },
    { "uint32", Type::UINT32},
    { "float32", Type::FLOAT32 },
    { "float64", Type::FLOAT64 }
  };

  typedef std::unordered_map<Type, unsigned int> TypeSizeMap;
  const TypeSizeMap TYPE_SIZE_MAP =
  {
    { Type::INT8, 1 },
    { Type::UINT8, 1},
    { Type::INT16, 2},
    { Type::UINT16, 2},
    { Type::INT32, 4},
    { Type::UINT32, 4},
    { Type::FLOAT32, 4},
    { Type::FLOAT64, 8},
    { Type::COORDINATE, 8}
  };

  /// Type conversion functions.

  inline void convert_UINT( const textio::SubString &token, IProperty &property )
  {
    property = textio::stoi<int>( token );
  }

  inline void convert_INT( const textio::SubString &token, IProperty &property )
  {
    property = textio::stoi<int>( token );
  }

  inline void convert_FLOAT( const textio::SubString &token, IProperty &property )
  {
    property = textio::stor<float>( token );
  }

  inline void convert_DOUBLE( const textio::SubString &token, IProperty &property )
  {
    property = textio::stor<double>( token );
  }

  typedef void( *ConversionFunction )( const textio::SubString &, IProperty & );
  typedef std::unordered_map<Type, ConversionFunction> ConversionFunctionMap;

  const ConversionFunctionMap CONVERSION_MAP =
  {
    { Type::INT8, convert_INT },
    { Type::UINT8, convert_UINT },
    { Type::INT16, convert_INT },
    { Type::UINT16, convert_UINT },
    { Type::INT32, convert_INT },
    { Type::UINT32, convert_UINT },
    { Type::FLOAT32, convert_FLOAT },
    { Type::FLOAT64, convert_DOUBLE },
    { Type::COORDINATE, convert_DOUBLE }
  };

  /// Type casting functions.

  inline void cast_UINT8( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<unsigned char *>( buffer );
  }

  inline void cast_INT8( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast< char *>( buffer );
  }

  inline void cast_UINT16( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<unsigned short *>( buffer );
  }

  inline void cast_INT16( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<short *>( buffer );
  }

  inline void cast_UINT32( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<unsigned int *>( buffer );
  }

  inline void cast_INT32( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<int *>( buffer );
  }

  inline void cast_FLOAT( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<float *>( buffer );
  }

  inline void cast_DOUBLE( char *buffer, IProperty &property )
  {
    property = *reinterpret_cast<double *>( buffer );
  }

  typedef void( *CastFunction )( char *buffer, IProperty & );
  typedef std::unordered_map<Type, CastFunction> CastFunctionMap;

  const CastFunctionMap CAST_MAP =
  {
    { Type::INT8, cast_INT8 },
    { Type::UINT8, cast_UINT8 },
    { Type::INT16, cast_INT16 },
    { Type::UINT16, cast_UINT16 },
    { Type::INT32, cast_INT32 },
    { Type::UINT32, cast_UINT32 },
    { Type::FLOAT32, cast_FLOAT },
    { Type::FLOAT64, cast_DOUBLE },
    { Type::COORDINATE, cast_DOUBLE }
  };

  inline std::stringstream &write_convert_UINT( IProperty &property, std::stringstream &ss )
  {
    ss << std::to_string( static_cast<unsigned int>( property ) );
    return ss;
  }

  inline std::stringstream &write_convert_INT( IProperty &property, std::stringstream &ss )
  {
    ss << std::to_string( static_cast<int>( property ) );
    return ss;
  }

  inline std::stringstream &write_convert_FLOAT( IProperty &property, std::stringstream &ss )
  {
    ss << std::to_string( static_cast<float>( property ) );
    return ss;
  }

  inline std::stringstream &write_convert_DOUBLE( IProperty &property, std::stringstream &ss )
  {
    ss << MDAL::doubleToString( static_cast<double>( property ) );
    return ss;
  }

  inline std::stringstream &write_convert_COORDINATE( IProperty &property, std::stringstream &ss )
  {
    ss << MDAL::coordinateToString( static_cast<double>( property ) );
    return ss;
  }

  typedef std::stringstream &( *WriteConvertFunction )( IProperty &, std::stringstream & );
  typedef std::unordered_map<Type, WriteConvertFunction> WriteConvertFunctionMap;

  const WriteConvertFunctionMap WRITE_CONVERT_MAP =
  {
    { Type::INT8, write_convert_INT },
    { Type::UINT8, write_convert_UINT },
    { Type::INT16, write_convert_INT },
    { Type::UINT16, write_convert_UINT },
    { Type::INT32, write_convert_INT },
    { Type::UINT32, write_convert_UINT },
    { Type::FLOAT32, write_convert_FLOAT },
    { Type::FLOAT64, write_convert_DOUBLE },
    { Type::COORDINATE, write_convert_COORDINATE }
  };

  inline void write_cast_UINT( IProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<unsigned int *>( buffer ) = static_cast<unsigned int>( property );
    size = sizeof( unsigned char );
  }

  inline void write_cast_INT( IProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<int *>( buffer ) = static_cast<int>( property );
    size = sizeof( int );
  }

  inline void write_cast_FLOAT( IProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<float *>( buffer ) = static_cast<float>( property );
    size = sizeof( float );
  }

  inline void write_cast_DOUBLE( IProperty &property, char *buffer, size_t &size )
  {
    *reinterpret_cast<double *>( buffer ) = static_cast<double>( property );
    size = sizeof( double );
  }

  typedef void( *WriteCastFunction )( IProperty &property, char *buffer, size_t &size );
  typedef std::unordered_map<Type, WriteCastFunction> WriteCastFunctionMap;

  const WriteCastFunctionMap WRITE_CAST_MAP =
  {
    { Type::INT8, write_cast_INT },
    { Type::UINT8, write_cast_UINT },
    { Type::INT16, write_cast_INT },
    { Type::UINT16, write_cast_UINT },
    { Type::INT32, write_cast_INT },
    { Type::UINT32, write_cast_UINT },
    { Type::FLOAT32, write_cast_FLOAT },
    { Type::FLOAT64, write_cast_DOUBLE },
    { Type::COORDINATE, write_cast_DOUBLE }
  };

  struct PropertyDefinition
  {
    PropertyDefinition( const std::string &name, Type type, bool isList, Type listLengthType = Type::UINT8 )
      : name( name ), type( type ), isList( isList ), listLengthType( listLengthType ),
        conversionFunction( CONVERSION_MAP.at( type ) ),
        castFunction( CAST_MAP.at( type ) ),
        writeConvertFunction( WRITE_CONVERT_MAP.at( type ) ),
        writeCastFunction( WRITE_CAST_MAP.at( type ) )
    {};
    PropertyDefinition( const Property &p )
      : PropertyDefinition( p.name, p.type, p.isList )
    {};

    Property getProperty() const;

    std::string name;
    Type type;
    bool isList;
    Type listLengthType;
    ConversionFunction conversionFunction;
    CastFunction castFunction;
    WriteConvertFunction writeConvertFunction;
    WriteCastFunction writeCastFunction;
  };

  struct ElementDefinition
  {
    ElementDefinition() : ElementDefinition( "", 0, 0 ) {};
    ElementDefinition( const std::string &name, ElementSize size, std::size_t startLine )
      : name( name ), size( size ), startLine( startLine ) {};
    ElementDefinition( const Element &e )
      : name( e.name ), size( e.size )
    {
      for ( const auto &p : e.properties )
      {
        properties.emplace_back( p );
      }
    };

    Element getElement() const;

    std::string name;
    ElementSize size;
    std::vector<PropertyDefinition> properties;
    std::size_t startLine;
  };

  class FileParser
  {
    public:
      explicit FileParser( const std::string &filename );
      FileParser( const FileParser &other ) = delete;
      ~FileParser();

      std::vector<Element> definitions() const;
      Metadata metadata;
      //void setElementInserter(std::string elementName, IElementInserter* inserter);
      void setElementReadCallback( std::string elementName, ElementReadCallback &readCallback );
      void read();

    private:
      void readHeader();
      void parseLine( const textio::SubString &substr, const ElementDefinition &elementDefinition, ElementBuffer &buffer );
      void readBinaryElement( std::ifstream &fs, const ElementDefinition &elementDefinition, ElementBuffer &buffer );

    private:
      typedef std::map<std::string, ElementReadCallback> CallbackMap;

    private:
      std::string m_filename;
      File::Format m_format;
      std::streamsize m_dataOffset;
      textio::LineReader m_lineReader;
      textio::Tokenizer m_lineTokenizer;
      textio::Tokenizer::TokenList m_tokens;
      std::vector<ElementDefinition> m_elements;
      CallbackMap m_readCallbackMap;
  };

  std::string formatString( File::Format format );
  std::string typeString( Type type );
}
