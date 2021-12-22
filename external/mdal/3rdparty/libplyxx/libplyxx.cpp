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

#include "libplyxx_internal.h"
#include "mdal.h"

#include <fstream>
#include <string>
#include <iostream>

namespace libply
{
  File::File( const std::string &filename )
    : m_filename( filename ),
      m_parser( std::make_unique<FileParser>( filename ) )
  {
  }

  File::~File() = default;

  std::vector<Element> File::definitions() const
  {
    return m_parser->definitions();
  }

  Metadata File::metadata() const
  {
    return m_parser->metadata;
  }


  void File::setElementReadCallback( std::string elementName, ElementReadCallback &readCallback )
  {
    m_parser->setElementReadCallback( elementName, readCallback );
  }

  void File::read()
  {
    m_parser->read();
  }

  void addElementDefinition( const textio::Tokenizer::TokenList &tokens, std::vector<ElementDefinition> &elementDefinitions )
  {
    assert( std::string( tokens.at( 0 ) ) == "element" );
    if ( tokens.size() != 3 || tokens.at( 2 ).size() == 0 )
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "PLY: Invalid Element Definition" );
      elementDefinitions.emplace_back( );
    }
    else
    {
      size_t startLine = 0;
      if ( !elementDefinitions.empty() )
      {
        const auto &previousElement = elementDefinitions.back();
        startLine = previousElement.startLine + previousElement.size;
      }
      ElementSize elementCount = std::stoul( tokens.at( 2 ) );
      elementDefinitions.emplace_back( tokens.at( 1 ), elementCount, startLine );
    }
  }

  void addProperty( const textio::Tokenizer::TokenList &tokens, ElementDefinition &elementDefinition )
  {
    auto &properties = elementDefinition.properties;
    if ( std::string( tokens.at( 1 ) ) == "list" )
    {
      if ( tokens.size() != 5 )
      {
        MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, std::string( "PLY: Invalid Property Definition : " ).append( textio::Tokenizer::toString( tokens ) ).c_str() );
      }
      else
      {
        properties.emplace_back( tokens.back(), TYPE_MAP.at( tokens.at( 3 ) ), true, TYPE_MAP.at( tokens.at( 2 ) ) );
      }
    }
    else
    {
      if ( tokens.size() != 3 )
      {
        MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, std::string( "PLY: Invalid Property Definition : " ).append( textio::Tokenizer::toString( tokens ) ).c_str() );
      }
      else
      {
        properties.emplace_back( tokens.back(), TYPE_MAP.at( tokens.at( 1 ) ), false );
      }
    }
  }

  void addMetadata( const textio::SubString line, Metadata &metadata )
  {
    textio::SubString::const_iterator next = textio::find( line.begin(), line.end(), ' ' );
    assert( std::string( textio::SubString( line.begin(), next ) ) == "comment" );
    line.begin() = next;
    next++;
    textio::SubString::const_iterator comment = textio::find( next, line.end(), ':' );
    if ( comment != line.end() )
    {
      metadata.emplace( std::string( textio::SubString( next, comment ) ), std::string( textio::SubString( comment + 1, line.end() ) ) );
    }
    else
    {
      int idx = 1;
      while ( idx < 100 )
      {
        std::string keyword = "comment" + std::to_string( idx );
        if ( metadata.find( keyword ) ==  metadata.end() )
        {
          metadata.emplace( keyword, std::string( textio::SubString( next, line.end() ) ) );
          break;
        }
        idx++;
      }
    }
  }

  Property PropertyDefinition::getProperty() const
  {
    return Property( name, type, isList );
  }

  Element ElementDefinition::getElement() const
  {
    std::vector<Property> props;
    for ( const PropertyDefinition &p : properties )
    {
      props.emplace_back( p.getProperty() );
    }
    return Element( name, size, props );
  }

  FileParser::FileParser( const std::string &filename )
    : m_filename( filename ),
      m_lineReader( filename ),
      m_lineTokenizer( ' ' )
  {
    readHeader();
  }

  FileParser::~FileParser() = default;

  std::vector<Element> FileParser::definitions() const
  {
    std::vector<Element> elements;
    for ( const auto &e : m_elements )
    {
      elements.emplace_back( e.getElement() );
    }
    return elements;
  }

  void FileParser::readHeader()
  {
    // Read PLY magic number
    std::string line = m_lineReader.getline();
    if ( line != "ply" )
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_UnknownFormat, "Invalid file format." );
      return;
    }

    // Read file format.
    line = m_lineReader.getline();
    if ( line == "format ascii 1.0" )
    {
      m_format = File::Format::ASCII;
    }
    else if ( line == "format binary_little_endian 1.0" )
    {
      m_format = File::Format::BINARY_LITTLE_ENDIAN;
    }
    else if ( line == "format binary_big_endian 1.0" )
    {
      m_format = File::Format::BINARY_BIG_ENDIAN;
    }
    else
    {
      MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_UnknownFormat, "Unsupported PLY format" );
      return;
    }

    // Read mesh elements properties.
    textio::SubString line_substring;
    line_substring = m_lineReader.getline();
    line = line_substring;
    textio::Tokenizer spaceTokenizer( ' ' );
    auto tokens = spaceTokenizer.tokenize( line );
    while ( std::string( tokens.at( 0 ) ) != "end_header" )
    {
      const std::string lineType = tokens.at( 0 );
      if ( lineType == "element" )
      {
        addElementDefinition( tokens, m_elements );
      }
      else if ( lineType == "property" )
      {
        addProperty( tokens, m_elements.back() );
      }
      else if ( lineType == "comment" )
      {
        addMetadata( line_substring, metadata );
      }
      else
      {
        MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_UnknownFormat, "Invalid header line." );
        return;
      }

      line_substring = m_lineReader.getline();
      line = line_substring;
      tokens = spaceTokenizer.tokenize( line );
    }

    m_dataOffset = m_lineReader.position( line_substring.end() ) + 1;
  }

  void FileParser::setElementReadCallback( std::string elementName, ElementReadCallback &callback )
  {
    m_readCallbackMap[elementName] = callback;
  }

  void FileParser::read()
  {
    std::size_t totalLines = 0;
    for ( auto &e : m_elements )
    {
      totalLines += e.size;
    }

    std::vector<std::shared_ptr<ElementBuffer>> buffers;
    for ( auto &e : m_elements )
    {
      buffers.emplace_back( std::make_shared<ElementBuffer>( e ) );
    }

    std::size_t lineIndex = 0;
    std::size_t elementIndex = 0;
    ElementReadCallback readCallback = m_readCallbackMap.at( m_elements.at( elementIndex ).name );
    auto &elementDefinition = m_elements.at( elementIndex );
    const std::size_t maxElementIndex = m_elements.size();

    std::shared_ptr<ElementBuffer> buffer = buffers[elementIndex];

    std::ifstream &filestream = m_lineReader.filestream();

    if ( m_format == File::Format::BINARY_BIG_ENDIAN || m_format == File::Format::BINARY_LITTLE_ENDIAN )
    {
      filestream.clear();
      filestream.seekg( m_dataOffset );
    }

    while ( lineIndex < totalLines )
    {
      const auto nextElementIndex = elementIndex + 1;
      if ( nextElementIndex < maxElementIndex && lineIndex >= m_elements[nextElementIndex].startLine )
      {
        elementIndex = nextElementIndex;
        readCallback = m_readCallbackMap.at( m_elements.at( elementIndex ).name );
        elementDefinition = m_elements.at( elementIndex );

        buffer = buffers[elementIndex];
      }

      if ( m_format == File::Format::ASCII )
      {
        auto line = m_lineReader.getline();
        parseLine( line, elementDefinition, *buffer );
      }
      else
      {
        readBinaryElement( filestream, elementDefinition, *buffer );
      }

      readCallback( *buffer );
      ++lineIndex;
    }
  }

  void FileParser::parseLine( const textio::SubString &line, const ElementDefinition &elementDefinition, ElementBuffer &elementBuffer )
  {
    m_lineTokenizer.tokenize( line, m_tokens );
    const std::vector<PropertyDefinition> properties = elementDefinition.properties;
    size_t t_idx = 0;
    size_t e_idx = 0;
    for ( PropertyDefinition p : properties )
    {
      if ( t_idx == m_tokens.size() ||  e_idx == elementBuffer.size() )
      {
        MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "Incomplete Element" );
        return;
      }
      if ( !p.isList )
      {
        p.conversionFunction( m_tokens[t_idx], elementBuffer[e_idx] );
        t_idx++;
        e_idx++;
      }
      else
      {
        const size_t listLength = std::stoi( m_tokens[t_idx] );
        t_idx++;
        ListProperty *lp = dynamic_cast<ListProperty *>( &elementBuffer[e_idx] );
        lp->define( p.type, listLength );
        for ( size_t i = 0; i < listLength; i++ )
        {
          if ( t_idx == m_tokens.size() )
          {
            MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "Incomplete Element" );
            return;
          }
          p.conversionFunction( m_tokens[t_idx], lp->value( i ) );
          t_idx++;
        }
        e_idx++;
      }
    }
  }

  void FileParser::readBinaryElement( std::ifstream &fs, const ElementDefinition &elementDefinition, ElementBuffer &elementBuffer )
  {
    const auto &properties = elementDefinition.properties;
    const unsigned int MAX_PROPERTY_SIZE = 8;
    char buffer[MAX_PROPERTY_SIZE];
    size_t e_idx = 0;

    for ( PropertyDefinition p : properties )
    {
      if ( !p.isList )
      {
        if ( e_idx == elementBuffer.size() ) return; //TODO throw an error
        const auto size = TYPE_SIZE_MAP.at( p.type );
        fs.read( buffer, size );
        p.castFunction( buffer, elementBuffer[e_idx] );
        e_idx++;
      }
      else
      {
        if ( e_idx == elementBuffer.size() ) return; //TODO throw an error
        const auto lengthType = p.listLengthType;
        const auto lengthTypeSize = TYPE_SIZE_MAP.at( lengthType );
        fs.read( buffer, lengthTypeSize );
        size_t listLength = static_cast<size_t>( *buffer );

        ListProperty *lp = dynamic_cast<ListProperty *>( &elementBuffer[e_idx] );
        lp->define( p.type, listLength );

        const auto &castFunction = p.castFunction;
        const auto size = TYPE_SIZE_MAP.at( p.type );
        for ( size_t i = 0; i < listLength; i++ )
        {
          fs.read( buffer, size );
          castFunction( buffer, lp->value( i ) );
        }
        e_idx++;
      }
    }
  }

  ElementBuffer::ElementBuffer( const ElementDefinition &definition )
  {
    auto &properties = definition.properties;
    for ( auto &p : properties )
    {
      if ( p.isList )
      {
        appendListProperty( p.type );
      }
      else
      {
        appendScalarProperty( p.type );
      }
    }

  }

  IProperty &ElementBuffer::operator[]( size_t index )
  {
    return *properties[index];
  }

  void ElementBuffer::appendScalarProperty( Type type )
  {
    std::unique_ptr<IProperty> prop = getScalarProperty( type );
    properties.push_back( std::move( prop ) );
  }

  void ElementBuffer::appendListProperty( Type )
  {
    std::unique_ptr<IProperty> prop = std::make_unique<ListProperty>();
    properties.push_back( std::move( prop ) );
  }

  std::unique_ptr<IProperty> ElementBuffer::getScalarProperty( Type type )
  {
    std::unique_ptr<IProperty> prop;
    switch ( type )
    {
      case Type::INT8:
        prop = std::make_unique<ScalarProperty<char>>();
        break;
      case Type::UINT8:
        prop = std::make_unique<ScalarProperty<unsigned char>>();
        break;
      case Type::INT16:
        prop = std::make_unique<ScalarProperty<short>>();
        break;
      case Type::UINT16:
        prop = std::make_unique<ScalarProperty<unsigned short>>();
        break;
      case Type::UINT32:
        prop = std::make_unique<ScalarProperty<unsigned int>>();
        break;
      case Type::INT32:
        prop = std::make_unique<ScalarProperty<int>>();
        break;
      case Type::FLOAT32:
        prop = std::make_unique<ScalarProperty<float>>();
        break;
      case Type::FLOAT64:
        prop = std::make_unique<ScalarProperty<double>>();
        break;
      case Type::COORDINATE:
        prop = std::make_unique<ScalarProperty<double>>();
        break;
    }
    return prop;
  }

  std::string formatString( File::Format format )
  {
    switch ( format )
    {
      case File::Format::ASCII: return "ascii";
      case File::Format::BINARY_BIG_ENDIAN: return "binary_big_endian";
      case File::Format::BINARY_LITTLE_ENDIAN: return "binary_little_endian";
    }
    return "";
  }

  std::string typeString( Type type )
  {
    switch ( type )
    {
      case Type::INT8: return "char";
      case Type::UINT8: return "uchar";
      case Type::INT16: return "short";
      case Type::UINT16: return "ushort";
      case Type::UINT32: return "uint";
      case Type::INT32: return "int";
      case Type::FLOAT32: return "float";
      case Type::FLOAT64: return "double";
      case Type::COORDINATE: return "double";
    }
    return "";
  }

  void writePropertyDefinition( std::ofstream &file, const Property &propertyDefinition )
  {
    if ( propertyDefinition.isList )
    {
      file << "property list uchar ";
    }
    else
    {
      file << "property ";
    }
    file << typeString( propertyDefinition.type ) << " " << propertyDefinition.name << '\n';
  }

  void writeElementDefinition( std::ofstream &file, const Element &elementDefinition )
  {
    file << "element " << elementDefinition.name << " " << elementDefinition.size << '\n';
    for ( const auto &prop : elementDefinition.properties )
    {
      writePropertyDefinition( file, prop );
    }
  }

  void writeTextProperties( std::ofstream &file, ElementBuffer &buffer, const ElementDefinition &elementDefinition )
  {
    std::stringstream ss;
    const std::vector<PropertyDefinition> properties = elementDefinition.properties;
    size_t e_idx = 0;
    for ( PropertyDefinition p : properties )
    {
      if ( !p.isList )
      {
        auto &convert = p.writeConvertFunction;
        ss.clear();
        ss.str( std::string() );
        file << convert( buffer[e_idx], ss ).str() << " ";
        e_idx++;
      }
      else
      {
        auto &convert = p.writeConvertFunction;
        ListProperty *lp = dynamic_cast<ListProperty *>( &buffer[e_idx] );
        file << lp->size() << " ";
        for ( size_t i = 0; i < lp->size(); i++ )
        {
          ss.clear();
          ss.str( std::string() );
          file << convert( lp->value( i ), ss ).str() << " ";
        }
        e_idx++;
        for ( size_t i = 0; i < buffer.size(); ++i )
        {

        }
      }
    }
    file << '\n';
  }

  void writeBinaryProperties( std::ofstream &file, ElementBuffer &buffer, const ElementDefinition &elementDefinition )
  {
    const unsigned int MAX_PROPERTY_SIZE = 8;
    char write_buffer[MAX_PROPERTY_SIZE];

    if ( elementDefinition.properties.front().isList )
    {
      unsigned char list_size = static_cast<unsigned char>( buffer.size() );
      file.write( reinterpret_cast<char *>( &list_size ), sizeof( list_size ) );

      auto &cast = elementDefinition.properties.front().writeCastFunction;
      for ( size_t i = 0; i < buffer.size(); ++i )
      {
        size_t write_size;
        cast( buffer[i], write_buffer, write_size );
        file.write( reinterpret_cast<char *>( write_buffer ), write_size );
      }
    }
    else
    {
      for ( size_t i = 0; i < buffer.size(); ++i )
      {
        auto &cast = elementDefinition.properties.at( i ).writeCastFunction;
        size_t write_size;
        cast( buffer[i], write_buffer, write_size );
        file.write( reinterpret_cast<char *>( write_buffer ), write_size );
      }
    }
  }

  void writeProperties( std::ofstream &file, ElementBuffer &buffer, size_t index, const ElementDefinition &elementDefinition, File::Format format, ElementWriteCallback &callback )
  {
    callback( buffer, index );
    if ( format == File::Format::ASCII )
    {
      writeTextProperties( file, buffer, elementDefinition );
    }
    else
    {
      writeBinaryProperties( file, buffer, elementDefinition );
    }
  }

  void writeElements( std::ofstream &file, const Element &elementDefinition, File::Format format, ElementWriteCallback &callback )
  {
    const size_t size = elementDefinition.size;
    ElementBuffer buffer( elementDefinition );
    //buffer.reset( elementDefinition.properties.size() );
    for ( size_t i = 0; i < size; ++i )
    {
      writeProperties( file, buffer, i, elementDefinition, format, callback );
    }
  }

  FileOut::FileOut( const std::string &filename, File::Format format )
    : m_filename( filename ), m_format( format )
  {
    createFile();
  }

  void FileOut::setElementsDefinition( const ElementsDefinition &definitions )
  {
    m_definitions = definitions;
  }

  void FileOut::setElementWriteCallback( const std::string &elementName, ElementWriteCallback &writeCallback )
  {
    m_writeCallbacks[elementName] = writeCallback;
  }

  void FileOut::write()
  {
    writeHeader();
    writeData();
  }

  void FileOut::createFile()
  {
    std::ofstream f( m_filename, std::ios::trunc );
    f.close();
  }

  void FileOut::writeHeader()
  {
    std::ofstream file( m_filename, std::ios::out | std::ios::binary );

    file << "ply" << std::endl;
    file << "format " << formatString( m_format ) << " 1.0" << std::endl;
    for ( const auto &def : m_definitions )
    {
      writeElementDefinition( file, def );
    }
    file << "end_header" << std::endl;

    file.close();
  }

  void FileOut::writeData()
  {
    std::ofstream file( m_filename, std::ios::out | std::ios::binary | std::ios::app );
    for ( const auto &elem : m_definitions )
    {
      writeElements( file, elem, m_format, m_writeCallbacks[elem.name] );
    }
    file.close();
  }

  IProperty &ListProperty::value( size_t index )
  {
    return *list[index];
  }

  void ListProperty::define( Type type, size_t isize )
  {
    list.clear();
    for ( size_t i = 0; i < isize; i++ )
    {
      std::unique_ptr<IProperty> prop = getScalarProperty( type );
      list.push_back( std::move( prop ) );
    }
  }

  std::unique_ptr<IProperty> ListProperty::getScalarProperty( Type type )
  {
    std::unique_ptr<IProperty> prop;
    switch ( type )
    {
      case Type::INT8:
        prop = std::make_unique<ScalarProperty<char>>();
        break;
      case Type::UINT8:
        prop = std::make_unique<ScalarProperty<unsigned char>>();
        break;
      case Type::INT16:
        prop = std::make_unique<ScalarProperty<short>>();
        break;
      case Type::UINT16:
        prop = std::make_unique<ScalarProperty<unsigned short>>();
        break;
      case Type::UINT32:
        prop = std::make_unique<ScalarProperty<unsigned int>>();
        break;
      case Type::INT32:
        prop = std::make_unique<ScalarProperty<int>>();
        break;
      case Type::FLOAT32:
        prop = std::make_unique<ScalarProperty<float>>();
        break;
      case Type::FLOAT64:
        prop = std::make_unique<ScalarProperty<double>>();
        break;
      case Type::COORDINATE:
        prop = std::make_unique<ScalarProperty<double>>();
        break;
    }
    return prop;
  }

}
