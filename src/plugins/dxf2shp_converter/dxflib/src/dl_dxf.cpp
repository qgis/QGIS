/****************************************************************************
** $Id: dl_dxf.cpp 2719 2005-09-24 20:41:23Z andrew $
**
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This program is free software; you can redistribute it and/or modify  
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; version 2 of the License
**
** Licensees holding valid dxflib Professional Edition licenses may use
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "dl_dxf.h"

#include <algorithm>
#include <string>
#include <cstdio>
#include <cassert>
#include <cmath>

#include "dl_attributes.h"
#include "dl_codes.h"
#include "dl_creationinterface.h"
#include "dl_writer_ascii.h"
#include "qgslogger.h"


/**
 * Default constructor.
 */
DL_Dxf::DL_Dxf()
{
  styleHandleStd = 0;
  version = VER_2000;

  vertices = NULL;
  maxVertices = 0;
  vertexIndex = 0;

  knots = NULL;
  maxKnots = 0;
  knotIndex = 0;

  controlPoints = NULL;
  maxControlPoints = 0;
  controlPointIndex = 0;

  leaderVertices = NULL;
  maxLeaderVertices = 0;
  leaderVertexIndex = 0;

  hatchLoops = NULL;
  maxHatchLoops = 0;
  hatchLoopIndex = -1;
  hatchEdges = NULL;
  maxHatchEdges = NULL;
  hatchEdgeIndex = NULL;
  dropEdges = false;

  //bulge = 0.0;
}



/**
 * Destructor.
 */
DL_Dxf::~DL_Dxf()
{
  if ( vertices != NULL )
  {
    delete[] vertices;
  }
  if ( knots != NULL )
  {
    delete[] knots;
  }
  if ( controlPoints != NULL )
  {
    delete[] controlPoints;
  }
  if ( leaderVertices != NULL )
  {
    delete[] leaderVertices;
  }
  if ( hatchLoops != NULL )
  {
    delete[] hatchLoops;
  }
  if ( hatchEdges != NULL )
  {
    for ( int i = 0; i < maxHatchLoops; ++i )
    {
      if ( hatchEdges[i] != NULL )
      {
        delete[] hatchEdges[i];
      }
    }
    delete[] hatchEdges;
  }
  if ( maxHatchEdges != NULL )
  {
    delete[] maxHatchEdges;
  }
  if ( hatchEdgeIndex != NULL )
  {
    delete[] hatchEdgeIndex;
  }
}



/**
 * @brief Reads the given file and calls the appropriate functions in
 * the given creation interface for every entity found in the file.
 *
 * @param file Input
 *  Path and name of file to read
 * @param creationInterface
 *  Pointer to the class which takes care of the entities in the file.
 *
 * @retval true If \p file could be opened.
 * @retval false If \p file could not be opened.
 */
bool DL_Dxf::in( const string& file, DL_CreationInterface* creationInterface )
{
  FILE *fp;
  firstCall = true;
  currentEntity = DL_Unknown;
  int errorCounter = 0;

  fp = fopen( file.c_str(), "rt" );
  if ( fp )
  {
    while ( readDxfGroups( fp, creationInterface, &errorCounter ) ) {}
    fclose( fp );
    if ( errorCounter > 0 )
    {
      QgsDebugMsg( QString( "DXF Filter: There have been %1 errors. The drawing might be incomplete / incorrect." ).arg( errorCounter ) );
    }
    return true;
  }

  return false;
}



/**
 * Reads a DXF file from an existing stream.
 *
 * @param stream The string stream.
 * @param creationInterface
 *  Pointer to the class which takes care of the entities in the file.
 *
 * @retval true If \p file could be opened.
 * @retval false If \p file could not be opened.
 */
#ifndef __GCC2x__
bool DL_Dxf::in( std::stringstream& stream,
                 DL_CreationInterface* creationInterface )
{

  int errorCounter = 0;

  if ( stream.good() )
  {
    firstCall = true;
    currentEntity = DL_Unknown;
    while ( readDxfGroups( stream, creationInterface, &errorCounter ) ) {}
    if ( errorCounter > 0 )
    {
      QgsDebugMsg( QString( "DXF Filter: There have been %1 errors. The drawing might be incomplete / incorrect." ).arg( errorCounter ) );
    }
    return true;
  }
  return false;
}
#endif



/**
 * @brief Reads a group couplet from a DXF file.  Calls another function
 * to process it.
 *
 * A group couplet consists of two lines that represent a single
 * piece of data.  An integer constant on the first line indicates
 * the type of data.  The value is on the next line.\n
 *
 * This function reads a couplet, determines the type of data, and
 * passes the value to the appropriate handler function of
 * \p creationInterface.\n
 *
 * \p fp is advanced so that the next call to \p readDXFGroups() reads
 * the next couplet in the file.
 *
 * @param fp Handle of input file
 * @param creationInterface Handle of class which processes entities
 *  in the file
 *
 * @retval true If EOF not reached.
 * @retval false If EOF reached.
 */
bool DL_Dxf::readDxfGroups( FILE *fp, DL_CreationInterface* creationInterface,
                            int* errorCounter )
{

  bool ok = true;
  static int line = 1;

  // Read one group of the DXF file and chop the lines:
  if ( DL_Dxf::getChoppedLine( groupCodeTmp, DL_DXF_MAXLINE, fp ) &&
       DL_Dxf::getChoppedLine( groupValue, DL_DXF_MAXLINE, fp ) )
  {

    groupCode = ( unsigned int )stringToInt( groupCodeTmp, &ok );

    if ( ok )
    {
// QgsDebugMsg(groupCode);
// QgsDebugMsg(groupValue);
      line += 2;
      processDXFGroup( creationInterface, groupCode, groupValue );
    }
    else
    {
      QgsDebugMsg( QString( "DXF read error: Line: %1" ).arg( line ) );
      if ( errorCounter != NULL )
      {
        ( *errorCounter )++;
      }
      // try to fix:
      QgsDebugMsg( "DXF read error: trying to fix.." );
      // drop a line to sync:
      DL_Dxf::getChoppedLine( groupCodeTmp, DL_DXF_MAXLINE, fp );
    }
  }

  return !feof( fp );
}



/**
 * Same as above but for stringstreams.
 */
#ifndef __GCC2x__
bool DL_Dxf::readDxfGroups( std::stringstream& stream,
                            DL_CreationInterface* creationInterface,
                            int* errorCounter )
{

  bool ok = true;
  static int line = 1;

  // Read one group of the DXF file and chop the lines:
  if ( DL_Dxf::getChoppedLine( groupCodeTmp, DL_DXF_MAXLINE, stream ) &&
       DL_Dxf::getChoppedLine( groupValue, DL_DXF_MAXLINE, stream ) )
  {

    groupCode = ( unsigned int )stringToInt( groupCodeTmp, &ok );

    if ( ok )
    {
// QgsDebugMsg(groupCode);
// QgsDebugMsg(groupValue);
      line += 2;
      processDXFGroup( creationInterface, groupCode, groupValue );
    }
    else
    {
      QgsDebugMsg( QString( "DXF read error: Line: %1" ).arg( line ) );
      if ( errorCounter != NULL )
      {
        ( *errorCounter )++;
      }
      // try to fix:
      QgsDebugMsg( "DXF read error: trying to fix.." );
      // drop a line to sync:
      DL_Dxf::getChoppedLine( groupCodeTmp, DL_DXF_MAXLINE, stream );
    }
  }
  return !stream.eof();
}
#endif



/**
 * @brief Reads line from file & strips whitespace at start and newline
 * at end.
 *
 * @param s Output\n
 *  Pointer to character array that chopped line will be returned in.
 * @param size Size of \p s.  (Including space for NULL.)
 * @param fp Input\n
 *  Handle of input file.
 *
 * @retval true if line could be read
 * @retval false if \p fp is already at end of file
 *
 * @todo Change function to use safer FreeBSD strl* functions
 * @todo Is it a problem if line is blank (i.e., newline only)?
 *  Then, when function returns, (s==NULL).
 */
bool DL_Dxf::getChoppedLine( char *s, unsigned int size, FILE *fp )
{
  if ( !feof( fp ) )
  {
    // The whole line in the file.  Includes space for NULL.
    char* wholeLine = new char[size];
    // Only the useful part of the line
    char* line;

    line = fgets( wholeLine, size, fp );

    if ( line != NULL && line[0] != '\0' )   // Evaluates to fgets() retval
    {
      // line == wholeLine at this point.
      // Both guaranteed to be NULL terminated.

      // Strip leading whitespace and trailing CR/LF.
      stripWhiteSpace( &line );

      strncpy( s, line, size );
      s[size] = '\0';
      // s should always be NULL terminated, because:
      assert( size > strlen( line ) );
    }

    delete[] wholeLine; // Done with wholeLine

    return true;
  }
  else
  {
    s[0] = '\0';
    return false;
  }
}



/**
 * Same as above but for stringstreams.
 */
#ifndef __GCC2x__
bool DL_Dxf::getChoppedLine( char *s, unsigned int size,
                             std::stringstream& stream )
{

  if ( !stream.eof() )
  {
    // Only the useful part of the line
    stream.getline( s, size );
    stripWhiteSpace( &s );
    assert( size > strlen( s ) );
    return true;
  }
  else
  {
    s[0] = '\0';
    return false;
  }
}
#endif



/**
 * @brief Strips leading whitespace and trailing Carriage Return (CR)
 * and Line Feed (LF) from NULL terminated string.
 *
 * @param s Input and output.
 *  NULL terminates string.
 *
 * @retval true if \p s is non-NULL
 * @retval false if \p s is NULL
 */
bool DL_Dxf::stripWhiteSpace( char** s )
{
  // last non-NULL char:
  int lastChar = strlen( *s ) - 1;
// QgsDebugMsg(QString("lastChar: %1").arg(lastChar));

  // Is last character CR or LF?
  while (( lastChar >= 0 ) &&
         ((( *s )[lastChar] == 10 ) || (( *s )[lastChar] == 13 ) ||
          (( *s )[lastChar] == ' ' || (( *s )[lastChar] == '\t' ) ) ) )
  {
    ( *s )[lastChar] = '\0';
    lastChar--;
  }

  // Skip whitespace, excluding \n, at beginning of line
  while (( *s )[0] == ' ' || ( *s )[0] == '\t' )
  {
    ++( *s );
  }

  return (( *s ) ? true : false );
}



/**
 * Processes a group (pair of group code and value).
 *
 * @param creationInterface Handle to class that creates entities and
 * other CAD data from DXF group codes
 *
 * @param groupCode Constant indicating the data type of the group.
 * @param groupValue The data value.
 *
 * @retval true if done processing current entity and new entity begun
 * @retval false if not done processing current entity
*/
bool DL_Dxf::processDXFGroup( DL_CreationInterface* creationInterface,
                              int groupCode, const char *groupValue )
{


// QgsDebugMsg(QString("groupCode=%1 groupValue=%2").arg(groupCode).arg(groupValue));

  // Init on first call
  if ( firstCall )
  {
    for ( int i = 0; i < DL_DXF_MAXGROUPCODE; ++i )
    {
      values[i][0] = '\0';
    }
    settingValue[0] = '\0';
    firstCall = false;
  }

  // Indicates comment or dxflib version:
  if ( groupCode == 999 )
  {
// QgsDebugMsg(QString("999: %1").arg(groupValue));
    if ( groupValue != NULL )
    {
      if ( !strncmp( groupValue, "dxflib", 6 ) )
      {
// QgsDebugMsg("dxflib version found");
        libVersion = getLibVersion( &groupValue[7] );
      }
    }
  }

  // Indicates start of new entity or var
  else if ( groupCode == 0 || groupCode == 9 )
  {

    // If new entity is encountered, the last one must be complete
    // prepare attributes which can be used for most entities:
    char name[DL_DXF_MAXLINE+1];
    if (( values[8] )[0] != '\0' )
    {
      strcpy( name, values[8] );
    }
    // defaults to layer '0':
    else
    {
      strcpy( name, "0" );
    }

    int width;
    // Compatibillity with qcad1:
    if (( values[39] )[0] != '\0' &&
        ( values[370] )[0] == '\0' )
    {
      width = toInt( values[39], -1 );
    }
    // since autocad 2002:
    else if (( values[370] )[0] != '\0' )
    {
      width = toInt( values[370], -1 );
    }
    // default to BYLAYER:
    else
    {
      width = -1;
    }

    int color;
    color = toInt( values[62], 256 );

    char linetype[DL_DXF_MAXLINE+1];
    strcpy( linetype, toString( values[6], "BYLAYER" ) );

    attrib = DL_Attributes( values[8],         // layer
                            color,              // color
                            width,              // width
                            linetype );         // linetype
    creationInterface->setAttributes( attrib );

    creationInterface->setExtrusion( toReal( values[210], 0.0 ),
                                     toReal( values[220], 0.0 ),
                                     toReal( values[230], 1.0 ),
                                     toReal( values[30], 0.0 ) );

    // Add the last entity via creationInterface
    switch ( currentEntity )
    {
      case DL_SETTING:
        addSetting( creationInterface );
        break;

      case DL_LAYER:
        addLayer( creationInterface );
        break;

      case DL_BLOCK:
        addBlock( creationInterface );
        break;

      case DL_ENDBLK:
        endBlock( creationInterface );
        break;

      case DL_ENTITY_POINT:
        addPoint( creationInterface );
        break;

      case DL_ENTITY_LINE:
        addLine( creationInterface );
        break;

      case DL_ENTITY_POLYLINE:
        //bulge = toReal(values[42]);
        // fall through
      case DL_ENTITY_LWPOLYLINE:
        addPolyline( creationInterface );
        break;

      case DL_ENTITY_VERTEX:
        addVertex( creationInterface );
        break;

      case DL_ENTITY_SPLINE:
        addSpline( creationInterface );
        break;

      case DL_ENTITY_ARC:
        addArc( creationInterface );
        break;

      case DL_ENTITY_CIRCLE:
        addCircle( creationInterface );
        break;

      case DL_ENTITY_ELLIPSE:
        addEllipse( creationInterface );
        break;

      case DL_ENTITY_INSERT:
        addInsert( creationInterface );
        break;

      case DL_ENTITY_MTEXT:
        addMText( creationInterface );
        break;

      case DL_ENTITY_TEXT:
        addText( creationInterface );
        break;

      case DL_ENTITY_ATTRIB:
        addAttrib( creationInterface );
        break;

      case DL_ENTITY_DIMENSION:
      {
        int type = ( toInt( values[70], 0 ) & 0x07 );

        switch ( type )
        {
          case 0:
            addDimLinear( creationInterface );
            break;

          case 1:
            addDimAligned( creationInterface );
            break;

          case 2:
            addDimAngular( creationInterface );
            break;

          case 3:
            addDimDiametric( creationInterface );
            break;

          case 4:
            addDimRadial( creationInterface );
            break;

          case 5:
            addDimAngular3P( creationInterface );
            break;

          default:
            break;
        }
      }
      break;

      case DL_ENTITY_LEADER:
        addLeader( creationInterface );
        break;

      case DL_ENTITY_HATCH:
        addHatch( creationInterface );
        break;

      case DL_ENTITY_IMAGE:
        addImage( creationInterface );
        break;

      case DL_ENTITY_IMAGEDEF:
        addImageDef( creationInterface );
        break;

      case DL_ENTITY_TRACE:
        addTrace( creationInterface );
        break;

      case DL_ENTITY_SOLID:
        addSolid( creationInterface );
        break;

      case DL_ENTITY_SEQEND:
        endSequence( creationInterface );
        break;

      default:
        break;
    }


    // reset all values (they are not persistent and only this
    //  way we can detect default values for unstored settings)
    for ( int i = 0; i < DL_DXF_MAXGROUPCODE; ++i )
    {
      values[i][0] = '\0';
    }
    settingValue[0] = '\0';
    settingKey[0] = '\0';


    // Last DXF entity or setting has been handled
    // Now determine what the next entity or setting type is

    int prevEntity = currentEntity;

    // Read DXF settings:
    if ( groupValue[0] == '$' )
    {
      currentEntity = DL_SETTING;
      strncpy( settingKey, groupValue, DL_DXF_MAXLINE );
      settingKey[DL_DXF_MAXLINE] = '\0';
    }
    // Read Layers:
    else if ( !strcmp( groupValue, "LAYER" ) )
    {
      currentEntity = DL_LAYER;

    }
    // Read Blocks:
    else if ( !strcmp( groupValue, "BLOCK" ) )
    {
      currentEntity = DL_BLOCK;
    }
    else if ( !strcmp( groupValue, "ENDBLK" ) )
    {
      currentEntity = DL_ENDBLK;

    }
    // Read entities:
    else if ( !strcmp( groupValue, "POINT" ) )
    {
      currentEntity = DL_ENTITY_POINT;
    }
    else if ( !strcmp( groupValue, "LINE" ) )
    {
      currentEntity = DL_ENTITY_LINE;
    }
    else if ( !strcmp( groupValue, "POLYLINE" ) )
    {
      currentEntity = DL_ENTITY_POLYLINE;
    }
    else if ( !strcmp( groupValue, "LWPOLYLINE" ) )
    {
      currentEntity = DL_ENTITY_LWPOLYLINE;
    }
    else if ( !strcmp( groupValue, "VERTEX" ) )
    {
      currentEntity = DL_ENTITY_VERTEX;
    }
    else if ( !strcmp( groupValue, "SPLINE" ) )
    {
      currentEntity = DL_ENTITY_SPLINE;
    }
    else if ( !strcmp( groupValue, "ARC" ) )
    {
      currentEntity = DL_ENTITY_ARC;
    }
    else if ( !strcmp( groupValue, "ELLIPSE" ) )
    {
      currentEntity = DL_ENTITY_ELLIPSE;
    }
    else if ( !strcmp( groupValue, "CIRCLE" ) )
    {
      currentEntity = DL_ENTITY_CIRCLE;
    }
    else if ( !strcmp( groupValue, "INSERT" ) )
    {
      currentEntity = DL_ENTITY_INSERT;
    }
    else if ( !strcmp( groupValue, "TEXT" ) )
    {
      currentEntity = DL_ENTITY_TEXT;
    }
    else if ( !strcmp( groupValue, "MTEXT" ) )
    {
      currentEntity = DL_ENTITY_MTEXT;
    }
    else if ( !strcmp( groupValue, "ATTRIB" ) )
    {
      currentEntity = DL_ENTITY_ATTRIB;
    }
    else if ( !strcmp( groupValue, "DIMENSION" ) )
    {
      currentEntity = DL_ENTITY_DIMENSION;
    }
    else if ( !strcmp( groupValue, "LEADER" ) )
    {
      currentEntity = DL_ENTITY_LEADER;
    }
    else if ( !strcmp( groupValue, "HATCH" ) )
    {
      currentEntity = DL_ENTITY_HATCH;
    }
    else if ( !strcmp( groupValue, "IMAGE" ) )
    {
      currentEntity = DL_ENTITY_IMAGE;
    }
    else if ( !strcmp( groupValue, "IMAGEDEF" ) )
    {
      currentEntity = DL_ENTITY_IMAGEDEF;
    }
    else if ( !strcmp( groupValue, "TRACE" ) )
    {
      currentEntity = DL_ENTITY_TRACE;
    }
    else if ( !strcmp( groupValue, "SOLID" ) )
    {
      currentEntity = DL_ENTITY_SOLID;
    }
    else if ( !strcmp( groupValue, "SEQEND" ) )
    {
      currentEntity = DL_ENTITY_SEQEND;
    }
    else
    {
      currentEntity = DL_Unknown;
    }

    // end of old style POLYLINE entity
    if ( prevEntity == DL_ENTITY_VERTEX && currentEntity != DL_ENTITY_VERTEX )
    {
      endEntity( creationInterface );
    }

    return true;

  }
  else
  {
    // Group code does not indicate start of new entity or setting,
    // so this group must be continuation of data for the current
    // one.
    if ( groupCode < DL_DXF_MAXGROUPCODE )
    {

      bool handled = false;

      switch ( currentEntity )
      {
        case DL_ENTITY_MTEXT:
          handled = handleMTextData( creationInterface );
          break;

        case DL_ENTITY_LWPOLYLINE:
          handled = handleLWPolylineData( creationInterface );
          break;

        case DL_ENTITY_SPLINE:
          handled = handleSplineData( creationInterface );
          break;

        case DL_ENTITY_LEADER:
          handled = handleLeaderData( creationInterface );
          break;

        case DL_ENTITY_HATCH:
          handled = handleHatchData( creationInterface );
          break;

        default:
          break;
      }

      if ( !handled )
      {
        // Normal group / value pair:
        strncpy( values[groupCode], groupValue, DL_DXF_MAXLINE );
        values[groupCode][DL_DXF_MAXLINE] = '\0';
      }
    }

    return false;
  }
  return false;
}



/**
 * Adds a variable from the DXF file.
 */
void DL_Dxf::addSetting( DL_CreationInterface* creationInterface )
{
  int c = -1;
  for ( int i = 0; i <= 380; ++i )
  {
    if ( values[i][0] != '\0' )
    {
      c = i;
      break;
    }
  }

  // string
  if ( c >= 0 && c <= 9 )
  {
    creationInterface->setVariableString( settingKey,
                                          values[c], c );
  }
  // vector
  else if ( c >= 10 && c <= 39 )
  {
    if ( c == 10 )
    {
      creationInterface->setVariableVector(
        settingKey,
        toReal( values[c] ),
        toReal( values[c+10] ),
        toReal( values[c+20] ),
        c );
    }
  }
  // double
  else if ( c >= 40 && c <= 59 )
  {
    creationInterface->setVariableDouble( settingKey,
                                          toReal( values[c] ),
                                          c );
  }
  // int
  else if ( c >= 60 && c <= 99 )
  {
    creationInterface->setVariableInt( settingKey,
                                       toInt( values[c] ),
                                       c );
  }
  // misc
  else if ( c >= 0 )
  {
    creationInterface->setVariableString( settingKey,
                                          values[c],
                                          c );
  }
}



/**
 * Adds a layer that was read from the file via the creation interface.
 */
void DL_Dxf::addLayer( DL_CreationInterface* creationInterface )
{
  // correct some impossible attributes for layers:
  attrib = creationInterface->getAttributes();
  if ( attrib.getColor() == 256 || attrib.getColor() == 0 )
  {
    attrib.setColor( 7 );
  }
  if ( attrib.getWidth() < 0 )
  {
    attrib.setWidth( 1 );
  }
  if ( !strcasecmp( attrib.getLineType().c_str(), "BYLAYER" ) ||
       !strcasecmp( attrib.getLineType().c_str(), "BYBLOCK" ) )
  {
    attrib.setLineType( "CONTINUOUS" );
  }

  // add layer
  creationInterface->addLayer( DL_LayerData( values[2],
                               toInt( values[70] ) ) );
}



/**
 * Adds a block that was read from the file via the creation interface.
 */
void DL_Dxf::addBlock( DL_CreationInterface* creationInterface )
{
  DL_BlockData d(
    // Name:
    values[2],
    // flags:
    toInt( values[70] ),
    // base point:
    toReal( values[10] ),
    toReal( values[20] ),
    toReal( values[30] ) );

  creationInterface->addBlock( d );
}



/**
 * Ends a block that was read from the file via the creation interface.
 */
void DL_Dxf::endBlock( DL_CreationInterface* creationInterface )
{
  creationInterface->endBlock();
}



/**
 * Adds a point entity that was read from the file via the creation interface.
 */
void DL_Dxf::addPoint( DL_CreationInterface* creationInterface )
{
  DL_PointData d( toReal( values[10] ),
                  toReal( values[20] ),
                  toReal( values[30] ) );
  creationInterface->addPoint( d );
}



/**
 * Adds a line entity that was read from the file via the creation interface.
 */
void DL_Dxf::addLine( DL_CreationInterface* creationInterface )
{
  DL_LineData d( toReal( values[10] ),
                 toReal( values[20] ),
                 toReal( values[30] ),
                 toReal( values[11] ),
                 toReal( values[21] ),
                 toReal( values[31] ) );

  creationInterface->addLine( d );
}



/**
 * Adds a polyline entity that was read from the file via the creation interface.
 */
void DL_Dxf::addPolyline( DL_CreationInterface* creationInterface )
{
  DL_PolylineData pd( maxVertices, toInt( values[71], 0 ), toInt( values[72], 0 ), toInt( values[70], 0 ) );
  creationInterface->addPolyline( pd );

  if ( currentEntity == DL_ENTITY_LWPOLYLINE )
  {
    for ( int i = 0; i < maxVertices; i++ )
    {
      DL_VertexData d( vertices[i*4],
                       vertices[i*4+1],
                       vertices[i*4+2],
                       vertices[i*4+3] );

      creationInterface->addVertex( d );
    }
    creationInterface->endEntity();
  }
}



/**
 * Adds a polyline vertex entity that was read from the file
 * via the creation interface.
 */
void DL_Dxf::addVertex( DL_CreationInterface* creationInterface )
{
  DL_VertexData d( toReal( values[10] ),
                   toReal( values[20] ),
                   toReal( values[30] ),
                   //bulge);
                   toReal( values[42] ) );

  //bulge = toReal(values[42]);

  creationInterface->addVertex( d );
}



/**
 * Adds a spline entity that was read from the file via the creation interface.
 */
void DL_Dxf::addSpline( DL_CreationInterface* creationInterface )
{
  DL_SplineData sd( toInt( values[71], 3 ),
                    maxKnots,
                    maxControlPoints,
                    toInt( values[70], 4 ) );
  /*DL_SplineData sd(toInt(values[71], 3), toInt(values[72], 0),
                   toInt(values[73], 0), toInt(values[70], 4));*/
  creationInterface->addSpline( sd );

  int i;
  for ( i = 0; i < maxControlPoints; i++ )
  {
    DL_ControlPointData d( controlPoints[i*3],
                           controlPoints[i*3+1],
                           controlPoints[i*3+2] );

    creationInterface->addControlPoint( d );
  }
  for ( i = 0; i < maxKnots; i++ )
  {
    DL_KnotData k( knots[i] );

    creationInterface->addKnot( k );
  }
}



#if 0
/**
 * Adds a knot to the previously added spline.
 */
void DL_Dxf::addKnot( DL_CreationInterface* creationInterface )
{
  QgsDebugMsg( "entered." );
}
#endif

#if 0
/**
 * Adds a control point to the previously added spline.
 */
void DL_Dxf::addControlPoint( DL_CreationInterface* creationInterface )
{
  QgsDebugMsg( "entered." );
}
#endif



/**
 * Adds an arc entity that was read from the file via the creation interface.
 */
void DL_Dxf::addArc( DL_CreationInterface* creationInterface )
{
  DL_ArcData d( toReal( values[10] ),
                toReal( values[20] ),
                toReal( values[30] ),
                toReal( values[40] ),
                toReal( values[50] ),
                toReal( values[51] ) );

  creationInterface->addArc( d );
}



/**
 * Adds a circle entity that was read from the file via the creation interface.
 */
void DL_Dxf::addCircle( DL_CreationInterface* creationInterface )
{
  DL_CircleData d( toReal( values[10] ),
                   toReal( values[20] ),
                   toReal( values[30] ),
                   toReal( values[40] ) );

  creationInterface->addCircle( d );
}



/**
 * Adds an ellipse entity that was read from the file via the creation interface.
 */
void DL_Dxf::addEllipse( DL_CreationInterface* creationInterface )
{
  DL_EllipseData d( toReal( values[10] ),
                    toReal( values[20] ),
                    toReal( values[30] ),
                    toReal( values[11] ),
                    toReal( values[21] ),
                    toReal( values[31] ),
                    toReal( values[40], 1.0 ),
                    toReal( values[41], 0.0 ),
                    toReal( values[42], 2*M_PI ) );

  creationInterface->addEllipse( d );
}



/**
 * Adds an insert entity that was read from the file via the creation interface.
 */
void DL_Dxf::addInsert( DL_CreationInterface* creationInterface )
{
  DL_InsertData d( values[2],
                   // insertion point
                   toReal( values[10], 0.0 ),
                   toReal( values[20], 0.0 ),
                   toReal( values[30], 0.0 ),
                   // scale:
                   toReal( values[41], 1.0 ),
                   toReal( values[42], 1.0 ),
                   toReal( values[43], 1.0 ),
                   // angle:
                   toReal( values[50], 0.0 ),
                   // cols / rows:
                   toInt( values[70], 1 ),
                   toInt( values[71], 1 ),
                   // spacing:
                   toReal( values[44], 0.0 ),
                   toReal( values[45], 0.0 ) );

  creationInterface->addInsert( d );
}


/**
 * Adds a trace entity (4 edge closed polyline) that was read from the file via the creation interface.
 *
 * @author AHM
 */
void DL_Dxf::addTrace( DL_CreationInterface* creationInterface )
{
  DL_TraceData td;

  for ( int k = 0; k < 4; k++ )
  {
    td.x[k] = toReal( values[10 + k] );
    td.y[k] = toReal( values[20 + k] );
    td.z[k] = toReal( values[30 + k] );
  }
  creationInterface->addTrace( td );
}

/**
 * Adds a solid entity (filled trace) that was read from the file via the creation interface.
 *
 * @author AHM
 */
void DL_Dxf::addSolid( DL_CreationInterface* creationInterface )
{
  DL_SolidData sd;

  for ( int k = 0; k < 4; k++ )
  {
    sd.x[k] = toReal( values[10 + k] );
    sd.y[k] = toReal( values[20 + k] );
    sd.z[k] = toReal( values[30 + k] );
  }
  creationInterface->addSolid( sd );
}


/**
 * Adds an MText entity that was read from the file via the creation interface.
 */
void DL_Dxf::addMText( DL_CreationInterface* creationInterface )
{
  double angle = 0.0;

  if ( values[50][0] != '\0' )
  {
    if ( libVersion <= 0x02000200 )
    {
      // wrong but compatible with dxflib <=2.0.2.0:
      angle = toReal( values[50], 0.0 );
    }
    else
    {
      angle = ( toReal( values[50], 0.0 ) * 2 * M_PI ) / 360.0;
    }
  }
  else if ( values[11][0] != '\0' && values[21][0] != '\0' )
  {
    double x = toReal( values[11], 0.0 );
    double y = toReal( values[21], 0.0 );

    if ( qAbs( x ) < 1.0e-6 )
    {
      if ( y > 0.0 )
      {
        angle = M_PI / 2.0;
      }
      else
      {
        angle = M_PI / 2.0 * 3.0;
      }
    }
    else
    {
      angle = atan( y / x );
    }
  }

  DL_MTextData d(
    // insertion point
    toReal( values[10], 0.0 ),
    toReal( values[20], 0.0 ),
    toReal( values[30], 0.0 ),
    // height
    toReal( values[40], 2.5 ),
    // width
    toReal( values[41], 100.0 ),
    // attachment point
    toInt( values[71], 1 ),
    // drawing direction
    toInt( values[72], 1 ),
    // line spacing style
    toInt( values[73], 1 ),
    // line spacing factor
    toReal( values[44], 1.0 ),
    // text
    values[1],
    // style
    values[7],
    // angle
    angle );
  creationInterface->addMText( d );
}



/**
 * Handles additional MText data.
 */
bool DL_Dxf::handleMTextData( DL_CreationInterface* creationInterface )
{
  // Special handling of text chunks for MTEXT entities:
  if ( groupCode == 3 )
  {
    creationInterface->addMTextChunk( groupValue );
    return true;
  }

  return false;
}



/**
 * Handles additional polyline data.
 */
bool DL_Dxf::handleLWPolylineData( DL_CreationInterface* /*creationInterface*/ )
{
  // Allocate LWPolyline vertices (group code 90):
  if ( groupCode == 90 )
  {
    maxVertices = toInt( groupValue );
    if ( maxVertices > 0 )
    {
      if ( vertices != NULL )
      {
        delete[] vertices;
      }
      vertices = new double[4*maxVertices];
      for ( int i = 0; i < maxVertices; ++i )
      {
        vertices[i*4] = 0.0;
        vertices[i*4+1] = 0.0;
        vertices[i*4+2] = 0.0;
        vertices[i*4+3] = 0.0;
      }
    }
    vertexIndex = -1;
    return true;
  }

  // Compute LWPolylines vertices (group codes 10/20/30/42):
  else if ( groupCode == 10 || groupCode == 20 ||
            groupCode == 30 || groupCode == 42 )
  {

    if ( vertexIndex < maxVertices - 1 && groupCode == 10 )
    {
      vertexIndex++;
    }

    if ( groupCode <= 30 )
    {
      if ( vertexIndex >= 0 && vertexIndex < maxVertices )
      {
        vertices[4*vertexIndex + ( groupCode/10-1 )]
        = toReal( groupValue );
      }
    }
    else if ( groupCode == 42 && vertexIndex < maxVertices )
    {
      vertices[4*vertexIndex + 3] = toReal( groupValue );
    }
    return true;
  }
  return false;
}



/**
 * Handles additional spline data.
 */
bool DL_Dxf::handleSplineData( DL_CreationInterface* /*creationInterface*/ )
{
  // Allocate Spline knots (group code 72):
  if ( groupCode == 72 )
  {
    maxKnots = toInt( groupValue );
    if ( maxKnots > 0 )
    {
      if ( knots != NULL )
      {
        delete[] knots;
      }
      knots = new double[maxKnots];
      for ( int i = 0; i < maxKnots; ++i )
      {
        knots[i] = 0.0;
      }
    }
    knotIndex = -1;
    return true;
  }

  // Allocate Spline control points (group code 73):
  else if ( groupCode == 73 )
  {
    maxControlPoints = toInt( groupValue );
    if ( maxControlPoints > 0 )
    {
      if ( controlPoints != NULL )
      {
        delete[] controlPoints;
      }
      controlPoints = new double[3*maxControlPoints];
      for ( int i = 0; i < maxControlPoints; ++i )
      {
        controlPoints[i*3] = 0.0;
        controlPoints[i*3+1] = 0.0;
        controlPoints[i*3+2] = 0.0;
      }
    }
    controlPointIndex = -1;
    return true;
  }

  // Compute spline knot vertices (group code 40):
  else if ( groupCode == 40 )
  {
    if ( knotIndex < maxKnots - 1 )
    {
      knotIndex++;
      knots[knotIndex] = toReal( groupValue );
    }
    return true;
  }

  // Compute spline control points (group codes 10/20/30):
  else if ( groupCode == 10 || groupCode == 20 ||
            groupCode == 30 )
  {

    if ( controlPointIndex < maxControlPoints - 1 && groupCode == 10 )
    {
      controlPointIndex++;
    }

    if ( controlPointIndex >= 0 && controlPointIndex < maxControlPoints )
    {
      controlPoints[3*controlPointIndex + ( groupCode/10-1 )]
      = toReal( groupValue );
    }
    return true;
  }
  return false;
}



/**
 * Handles additional leader data.
 */
bool DL_Dxf::handleLeaderData( DL_CreationInterface* /*creationInterface*/ )
{
  // Allocate Leader vertices (group code 76):
  if ( groupCode == 76 )
  {
    maxLeaderVertices = toInt( groupValue );
    if ( maxLeaderVertices > 0 )
    {
      if ( leaderVertices != NULL )
      {
        delete[] leaderVertices;
      }
      leaderVertices = new double[3*maxLeaderVertices];
      for ( int i = 0; i < maxLeaderVertices; ++i )
      {
        leaderVertices[i*3] = 0.0;
        leaderVertices[i*3+1] = 0.0;
        leaderVertices[i*3+2] = 0.0;
      }
    }
    leaderVertexIndex = -1;
    return true;
  }

  // Compute Leader vertices (group codes 10/20/30):
  else if ( groupCode == 10 || groupCode == 20 || groupCode == 30 )
  {

    if ( leaderVertexIndex < maxLeaderVertices - 1 && groupCode == 10 )
    {
      leaderVertexIndex++;
    }

    if ( groupCode <= 30 )
    {
      if ( leaderVertexIndex >= 0 &&
           leaderVertexIndex < maxLeaderVertices )
      {
        leaderVertices[3*leaderVertexIndex + ( groupCode/10-1 )]
        = toReal( groupValue );
      }
    }
    return true;
  }

  return false;
}



/**
 * Handles additional hatch data.
 */
bool DL_Dxf::handleHatchData( DL_CreationInterface* /*creationInterface*/ )
{
  // Allocate hatch loops (group code 91):
  if ( groupCode == 91 && toInt( groupValue ) > 0 )
  {

// QgsDebugMsg(QString("allocating %1 loops").arg(toInt(groupValue)));

    if ( hatchLoops != NULL )
    {
      delete[] hatchLoops;
      hatchLoops = NULL;
    }
    if ( maxHatchEdges != NULL )
    {
      delete[] maxHatchEdges;
      maxHatchEdges = NULL;
    }
    if ( hatchEdgeIndex != NULL )
    {
      delete[] hatchEdgeIndex;
      hatchEdgeIndex = NULL;
    }
    if ( hatchEdges != NULL )
    {
      for ( int i = 0; i < maxHatchLoops; ++i )
      {
        delete[] hatchEdges[i];
      }
      delete[] hatchEdges;
      hatchEdges = NULL;
    }
    maxHatchLoops = toInt( groupValue );

// QgsDebugMsg(QString("maxHatchLoops: %1").arg(maxHatchLoops));

    if ( maxHatchLoops > 0 )
    {
      hatchLoops = new DL_HatchLoopData[maxHatchLoops];
      maxHatchEdges = new int[maxHatchLoops];
      hatchEdgeIndex = new int[maxHatchLoops];
      hatchEdges = new DL_HatchEdgeData*[maxHatchLoops];
// QgsDebugMsg(QString("new hatchEdges[%1]").arg(maxHatchLoops));
      for ( int i = 0; i < maxHatchLoops; ++i )
      {
        hatchEdges[i] = NULL;
// QgsDebugMsg(QString("hatchEdges[%1] = NULL").arg(i));
        maxHatchEdges[i] = 0;
      }
      hatchLoopIndex = -1;
      dropEdges = false;
    }
// QgsDebugMsg("done");
    return true;
  }

  // Allocate hatch edges, group code 93
  if ( groupCode == 93 && toInt( groupValue ) > 0 )
  {
    if ( hatchLoopIndex < maxHatchLoops - 1 && hatchLoops != NULL &&
         maxHatchEdges != NULL && hatchEdgeIndex != NULL &&
         hatchEdges != NULL )
    {

// QgsDebugMsg(QString("  allocating %1 edges").arg(toInt(groupValue)));
      dropEdges = false;

      hatchLoopIndex++;
      hatchLoops[hatchLoopIndex] = DL_HatchLoopData( toInt( groupValue ) );

      maxHatchEdges[hatchLoopIndex] = toInt( groupValue );
      hatchEdgeIndex[hatchLoopIndex] = -1;
      hatchEdges[hatchLoopIndex] = new DL_HatchEdgeData[toInt( groupValue )];

// QgsDebugMsg(QString("hatchEdges[%1] = new %2").arg(hatchLoopIndex).arg(toInt(groupValue)));
    }
    else
    {
// QgsDebugMsg(QString("dropping %1 edges").arg(toInt(groupValue)));
      dropEdges = true;
    }
// QgsDebugMsg("done");
    return true;
  }

  // Init hatch edge for non-polyline boundary (group code 72)
  if ( hatchEdges != NULL &&
       hatchEdgeIndex != NULL &&
       maxHatchEdges != NULL &&
       hatchLoopIndex >= 0 &&
       hatchLoopIndex < maxHatchLoops &&
       hatchEdgeIndex[hatchLoopIndex] <
       maxHatchEdges[hatchLoopIndex] &&
       ( atoi( values[92] )&2 ) == 0 &&   // not a polyline
       groupCode == 72 &&
       !dropEdges )
  {

// QgsDebugMsg("Init hatch edge for non-polyline boundary");
// QgsDebugMsg(QString("hatchLoopIndex: %1").arg(hatchLoopIndex));
// QgsDebugMsg(QString("maxHatchLoops: %1").arg(maxHatchLoops));

    hatchEdgeIndex[hatchLoopIndex]++;

// QgsDebugMsg(QString("  init edge: type: %1 index: %2").arg(toInt(groupValue)).arg(hatchEdgeIndex[hatchLoopIndex]));

    hatchEdges[hatchLoopIndex][hatchEdgeIndex[hatchLoopIndex]]
    .type = toInt( groupValue );
    hatchEdges[hatchLoopIndex][hatchEdgeIndex[hatchLoopIndex]]
    .defined = false;

// QgsDebugMsg("done");
    return true;
  }

  // Handle hatch edges for non-polyline boundaries
  //   (group codes 10, 20, 11, 21, 40, 50, 51, 73)
  if ( !dropEdges &&
       hatchEdges != NULL &&
       hatchEdgeIndex != NULL &&
       hatchLoopIndex >= 0 &&
       hatchLoopIndex < maxHatchLoops &&
       hatchEdges[hatchLoopIndex] != NULL &&
       hatchEdgeIndex[hatchLoopIndex] >= 0 &&
       hatchEdgeIndex[hatchLoopIndex] <
       maxHatchEdges[hatchLoopIndex] &&
       (( atoi( values[92] )&2 ) == 0 ) && // not a polyline
       ( groupCode == 10 || groupCode == 20 ||
         groupCode == 11 || groupCode == 21 ||
         groupCode == 40 || groupCode == 50 ||
         groupCode == 51 || groupCode == 73 ) )
  {

// QgsDebugMsg("Handle hatch edge for non-polyline boundary");
// QgsDebugMsg(QString("  found edge data: %1").arg(groupCode));
// QgsDebugMsg(QString("     value: %1").arg(toReal(groupValue)));

    // can crash:
// QgsDebugMsg(QString("     defined: %1").arg((int)hatchEdges[hatchLoopIndex][hatchEdgeIndex[hatchLoopIndex]].defined));

// QgsDebugMsg(QString("92 flag: '%1'").arg(values[92]));
// QgsDebugMsg(QString("92 flag (int): '%1'").arg(atoi(values[92])));

    if ( hatchEdges[hatchLoopIndex]
         [hatchEdgeIndex[hatchLoopIndex]].defined == false )
    {
      if ( hatchEdges[hatchLoopIndex]
           [hatchEdgeIndex[hatchLoopIndex]].type == 1 )
      {
        switch ( groupCode )
        {
          case 10:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].x1
            = toReal( groupValue );
            break;
          case 20:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].y1
            = toReal( groupValue );
            break;
          case 11:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].x2
            = toReal( groupValue );
            break;
          case 21:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].y2
            = toReal( groupValue );
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].defined = true;
            break;
          default:
            break;
        }
      }

      if ( hatchEdges[hatchLoopIndex]
           [hatchEdgeIndex[hatchLoopIndex]].type == 2 )
      {
        switch ( groupCode )
        {
          case 10:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].cx
            = toReal( groupValue );
            break;
          case 20:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].cy
            = toReal( groupValue );
            break;
          case 40:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].radius
            = toReal( groupValue );
            break;
          case 50:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].angle1
            = toReal( groupValue ) / 360.0 * 2 * M_PI;
            break;
          case 51:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].angle2
            = toReal( groupValue ) / 360.0 * 2 * M_PI;
            break;
          case 73:
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].ccw
            = ( bool )toInt( groupValue );
            hatchEdges[hatchLoopIndex]
            [hatchEdgeIndex[hatchLoopIndex]].defined = true;
            break;
          default:
            break;
        }
      }
    }
    return true;
  }

#if 0
  // 2003/12/31: polyline hatches can be extremely slow and are rarely used
  //
  // Handle hatch edges for polyline boundaries
  //  (group codes 10, 20, 42)
  if ( !dropEdges &&
       hatchEdges != NULL &&
       hatchEdgeIndex != NULL &&
       hatchLoopIndex >= 0 &&
       hatchLoopIndex < maxHatchLoops &&
       hatchEdges[hatchLoopIndex] != NULL &&
       //hatchEdgeIndex[hatchLoopIndex]>=0 &&
       hatchEdgeIndex[hatchLoopIndex] <
       maxHatchEdges[hatchLoopIndex] &&
       (( atoi( values[92] )&2 ) == 2 ) )  // a polyline
  {

    if ( groupCode == 10 || groupCode == 20 ||
         groupCode == 42 )
    {

      QgsDebugMsg( QString( "  found polyline edge data: %1" ).arg( groupCode ) );
      QgsDebugMsg( QString( "     value: %1" ).arg( toReal( groupValue ) ) );

      static double lastX = 0.0;
      static double lastY = 0.0;
      static double lastB = 0.0;

      if ( firstPolylineStatus < 2 )
      {
        switch ( groupCode )
        {
          case 10:
            firstPolylineStatus++;
            if ( firstPolylineStatus == 1 )
            {
              lastX = toReal( groupValue );
              QgsDebugMsg( QString( "     firstX: %1" ).arg( lastX ) );
            }
            break;

          case 20:
            lastY = toReal( groupValue );
            QgsDebugMsg( QString( "     firstY: %1" ).arg( lastY ) );
            break;

          case 42:
            lastB = toReal( groupValue );
            break;

          default:
            break;
        }

        if ( firstPolylineStatus != 2 )
        {
          return true;
        }
      }


      switch ( groupCode )
      {
        case 10:
          hatchEdgeIndex[hatchLoopIndex]++;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].type = 1;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].x1
          = lastX;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].x2
          = lastX = toReal( groupValue );
          QgsDebugMsg( QString( "     X: %1" ).arg( lastX ) );
          break;
        case 20:
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].y1
          = lastY;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].y2
          = lastY = toReal( groupValue );
          QgsDebugMsg( QString( "     Y: %1" ).arg( lastY ) );
          break;
          / *
        case 42:
        {
          // convert to arc:
          double x1 = hatchEdges[hatchLoopIndex]
                      [hatchEdgeIndex[hatchLoopIndex]].x1;
          double y1 = hatchEdges[hatchLoopIndex]
                      [hatchEdgeIndex[hatchLoopIndex]].y1;
          double x2 = hatchEdges[hatchLoopIndex]
                      [hatchEdgeIndex[hatchLoopIndex]].x2;
          double y2 = hatchEdges[hatchLoopIndex]
                      [hatchEdgeIndex[hatchLoopIndex]].y2;

          double bulge = toReal( groupValue );

          bool reversed = ( bulge < 0.0 );
          double alpha = atan( bulge ) * 4.0;
          double radius;
          double cx;
          double cy;
          double a1;
          double a2;
          double mx = ( x2 + x1 ) / 2.0;
          double my = ( y2 + y1 ) / 2.0;
          double dist = sqrt( pow( x2 - x1, 2 ) + pow( y2 - y1, 2 ) ) / 2.0;

          // alpha can't be 0.0 at this point
          radius = qAbs( dist / sin( alpha / 2.0 ) );

          double wu = qAbs( pow( radius, 2.0 ) - pow( dist, 2.0 ) );
          double h = sqrt( wu );
          double angle = acos(( x2 - x1 ) / dist );

          if ( bulge > 0.0 )
          {
            angle += M_PI / 2.0;
          }
          else
          {
            angle -= M_PI / 2.0;
          }

          if ( qAbs( alpha ) > M_PI )
          {
            h *= -1.0;
          }

          cx = mx + cos( angle ) * h;
          cy = my + sin( angle ) * h;

          a1 = hatchEdges[hatchLoopIndex]
               [hatchEdgeIndex[hatchLoopIndex]].type = 2;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].ccw = ( toReal( groupValue ) > 0.0 );
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].cx = cx;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].cy = cy;
          hatchEdges[hatchLoopIndex]
          [hatchEdgeIndex[hatchLoopIndex]].radius = radius;
        }
        break;
        * /

        default:
          break;
      }
    }
    else
    {
      // end polyline boundary
      dropEdges = true;
    }

    return true;
  }
#endif

  return false;
}




/**
 * Adds an text entity that was read from the file via the creation interface.
 */
void DL_Dxf::addText( DL_CreationInterface* creationInterface )
{
  DL_TextData d(
    // insertion point
    toReal( values[10], 0.0 ),
    toReal( values[20], 0.0 ),
    toReal( values[30], 0.0 ),
    // alignment point
    toReal( values[11], 0.0 ),
    toReal( values[21], 0.0 ),
    toReal( values[31], 0.0 ),
    // height
    toReal( values[40], 2.5 ),
    // x scale
    toReal( values[41], 1.0 ),
    // generation flags
    toInt( values[71], 0 ),
    // h just
    toInt( values[72], 0 ),
    // v just
    toInt( values[73], 0 ),
    // text
    values[1],
    // style
    values[7],
    // angle
    ( toReal( values[50], 0.0 )*2*M_PI ) / 360.0 );

  creationInterface->addText( d );
}



/**
 * Adds an attrib entity that was read from the file via the creation interface.
 * @todo add attrib instead of normal text
 */
void DL_Dxf::addAttrib( DL_CreationInterface* creationInterface )
{
  DL_TextData d(
    // insertion point
    toReal( values[10], 0.0 ),
    toReal( values[20], 0.0 ),
    toReal( values[30], 0.0 ),
    // alignment point
    toReal( values[11], 0.0 ),
    toReal( values[21], 0.0 ),
    toReal( values[31], 0.0 ),
    // height
    toReal( values[40], 2.5 ),
    // x scale
    toReal( values[41], 1.0 ),
    // generation flags
    toInt( values[71], 0 ),
    // h just
    toInt( values[72], 0 ),
    // v just
    toInt( values[74], 0 ),
    // text
    values[1],
    // style
    values[7],
    // angle
    ( toReal( values[50], 0.0 )*2*M_PI ) / 360.0 );

  creationInterface->addText( d );
}



/**
 * @return dimension data from current values.
 */
DL_DimensionData DL_Dxf::getDimData()
{
  // generic dimension data:
  return DL_DimensionData(
           // def point
           toReal( values[10], 0.0 ),
           toReal( values[20], 0.0 ),
           toReal( values[30], 0.0 ),
           // text middle point
           toReal( values[11], 0.0 ),
           toReal( values[21], 0.0 ),
           toReal( values[31], 0.0 ),
           // type
           toInt( values[70], 0 ),
           // attachment point
           toInt( values[71], 5 ),
           // line sp. style
           toInt( values[72], 1 ),
           // line sp. factor
           toReal( values[41], 1.0 ),
           // text
           values[1],
           // style
           values[3],
           // angle
           toReal( values[53], 0.0 ) );
}



/**
 * Adds a linear dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimLinear( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  // horizontal / vertical / rotated dimension:
  DL_DimLinearData dl(
    // definition point 1
    toReal( values[13], 0.0 ),
    toReal( values[23], 0.0 ),
    toReal( values[33], 0.0 ),
    // definition point 2
    toReal( values[14], 0.0 ),
    toReal( values[24], 0.0 ),
    toReal( values[34], 0.0 ),
    // angle
    toReal( values[50], 0.0 ),
    // oblique
    toReal( values[52], 0.0 ) );
  creationInterface->addDimLinear( d, dl );
}



/**
 * Adds an aligned dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAligned( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  // aligned dimension:
  DL_DimAlignedData da(
    // extension point 1
    toReal( values[13], 0.0 ),
    toReal( values[23], 0.0 ),
    toReal( values[33], 0.0 ),
    // extension point 2
    toReal( values[14], 0.0 ),
    toReal( values[24], 0.0 ),
    toReal( values[34], 0.0 ) );
  creationInterface->addDimAlign( d, da );
}



/**
 * Adds a radial dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimRadial( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  DL_DimRadialData dr(
    // definition point
    toReal( values[15], 0.0 ),
    toReal( values[25], 0.0 ),
    toReal( values[35], 0.0 ),
    // leader length:
    toReal( values[40], 0.0 ) );
  creationInterface->addDimRadial( d, dr );
}



/**
 * Adds a diametric dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimDiametric( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  // diametric dimension:
  DL_DimDiametricData dr(
    // definition point
    toReal( values[15], 0.0 ),
    toReal( values[25], 0.0 ),
    toReal( values[35], 0.0 ),
    // leader length:
    toReal( values[40], 0.0 ) );
  creationInterface->addDimDiametric( d, dr );
}



/**
 * Adds an angular dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAngular( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  // angular dimension:
  DL_DimAngularData da(
    // definition point 1
    toReal( values[13], 0.0 ),
    toReal( values[23], 0.0 ),
    toReal( values[33], 0.0 ),
    // definition point 2
    toReal( values[14], 0.0 ),
    toReal( values[24], 0.0 ),
    toReal( values[34], 0.0 ),
    // definition point 3
    toReal( values[15], 0.0 ),
    toReal( values[25], 0.0 ),
    toReal( values[35], 0.0 ),
    // definition point 4
    toReal( values[16], 0.0 ),
    toReal( values[26], 0.0 ),
    toReal( values[36], 0.0 ) );
  creationInterface->addDimAngular( d, da );
}


/**
 * Adds an angular dimension entity that was read from the file via the creation interface.
 */
void DL_Dxf::addDimAngular3P( DL_CreationInterface* creationInterface )
{
  DL_DimensionData d = getDimData();

  // angular dimension (3P):
  DL_DimAngular3PData da(
    // definition point 1
    toReal( values[13], 0.0 ),
    toReal( values[23], 0.0 ),
    toReal( values[33], 0.0 ),
    // definition point 2
    toReal( values[14], 0.0 ),
    toReal( values[24], 0.0 ),
    toReal( values[34], 0.0 ),
    // definition point 3
    toReal( values[15], 0.0 ),
    toReal( values[25], 0.0 ),
    toReal( values[35], 0.0 ) );
  creationInterface->addDimAngular3P( d, da );
}



/**
 * Adds a leader entity that was read from the file via the creation interface.
 */
void DL_Dxf::addLeader( DL_CreationInterface* creationInterface )
{
  // leader (arrow)
  DL_LeaderData le(
    // arrow head flag
    toInt( values[71], 1 ),
    // leader path type
    toInt( values[72], 0 ),
    // Leader creation flag
    toInt( values[73], 3 ),
    // Hookline direction flag
    toInt( values[74], 1 ),
    // Hookline flag
    toInt( values[75], 0 ),
    // Text annotation height
    toReal( values[40], 1.0 ),
    // Text annotation width
    toReal( values[41], 1.0 ),
    // Number of vertices in leader
    toInt( values[76], 0 )
  );
  creationInterface->addLeader( le );

  for ( int i = 0; i < maxLeaderVertices; i++ )
  {
    DL_LeaderVertexData d( leaderVertices[i*3],
                           leaderVertices[i*3+1],
                           leaderVertices[i*3+2] );

    creationInterface->addLeaderVertex( d );
  }
}



/**
 * Adds a hatch entity that was read from the file via the creation interface.
 */
void DL_Dxf::addHatch( DL_CreationInterface* creationInterface )
{
  DL_HatchData hd( toInt( values[91], 1 ),
                   toInt( values[70], 0 ),
                   toReal( values[41], 1.0 ),
                   toReal( values[52], 0.0 ),
                   values[2] );
  creationInterface->addHatch( hd );

  for ( int l = 0; l < maxHatchLoops; l++ )
  {
    DL_HatchLoopData ld( maxHatchEdges[l] );
    creationInterface->addHatchLoop( ld );
    for ( int b = 0; b < maxHatchEdges[l]; b++ )
    {
      creationInterface->addHatchEdge( hatchEdges[l][b] );
    }
  }
  creationInterface->endEntity();
  currentEntity = DL_Unknown;
}



/**
 * Adds an image entity that was read from the file via the creation interface.
 */
void DL_Dxf::addImage( DL_CreationInterface* creationInterface )
{
  DL_ImageData id( // pass ref insead of name we don't have yet
    values[340],
    // ins point:
    toReal( values[10], 0.0 ),
    toReal( values[20], 0.0 ),
    toReal( values[30], 0.0 ),
    // u vector:
    toReal( values[11], 1.0 ),
    toReal( values[21], 0.0 ),
    toReal( values[31], 0.0 ),
    // v vector:
    toReal( values[12], 0.0 ),
    toReal( values[22], 1.0 ),
    toReal( values[32], 0.0 ),
    // image size (pixel):
    toInt( values[13], 1 ),
    toInt( values[23], 1 ),
    // brightness, contrast, fade
    toInt( values[281], 50 ),
    toInt( values[282], 50 ),
    toInt( values[283], 0 ) );

  creationInterface->addImage( id );
  creationInterface->endEntity();
  currentEntity = DL_Unknown;
}



/**
 * Adds an image definition that was read from the file via the creation interface.
 */
void DL_Dxf::addImageDef( DL_CreationInterface* creationInterface )
{
  DL_ImageDefData id( // handle
    values[5],
    values[1] );

  creationInterface->linkImage( id );
  creationInterface->endEntity();
  currentEntity = DL_Unknown;
}



/**
 * Ends some special entities like hatches or old style polylines.
 */
void DL_Dxf::endEntity( DL_CreationInterface* creationInterface )
{
  creationInterface->endEntity();
}


/**
 * Ends a sequence and notifies the creation interface.
 */
void DL_Dxf::endSequence( DL_CreationInterface* creationInterface )
{
  creationInterface->endSequence();
}


/**
 * Converts the given string into an int.
 * ok is set to false if there was an error.
 */
int DL_Dxf::stringToInt( const char* s, bool* ok )
{
  if ( ok != NULL )
  {
    // check string:
    *ok = true;
    int i = 0;
    bool dot = false;
    do
    {
      if ( s[i] == '\0' )
      {
        break;
      }
      else if ( s[i] == '.' )
      {
        if ( dot == true )
        {
// QgsDebugMsg("two dots");
          *ok = false;
        }
        else
        {
          dot = true;
        }
      }
      else if ( s[i] < '0' || s[i] > '9' )
      {
// QgsDebugMsg(QString("NaN: '%1'").arg(s[i]));
        *ok = false;
      }
      i++;
    }
    while ( s[i] != '\0' && *ok == true );
  }

  return atoi( s );
}


/**
 * @brief Opens the given file for writing and returns a pointer
 * to the dxf writer. This pointer needs to be passed on to other
 * writing functions.
 *
 * @param file Full path of the file to open.
 *
 * @return Pointer to an ascii dxf writer object.
 */
DL_WriterA* DL_Dxf::out( const char* file, DL_Codes::version version )
{
  char* f = new char[strlen( file )+1];
  strcpy( f, file );
  this->version = version;

  DL_WriterA* dw = new DL_WriterA( f, version );
  if ( dw->openFailed() )
  {
    delete dw;
    delete[] f;
    return NULL;
  }
  else
  {
    delete[] f;
    return dw;
  }
}



/**
 * @brief Writes a DXF header to the file currently opened
 * by the given DXF writer object.
 */
void DL_Dxf::writeHeader( DL_WriterA& dw )
{
  dw.comment( "dxflib " DL_VERSION );
  dw.sectionHeader();

  dw.dxfString( 9, "$ACADVER" );
  switch ( version )
  {
    case DL_Codes::AC1009:
      dw.dxfString( 1, "AC1009" );
      break;
    case DL_Codes::AC1012:
      dw.dxfString( 1, "AC1012" );
      break;
    case DL_Codes::AC1014:
      dw.dxfString( 1, "AC1014" );
      break;
    case DL_Codes::AC1015:
      dw.dxfString( 1, "AC1015" );
      break;
  }

  // Newer version require that (otherwise a*cad crashes..)
  if ( version == VER_2000 )
  {
    dw.dxfString( 9, "$HANDSEED" );
    dw.dxfHex( 5, 0xFFFF );
  }

  //dw.sectionEnd();
}




/**
 * Writes a point entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writePoint( DL_WriterA& dw,
                         const DL_PointData& data,
                         const DL_Attributes& attrib )
{
  dw.entity( "POINT" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbPoint" );
  }
  dw.entityAttributes( attrib );
  dw.coord( POINT_COORD_CODE, data.x, data.y );
}



/**
 * Writes a line entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeLine( DL_WriterA& dw,
                        const DL_LineData& data,
                        const DL_Attributes& attrib )
{
  dw.entity( "LINE" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbLine" );
  }
  dw.entityAttributes( attrib );
  dw.coord( LINE_START_CODE, data.x1, data.y1 );
  dw.coord( LINE_END_CODE, data.x2, data.y2 );
}



/**
 * Writes a polyline entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeVertex
 */
void DL_Dxf::writePolyline( DL_WriterA& dw,
                            const DL_PolylineData& data,
                            const DL_Attributes& attrib )
{
  if ( version == VER_2000 )
  {
    dw.entity( "LWPOLYLINE" );
    dw.entityAttributes( attrib );
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbPolyline" );
    dw.dxfInt( 90, ( int )data.number );
    dw.dxfInt( 70, data.flags );
  }
  else
  {
    dw.entity( "POLYLINE" );
    dw.entityAttributes( attrib );
    polylineLayer = attrib.getLayer();
    dw.dxfInt( 66, 1 );
    dw.dxfInt( 70, data.flags );
    dw.coord( VERTEX_COORD_CODE, 0.0, 0.0 );
  }
}



/**
 * Writes a single vertex of a polyline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeVertex( DL_WriterA& dw,
                          const DL_VertexData& data )
{


  if ( version == VER_2000 )
  {
    dw.dxfReal( 10, data.x );
    dw.dxfReal( 20, data.y );
    if ( qAbs( data.bulge ) > 1.0e-10 )
    {
      dw.dxfReal( 42, data.bulge );
    }
  }
  else
  {
    dw.entity( "VERTEX" );
    //dw.entityAttributes(attrib);
    dw.dxfString( 8, polylineLayer );
    dw.coord( VERTEX_COORD_CODE, data.x, data.y );
    if ( qAbs( data.bulge ) > 1.0e-10 )
    {
      dw.dxfReal( 42, data.bulge );
    }
  }
}



/**
 * Writes the polyline end. Only needed for DXF R12.
 */
void DL_Dxf::writePolylineEnd( DL_WriterA& dw )
{
  if ( version == VER_2000 )
  {
  }
  else
  {
    dw.entity( "SEQEND" );
  }
}


/**
 * Writes a spline entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeControlPoint
 */
void DL_Dxf::writeSpline( DL_WriterA& dw,
                          const DL_SplineData& data,
                          const DL_Attributes& attrib )
{

  dw.entity( "SPLINE" );
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbSpline" );
  }
  dw.dxfInt( 70, data.flags );
  dw.dxfInt( 71, data.degree );
  dw.dxfInt( 72, data.nKnots );          // number of knots
  dw.dxfInt( 73, data.nControl );        // number of control points
  dw.dxfInt( 74, 0 );                    // number of fit points
}



/**
 * Writes a single control point of a spline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeControlPoint( DL_WriterA& dw,
                                const DL_ControlPointData& data )
{

  dw.dxfReal( 10, data.x );
  dw.dxfReal( 20, data.y );
  dw.dxfReal( 30, data.z );
}



/**
 * Writes a single knot of a spline to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeKnot( DL_WriterA& dw,
                        const DL_KnotData& data )
{

  dw.dxfReal( 40, data.k );
}



/**
 * Writes a circle entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeCircle( DL_WriterA& dw,
                          const DL_CircleData& data,
                          const DL_Attributes& attrib )
{
  dw.entity( "CIRCLE" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbCircle" );
  }
  dw.entityAttributes( attrib );
  dw.coord( 10, data.cx, data.cy );
  dw.dxfReal( 40, data.radius );
}



/**
 * Writes an arc entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeArc( DL_WriterA& dw,
                       const DL_ArcData& data,
                       const DL_Attributes& attrib )
{
  dw.entity( "ARC" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbCircle" );
  }
  dw.coord( 10, data.cx, data.cy );
  dw.dxfReal( 40, data.radius );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbArc" );
  }
  dw.dxfReal( 50, data.angle1 );
  dw.dxfReal( 51, data.angle2 );
}



/**
 * Writes an ellipse entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeEllipse( DL_WriterA& dw,
                           const DL_EllipseData& data,
                           const DL_Attributes& attrib )
{

  if ( version > VER_R12 )
  {
    dw.entity( "ELLIPSE" );
    if ( version == VER_2000 )
    {
      dw.dxfString( 100, "AcDbEntity" );
      dw.dxfString( 100, "AcDbEllipse" );
    }
    dw.entityAttributes( attrib );
    dw.coord( 10, data.cx, data.cy );
    dw.coord( 11, data.mx, data.my );
    dw.dxfReal( 40, data.ratio );
    dw.dxfReal( 41, data.angle1 );
    dw.dxfReal( 42, data.angle2 );
  }
}



/**
 * Writes an insert to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeInsert( DL_WriterA& dw,
                          const DL_InsertData& data,
                          const DL_Attributes& attrib )
{

  if ( data.name.empty() )
  {
    QgsDebugMsg( "Block name must not be empty" );
    return;
  }

  dw.entity( "INSERT" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbBlockReference" );
  }
  dw.entityAttributes( attrib );
  dw.dxfString( 2, data.name );
  dw.dxfReal( 10, data.ipx );
  dw.dxfReal( 20, data.ipy );
  dw.dxfReal( 30, 0.0 );
  if ( data.sx != 1.0 || data.sy != 1.0 )
  {
    dw.dxfReal( 41, data.sx );
    dw.dxfReal( 42, data.sy );
    dw.dxfReal( 43, 1.0 );
  }
  if ( data.angle != 0.0 )
  {
    dw.dxfReal( 50, data.angle );
  }
  if ( data.cols != 1 || data.rows != 1 )
  {
    dw.dxfInt( 70, data.cols );
    dw.dxfInt( 71, data.rows );
  }
  if ( data.colSp != 0.0 || data.rowSp != 0.0 )
  {
    dw.dxfReal( 44, data.colSp );
    dw.dxfReal( 45, data.rowSp );
  }

}



/**
 * Writes a multi text entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeMText( DL_WriterA& dw,
                         const DL_MTextData& data,
                         const DL_Attributes& attrib )
{

  dw.entity( "MTEXT" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbMText" );
  }
  dw.entityAttributes( attrib );
  dw.dxfReal( 10, data.ipx );
  dw.dxfReal( 20, data.ipy );
  dw.dxfReal( 30, 0.0 );
  dw.dxfReal( 40, data.height );
  dw.dxfReal( 41, data.width );

  dw.dxfInt( 71, data.attachmentPoint );
  dw.dxfInt( 72, data.drawingDirection );

  // Creare text chunks of 250 characters each:
  int length = data.text.length();
  char chunk[251];
  int i;
  for ( i = 250; i < length; i += 250 )
  {
    strncpy( chunk, &data.text.c_str()[i-250], 250 );
    chunk[250] = '\0';
    dw.dxfString( 3, chunk );
  }
  strncpy( chunk, &data.text.c_str()[i-250], 250 );
  chunk[250] = '\0';
  dw.dxfString( 1, chunk );

  dw.dxfString( 7, data.style );

  // since dxflib 2.0.2.1: degrees not rad (error in autodesk dxf doc)
  dw.dxfReal( 50, data.angle / ( 2.0*M_PI )*360.0 );

  dw.dxfInt( 73, data.lineSpacingStyle );
  dw.dxfReal( 44, data.lineSpacingFactor );
}



/**
 * Writes a text entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeText( DL_WriterA& dw,
                        const DL_TextData& data,
                        const DL_Attributes& attrib )
{

  dw.entity( "TEXT" );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbText" );
  }
  dw.entityAttributes( attrib );
  dw.dxfReal( 10, data.ipx );
  dw.dxfReal( 20, data.ipy );
  dw.dxfReal( 30, 0.0 );
  dw.dxfReal( 40, data.height );
  dw.dxfString( 1, data.text );
  dw.dxfReal( 50, data.angle / ( 2*M_PI )*360.0 );
  dw.dxfReal( 41, data.xScaleFactor );
  dw.dxfString( 7, data.style );

  dw.dxfInt( 71, data.textGenerationFlags );
  dw.dxfInt( 72, data.hJustification );

  dw.dxfReal( 11, data.apx );
  dw.dxfReal( 21, data.apy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 73, data.vJustification );
}


/**
 * Writes an aligned dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific aligned dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAligned( DL_WriterA& dw,
                              const DL_DimensionData& data,
                              const DL_DimAlignedData& edata,
                              const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 1 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbAlignedDimension" );
  }

  dw.dxfReal( 13, edata.epx1 );
  dw.dxfReal( 23, edata.epy1 );
  dw.dxfReal( 33, 0.0 );

  dw.dxfReal( 14, edata.epx2 );
  dw.dxfReal( 24, edata.epy2 );
  dw.dxfReal( 34, 0.0 );
}



/**
 * Writes a linear dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific linear dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimLinear( DL_WriterA& dw,
                             const DL_DimensionData& data,
                             const DL_DimLinearData& edata,
                             const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 0 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbAlignedDimension" );
  }

  dw.dxfReal( 13, edata.dpx1 );
  dw.dxfReal( 23, edata.dpy1 );
  dw.dxfReal( 33, 0.0 );

  dw.dxfReal( 14, edata.dpx2 );
  dw.dxfReal( 24, edata.dpy2 );
  dw.dxfReal( 34, 0.0 );

  dw.dxfReal( 50, edata.angle / ( 2.0*M_PI )*360.0 );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbRotatedDimension" );
    /*
    dw.dxfString(1001, "ACAD");
    dw.dxfString(1000, "DSTYLE");
    dw.dxfString(1002, "{");
    dw.dxfInt(1070, 340);
    dw.dxfInt(1005, 11);
    dw.dxfString(1002, "}");
    */
  }
}



/**
 * Writes a radial dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific radial dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimRadial( DL_WriterA& dw,
                             const DL_DimensionData& data,
                             const DL_DimRadialData& edata,
                             const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 4 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbRadialDimension" );
  }

  dw.dxfReal( 15, edata.dpx );
  dw.dxfReal( 25, edata.dpy );
  dw.dxfReal( 35, 0.0 );

  dw.dxfReal( 40, edata.leader );
}



/**
 * Writes a diametric dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific diametric dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimDiametric( DL_WriterA& dw,
                                const DL_DimensionData& data,
                                const DL_DimDiametricData& edata,
                                const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 3 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDiametricDimension" );
  }

  dw.dxfReal( 15, edata.dpx );
  dw.dxfReal( 25, edata.dpy );
  dw.dxfReal( 35, 0.0 );

  dw.dxfReal( 40, edata.leader );
}



/**
 * Writes an angular dimension entity to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific angular dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAngular( DL_WriterA& dw,
                              const DL_DimensionData& data,
                              const DL_DimAngularData& edata,
                              const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 2 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDb2LineAngularDimension" );
  }

  dw.dxfReal( 13, edata.dpx1 );
  dw.dxfReal( 23, edata.dpy1 );
  dw.dxfReal( 33, 0.0 );

  dw.dxfReal( 14, edata.dpx2 );
  dw.dxfReal( 24, edata.dpy2 );
  dw.dxfReal( 34, 0.0 );

  dw.dxfReal( 15, edata.dpx3 );
  dw.dxfReal( 25, edata.dpy3 );
  dw.dxfReal( 35, 0.0 );

  dw.dxfReal( 16, edata.dpx4 );
  dw.dxfReal( 26, edata.dpy4 );
  dw.dxfReal( 36, 0.0 );
}



/**
 * Writes an angular dimension entity (3 points version) to the file.
 *
 * @param dw DXF writer
 * @param data Generic dimension data for from the file
 * @param data Specific angular dimension data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeDimAngular3P( DL_WriterA& dw,
                                const DL_DimensionData& data,
                                const DL_DimAngular3PData& edata,
                                const DL_Attributes& attrib )
{

  dw.entity( "DIMENSION" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
  }
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimension" );
  }

  dw.dxfReal( 10, data.dpx );
  dw.dxfReal( 20, data.dpy );
  dw.dxfReal( 30, 0.0 );

  dw.dxfReal( 11, data.mpx );
  dw.dxfReal( 21, data.mpy );
  dw.dxfReal( 31, 0.0 );

  dw.dxfInt( 70, 5 );
  if ( version > VER_R12 )
  {
    dw.dxfInt( 71, data.attachmentPoint );
    dw.dxfInt( 72, data.lineSpacingStyle ); // opt
    dw.dxfReal( 41, data.lineSpacingFactor ); // opt
  }

  dw.dxfReal( 42, data.angle );

  dw.dxfString( 1, data.text ); // opt
  //dw.dxfString(3, data.style);
  dw.dxfString( 3, "Standard" );

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDb3PointAngularDimension" );
  }

  dw.dxfReal( 13, edata.dpx1 );
  dw.dxfReal( 23, edata.dpy1 );
  dw.dxfReal( 33, 0.0 );

  dw.dxfReal( 14, edata.dpx2 );
  dw.dxfReal( 24, edata.dpy2 );
  dw.dxfReal( 34, 0.0 );

  dw.dxfReal( 15, edata.dpx3 );
  dw.dxfReal( 25, edata.dpy3 );
  dw.dxfReal( 35, 0.0 );
}



/**
 * Writes a leader entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 * @see writeVertex
 */
void DL_Dxf::writeLeader( DL_WriterA& dw,
                          const DL_LeaderData& data,
                          const DL_Attributes& attrib )
{
  if ( version > VER_R12 )
  {
    dw.entity( "LEADER" );
    dw.entityAttributes( attrib );
    if ( version == VER_2000 )
    {
      dw.dxfString( 100, "AcDbEntity" );
      dw.dxfString( 100, "AcDbLeader" );
    }
    dw.dxfString( 3, "Standard" );
    dw.dxfInt( 71, data.arrowHeadFlag );
    dw.dxfInt( 72, data.leaderPathType );
    dw.dxfInt( 73, data.leaderCreationFlag );
    dw.dxfInt( 74, data.hooklineDirectionFlag );
    dw.dxfInt( 75, data.hooklineFlag );
    dw.dxfReal( 40, data.textAnnotationHeight );
    dw.dxfReal( 41, data.textAnnotationWidth );
    dw.dxfInt( 76, data.number );
  }
}



/**
 * Writes a single vertex of a leader to the file.
 *
 * @param dw DXF writer
 * @param data Entity data
 */
void DL_Dxf::writeLeaderVertex( DL_WriterA& dw,
                                const DL_LeaderVertexData& data )
{
  if ( version > VER_R12 )
  {
    dw.dxfReal( 10, data.x );
    dw.dxfReal( 20, data.y );
  }
}



/**
 * Writes the beginning of a hatch entity to the file.
 * This must be followed by one or more writeHatchLoop()
 * calls and a writeHatch2() call.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatch1( DL_WriterA& dw,
                          const DL_HatchData& data,
                          const DL_Attributes& attrib )
{

  dw.entity( "HATCH" );
  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbHatch" );
  }
  dw.dxfReal( 10, 0.0 );           // elevation
  dw.dxfReal( 20, 0.0 );
  dw.dxfReal( 30, 0.0 );
  dw.dxfReal( 210, 0.0 );           // extrusion dir.
  dw.dxfReal( 220, 0.0 );
  dw.dxfReal( 230, 1.0 );
  if ( data.solid == false )
  {
    dw.dxfString( 2, data.pattern );
  }
  else
  {
    dw.dxfString( 2, "SOLID" );
  }
  dw.dxfInt( 70, ( int )data.solid );
  dw.dxfInt( 71, 0 );              // associative
  dw.dxfInt( 91, data.numLoops );
}



/**
 * Writes the end of a hatch entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatch2( DL_WriterA& dw,
                          const DL_HatchData& data,
                          const DL_Attributes& /*attrib*/ )
{

  dw.dxfInt( 75, 0 );              // odd parity
  dw.dxfInt( 76, 1 );              // pattern type
  if ( data.solid == false )
  {
    dw.dxfReal( 52, data.angle );
    dw.dxfReal( 41, data.scale );
    dw.dxfInt( 77, 0 );          // not double
    //dw.dxfInt(78, 0);
    dw.dxfInt( 78, 1 );
    dw.dxfReal( 53, 45.0 );
    dw.dxfReal( 43, 0.0 );
    dw.dxfReal( 44, 0.0 );
    dw.dxfReal( 45, -0.0883883476483184 );
    dw.dxfReal( 46, 0.0883883476483185 );
    dw.dxfInt( 79, 0 );
  }
  dw.dxfInt( 98, 0 );
}



/**
 * Writes the beginning of a hatch loop to the file. This
 * must happen after writing the beginning of a hatch entity.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchLoop1( DL_WriterA& dw,
                              const DL_HatchLoopData& data )
{

  dw.dxfInt( 92, 1 );
  dw.dxfInt( 93, data.numEdges );
  //dw.dxfInt(97, 0);
}



/**
 * Writes the end of a hatch loop to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchLoop2( DL_WriterA& dw,
                              const DL_HatchLoopData& /*data*/ )
{

  dw.dxfInt( 97, 0 );
}


/**
 * Writes the beginning of a hatch entity to the file.
 *
 * @param dw DXF writer
 * @param data Entity data.
 * @param attrib Attributes
 */
void DL_Dxf::writeHatchEdge( DL_WriterA& dw,
                             const DL_HatchEdgeData& data )
{

  dw.dxfInt( 72, data.type );

  switch ( data.type )
  {
    case 1:
      dw.dxfReal( 10, data.x1 );
      dw.dxfReal( 20, data.y1 );
      dw.dxfReal( 11, data.x2 );
      dw.dxfReal( 21, data.y2 );
      break;
    case 2:
      dw.dxfReal( 10, data.cx );
      dw.dxfReal( 20, data.cy );
      dw.dxfReal( 40, data.radius );
      dw.dxfReal( 50, data.angle1 / ( 2*M_PI )*360.0 );
      dw.dxfReal( 51, data.angle2 / ( 2*M_PI )*360.0 );
      dw.dxfInt( 73, ( int )( data.ccw ) );
      break;
    default:
      break;
  }
}



/**
 * Writes an image entity.
 *
 * @return IMAGEDEF handle. Needed for the IMAGEDEF counterpart.
 */
int DL_Dxf::writeImage( DL_WriterA& dw,
                        const DL_ImageData& data,
                        const DL_Attributes& attrib )
{
#if 0
  if ( data.file.empty() )
  {
    QgsDebugMsg( "Image file must not be empty" );
    return;
  }
#endif

  dw.entity( "IMAGE" );

  dw.entityAttributes( attrib );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbEntity" );
    dw.dxfString( 100, "AcDbRasterImage" );
    dw.dxfInt( 90, 0 );
  }
  // insertion point
  dw.dxfReal( 10, data.ipx );
  dw.dxfReal( 20, data.ipy );
  dw.dxfReal( 30, 0.0 );

  // vector along bottom side (1 pixel long)
  dw.dxfReal( 11, data.ux );
  dw.dxfReal( 21, data.uy );
  dw.dxfReal( 31, 0.0 );

  // vector along left side (1 pixel long)
  dw.dxfReal( 12, data.vx );
  dw.dxfReal( 22, data.vy );
  dw.dxfReal( 32, 0.0 );

  // image size in pixel
  dw.dxfReal( 13, data.width );
  dw.dxfReal( 23, data.height );

  // handle of IMAGEDEF object
  int handle = dw.incHandle();
  dw.dxfHex( 340, handle );

  // flags
  dw.dxfInt( 70, 15 );

  // clipping:
  dw.dxfInt( 280, 0 );

  // brightness, contrast, fade
  dw.dxfInt( 281, data.brightness );
  dw.dxfInt( 282, data.contrast );
  dw.dxfInt( 283, data.fade );

  return handle;
}



/**
 * Writes an image definiition entity.
 */
void DL_Dxf::writeImageDef( DL_WriterA& dw,
                            int handle,
                            const DL_ImageData& data )
{

  /*if (data.file.empty()) {
      QgsDebugMsg("DL_Dxf::writeImage: Image file must not be empty");
      return;
  }*/

  dw.dxfString( 0, "IMAGEDEF" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, handle );
  }

  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbRasterImageDef" );
    dw.dxfInt( 90, 0 );
  }
  // file name:
  dw.dxfString( 1, data.ref );

  // image size in pixel
  dw.dxfReal( 10, data.width );
  dw.dxfReal( 20, data.height );

  dw.dxfReal( 11, 1.0 );
  dw.dxfReal( 21, 1.0 );

  // loaded:
  dw.dxfInt( 280, 1 );
  // units:
  dw.dxfInt( 281, 0 );
}


/**
 * Writes a layer to the file. Layers are stored in the
 * tables section of a DXF file.
 *
 * @param dw DXF writer
 * @param data Entity data from the file
 * @param attrib Attributes
 */
void DL_Dxf::writeLayer( DL_WriterA& dw,
                         const DL_LayerData& data,
                         const DL_Attributes& attrib )
{

  if ( data.name.empty() )
  {
    QgsDebugMsg( "DL_Dxf::writeLayer: Layer name must not be empty" );
    return;
  }

  int color = attrib.getColor();
  if ( color <= 0 || color >= 256 )
  {
    QgsDebugMsg( QString( "Layer color cannot be %1. Changed to 7." ).arg( color ) );
    color = 7;
  }

  if ( data.name == "0" )
  {
    dw.tableLayerEntry( 0x10 );
  }
  else
  {
    dw.tableLayerEntry();
  }

  dw.dxfString( 2, data.name );
  dw.dxfInt( 70, data.flags );
  dw.dxfInt( 62, color );

  dw.dxfString( 6, ( attrib.getLineType().length() == 0 ?
                     string( "CONTINUOUS" ) : attrib.getLineType() ) );

  if ( version >= VER_2000 )
  {
    // layer defpoints cannot be plotted
    std::string lstr = data.name;
    std::transform( lstr.begin(), lstr.end(), lstr.begin(), tolower );
    if ( lstr == "defpoints" )
    {
      dw.dxfInt( 290, 0 );
    }
  }
  if ( version >= VER_2000 && attrib.getWidth() != -1 )
  {
    dw.dxfInt( 370, attrib.getWidth() );
  }
  if ( version >= VER_2000 )
  {
    dw.dxfHex( 390, 0xF );
  }
}



/**
 * Writes a line type to the file. Line types are stored in the
 * tables section of a DXF file.
 */
void DL_Dxf::writeLineType( DL_WriterA& dw,
                            const DL_LineTypeData& data )
{
  //const char* description,
  //int elements,
  //double patternLength) {

  if ( data.name.empty() )
  {
    QgsDebugMsg( "DL_Dxf::writeLineType: Line type name must not be empty" );
    return;
  }

  // ignore BYLAYER, BYBLOCK for R12
  if ( version < VER_2000 )
  {
    if ( !strcasecmp( data.name.c_str(), "BYBLOCK" ) ||
         !strcasecmp( data.name.c_str(), "BYLAYER" ) )
    {
      return;
    }
  }

  // write id (not for R12)
  if ( !strcasecmp( data.name.c_str(), "BYBLOCK" ) )
  {
    dw.tableLineTypeEntry( 0x14 );
  }
  else if ( !strcasecmp( data.name.c_str(), "BYLAYER" ) )
  {
    dw.tableLineTypeEntry( 0x15 );
  }
  else if ( !strcasecmp( data.name.c_str(), "CONTINUOUS" ) )
  {
    dw.tableLineTypeEntry( 0x16 );
  }
  else
  {
    dw.tableLineTypeEntry();
  }

  dw.dxfString( 2, data.name );
  //if (version>=VER_2000) {
  dw.dxfInt( 70, data.flags );
  //}

  if ( !strcasecmp( data.name.c_str(), "BYBLOCK" ) )
  {
    dw.dxfString( 3, "" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 0 );
    dw.dxfReal( 40, 0.0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "BYLAYER" ) )
  {
    dw.dxfString( 3, "" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 0 );
    dw.dxfReal( 40, 0.0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "CONTINUOUS" ) )
  {
    dw.dxfString( 3, "Solid line" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 0 );
    dw.dxfReal( 40, 0.0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "ACAD_ISO02W100" ) )
  {
    dw.dxfString( 3, "ISO Dashed __ __ __ __ __ __ __ __ __ __ _" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 15.0 );
    dw.dxfReal( 49, 12.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "ACAD_ISO03W100" ) )
  {
    dw.dxfString( 3, "ISO Dashed with Distance __    __    __    _" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 30.0 );
    dw.dxfReal( 49, 12.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -18.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "ACAD_ISO04W100" ) )
  {
    dw.dxfString( 3, "ISO Long Dashed Dotted ____ . ____ . __" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 30.0 );
    dw.dxfReal( 49, 24.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "ACAD_ISO05W100" ) )
  {
    dw.dxfString( 3, "ISO Long Dashed Double Dotted ____ .. __" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 33.0 );
    dw.dxfReal( 49, 24.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "BORDER" ) )
  {
    dw.dxfString( 3, "Border __ __ . __ __ . __ __ . __ __ . __ __ ." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 44.45 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "BORDER2" ) )
  {
    dw.dxfString( 3, "Border (.5x) __.__.__.__.__.__.__.__.__.__.__." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 22.225 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "BORDERX2" ) )
  {
    dw.dxfString( 3, "Border (2x) ____  ____  .  ____  ____  .  ___" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 88.9 );
    dw.dxfReal( 49, 25.4 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 25.4 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "CENTER" ) )
  {
    dw.dxfString( 3, "Center ____ _ ____ _ ____ _ ____ _ ____ _ ____" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 50.8 );
    dw.dxfReal( 49, 31.75 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "CENTER2" ) )
  {
    dw.dxfString( 3, "Center (.5x) ___ _ ___ _ ___ _ ___ _ ___ _ ___" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 28.575 );
    dw.dxfReal( 49, 19.05 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "CENTERX2" ) )
  {
    dw.dxfString( 3, "Center (2x) ________  __  ________  __  _____" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 101.6 );
    dw.dxfReal( 49, 63.5 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHDOT" ) )
  {
    dw.dxfString( 3, "Dash dot __ . __ . __ . __ . __ . __ . __ . __" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 25.4 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHDOT2" ) )
  {
    dw.dxfString( 3, "Dash dot (.5x) _._._._._._._._._._._._._._._." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 12.7 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHDOTX2" ) )
  {
    dw.dxfString( 3, "Dash dot (2x) ____  .  ____  .  ____  .  ___" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 4 );
    dw.dxfReal( 40, 50.8 );
    dw.dxfReal( 49, 25.4 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHED" ) )
  {
    dw.dxfString( 3, "Dashed __ __ __ __ __ __ __ __ __ __ __ __ __ _" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 19.05 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHED2" ) )
  {
    dw.dxfString( 3, "Dashed (.5x) _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 9.525 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DASHEDX2" ) )
  {
    dw.dxfString( 3, "Dashed (2x) ____  ____  ____  ____  ____  ___" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 38.1 );
    dw.dxfReal( 49, 25.4 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DIVIDE" ) )
  {
    dw.dxfString( 3, "Divide ____ . . ____ . . ____ . . ____ . . ____" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 31.75 );
    dw.dxfReal( 49, 12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DIVIDE2" ) )
  {
    dw.dxfString( 3, "Divide (.5x) __..__..__..__..__..__..__..__.._" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 15.875 );
    dw.dxfReal( 49, 6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DIVIDEX2" ) )
  {
    dw.dxfString( 3, "Divide (2x) ________  .  .  ________  .  .  _" );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 6 );
    dw.dxfReal( 40, 63.5 );
    dw.dxfReal( 49, 25.4 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DOT" ) )
  {
    dw.dxfString( 3, "Dot . . . . . . . . . . . . . . . . . . . . . ." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 6.35 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -6.35 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DOT2" ) )
  {
    dw.dxfString( 3, "Dot (.5x) ....................................." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 3.175 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -3.175 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else if ( !strcasecmp( data.name.c_str(), "DOTX2" ) )
  {
    dw.dxfString( 3, "Dot (2x) .  .  .  .  .  .  .  .  .  .  .  .  ." );
    dw.dxfInt( 72, 65 );
    dw.dxfInt( 73, 2 );
    dw.dxfReal( 40, 12.7 );
    dw.dxfReal( 49, 0.0 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
    dw.dxfReal( 49, -12.7 );
    if ( version >= VER_R13 )
      dw.dxfInt( 74, 0 );
  }
  else
  {
    QgsDebugMsg( "dxflib warning: DL_Dxf::writeLineType: Unknown Line Type" );
  }
}



/**
 * Writes the APPID section to the DXF file.
 *
 * @param name Application name
 */
void DL_Dxf::writeAppid( DL_WriterA& dw, const string& name )
{
  if ( name.empty() )
  {
    QgsDebugMsg( "DL_Dxf::writeAppid: Application  name must not be empty" );
    return;
  }

  if ( !strcasecmp( name.c_str(), "ACAD" ) )
  {
    dw.tableAppidEntry( 0x12 );
  }
  else
  {
    dw.tableAppidEntry();
  }
  dw.dxfString( 2, name );
  dw.dxfInt( 70, 0 );
}



/**
 * Writes a block's definition (no entities) to the DXF file.
 */
void DL_Dxf::writeBlock( DL_WriterA& dw, const DL_BlockData& data )
{
  if ( data.name.empty() )
  {
    QgsDebugMsg( "DL_Dxf::writeBlock: Block name must not be empty" );
    return;
  }

  //bool paperSpace = !strcasecmp(name, "*paper_space");
  //!strcasecmp(name, "*paper_space0");

  if ( !strcasecmp( data.name.c_str(), "*paper_space" ) )
  {
    dw.sectionBlockEntry( 0x1C );
  }
  else if ( !strcasecmp( data.name.c_str(), "*model_space" ) )
  {
    dw.sectionBlockEntry( 0x20 );
  }
  else if ( !strcasecmp( data.name.c_str(), "*paper_space0" ) )
  {
    dw.sectionBlockEntry( 0x24 );
  }
  else
  {
    dw.sectionBlockEntry();
  }
  dw.dxfString( 2, data.name );
  dw.dxfInt( 70, 0 );
  dw.coord( 10, data.bpx, data.bpy );
  dw.dxfString( 3, data.name );
  dw.dxfString( 1, "" );
}



/**
 * Writes a block end.
 *
 * @param name Block name
 */
void DL_Dxf::writeEndBlock( DL_WriterA& dw, const string& name )
{
  if ( !strcasecmp( name.c_str(), "*paper_space" ) )
  {
    dw.sectionBlockEntryEnd( 0x1D );
  }
  else if ( !strcasecmp( name.c_str(), "*model_space" ) )
  {
    dw.sectionBlockEntryEnd( 0x21 );
  }
  else if ( !strcasecmp( name.c_str(), "*paper_space0" ) )
  {
    dw.sectionBlockEntryEnd( 0x25 );
  }
  else
  {
    dw.sectionBlockEntryEnd();
  }
}



/**
 * Writes a viewport section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked VPORT section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeVPort( DL_WriterA& dw )
{
  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "VPORT" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0x8 );
  }
  //dw.dxfHex(330, 0);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 1 );
  dw.dxfString( 0, "VPORT" );
  //dw.dxfHex(5, 0x2F);
  if ( version == VER_2000 )
  {
    dw.handle();
  }
  //dw.dxfHex(330, 8);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbViewportTableRecord" );
  }
  dw.dxfString( 2, "*Active" );
  dw.dxfInt( 70, 0 );
  dw.dxfReal( 10, 0.0 );
  dw.dxfReal( 20, 0.0 );
  dw.dxfReal( 11, 1.0 );
  dw.dxfReal( 21, 1.0 );
  dw.dxfReal( 12, 286.3055555555555 );
  dw.dxfReal( 22, 148.5 );
  dw.dxfReal( 13, 0.0 );
  dw.dxfReal( 23, 0.0 );
  dw.dxfReal( 14, 10.0 );
  dw.dxfReal( 24, 10.0 );
  dw.dxfReal( 15, 10.0 );
  dw.dxfReal( 25, 10.0 );
  dw.dxfReal( 16, 0.0 );
  dw.dxfReal( 26, 0.0 );
  dw.dxfReal( 36, 1.0 );
  dw.dxfReal( 17, 0.0 );
  dw.dxfReal( 27, 0.0 );
  dw.dxfReal( 37, 0.0 );
  dw.dxfReal( 40, 297.0 );
  dw.dxfReal( 41, 1.92798353909465 );
  dw.dxfReal( 42, 50.0 );
  dw.dxfReal( 43, 0.0 );
  dw.dxfReal( 44, 0.0 );
  dw.dxfReal( 50, 0.0 );
  dw.dxfReal( 51, 0.0 );
  dw.dxfInt( 71, 0 );
  dw.dxfInt( 72, 100 );
  dw.dxfInt( 73, 1 );
  dw.dxfInt( 74, 3 );
  dw.dxfInt( 75, 1 );
  dw.dxfInt( 76, 1 );
  dw.dxfInt( 77, 0 );
  dw.dxfInt( 78, 0 );

  if ( version == VER_2000 )
  {
    dw.dxfInt( 281, 0 );
    dw.dxfInt( 65, 1 );
    dw.dxfReal( 110, 0.0 );
    dw.dxfReal( 120, 0.0 );
    dw.dxfReal( 130, 0.0 );
    dw.dxfReal( 111, 1.0 );
    dw.dxfReal( 121, 0.0 );
    dw.dxfReal( 131, 0.0 );
    dw.dxfReal( 112, 0.0 );
    dw.dxfReal( 122, 1.0 );
    dw.dxfReal( 132, 0.0 );
    dw.dxfInt( 79, 0 );
    dw.dxfReal( 146, 0.0 );
  }
  dw.dxfString( 0, "ENDTAB" );
}



/**
 * Writes a style section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked STYLE section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeStyle( DL_WriterA& dw )
{
  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "STYLE" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 3 );
  }
  //dw.dxfHex(330, 0);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 1 );
  dw.dxfString( 0, "STYLE" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0x11 );
  }
  //styleHandleStd = dw.handle();
  //dw.dxfHex(330, 3);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbTextStyleTableRecord" );
  }
  dw.dxfString( 2, "Standard" );
  dw.dxfInt( 70, 0 );
  dw.dxfReal( 40, 0.0 );
  dw.dxfReal( 41, 0.75 );
  dw.dxfReal( 50, 0.0 );
  dw.dxfInt( 71, 0 );
  dw.dxfReal( 42, 2.5 );
  dw.dxfString( 3, "txt" );
  dw.dxfString( 4, "" );
  dw.dxfString( 0, "ENDTAB" );
}



/**
 * Writes a view section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked VIEW section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeView( DL_WriterA& dw )
{
  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "VIEW" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 6 );
  }
  //dw.dxfHex(330, 0);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 0 );
  dw.dxfString( 0, "ENDTAB" );
}



/**
 * Writes a ucs section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked UCS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeUcs( DL_WriterA& dw )
{
  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "UCS" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 7 );
  }
  //dw.dxfHex(330, 0);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 0 );
  dw.dxfString( 0, "ENDTAB" );
}



/**
 * Writes a dimstyle section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked DIMSTYLE section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeDimStyle( DL_WriterA& dw,
                            double dimasz, double dimexe, double dimexo,
                            double dimgap, double dimtxt )
{

  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "DIMSTYLE" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0xA );
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 1 );
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbDimStyleTable" );
    dw.dxfInt( 71, 0 );
  }


  dw.dxfString( 0, "DIMSTYLE" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 105, 0x27 );
  }
  //dw.handle(105);
  //dw.dxfHex(330, 0xA);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbDimStyleTableRecord" );
  }
  dw.dxfString( 2, "Standard" );
  if ( version == VER_R12 )
  {
    dw.dxfString( 3, "" );
    dw.dxfString( 4, "" );
    dw.dxfString( 5, "" );
    dw.dxfString( 6, "" );
    dw.dxfString( 7, "" );
    dw.dxfReal( 40, 1.0 );
  }

  dw.dxfReal( 41, dimasz );
  dw.dxfReal( 42, dimexo );
  dw.dxfReal( 43, 3.75 );
  dw.dxfReal( 44, dimexe );
  if ( version == VER_R12 )
  {
    dw.dxfReal( 45, 0.0 );
    dw.dxfReal( 46, 0.0 );
    dw.dxfReal( 47, 0.0 );
    dw.dxfReal( 48, 0.0 );
  }
  dw.dxfInt( 70, 0 );
  if ( version == VER_R12 )
  {
    dw.dxfInt( 71, 0 );
    dw.dxfInt( 72, 0 );
  }
  dw.dxfInt( 73, 0 );
  dw.dxfInt( 74, 0 );
  if ( version == VER_R12 )
  {
    dw.dxfInt( 75, 0 );
    dw.dxfInt( 76, 0 );
  }
  dw.dxfInt( 77, 1 );
  dw.dxfInt( 78, 8 );
  dw.dxfReal( 140, dimtxt );
  dw.dxfReal( 141, 2.5 );
  if ( version == VER_R12 )
  {
    dw.dxfReal( 142, 0.0 );
  }
  dw.dxfReal( 143, 0.03937007874016 );
  if ( version == VER_R12 )
  {
    dw.dxfReal( 144, 1.0 );
    dw.dxfReal( 145, 0.0 );
    dw.dxfReal( 146, 1.0 );
  }
  dw.dxfReal( 147, dimgap );
  if ( version == VER_R12 )
  {
    dw.dxfInt( 170, 0 );
  }
  dw.dxfInt( 171, 3 );
  dw.dxfInt( 172, 1 );
  if ( version == VER_R12 )
  {
    dw.dxfInt( 173, 0 );
    dw.dxfInt( 174, 0 );
    dw.dxfInt( 175, 0 );
    dw.dxfInt( 176, 0 );
    dw.dxfInt( 177, 0 );
    dw.dxfInt( 178, 0 );
  }
  if ( version == VER_2000 )
  {
    dw.dxfInt( 271, 2 );
    dw.dxfInt( 272, 2 );
    dw.dxfInt( 274, 3 );
    dw.dxfInt( 278, 44 );
    dw.dxfInt( 283, 0 );
    dw.dxfInt( 284, 8 );
    //dw.dxfHex(340, styleHandleStd);
    dw.dxfHex( 340, 0x11 );
  }
  // * /
  dw.dxfString( 0, "ENDTAB" );
}



/**
 * Writes a blockrecord section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked BLOCKRECORD section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeBlockRecord( DL_WriterA& dw )
{
  dw.dxfString( 0, "TABLE" );
  dw.dxfString( 2, "BLOCK_RECORD" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 1 );
  }
  //dw.dxfHex(330, 0);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTable" );
  }
  dw.dxfInt( 70, 1 );

  dw.dxfString( 0, "BLOCK_RECORD" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0x1F );
  }
  //int msh = dw.handle();
  //dw.setModelSpaceHandle(msh);
  //dw.dxfHex(330, 1);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbBlockTableRecord" );
  }
  dw.dxfString( 2, "*Model_Space" );
  dw.dxfHex( 340, 0x22 );

  dw.dxfString( 0, "BLOCK_RECORD" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0x1B );
  }
  //int psh = dw.handle();
  //dw.setPaperSpaceHandle(psh);
  //dw.dxfHex(330, 1);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbBlockTableRecord" );
  }
  dw.dxfString( 2, "*Paper_Space" );
  dw.dxfHex( 340, 0x1E );

  dw.dxfString( 0, "BLOCK_RECORD" );
  if ( version == VER_2000 )
  {
    dw.dxfHex( 5, 0x23 );
  }
  //int ps0h = dw.handle();
  //dw.setPaperSpace0Handle(ps0h);
  //dw.dxfHex(330, 1);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbBlockTableRecord" );
  }
  dw.dxfString( 2, "*Paper_Space0" );
  dw.dxfHex( 340, 0x26 );

  //dw.dxfString(  0, "ENDTAB");
}



/**
 * Writes a single block record with the given name.
 */
void DL_Dxf::writeBlockRecord( DL_WriterA& dw, const string& name )
{
  dw.dxfString( 0, "BLOCK_RECORD" );
  if ( version == VER_2000 )
  {
    dw.handle();
  }
  //dw->dxfHex(330, 1);
  if ( version == VER_2000 )
  {
    dw.dxfString( 100, "AcDbSymbolTableRecord" );
    dw.dxfString( 100, "AcDbBlockTableRecord" );
  }
  dw.dxfString( 2, name );
  dw.dxfHex( 340, 0 );
}



/**
 * Writes a objects section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked OBJECTS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeObjects( DL_WriterA& dw )
{
  //int dicId, dicId2, dicId3, dicId4, dicId5;
  //int dicId5;

  dw.dxfString( 0, "SECTION" );
  dw.dxfString( 2, "OBJECTS" );
  dw.dxfString( 0, "DICTIONARY" );
  dw.dxfHex( 5, 0xC );                          // C
  //dw.dxfHex(330, 0);
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 3, "ACAD_GROUP" );
  //dw.dxfHex(350, dw.getNextHandle());          // D
  dw.dxfHex( 350, 0xD );        // D
  dw.dxfString( 3, "ACAD_LAYOUT" );
  dw.dxfHex( 350, 0x1A );
  //dw.dxfHex(350, dw.getNextHandle()+0);        // 1A
  dw.dxfString( 3, "ACAD_MLINESTYLE" );
  dw.dxfHex( 350, 0x17 );
  //dw.dxfHex(350, dw.getNextHandle()+1);        // 17
  dw.dxfString( 3, "ACAD_PLOTSETTINGS" );
  dw.dxfHex( 350, 0x19 );
  //dw.dxfHex(350, dw.getNextHandle()+2);        // 19
  dw.dxfString( 3, "ACAD_PLOTSTYLENAME" );
  dw.dxfHex( 350, 0xE );
  //dw.dxfHex(350, dw.getNextHandle()+3);        // E
  dw.dxfString( 3, "AcDbVariableDictionary" );
  dw.dxfHex( 350, dw.getNextHandle() );      // 2C
  dw.dxfString( 0, "DICTIONARY" );
  dw.dxfHex( 5, 0xD );
  //dw.handle();                                    // D
  //dw.dxfHex(330, 0xC);
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 0, "ACDBDICTIONARYWDFLT" );
  dw.dxfHex( 5, 0xE );
  //dicId4 = dw.handle();                           // E
  //dw.dxfHex(330, 0xC);                       // C
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 3, "Normal" );
  dw.dxfHex( 350, 0xF );
  //dw.dxfHex(350, dw.getNextHandle()+5);        // F
  dw.dxfString( 100, "AcDbDictionaryWithDefault" );
  dw.dxfHex( 340, 0xF );
  //dw.dxfHex(340, dw.getNextHandle()+5);        // F
  dw.dxfString( 0, "ACDBPLACEHOLDER" );
  dw.dxfHex( 5, 0xF );
  //dw.handle();                                    // F
  //dw.dxfHex(330, dicId4);                      // E
  dw.dxfString( 0, "DICTIONARY" );
  //dicId3 = dw.handle();                           // 17
  dw.dxfHex( 5, 0x17 );
  //dw.dxfHex(330, 0xC);                       // C
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 3, "Standard" );
  dw.dxfHex( 350, 0x18 );
  //dw.dxfHex(350, dw.getNextHandle()+5);        // 18
  dw.dxfString( 0, "MLINESTYLE" );
  dw.dxfHex( 5, 0x18 );
  //dw.handle();                                    // 18
  //dw.dxfHex(330, dicId3);                      // 17
  dw.dxfString( 100, "AcDbMlineStyle" );
  dw.dxfString( 2, "STANDARD" );
  dw.dxfInt( 70, 0 );
  dw.dxfString( 3, "" );
  dw.dxfInt( 62, 256 );
  dw.dxfReal( 51, 90.0 );
  dw.dxfReal( 52, 90.0 );
  dw.dxfInt( 71, 2 );
  dw.dxfReal( 49, 0.5 );
  dw.dxfInt( 62, 256 );
  dw.dxfString( 6, "BYLAYER" );
  dw.dxfReal( 49, -0.5 );
  dw.dxfInt( 62, 256 );
  dw.dxfString( 6, "BYLAYER" );
  dw.dxfString( 0, "DICTIONARY" );
  dw.dxfHex( 5, 0x19 );
  //dw.handle();                           // 17
  //dw.dxfHex(330, 0xC);                       // C
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 0, "DICTIONARY" );
  //dicId2 = dw.handle();                           // 1A
  dw.dxfHex( 5, 0x1A );
  //dw.dxfHex(330, 0xC);
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 3, "Layout1" );
  dw.dxfHex( 350, 0x1E );
  //dw.dxfHex(350, dw.getNextHandle()+2);        // 1E
  dw.dxfString( 3, "Layout2" );
  dw.dxfHex( 350, 0x26 );
  //dw.dxfHex(350, dw.getNextHandle()+4);        // 26
  dw.dxfString( 3, "Model" );
  dw.dxfHex( 350, 0x22 );
  //dw.dxfHex(350, dw.getNextHandle()+5);        // 22

  dw.dxfString( 0, "LAYOUT" );
  dw.dxfHex( 5, 0x1E );
  //dw.handle();                                    // 1E
  //dw.dxfHex(330, dicId2);                      // 1A
  dw.dxfString( 100, "AcDbPlotSettings" );
  dw.dxfString( 1, "" );
  dw.dxfString( 2, "C:\\Program Files\\AutoCAD 2002\\plotters\\DWF ePlot (optimized for plotting).pc3" );
  dw.dxfString( 4, "" );
  dw.dxfString( 6, "" );
  dw.dxfReal( 40, 0.0 );
  dw.dxfReal( 41, 0.0 );
  dw.dxfReal( 42, 0.0 );
  dw.dxfReal( 43, 0.0 );
  dw.dxfReal( 44, 0.0 );
  dw.dxfReal( 45, 0.0 );
  dw.dxfReal( 46, 0.0 );
  dw.dxfReal( 47, 0.0 );
  dw.dxfReal( 48, 0.0 );
  dw.dxfReal( 49, 0.0 );
  dw.dxfReal( 140, 0.0 );
  dw.dxfReal( 141, 0.0 );
  dw.dxfReal( 142, 1.0 );
  dw.dxfReal( 143, 1.0 );
  dw.dxfInt( 70, 688 );
  dw.dxfInt( 72, 0 );
  dw.dxfInt( 73, 0 );
  dw.dxfInt( 74, 5 );
  dw.dxfString( 7, "" );
  dw.dxfInt( 75, 16 );
  dw.dxfReal( 147, 1.0 );
  dw.dxfReal( 148, 0.0 );
  dw.dxfReal( 149, 0.0 );
  dw.dxfString( 100, "AcDbLayout" );
  dw.dxfString( 1, "Layout1" );
  dw.dxfInt( 70, 1 );
  dw.dxfInt( 71, 1 );
  dw.dxfReal( 10, 0.0 );
  dw.dxfReal( 20, 0.0 );
  dw.dxfReal( 11, 420.0 );
  dw.dxfReal( 21, 297.0 );
  dw.dxfReal( 12, 0.0 );
  dw.dxfReal( 22, 0.0 );
  dw.dxfReal( 32, 0.0 );
  dw.dxfReal( 14, 1.000000000000000E+20 );
  dw.dxfReal( 24, 1.000000000000000E+20 );
  dw.dxfReal( 34, 1.000000000000000E+20 );
  dw.dxfReal( 15, -1.000000000000000E+20 );
  dw.dxfReal( 25, -1.000000000000000E+20 );
  dw.dxfReal( 35, -1.000000000000000E+20 );
  dw.dxfReal( 146, 0.0 );
  dw.dxfReal( 13, 0.0 );
  dw.dxfReal( 23, 0.0 );
  dw.dxfReal( 33, 0.0 );
  dw.dxfReal( 16, 1.0 );
  dw.dxfReal( 26, 0.0 );
  dw.dxfReal( 36, 0.0 );
  dw.dxfReal( 17, 0.0 );
  dw.dxfReal( 27, 1.0 );
  dw.dxfReal( 37, 0.0 );
  dw.dxfInt( 76, 0 );
  //dw.dxfHex(330, dw.getPaperSpaceHandle());    // 1B
  dw.dxfHex( 330, 0x1B );
  dw.dxfString( 0, "LAYOUT" );
  dw.dxfHex( 5, 0x22 );
  //dw.handle();                                    // 22
  //dw.dxfHex(330, dicId2);                      // 1A
  dw.dxfString( 100, "AcDbPlotSettings" );
  dw.dxfString( 1, "" );
  dw.dxfString( 2, "C:\\Program Files\\AutoCAD 2002\\plotters\\DWF ePlot (optimized for plotting).pc3" );
  dw.dxfString( 4, "" );
  dw.dxfString( 6, "" );
  dw.dxfReal( 40, 0.0 );
  dw.dxfReal( 41, 0.0 );
  dw.dxfReal( 42, 0.0 );
  dw.dxfReal( 43, 0.0 );
  dw.dxfReal( 44, 0.0 );
  dw.dxfReal( 45, 0.0 );
  dw.dxfReal( 46, 0.0 );
  dw.dxfReal( 47, 0.0 );
  dw.dxfReal( 48, 0.0 );
  dw.dxfReal( 49, 0.0 );
  dw.dxfReal( 140, 0.0 );
  dw.dxfReal( 141, 0.0 );
  dw.dxfReal( 142, 1.0 );
  dw.dxfReal( 143, 1.0 );
  dw.dxfInt( 70, 1712 );
  dw.dxfInt( 72, 0 );
  dw.dxfInt( 73, 0 );
  dw.dxfInt( 74, 0 );
  dw.dxfString( 7, "" );
  dw.dxfInt( 75, 0 );
  dw.dxfReal( 147, 1.0 );
  dw.dxfReal( 148, 0.0 );
  dw.dxfReal( 149, 0.0 );
  dw.dxfString( 100, "AcDbLayout" );
  dw.dxfString( 1, "Model" );
  dw.dxfInt( 70, 1 );
  dw.dxfInt( 71, 0 );
  dw.dxfReal( 10, 0.0 );
  dw.dxfReal( 20, 0.0 );
  dw.dxfReal( 11, 12.0 );
  dw.dxfReal( 21, 9.0 );
  dw.dxfReal( 12, 0.0 );
  dw.dxfReal( 22, 0.0 );
  dw.dxfReal( 32, 0.0 );
  dw.dxfReal( 14, 0.0 );
  dw.dxfReal( 24, 0.0 );
  dw.dxfReal( 34, 0.0 );
  dw.dxfReal( 15, 0.0 );
  dw.dxfReal( 25, 0.0 );
  dw.dxfReal( 35, 0.0 );
  dw.dxfReal( 146, 0.0 );
  dw.dxfReal( 13, 0.0 );
  dw.dxfReal( 23, 0.0 );
  dw.dxfReal( 33, 0.0 );
  dw.dxfReal( 16, 1.0 );
  dw.dxfReal( 26, 0.0 );
  dw.dxfReal( 36, 0.0 );
  dw.dxfReal( 17, 0.0 );
  dw.dxfReal( 27, 1.0 );
  dw.dxfReal( 37, 0.0 );
  dw.dxfInt( 76, 0 );
  //dw.dxfHex(330, dw.getModelSpaceHandle());    // 1F
  dw.dxfHex( 330, 0x1F );
  dw.dxfString( 0, "LAYOUT" );
  //dw.handle();                                    // 26
  dw.dxfHex( 5, 0x26 );
  //dw.dxfHex(330, dicId2);                      // 1A
  dw.dxfString( 100, "AcDbPlotSettings" );
  dw.dxfString( 1, "" );
  dw.dxfString( 2, "C:\\Program Files\\AutoCAD 2002\\plotters\\DWF ePlot (optimized for plotting).pc3" );
  dw.dxfString( 4, "" );
  dw.dxfString( 6, "" );
  dw.dxfReal( 40, 0.0 );
  dw.dxfReal( 41, 0.0 );
  dw.dxfReal( 42, 0.0 );
  dw.dxfReal( 43, 0.0 );
  dw.dxfReal( 44, 0.0 );
  dw.dxfReal( 45, 0.0 );
  dw.dxfReal( 46, 0.0 );
  dw.dxfReal( 47, 0.0 );
  dw.dxfReal( 48, 0.0 );
  dw.dxfReal( 49, 0.0 );
  dw.dxfReal( 140, 0.0 );
  dw.dxfReal( 141, 0.0 );
  dw.dxfReal( 142, 1.0 );
  dw.dxfReal( 143, 1.0 );
  dw.dxfInt( 70, 688 );
  dw.dxfInt( 72, 0 );
  dw.dxfInt( 73, 0 );
  dw.dxfInt( 74, 5 );
  dw.dxfString( 7, "" );
  dw.dxfInt( 75, 16 );
  dw.dxfReal( 147, 1.0 );
  dw.dxfReal( 148, 0.0 );
  dw.dxfReal( 149, 0.0 );
  dw.dxfString( 100, "AcDbLayout" );
  dw.dxfString( 1, "Layout2" );
  dw.dxfInt( 70, 1 );
  dw.dxfInt( 71, 2 );
  dw.dxfReal( 10, 0.0 );
  dw.dxfReal( 20, 0.0 );
  dw.dxfReal( 11, 12.0 );
  dw.dxfReal( 21, 9.0 );
  dw.dxfReal( 12, 0.0 );
  dw.dxfReal( 22, 0.0 );
  dw.dxfReal( 32, 0.0 );
  dw.dxfReal( 14, 0.0 );
  dw.dxfReal( 24, 0.0 );
  dw.dxfReal( 34, 0.0 );
  dw.dxfReal( 15, 0.0 );
  dw.dxfReal( 25, 0.0 );
  dw.dxfReal( 35, 0.0 );
  dw.dxfReal( 146, 0.0 );
  dw.dxfReal( 13, 0.0 );
  dw.dxfReal( 23, 0.0 );
  dw.dxfReal( 33, 0.0 );
  dw.dxfReal( 16, 1.0 );
  dw.dxfReal( 26, 0.0 );
  dw.dxfReal( 36, 0.0 );
  dw.dxfReal( 17, 0.0 );
  dw.dxfReal( 27, 1.0 );
  dw.dxfReal( 37, 0.0 );
  dw.dxfInt( 76, 0 );
  //dw.dxfHex(330, dw.getPaperSpace0Handle());   // 23
  dw.dxfHex( 330, 0x23 );
  dw.dxfString( 0, "DICTIONARY" );
  //dw.dxfHex(5, 0x2C);
  //dicId5 =
  dw.handle();                           // 2C
  //dw.dxfHex(330, 0xC);                       // C
  dw.dxfString( 100, "AcDbDictionary" );
  dw.dxfInt( 281, 1 );
  dw.dxfString( 3, "DIMASSOC" );
  //dw.dxfHex(350, 0x2F);
  dw.dxfHex( 350, dw.getNextHandle() + 1 );    // 2E
  dw.dxfString( 3, "HIDETEXT" );
  //dw.dxfHex(350, 0x2E);
  dw.dxfHex( 350, dw.getNextHandle() );      // 2D
  dw.dxfString( 0, "DICTIONARYVAR" );
  //dw.dxfHex(5, 0x2E);
  dw.handle();                                    // 2E
  //dw.dxfHex(330, dicId5);                      // 2C
  dw.dxfString( 100, "DictionaryVariables" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 1, 2 );
  dw.dxfString( 0, "DICTIONARYVAR" );
  //dw.dxfHex(5, 0x2D);
  dw.handle();                                    // 2D
  //dw.dxfHex(330, dicId5);                      // 2C
  dw.dxfString( 100, "DictionaryVariables" );
  dw.dxfInt( 280, 0 );
  dw.dxfInt( 1, 1 );
}


/**
 * Writes the end of the objects section. This section is needed in VER_R13.
 * Note that this method currently only writes a faked OBJECTS section
 * to make the file readable by Aut*cad.
 */
void DL_Dxf::writeObjectsEnd( DL_WriterA& dw )
{
  dw.dxfString( 0, "ENDSEC" );
}



/**
 * Checks if the given variable is known by the given DXF version.
 */
bool DL_Dxf::checkVariable( const char* var, DL_Codes::version version )
{
  if ( version >= VER_2000 )
  {
    return true;
  }
  else if ( version == VER_R12 )
  {
    // these are all the variables recognized by dxf r12:
    if ( !strcmp( var, "$ACADVER" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ACADVER" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ANGBASE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ANGDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ATTDIA" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ATTMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ATTREQ" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$AUNITS" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$AUPREC" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$AXISMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$AXISUNIT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$BLIPMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$CECOLOR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$CELTYPE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$CHAMFERA" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$CHAMFERB" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$CLAYER" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$COORDS" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMALT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMALTD" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMALTF" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMAPOST" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMASO" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMASZ" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMBLK" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMBLK1" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMBLK2" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMCEN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMCLRD" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMCLRE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMCLRT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMDLE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMDLI" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMEXE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMEXO" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMGAP" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMLFAC" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMLIM" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMPOST" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMRND" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSAH" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSCALE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSE1" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSE2" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSHO" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSOXD" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMSTYLE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTAD" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTFAC" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTIH" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTIX" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTM" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTOFL" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTOH" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTOL" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTP" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTSZ" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTVP" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMTXT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DIMZIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DWGCODEPAGE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$DRAGMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ELEVATION" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$EXTMAX" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$EXTMIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$FILLETRAD" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$FILLMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$HANDLING" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$HANDSEED" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$INSBASE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LIMCHECK" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LIMMAX" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LIMMIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LTSCALE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LUNITS" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$LUPREC" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$MAXACTVP" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$MENU" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$MIRRTEXT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$ORTHOMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$OSMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PDMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PDSIZE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PELEVATION" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PEXTMAX" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PEXTMIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PLIMCHECK" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PLIMMAX" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PLIMMIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PLINEGEN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PLINEWID" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PSLTSCALE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PUCSNAME" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PUCSORG" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PUCSXDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$PUCSYDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$QTEXTMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$REGENMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SHADEDGE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SHADEDIF" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SKETCHINC" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SKPOLY" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SPLFRAME" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SPLINESEGS" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SPLINETYPE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SURFTAB1" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SURFTAB2" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SURFTYPE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SURFU" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SURFV" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TDCREATE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TDINDWG" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TDUPDATE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TDUSRTIMER" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TEXTSIZE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TEXTSTYLE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$THICKNESS" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TILEMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$TRACEWID" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$UCSNAME" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$UCSORG" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$UCSXDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$UCSYDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$UNITMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$USERI1" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$USERR1" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$USRTIMER" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$VISRETAIN" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$WORLDVIEW" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$FASTZOOM" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$GRIDMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$GRIDUNIT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPANG" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPBASE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPISOPAIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPMODE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPSTYLE" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$SNAPUNIT" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$VIEWCTR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$VIEWDIR" ) )
    {
      return true;
    }
    if ( !strcmp( var, "$VIEWSIZE" ) )
    {
      return true;
    }
    return false;
  }

  return false;
}



/**
 * @returns the library version as int (4 bytes, each byte one version number).
 * e.g. if str = "2.0.2.0" getLibVersion returns 0x02000200
 */
int DL_Dxf::getLibVersion( const char* str )
{
  int d[4];
  int idx = 0;
  char v[4][5];
  int ret = 0;

  for ( unsigned int i = 0; i < strlen( str ) && idx < 3; ++i )
  {
    if ( str[i] == '.' )
    {
      d[idx] = i;
      idx++;
    }
  }

  if ( idx == 3 )
  {
    d[3] = strlen( str );

    strncpy( v[0], str, d[0] );
    v[0][d[0]] = '\0';
// QgsDebugMsg(QString("v[0]: %1").arg(atoi(v[0])));

    strncpy( v[1], &str[d[0] + 1], d[1] - d[0] - 1 );
    v[1][d[1] - d[0] - 1] = '\0';
// QgsDebugMsg(QString("v[1]: %1").arg(atoi(v[1])));

    strncpy( v[2], &str[d[1] + 1], d[2] - d[1] - 1 );
    v[2][d[2] - d[1] - 1] = '\0';
// QgsDebugMsg(QString("v[2]: %1").arg(atoi(v[2])));

    strncpy( v[3], &str[d[2] + 1], d[3] - d[2] - 1 );
    v[3][d[3] - d[2] - 1] = '\0';
// QgsDebugMsg(QString("v[3]: %1").arg(atoi(v[3])));

    ret = ( atoi( v[0] ) << ( 3 * 8 ) ) +
          ( atoi( v[1] ) << ( 2 * 8 ) ) +
          ( atoi( v[2] ) << ( 1 * 8 ) ) +
          ( atoi( v[3] ) << ( 0 * 8 ) );

    return ret;
  }
  else
  {
    QgsDebugMsg( QString( "DL_Dxf::getLibVersion: invalid version number: %1" ).arg( str ) );
    return 0;
  }
}



/**
 * Some test routines.
 */
void DL_Dxf::test()
{
  char* buf1;
  char* buf2;
  char* buf3;
  char* buf4;
  char* buf5;
  char* buf6;

  buf1 = new char[10];
  buf2 = new char[10];
  buf3 = new char[10];
  buf4 = new char[10];
  buf5 = new char[10];
  buf6 = new char[10];

  strcpy( buf1, "  10\n" );
  strcpy( buf2, "10" );
  strcpy( buf3, "10\n" );
  strcpy( buf4, "  10 \n" );
  strcpy( buf5, "  10 \r" );
  strcpy( buf6, "\t10 \n" );

  QgsDebugMsg( QString( "1 buf1: '%1'" ).arg( buf1 ) );
  stripWhiteSpace( &buf1 );
  QgsDebugMsg( QString( "2 buf1: '%1'" ).arg( buf1 ) );
  //assert(!strcmp(buf1, "10"));

  QgsDebugMsg( QString( "1 buf2: '%1'" ).arg( buf2 ) );
  stripWhiteSpace( &buf2 );
  QgsDebugMsg( QString( "2 buf2: '%1'" ).arg( buf2 ) );

  QgsDebugMsg( QString( "1 buf3: '%1'" ).arg( buf3 ) );
  stripWhiteSpace( &buf3 );
  QgsDebugMsg( QString( "2 buf3: '%1'" ).arg( buf3 ) );

  QgsDebugMsg( QString( "1 buf4: '%1'" ).arg( buf4 ) );
  stripWhiteSpace( &buf4 );
  QgsDebugMsg( QString( "2 buf4: '%1'" ).arg( buf4 ) );

  QgsDebugMsg( QString( "1 buf5: '%1'" ).arg( buf5 ) );
  stripWhiteSpace( &buf5 );
  QgsDebugMsg( QString( "2 buf5: '%1'" ).arg( buf5 ) );

  QgsDebugMsg( QString( "1 buf6: '%1'" ).arg( buf6 ) );
  stripWhiteSpace( &buf6 );
  QgsDebugMsg( QString( "2 buf6: '%1'" ).arg( buf6 ) );

}


