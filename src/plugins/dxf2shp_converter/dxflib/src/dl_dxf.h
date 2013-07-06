/****************************************************************************
** $Id: dl_dxf.h 2398 2005-06-06 18:12:14Z andrew $
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

#ifndef DL_DXF_H
#define DL_DXF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __GCC2x__
#include <sstream>
#endif

#include "dl_attributes.h"
#include "dl_codes.h"
#include "dl_entities.h"
#include "dl_writer_ascii.h"

#ifdef _WIN32
#undef M_PI
#define M_PI   3.14159265358979323846
#pragma warning(disable : 4800)
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

class DL_CreationInterface;
class DL_WriterA;


#define DL_VERSION     "2.0.4.8"

#define DL_Unknown               0
#define DL_LAYER                10
#define DL_BLOCK                11
#define DL_ENDBLK               12
#define DL_LINETYPE             13
#define DL_SETTING              50
#define DL_ENTITY_POINT        100
#define DL_ENTITY_LINE         101
#define DL_ENTITY_POLYLINE     102
#define DL_ENTITY_LWPOLYLINE   103
#define DL_ENTITY_VERTEX       104
#define DL_ENTITY_SPLINE       105
#define DL_ENTITY_KNOT         106
#define DL_ENTITY_CONTROLPOINT 107
#define DL_ENTITY_ARC          108
#define DL_ENTITY_CIRCLE       109
#define DL_ENTITY_ELLIPSE      110
#define DL_ENTITY_INSERT       111
#define DL_ENTITY_TEXT         112
#define DL_ENTITY_MTEXT        113
#define DL_ENTITY_DIMENSION    114
#define DL_ENTITY_LEADER       115
#define DL_ENTITY_HATCH        116
#define DL_ENTITY_ATTRIB       117
#define DL_ENTITY_IMAGE        118
#define DL_ENTITY_IMAGEDEF     119
#define DL_ENTITY_TRACE        120
#define DL_ENTITY_SOLID        121
#define DL_ENTITY_SEQEND       122


/**
 * Reading and writing of DXF files.
 *
 * This class can read in a DXF file and calls methods from the
 * interface DL_EntityContainer to add the entities to the
 * contianer provided by the user of the library.
 *
 * It can also be used to write DXF files to a certain extent.
 *
 * When saving entities, special values for colors and linetypes
 * can be used:
 *
 * Special colors are 0 (=BYBLOCK) and 256 (=BYLAYER).
 * Special linetypes are "BYLAYER" and "BYBLOCK".
 *
 * @author Andrew Mustun
 */
class DL_Dxf
{
  public:
    DL_Dxf();
    ~DL_Dxf();

    bool in( const string& file,
             DL_CreationInterface* creationInterface );
    bool readDxfGroups( FILE* fp,
                        DL_CreationInterface* creationInterface,
                        int* errorCounter = NULL );
    static bool getChoppedLine( char* s, unsigned int size,
                                FILE *stream );

#ifndef __GCC2x__

    bool readDxfGroups( std::stringstream &stream,
                        DL_CreationInterface* creationInterface,
                        int* errorCounter = NULL );
    bool in( std::stringstream &stream,
             DL_CreationInterface* creationInterface );
    static bool getChoppedLine( char *s, unsigned int size,
                                std::stringstream &stream );
#endif

    static bool stripWhiteSpace( char** s );

    bool processDXFGroup( DL_CreationInterface* creationInterface,
                          int groupCode, const char* groupValue );
    void addSetting( DL_CreationInterface* creationInterface );
    void addLayer( DL_CreationInterface* creationInterface );
    void addBlock( DL_CreationInterface* creationInterface );
    void endBlock( DL_CreationInterface* creationInterface );

    void addPoint( DL_CreationInterface* creationInterface );
    void addLine( DL_CreationInterface* creationInterface );

    void addPolyline( DL_CreationInterface* creationInterface );
    void addVertex( DL_CreationInterface* creationInterface );

    void addSpline( DL_CreationInterface* creationInterface );
    //void addKnot(DL_CreationInterface* creationInterface);
    //void addControlPoint(DL_CreationInterface* creationInterface);

    void addArc( DL_CreationInterface* creationInterface );
    void addCircle( DL_CreationInterface* creationInterface );
    void addEllipse( DL_CreationInterface* creationInterface );
    void addInsert( DL_CreationInterface* creationInterface );

    void addTrace( DL_CreationInterface* creationInterface );
    void addSolid( DL_CreationInterface* creationInterface );

    void addMText( DL_CreationInterface* creationInterface );
    bool handleMTextData( DL_CreationInterface* creationInterface );
    bool handleLWPolylineData( DL_CreationInterface* creationInterface );
    bool handleSplineData( DL_CreationInterface* creationInterface );
    bool handleLeaderData( DL_CreationInterface* creationInterface );
    bool handleHatchData( DL_CreationInterface* creationInterface );

    void addText( DL_CreationInterface* creationInterface );
    void addAttrib( DL_CreationInterface* creationInterface );
    DL_DimensionData getDimData();
    void addDimLinear( DL_CreationInterface* creationInterface );
    void addDimAligned( DL_CreationInterface* creationInterface );
    void addDimRadial( DL_CreationInterface* creationInterface );
    void addDimDiametric( DL_CreationInterface* creationInterface );
    void addDimAngular( DL_CreationInterface* creationInterface );
    void addDimAngular3P( DL_CreationInterface* creationInterface );
    void addLeader( DL_CreationInterface* creationInterface );
    void addHatch( DL_CreationInterface* creationInterface );
    void addImage( DL_CreationInterface* creationInterface );
    void addImageDef( DL_CreationInterface* creationInterface );

    void endEntity( DL_CreationInterface* creationInterface );

    void endSequence( DL_CreationInterface* creationInterface );

    int  stringToInt( const char* s, bool* ok = NULL );

    DL_WriterA* out( const char* file,
                     DL_Codes::version version = VER_2000 );

    void writeHeader( DL_WriterA& dw );

    void writePoint( DL_WriterA& dw,
                     const DL_PointData& data,
                     const DL_Attributes& attrib );
    void writeLine( DL_WriterA& dw,
                    const DL_LineData& data,
                    const DL_Attributes& attrib );
    void writePolyline( DL_WriterA& dw,
                        const DL_PolylineData& data,
                        const DL_Attributes& attrib );
    void writeVertex( DL_WriterA& dw,
                      const DL_VertexData& data );
    void writePolylineEnd( DL_WriterA& dw );
    void writeSpline( DL_WriterA& dw,
                      const DL_SplineData& data,
                      const DL_Attributes& attrib );
    void writeControlPoint( DL_WriterA& dw,
                            const DL_ControlPointData& data );
    void writeKnot( DL_WriterA& dw,
                    const DL_KnotData& data );
    void writeCircle( DL_WriterA& dw,
                      const DL_CircleData& data,
                      const DL_Attributes& attrib );
    void writeArc( DL_WriterA& dw,
                   const DL_ArcData& data,
                   const DL_Attributes& attrib );
    void writeEllipse( DL_WriterA& dw,
                       const DL_EllipseData& data,
                       const DL_Attributes& attrib );
    void writeInsert( DL_WriterA& dw,
                      const DL_InsertData& data,
                      const DL_Attributes& attrib );
    void writeMText( DL_WriterA& dw,
                     const DL_MTextData& data,
                     const DL_Attributes& attrib );
    void writeText( DL_WriterA& dw,
                    const DL_TextData& data,
                    const DL_Attributes& attrib );
    void writeDimAligned( DL_WriterA& dw,
                          const DL_DimensionData& data,
                          const DL_DimAlignedData& edata,
                          const DL_Attributes& attrib );
    void writeDimLinear( DL_WriterA& dw,
                         const DL_DimensionData& data,
                         const DL_DimLinearData& edata,
                         const DL_Attributes& attrib );
    void writeDimRadial( DL_WriterA& dw,
                         const DL_DimensionData& data,
                         const DL_DimRadialData& edata,
                         const DL_Attributes& attrib );
    void writeDimDiametric( DL_WriterA& dw,
                            const DL_DimensionData& data,
                            const DL_DimDiametricData& edata,
                            const DL_Attributes& attrib );
    void writeDimAngular( DL_WriterA& dw,
                          const DL_DimensionData& data,
                          const DL_DimAngularData& edata,
                          const DL_Attributes& attrib );
    void writeDimAngular3P( DL_WriterA& dw,
                            const DL_DimensionData& data,
                            const DL_DimAngular3PData& edata,
                            const DL_Attributes& attrib );
    void writeLeader( DL_WriterA& dw,
                      const DL_LeaderData& data,
                      const DL_Attributes& attrib );
    void writeLeaderVertex( DL_WriterA& dw,
                            const DL_LeaderVertexData& data );
    void writeHatch1( DL_WriterA& dw,
                      const DL_HatchData& data,
                      const DL_Attributes& attrib );
    void writeHatch2( DL_WriterA& dw,
                      const DL_HatchData& data,
                      const DL_Attributes& attrib );
    void writeHatchLoop1( DL_WriterA& dw,
                          const DL_HatchLoopData& data );
    void writeHatchLoop2( DL_WriterA& dw,
                          const DL_HatchLoopData& data );
    void writeHatchEdge( DL_WriterA& dw,
                         const DL_HatchEdgeData& data );

    int writeImage( DL_WriterA& dw,
                    const DL_ImageData& data,
                    const DL_Attributes& attrib );

    void writeImageDef( DL_WriterA& dw, int handle,
                        const DL_ImageData& data );

    void writeLayer( DL_WriterA& dw,
                     const DL_LayerData& data,
                     const DL_Attributes& attrib );

    void writeLineType( DL_WriterA& dw,
                        const DL_LineTypeData& data );

    void writeAppid( DL_WriterA& dw, const string& name );

    void writeBlock( DL_WriterA& dw,
                     const DL_BlockData& data );
    void writeEndBlock( DL_WriterA& dw, const string& name );

    void writeVPort( DL_WriterA& dw );
    void writeStyle( DL_WriterA& dw );
    void writeView( DL_WriterA& dw );
    void writeUcs( DL_WriterA& dw );
    void writeDimStyle( DL_WriterA& dw,
                        double dimasz, double dimexe, double dimexo,
                        double dimgap, double dimtxt );
    void writeBlockRecord( DL_WriterA& dw );
    void writeBlockRecord( DL_WriterA& dw, const string& name );
    void writeObjects( DL_WriterA& dw );
    void writeObjectsEnd( DL_WriterA& dw );

    /**
     * Converts the given string into a double or returns the given
     * default valud (def) if value is NULL or empty.
     */
    static double toReal( const char* value, double def = 0.0 )
    {
      if ( value != NULL && value[0] != '\0' )
      {
        double ret;
        if ( strchr( value, ',' ) != NULL )
        {
          char* tmp = new char[strlen( value )+1];
          strcpy( tmp, value );
          DL_WriterA::strReplace( tmp, ',', '.' );
          ret = atof( tmp );
          delete[] tmp;
        }
        else
        {
          ret = atof( value );
        }
        return ret;
      }
      else
      {
        return def;
      }
    }
    /**
     * Converts the given string into an int or returns the given
     * default valud (def) if value is NULL or empty.
     */
    static int toInt( const char* value, int def = 0 )
    {
      if ( value != NULL && value[0] != '\0' )
      {
        return atoi( value );
      }
      else
      {
        return def;
      }
    }
    /**
     * Converts the given string into a string or returns the given
     * default valud (def) if value is NULL or empty.
     */
    static const char* toString( const char* value, const char* def = "" )
    {
      if ( value != NULL && value[0] != '\0' )
      {
        return value;
      }
      else
      {
        return def;
      }
    }

    static bool checkVariable( const char* var, DL_Codes::version version );

    DL_Codes::version getVersion()
    {
      return version;
    }

    int getLibVersion( const char* str );

    static void test();

  private:
    DL_Codes::version version;
    unsigned long styleHandleStd;

    string polylineLayer;
    double* vertices;
    int maxVertices;
    int vertexIndex;

    double* knots;
    int maxKnots;
    int knotIndex;

    double* controlPoints;
    int maxControlPoints;
    int controlPointIndex;

    double* leaderVertices;
    int maxLeaderVertices;
    int leaderVertexIndex;

    // array of hatch loops
    DL_HatchLoopData* hatchLoops;
    int maxHatchLoops;
    int hatchLoopIndex;
    // array in format [loop#][edge#]
    DL_HatchEdgeData** hatchEdges;
    int* maxHatchEdges;
    int* hatchEdgeIndex;
    bool dropEdges;

    // Only the useful part of the group code
    char groupCodeTmp[DL_DXF_MAXLINE+1];
    // ...same as integer
    unsigned int groupCode;
    // Only the useful part of the group value
    char groupValue[DL_DXF_MAXLINE+1];
    // Current entity type
    int currentEntity;
    // Value of the current setting
    char settingValue[DL_DXF_MAXLINE+1];
    // Key of the current setting (e.g. "$ACADVER")
    char settingKey[DL_DXF_MAXLINE+1];
    // Stores the group codes
    char values[DL_DXF_MAXGROUPCODE][DL_DXF_MAXLINE+1];
    // First call of this method. We initialize all group values in
    //  the first call.
    bool firstCall;
    // Attributes of the current entity (layer, color, width, line type)
    DL_Attributes attrib;
    // library version. hex: 0x20003001 = 2.0.3.1
    int libVersion;
};

#endif

// EOF
