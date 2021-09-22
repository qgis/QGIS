/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef LIBDXFRW_H
#define LIBDXFRW_H

#include <string>
#include "drw_entities.h"
#include "drw_objects.h"
#include "drw_header.h"
#include "drw_interface.h"


class dxfReader;
class dxfWriter;

class dxfRW
{
  public:
    explicit dxfRW( const char *name );
    ~dxfRW();
    void setDebug(DRW::DebugLevel lvl);
    /// reads the file specified in constructor
    /*!
     * An interface must be provided. It is used by the class to signal various
     * components being added.
     * \param interface_ the interface to use
     * \param ext should the extrusion be applied to convert in 2D?
     * \returns true for success
     */
    bool read( DRW_Interface *interface_, bool ext );
    void setBinary( bool b ) {binFile = b;}

    bool write( DRW_Interface *interface_, DRW::Version ver, bool bin );
    bool writeLineType( DRW_LType *ent );
    bool writeLayer( DRW_Layer *ent );
    bool writeDimstyle( DRW_Dimstyle *ent );
    bool writeTextstyle( DRW_Textstyle *ent );
    bool writeVport( DRW_Vport *ent );
    bool writeAppId( DRW_AppId *ent );
    bool writePoint( DRW_Point *ent );
    bool writeLine( DRW_Line *ent );
    bool writeRay( DRW_Ray *ent );
    bool writeXline( DRW_Xline *ent );
    bool writeCircle( DRW_Circle *ent );
    bool writeArc( DRW_Arc *ent );
    bool writeEllipse( DRW_Ellipse *ent );
    bool writeTrace( DRW_Trace *ent );
    bool writeSolid( DRW_Solid *ent );
    bool write3dface( DRW_3Dface *ent );
    bool writeLWPolyline( DRW_LWPolyline *ent );
    bool writePolyline( DRW_Polyline *ent );
    bool writeSpline( DRW_Spline *ent );
    bool writeBlockRecord( std::string name );
    bool writeBlock( DRW_Block *ent );
    bool writeInsert( DRW_Insert *ent );
    bool writeMText( DRW_MText *ent );
    bool writeText( DRW_Text *ent );
    bool writeHatch( DRW_Hatch *ent );
    bool writeViewport( DRW_Viewport *ent );
    DRW_ImageDef *writeImage( DRW_Image *ent, std::string name );
    bool writeLeader( DRW_Leader *ent );
    bool writeDimension( DRW_Dimension *ent );
    void setEllipseParts( int parts ) {elParts = parts;} //!< Set parts number when convert ellipse to polyline

  private:
    /// used by read() to parse the content of the file
    bool processDxf();
    bool processHeader();
    bool processTables();
    bool processBlocks();
    bool processBlock();
    bool processEntities( bool isblock );
    bool processObjects();

    bool processLType();
    bool processLayer();
    bool processDimStyle();
    bool processTextStyle();
    bool processVports();
    bool processAppId();

    bool processPoint();
    bool processLine();
    bool processRay();
    bool processXline();
    bool processCircle();
    bool processArc();
    bool processEllipse();
    bool processTrace();
    bool processSolid();
    bool processInsert();
    bool processLWPolyline();
    bool processPolyline();
    bool processVertex( DRW_Polyline *pl );
    bool processText();
    bool processMText();
    bool processHatch();
    bool processSpline();
    bool process3dface();
    bool processViewport();
    bool processImage();
    bool processImageDef();
    bool processDimension();
    bool processLeader();

//  bool writeHeader();
    bool writeEntity( DRW_Entity *ent );
    bool writeTables();
    bool writeBlocks();
    bool writeObjects();
    bool writeExtData( const std::vector<DRW_Variant *> &ed );
    /*use version from dwgutil.h*/
    std::string toHexStr( int n );//RLZ removeme

  private:
    DRW::Version version;
    std::string fileName;
    std::string codePage;
    bool binFile;
    dxfReader *reader = nullptr;
    dxfWriter *writer = nullptr;
    DRW_Interface *iface = nullptr;
    DRW_Header header;
//  int section;
    std::string nextentity;
    int entCount;
    bool wlayer0;
    bool dimstyleStd;
    bool applyExt;
    bool writingBlock;
    int elParts;  //!< Parts number when convert ellipse to polyline
    std::map<std::string, int> blockMap;
    std::vector<DRW_ImageDef *> imageDef; //!< ImageDef list

    int currHandle;
};

#endif // LIBDXFRW_H
