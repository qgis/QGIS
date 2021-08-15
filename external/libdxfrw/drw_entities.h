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

#ifndef DRW_ENTITIES_H
#define DRW_ENTITIES_H


#include <string>
#include <vector>
#include <list>
#include <memory>
#include "drw_base.h"

class dxfReader;
class dwgBuffer;
class DRW_Polyline;

namespace DRW {

   //! Entity's type.
    enum ETYPE {
        E3DFACE,
//        E3DSOLID, //encrypted proprietary data
//        ACAD_PROXY_ENTITY,
        ARC,
//        ATTDEF,
//        ATTRIB,
        BLOCK,// and ENDBLK
//        BODY, //encrypted proprietary data
        CIRCLE,
        DIMENSION,
        DIMALIGNED,
        DIMLINEAR,
        DIMRADIAL,
        DIMDIAMETRIC,
        DIMANGULAR,
        DIMANGULAR3P,
        DIMORDINATE,
        ELLIPSE,
        HATCH,
//        HELIX,
        IMAGE,
        INSERT,
        LEADER,
//        LIGHT,
        LINE,
        LWPOLYLINE,
//        MESH,
//        MLINE,
//        MLEADERSTYLE,
//        MLEADER,
        MTEXT,
//        OLEFRAME,
//        OLE2FRAME,
        POINT,
        POLYLINE,
        RAY,
//        REGION, //encrypted proprietary data
//        SECTION,
//        SEQEND,//not needed?? used in polyline and insert/attrib and dwg
//        SHAPE,
        SOLID,
        SPLINE,
//        SUN,
//        SURFACE, //encrypted proprietary data can be four types
//        TABLE,
        TEXT,
//        TOLERANCE,
        TRACE,
        UNDERLAY,
        VERTEX,
        VIEWPORT,
//        WIPEOUT, //WIPEOUTVARIABLE
        XLINE,
        UNKNOWN
    };

}
//only in DWG: MINSERT, 5 types of vertex, 4 types of polylines: 2d, 3d, pface & mesh
//shape, dictionary, MLEADER, MLEADERSTYLE

#define SETENTFRIENDS  friend class dxfRW; \
                       friend class dwgReader;

/**
 * Base class for entities
 *  @author Rallaz
 */
class DRW_Entity
{
    SETENTFRIENDS
public:
    //initializes default values
    //handles: default no handle (0), color: default BYLAYER (256), 24 bits color: default -1 (not set)
    //line weight: default BYLAYER  (dxf -1, dwg 29), space: default ModelSpace (0)
    DRW_Entity( enum DRW::ETYPE aType = DRW::UNKNOWN, const UTF8STRING &aLayer = "0" )
      : eType( aType )
      , handle( DRW::NoHandle )
      , appData( 0 )
      , layer( aLayer )
      , haveNextLinks( 0 )
      , plotFlags( 0 )
      , ltFlags( 0 )
      , materialFlag( 0 )
      , shadowFlag( 0 )
      , lTypeH( dwgHandle() )
      , layerH( dwgHandle() )
      , numReactors( 0 )
      , objSize( 0 )
      , oType( 0 )
      , extAxisX( DRW_Coord() )
      , extAxisY( DRW_Coord() )
      , curr( nullptr )
    {
    }

    DRW_Entity( const DRW_Entity &e )
      : eType( e.eType )
      , handle( e.handle )
      , parentHandle( e.parentHandle ) //no handle (0)
      , space( e.space )
      , layer( e.layer )
      , lineType( e.lineType )
      , material( e.material )
      , color( e.color ) // default BYLAYER (256)
      , lWeight( e.lWeight )
      , ltypeScale( e.ltypeScale )
      , visible( e.visible )
      , numProxyGraph( e.numProxyGraph )
      , color24( e.color24 ) //default -1 not set
      , transparency( e.transparency )
      , plotStyle( e.plotStyle )
      , shadow( e.shadow )
      , haveExtrusion( e.haveExtrusion )
      , nextEntLink( e.nextEntLink )
      , prevEntLink( e.prevEntLink )
      , ownerHandle( false )
      , xDictFlag( e.xDictFlag )
      , numReactors( e.numReactors )
      , curr( nullptr )
    {
      for ( auto it = e.extData.begin(); it != e.extData.end(); ++it )
      {
        extData.push_back( std::make_shared<DRW_Variant>( *( *it ) ) );
      }
    }

    virtual ~DRW_Entity() = default;

    void reset()
    {
      extData.clear();
      curr.reset();
    }

    virtual void applyExtrusion() = 0;

protected:
    //parses dxf pair to read entity
    bool parseCode(int code, dxfReader *reader);
    //calculates extrusion axis (normal vector)
    void calculateAxis(const DRW_Coord &extPoint);
    //apply extrusion to @extPoint and return data in @point
    void extrudePoint(const DRW_Coord &extPoint, DRW_Coord *point);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0)=0;
    //parses dwg common start part to read entity
    bool parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer* strBuf, duint32 bs=0);
    //parses dwg common handles part to read entity
    bool parseDwgEntHandle(DRW::Version version, dwgBuffer *buf);

    //parses dxf 102 groups to read entity
    bool parseDxfGroups(int code, dxfReader *reader);

public:
	enum DRW::ETYPE eType{DRW::UNKNOWN};     /*!< enum: entity type, code 0 */
	duint32 handle{DRW::NoHandle};            /*!< entity identifier, code 5 */
        std::list<std::list<DRW_Variant> > appData; /*!< list of application data, code 102 */
	duint32 parentHandle{DRW::NoHandle};      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
	DRW::Space space{DRW::ModelSpace};          /*!< space indicator, code 67*/
	UTF8STRING layer{"0"};          /*!< layer name, code 8 */
	UTF8STRING lineType{"BYLAYER"};       /*!< line type, code 6 */
	duint32 material{DRW::MaterialByLayer};          /*!< hard pointer id to material object, code 347 */
	int color{DRW::ColorByLayer};                 /*!< entity color, code 62 */
	enum DRW_LW_Conv::lineWidth lWeight{DRW_LW_Conv::widthByLayer}; /*!< entity lineweight, code 370 */
	double ltypeScale{1.0};         /*!< linetype scale, code 48 */
	bool visible{true};              /*!< entity visibility, code 60 */
	int numProxyGraph{0};         /*!< Number of bytes in proxy graphics, code 92 */
        std::string proxyGraphics; /*!< proxy graphics bytes, code 310 */
	int color24{-1};               /*!< 24-bit color, code 420 */
        std::string colorName;     /*!< color name, code 430 */
	int transparency{DRW::Opaque};          /*!< transparency, code 440 */
	int plotStyle{DRW::DefaultPlotStyle};             /*!< hard pointer id to plot style object, code 390 */
    DRW::ShadowMode shadow{DRW::CastAndReceiveShadows};    /*!< shadow mode, code 284 */
	bool haveExtrusion{false};        /*!< set to true if the entity have extrusion*/
	std::vector<std::shared_ptr<DRW_Variant>> extData; /*!< FIFO list of extended data, codes 1000 to 1071*/

protected: //only for read dwg
    duint8 haveNextLinks; //aka nolinks //B
    duint8 plotFlags; //presence of plot style //BB
    duint8 ltFlags; //presence of linetype handle //BB
    duint8 materialFlag; //presence of material handle //BB
    duint8 shadowFlag; //presence of shadow handle ?? (in dwg may be plotflag)//RC
    dwgHandle lTypeH;
    dwgHandle layerH;
    duint32 nextEntLink{0};
    duint32 prevEntLink{0};
    bool ownerHandle{false};

    duint8 xDictFlag{0};
    dint32 numReactors{0}; //
    duint32 objSize;  //RL 32bits object data size in bits
    dint16 oType;

private:
    DRW_Coord extAxisX;
    DRW_Coord extAxisY;
    std::shared_ptr<DRW_Variant> curr;
};


/*!
*  Class to handle point entity
*  @author Rallaz
*/
class DRW_Point : public DRW_Entity {
    SETENTFRIENDS
  public:
    DRW_Point( enum DRW::ETYPE type = DRW::POINT, double sx = 0., double sy = 0., double sz = 0. )
      : DRW_Entity( type )
      , basePoint( sx, sy, sz )
      , extPoint( 0., 0., 1. )
    {
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    DRW_Coord basePoint;      /*!<  base point, code 10, 20 & 30 */
    double thickness{0};         /*!< thickness, code 39 */
    DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
    // TNick: we're not handling code 50 - Angle of the X axis for
    // the UCS in effect when the point was drawn
};

/*!
*  Class to handle line entity
*  @author Rallaz
*/
class DRW_Line : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Line( enum DRW::ETYPE type = DRW::LINE )
      : DRW_Point( type )
      , secPoint( 0., 0., 0. )
    {
    }

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    DRW_Coord secPoint;        /*!< second point, code 11, 21 & 31 */
};

/*!
*  Class to handle ray entity
*  @author Rallaz
*/
class DRW_Ray : public DRW_Line {
    SETENTFRIENDS
  public:
    DRW_Ray( enum DRW::ETYPE type = DRW::RAY )
      : DRW_Line( type )
    {
    }
protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};

/*!
*  Class to handle xline entity
*  @author Rallaz
*/
class DRW_Xline : public DRW_Ray
{
  public:
    DRW_Xline( enum DRW::ETYPE type = DRW::XLINE )
      : DRW_Ray( type )
    {
    }
};

/*!
*  Class to handle circle entity
*  @author Rallaz
*/
class DRW_Circle : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Circle( enum DRW::ETYPE type = DRW::CIRCLE )
      : DRW_Point( type )
    {
    }

    virtual void applyExtrusion();

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    double radius{0};                 /*!< radius, code 40 */
};

/*!
*  Class to handle arc entity
*  @author Rallaz
*/
class DRW_Arc : public DRW_Circle {
    SETENTFRIENDS
  public:
    DRW_Arc( enum DRW::ETYPE type = DRW::ARC )
      : DRW_Circle( type )
    {
    }

    virtual void applyExtrusion();

    //! center point in OCS
    const DRW_Coord & center() { return basePoint; }
    //! the radius of the circle
    double getRadius() { return radius; }
    //! start angle in radians
    double startAngle() { return staangle; }
    //! end angle in radians
    double endAngle() { return endangle; }
    //! thickness
    double thick() { return thickness; }
    //! extrusion
    const DRW_Coord & extrusion() { return extPoint; }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode(int code, dxfReader *reader);
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    double staangle{0};            /*!< start angle, code 50 in radians*/
    double endangle{M_PIx2};            /*!< end angle, code 51 in radians */
    int isccw{1};                  /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

/*!
*  Class to handle ellipse and elliptic arc entity
*  Note: start/end parameter are in radians for ellipse entity but
*  for hatch boundary are in degrees
*  @author Rallaz
*/
class DRW_Ellipse : public DRW_Line {
    SETENTFRIENDS
  public:
    DRW_Ellipse( enum DRW::ETYPE type = DRW::ELLIPSE )
      : DRW_Line( type )
    {
    }

    void toPolyline(DRW_Polyline *pol, int parts = 128) const;
    virtual void applyExtrusion();

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode(int code, dxfReader *reader);
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

private:
    void correctAxis();

public:
    double ratio{1.0};        /*!< ratio, code 40 */
    double staparam{0.0};     /*!< start parameter, code 41, 0.0 for full ellipse*/
    double endparam{M_PIx2};     /*!< end parameter, code 42, 2*PI for full ellipse */
    int isccw{1};           /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

/*!
*  Class to handle trace entity
*  @author Rallaz
*/
class DRW_Trace : public DRW_Line {
    SETENTFRIENDS
  public:
    DRW_Trace( enum DRW::ETYPE type = DRW::TRACE )
      : DRW_Line( type )
      , thirdPoint( 0., 0., 0. )
      , forthPoint( 0., 0., 0. )
    {
    }

    virtual void applyExtrusion();

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    DRW_Coord thirdPoint;        /*!< third point, code 12, 22 & 32 */
    DRW_Coord forthPoint;        /*!< forth point, code 13, 23 & 33 */
};

/*!
*  Class to handle solid entity
*  @author Rallaz
*/
class DRW_Solid : public DRW_Trace {
    SETENTFRIENDS
  public:
    DRW_Solid( enum DRW::ETYPE type = DRW::SOLID )
      : DRW_Trace( type )
    {
    }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode(int code, dxfReader *reader);
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    //! first corner (2D)
    const DRW_Coord & firstCorner() { return basePoint; }
    //! second corner (2D)
    const DRW_Coord & secondCorner() { return secPoint; }
    //! third corner (2D)
    const DRW_Coord & thirdCorner() { return thirdPoint; }
    //! fourth corner (2D)
    const DRW_Coord & fourthCorner() { return thirdPoint; }
    //! thickness
    double thick() { return thickness; }
    //! elevation
    double elevation() { return basePoint.z; }
    //! extrusion
    const DRW_Coord & extrusion() { return extPoint; }

};

/*!
*  Class to handle 3dface entity
*  @author Rallaz
*/
class DRW_3Dface : public DRW_Trace {
    SETENTFRIENDS
public:
    enum InvisibleEdgeFlags {
        NoEdge = 0x00,
        FirstEdge = 0x01,
        SecodEdge = 0x02,
        ThirdEdge = 0x04,
        FourthEdge = 0x08,
        AllEdges = 0x0F
    };

    DRW_3Dface( enum DRW::ETYPE type = DRW::E3DFACE )
      : DRW_Trace( type )
    {
    }

    virtual void applyExtrusion(){}

    //! first corner in WCS
    const DRW_Coord & firstCorner() { return basePoint; }
    //! second corner in WCS
    const DRW_Coord & secondCorner() { return secPoint; }
    //! third corner in WCS
    const DRW_Coord & thirdCorner() { return thirdPoint; }
    //! fourth corner in WCS
    const DRW_Coord & fourthCorner() { return forthPoint; }
    //! edge visibility flags
    InvisibleEdgeFlags edgeFlags() { return static_cast<InvisibleEdgeFlags>(invisibleflag); }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode(int code, dxfReader *reader);
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    int invisibleflag{0};       /*!< invisible edge flag, code 70 */

};

/*!
*  Class to handle block entries
*  @author Rallaz
*/
class DRW_Block : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Block( enum DRW::ETYPE type = DRW::BLOCK )
      : DRW_Point( type )
    {
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    UTF8STRING name{"*U0"};             /*!< block name, code 2 */
    int flags{0};                   /*!< block type, code 70 */
private:
    bool isEnd{false}; //for dwg parsing
};


/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Insert : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Insert( enum DRW::ETYPE type = DRW::INSERT )
      : DRW_Point( type )
    {
    }

    virtual void applyExtrusion(){DRW_Point::applyExtrusion();}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    UTF8STRING name;         /*!< block name, code 2 */
    double xscale{1};           /*!< x scale factor, code 41 */
    double yscale{1};           /*!< y scale factor, code 42 */
    double zscale{1};           /*!< z scale factor, code 43 */
    double angle{0};            /*!< rotation angle in radians, code 50 */
    int colcount{1};            /*!< column count, code 70 */
    int rowcount{1};            /*!< row count, code 71 */
    double colspace{0};         /*!< column space, code 44 */
    double rowspace{0};         /*!< row space, code 45 */
public: //only for read dwg
    dwgHandle blockRecH;
    dwgHandle seqendH; //RLZ: on implement attrib remove this handle from obj list (see pline/vertex code)
};

/*!
*  Class to handle lwpolyline entity
*  @author Rallaz
*/
class DRW_LWPolyline : public DRW_Entity {
    SETENTFRIENDS
  public:
    DRW_LWPolyline( enum DRW::ETYPE type = DRW::LWPOLYLINE )
      : DRW_Entity( type )
    {
    }

    DRW_LWPolyline( const DRW_LWPolyline &p )
      : DRW_Entity( p )
      , flags( p.flags )
      , width( p.width )
      , elevation( p.elevation )
      , thickness( p.thickness )
      , extPoint( p.extPoint )
    {
        for (unsigned i=0; i<p.vertlist.size(); i++)// RLZ ok or new
          this->vertlist.push_back(
                                   std::make_shared<DRW_Vertex2D>(*p.vertlist.at(i))
                                  );
    }
	// TODO rule of 5

    virtual void applyExtrusion();
    void addVertex (DRW_Vertex2D v) {
        std::shared_ptr<DRW_Vertex2D> vert = std::make_shared<DRW_Vertex2D>(v);
        vertlist.push_back(vert);
    }
        std::shared_ptr<DRW_Vertex2D> addVertex () {
        std::shared_ptr<DRW_Vertex2D> vert = std::make_shared<DRW_Vertex2D>();
        vert->stawidth = 0;
        vert->endwidth = 0;
        vert->bulge = 0;
        vertlist.push_back(vert);
        return vert;
    }

protected:
    void parseCode(int code, dxfReader *reader);
     bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0);

public:
    int vertexnum{0};            /*!< number of vertices, code 90 */
    int flags{0};                /*!< polyline flag, code 70, default 0 */
    double width{0.0};             /*!< constant width, code 43 */
    double elevation{0.0};         /*!< elevation, code 38 */
    double thickness{0.0};         /*!< thickness, code 39 */
    DRW_Coord extPoint{0.0, 0.0, 0.1};       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
    std::shared_ptr<DRW_Vertex2D> vertex;       /*!< current vertex to add data */
    std::vector<std::shared_ptr<DRW_Vertex2D>> vertlist;  /*!< vertex list */
};

/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Text : public DRW_Line {
    SETENTFRIENDS
public:
    //! Vertical alignments.
        enum VAlign {
            VBaseLine = 0,  /*!< Top = 0 */
            VBottom,        /*!< Bottom = 1 */
            VMiddle,        /*!< Middle = 2 */
            VTop            /*!< Top = 3 */
        };

    //! Horizontal alignments.
        enum HAlign {
            HLeft = 0,     /*!< Left = 0 */
            HCenter,       /*!< Centered = 1 */
            HRight,        /*!< Right = 2 */
            HAligned,      /*!< Aligned = 3 (if VAlign==0) */
            HMiddle,       /*!< middle = 4 (if VAlign==0) */
            HFit           /*!< fit into point = 5 (if VAlign==0) */
        };

    DRW_Text( enum DRW::ETYPE type = DRW::TEXT )
      : DRW_Line( type )
    {
    }

    virtual void applyExtrusion(){} //RLZ TODO

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    double height{0};             /*!< height text, code 40 */
    UTF8STRING text;           /*!< text string, code 1 */
    double angle{0};              /*!< rotation angle in degrees (360), code 50 */
    double widthscale{1};         /*!< width factor, code 41 */
    double oblique{0};            /*!< oblique angle, code 51 */
    UTF8STRING style{"STANDARD"};          /*!< style name, code 7 */
    int textgen{0};               /*!< text generation, code 71 */
    enum HAlign alignH{HLeft};        /*!< horizontal align, code 72 */
    enum VAlign alignV{VBaseLine};        /*!< vertical align, code 73 */
    dwgHandle styleH;          /*!< handle for text style */
};

/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_MText : public DRW_Text {
    SETENTFRIENDS
public:
    //! Attachments.
    enum Attach {
        TopLeft = 1,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    DRW_MText()
    {
      eType = DRW::MTEXT;
      alignV = static_cast< VAlign >(TopLeft );
      textgen = 1;
    }

protected:
    void parseCode(int code, dxfReader *reader);
    void updateAngle();    // recalculate angle if 'hasXAxisVec' is true
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    double interlin{1};     /*!< width factor, code 44 */
private:
    bool hasXAxisVec{false}; // if true need to calculate angle from secPoint vector 
};

/*!
*  Class to handle vertex  for polyline entity
*  @author Rallaz
*/
class DRW_Vertex : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Vertex( enum DRW::ETYPE type = DRW::VERTEX )
      : DRW_Point( type )
    {
    }

    DRW_Vertex( double sx, double sy, double sz, double b )
      : DRW_Point( DRW::VERTEX, sx, sy, sz )
      , bulge( b )
    {
    }

protected:
    void parseCode(int code, dxfReader *reader);
    bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0, double el=0);
    virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs=0){
        DRW_UNUSED(version); DRW_UNUSED(buf); DRW_UNUSED(bs); return true;}

public:
    double stawidth{0};          /*!< Start width, code 40 */
    double endwidth{0};          /*!< End width, code 41 */
    double bulge{0};             /*!< bulge, code 42 */

    int flags{0};                 /*!< vertex flag, code 70, default 0 */
    double tgdir{0};           /*!< curve fit tangent direction, code 50 */
    int vindex1{0};             /*!< polyface mesh vertex index, code 71, default 0 */
    int vindex2{0};             /*!< polyface mesh vertex index, code 72, default 0 */
    int vindex3{0};             /*!< polyface mesh vertex index, code 73, default 0 */
    int vindex4{0};             /*!< polyface mesh vertex index, code 74, default 0 */
    int identifier{0};           /*!< vertex identifier, code 91, default 0 */
};

/*!
*  Class to handle polyline entity
*  @author Rallaz
*/
class DRW_Polyline : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Polyline( enum DRW::ETYPE type = DRW::POLYLINE )
      : DRW_Point( type )
    {
    }
    void addVertex (DRW_Vertex v) {
        std::shared_ptr<DRW_Vertex> vert = std::make_shared<DRW_Vertex>();
        vert->basePoint.x = v.basePoint.x;
        vert->basePoint.y = v.basePoint.y;
        vert->basePoint.z = v.basePoint.z;
        vert->stawidth = v.stawidth;
        vert->endwidth = v.endwidth;
        vert->bulge = v.bulge;
        vertlist.push_back(vert);
    }
    void appendVertex (std::shared_ptr<DRW_Vertex> const& v) {
        vertlist.push_back(v);
    }

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    int flags{0};               /*!< polyline flag, code 70, default 0 */
    double defstawidth{0};      /*!< Start width, code 40, default 0 */
    double defendwidth{0};      /*!< End width, code 41, default 0 */
    int vertexcount{0};         /*!< polygon mesh M vertex or  polyface vertex num, code 71, default 0 */
    int facecount{0};           /*!< polygon mesh N vertex or  polyface face num, code 72, default 0 */
    int smoothM{0};             /*!< smooth surface M density, code 73, default 0 */
    int smoothN{0};             /*!< smooth surface M density, code 74, default 0 */
    int curvetype{0};           /*!< curves & smooth surface type, code 75, default 0 */

    std::vector<std::shared_ptr<DRW_Vertex>> vertlist;  /*!< vertex list */

private:
    std::list<duint32>handlesList; //list of handles, only in 2004+
    duint32 firstEH{0};      //handle of first entity, only in pre-2004
    duint32 lastEH{0};       //handle of last entity, only in pre-2004
    dwgHandle seqEndH;    //handle of SEQEND entity
};


/*!
*  Class to handle spline entity
*  @author Rallaz
*/
class DRW_Spline : public DRW_Entity {
    SETENTFRIENDS
  public:
    DRW_Spline()
      : DRW_Entity( DRW::SPLINE )
    {
    }
    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
//    double ex;                /*!< normal vector x coordinate, code 210 */
//    double ey;                /*!< normal vector y coordinate, code 220 */
//    double ez;                /*!< normal vector z coordinate, code 230 */
    DRW_Coord normalVec;      /*!< normal vector, code 210, 220, 230 */
    DRW_Coord tgStart;        /*!< start tangent, code 12, 22, 32 */
//    double tgsx;              /*!< start tangent x coordinate, code 12 */
//    double tgsy;              /*!< start tangent y coordinate, code 22 */
//    double tgsz;              /*!< start tangent z coordinate, code 32 */
    DRW_Coord tgEnd;          /*!< end tangent, code 13, 23, 33 */
//    double tgex;              /*!< end tangent x coordinate, code 13 */
//    double tgey;              /*!< end tangent y coordinate, code 23 */
//    double tgez;              /*!< end tangent z coordinate, code 33 */
    int flags{0};                /*!< spline flag, code 70 */
    int degree{0};               /*!< degree of the spline, code 71 */
    dint32 nknots{0};            /*!< number of knots, code 72, default 0 */
    dint32 ncontrol{0};          /*!< number of control points, code 73, default 0 */
    dint32 nfit{0};              /*!< number of fit points, code 74, default 0 */
    double tolknot{0.0000001};           /*!< knot tolerance, code 42, default 0.0000001 */
    double tolcontrol{0.0000001};        /*!< control point tolerance, code 43, default 0.0000001 */
    double tolfit{0.0000001};            /*!< fit point tolerance, code 44, default 0.0000001 */

    std::vector<double> knotslist;           /*!< knots list, code 40 */
    std::vector<double> weightlist;          /*!< weight list, code 41 */
    std::vector<std::shared_ptr<DRW_Coord>> controllist;  /*!< control points list, code 10, 20 & 30 */
    std::vector<std::shared_ptr<DRW_Coord>> fitlist;      /*!< fit points list, code 11, 21 & 31 */

private:
    std::shared_ptr<DRW_Coord> controlpoint;   /*!< current control point to add data */
    std::shared_ptr<DRW_Coord> fitpoint;       /*!< current fit point to add data */
};

/*!
*  Class to handle hatch loop
*  @author Rallaz
*/
class DRW_HatchLoop
{
  public:
    explicit DRW_HatchLoop( int t )
      : type( t )
      , numedges( 0 )
    {
    }

    void update() {
        numedges = objlist.size();
    }

public:
    int type;               /*!< boundary path type, code 92, polyline=2, default=0 */
    int numedges{0};           /*!< number of edges (if not a polyline), code 93 */
//TODO: store lwpolylines as entities
//    std::vector<DRW_LWPolyline *> pollist;  /*!< polyline list */
    std::vector<std::shared_ptr<DRW_Entity>> objlist;      /*!< entities list */
};

/*!
*  Class to handle hatch entity
*  @author Rallaz
*/
//TODO: handle lwpolylines, splines and ellipses
class DRW_Hatch : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Hatch( enum DRW::ETYPE type = DRW::HATCH )
      : DRW_Point( type )
    {
      clearEntities();
    }

    void appendLoop (std::shared_ptr<DRW_HatchLoop> const& v) {
        looplist.push_back(v);
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    UTF8STRING name;           /*!< hatch pattern name, code 2 */
    int solid{1};                 /*!< solid fill flag, code 70, solid=1, pattern=0 */
    int associative{0};           /*!< associativity, code 71, associatve=1, non-assoc.=0 */
    int hstyle{0};                /*!< hatch style, code 75 */
    int hpattern{1};              /*!< hatch pattern type, code 76 */
    int doubleflag{0};            /*!< hatch pattern double flag, code 77, double=1, single=0 */
    int loopsnum{0};              /*!< namber of boundary paths (loops), code 91 */
    double angle{0};              /*!< hatch pattern angle, code 52 */
    double scale{0};              /*!< hatch pattern scale, code 41 */
    int deflines{0};              /*!< number of pattern definition lines, code 78 */

    std::vector<std::shared_ptr<DRW_HatchLoop>> looplist;  /*!< polyline list */

private:
    void clearEntities(){
        pt.reset();
        line.reset();
        pline.reset();
        arc.reset();
        ellipse.reset();
        spline.reset();
        plvert.reset();
    }

    void addLine() {
        clearEntities();
        if (loop) {
            pt = line = std::make_shared<DRW_Line>();
            loop->objlist.push_back(line);
        }
    }

    void addArc() {
        clearEntities();
        if (loop) {
            pt = arc = std::make_shared<DRW_Arc>();
            loop->objlist.push_back(arc);
        }
    }

    void addEllipse() {
        clearEntities();
        if (loop) {
            pt = ellipse = std::make_shared<DRW_Ellipse>();
            loop->objlist.push_back(ellipse);
        }
    }

    void addSpline() {
        clearEntities();
        if (loop) {
            pt.reset();
            spline = std::make_shared<DRW_Spline>();
            loop->objlist.push_back(spline);
        }
    }

    std::shared_ptr<DRW_HatchLoop> loop;       /*!< current loop to add data */
    std::shared_ptr<DRW_Line> line;
    std::shared_ptr<DRW_Arc> arc;
    std::shared_ptr<DRW_Ellipse> ellipse;
    std::shared_ptr<DRW_Spline> spline;
    std::shared_ptr<DRW_LWPolyline> pline;
    std::shared_ptr<DRW_Point> pt;
    std::shared_ptr<DRW_Vertex2D> plvert;
    bool ispol;
};

/*!
*  Class to handle image entity
*  @author Rallaz
*/
class DRW_Image : public DRW_Line {
    SETENTFRIENDS
  public:
    DRW_Image( enum DRW::ETYPE type = DRW::IMAGE )
      : DRW_Line( type )
    {
    }

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    duint32 ref{0};               /*!< Hard reference to imagedef object, code 340 */
    DRW_Coord vVector;         /*!< V-vector of single pixel, x coordinate, code 12, 22 & 32 */
//    double vx;                 /*!< V-vector of single pixel, x coordinate, code 12 */
//    double vy;                 /*!< V-vector of single pixel, y coordinate, code 22 */
//    double vz;                 /*!< V-vector of single pixel, z coordinate, code 32 */
    double sizeu{0};              /*!< image size in pixels, U value, code 13 */
    double sizev{0};              /*!< image size in pixels, V value, code 23 */
    double dz{0};                 /*!< z coordinate, code 33 */
    int clip{0};                  /*!< Clipping state, code 280, 0=off 1=on */
    int brightness{50};            /*!< Brightness value, code 281, (0-100) default 50 */
    int contrast{50};              /*!< Brightness value, code 282, (0-100) default 50 */
    int fade{0};                  /*!< Brightness value, code 283, (0-100) default 0 */

};


/*!
*  Base class for dimension entity
*  @author Rallaz
*/
class DRW_Dimension : public DRW_Entity {
    SETENTFRIENDS
  public:
    DRW_Dimension( enum DRW::ETYPE type = DRW::DIMENSION )
      : DRW_Entity( type )
      , extPoint( 0., 0., 1. )
    {
    }

    DRW_Dimension( const DRW_Dimension &d )
      : DRW_Entity( d )
      , type( d.type )
      , name( d.name )
      , defPoint( d.defPoint )
      , textPoint( d.textPoint )
      , text( d.text )
      , style( d.style )
      , align( d.align )
      , linesty( d.linesty )
      , linefactor( d.linefactor )
      , rot( d.rot )
      , extPoint( d.extPoint )
      , hdir( d.hdir )
      , clonePoint( d.clonePoint )
      , def1( d.def1 )
      , def2( d.def2 )
      , angle( d.angle )
      , oblique( d.oblique )
      , circlePoint( d.circlePoint )
      , arcPoint( d.arcPoint )
      , length( d.length )
    {
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    bool parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *sBuf);
    virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs=0){
        DRW_UNUSED(version); DRW_UNUSED(buf); DRW_UNUSED(bs); return true;}

public:
    DRW_Coord getDefPoint() const {return defPoint;}      /*!< Definition point, code 10, 20 & 30 */
    void setDefPoint(const DRW_Coord p) {defPoint =p;}
    DRW_Coord getTextPoint() const {return textPoint;}    /*!< Middle point of text, code 11, 21 & 31 */
    void setTextPoint(const DRW_Coord p) {textPoint =p;}
    std::string getStyle() const {return style;}          /*!< Dimension style, code 3 */
    void setStyle(const std::string s) {style = s;}
    int getAlign() const { return align;}                 /*!< attachment point, code 71 */
    void setAlign(const int a) { align = a;}
    int getTextLineStyle() const { return linesty;}       /*!< Dimension text line spacing style, code 72, default 1 */
    void setTextLineStyle(const int l) { linesty = l;}
    std::string getText() const {return text;}            /*!< Dimension text explicitly entered by the user, code 1 */
    void setText(const std::string t) {text = t;}
    double getTextLineFactor() const { return linefactor;} /*!< Dimension text line spacing factor, code 41, default 1? */
    void setTextLineFactor(const double l) { linefactor = l;}
    double getDir() const { return rot;}                  /*!< rotation angle of the dimension text, code 53 (optional) default 0 */
    void setDir(const double d) { rot = d;}

    DRW_Coord getExtrusion() const {return extPoint;}            /*!< extrusion, code 210, 220 & 230 */
    void setExtrusion(const DRW_Coord p) {extPoint =p;}
    std::string getName(){return name;}                   /*!< Name of the block that contains the entities, code 2 */
    void setName(const std::string s) {name = s;}
//    int getType(){ return type;}                      /*!< Dimension type, code 70 */

protected:
    DRW_Coord getPt2() const {return clonePoint;}
    void setPt2(const DRW_Coord p) {clonePoint= p;}
    DRW_Coord getPt3() const {return def1;}
    void setPt3(const DRW_Coord p) {def1= p;}
    DRW_Coord getPt4() const {return def2;}
    void setPt4(const DRW_Coord p) {def2= p;}
    DRW_Coord getPt5() const {return circlePoint;}
    void setPt5(const DRW_Coord p) {circlePoint= p;}
    DRW_Coord getPt6() const {return arcPoint;}
    void setPt6(const DRW_Coord p) {arcPoint= p;}
    double getAn50() const {return angle;}      /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    void setAn50(const double d) {angle = d;}
    double getOb52() const {return oblique;}    /*!< oblique angle, code 52 */
    void setOb52(const double d) {oblique = d;}
    double getRa40() const {return length;}    /*!< Leader length, code 40 */
    void setRa40(const double d) {length = d;}
public:
    int type{0};                  /*!< Dimension type, code 70 */
private:
    std::string name;          /*!< Name of the block that contains the entities, code 2 */
    DRW_Coord defPoint;        /*!<  definition point, code 10, 20 & 30 (WCS) */
    DRW_Coord textPoint;       /*!< Middle point of text, code 11, 21 & 31 (OCS) */
    UTF8STRING text;           /*!< Dimension text explicitly entered by the user, code 1 */
    UTF8STRING style{"STANDARD"};          /*!< Dimension style, code 3 */
    int align{5};                 /*!< attachment point, code 71 */
    int linesty{1};               /*!< Dimension text line spacing style, code 72, default 1 */
    double linefactor{1};         /*!< Dimension text line spacing factor, code 41, default 1? (value range 0.25 to 4.00*/
    double rot{0};                /*!< rotation angle of the dimension text, code 53 */
    DRW_Coord extPoint;        /*!<  extrusion normal vector, code 210, 220 & 230 */

    double hdir{0};               /*!< horizontal direction for the dimension, code 51, default ? */
    DRW_Coord clonePoint;      /*!< Insertion point for clones (Baseline & Continue), code 12, 22 & 32 (OCS) */
    DRW_Coord def1;            /*!< Definition point 1for linear & angular, code 13, 23 & 33 (WCS) */
    DRW_Coord def2;            /*!< Definition point 2, code 14, 24 & 34 (WCS) */
    double angle{0};              /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    double oblique{0};            /*!< oblique angle, code 52 */

    DRW_Coord circlePoint;     /*!< Definition point for diameter, radius & angular dims code 15, 25 & 35 (WCS) */
    DRW_Coord arcPoint;        /*!< Point defining dimension arc, x coordinate, code 16, 26 & 36 (OCS) */
    double length{0};             /*!< Leader length, code 40 */

protected:
    dwgHandle dimStyleH;
    dwgHandle blockH;
};


/*!
*  Class to handle aligned dimension entity
*  @author Rallaz
*/
class DRW_DimAligned : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimAligned(){
        eType = DRW::DIMALIGNED;
    }
    explicit DRW_DimAligned(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMALIGNED;
    }

    DRW_Coord getClonepoint() const {return getPt2();}      /*!< Insertion for clones (Baseline & Continue), 12, 22 & 32 */
    void setClonePoint(DRW_Coord c){setPt2(c);}

    DRW_Coord getDimPoint() const {return getDefPoint();}   /*!< dim line location point, code 10, 20 & 30 */
    void setDimPoint(const DRW_Coord p){setDefPoint(p);}
    DRW_Coord getDef1Point() const {return getPt3();}       /*!< Definition point 1, code 13, 23 & 33 */
    void setDef1Point(const DRW_Coord p) {setPt3(p);}
    DRW_Coord getDef2Point() const {return getPt4();}       /*!< Definition point 2, code 14, 24 & 34 */
    void setDef2Point(const DRW_Coord p) {setPt4(p);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};

/*!
*  Class to handle linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimLinear : public DRW_DimAligned {
public:
    DRW_DimLinear() {
        eType = DRW::DIMLINEAR;
    }
    explicit DRW_DimLinear(const DRW_Dimension& d): DRW_DimAligned(d) {
        eType = DRW::DIMLINEAR;
    }

    double getAngle() const {return getAn50();}          /*!< Angle of rotated, horizontal, or vertical dimensions, code 50 */
    void setAngle(const double d) {setAn50(d);}
    double getOblique() const {return getOb52();}      /*!< oblique angle, code 52 */
    void setOblique(const double d) {setOb52(d);}
};

/*!
*  Class to handle aligned, linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimRadial : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimRadial() {
        eType = DRW::DIMRADIAL;
    }
    explicit DRW_DimRadial(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMRADIAL;
    }

    DRW_Coord getCenterPoint() const {return getDefPoint();}   /*!< center point, code 10, 20 & 30 */
    void setCenterPoint(const DRW_Coord p){setDefPoint(p);}
    DRW_Coord getDiameterPoint() const {return getPt5();}      /*!< Definition point for radius, code 15, 25 & 35 */
    void setDiameterPoint(const DRW_Coord p){setPt5(p);}
    double getLeaderLength() const {return getRa40();}         /*!< Leader length, code 40 */
    void setLeaderLength(const double d) {setRa40(d);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};

/*!
*  Class to handle aligned, linear or rotated dimension entity
*  @author Rallaz
*/
class DRW_DimDiametric : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimDiametric() {
        eType = DRW::DIMDIAMETRIC;
    }
    DRW_DimDiametric(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMDIAMETRIC;
    }

    DRW_Coord getDiameter1Point() const {return getPt5();}      /*!< First definition point for diameter, code 15, 25 & 35 */
    void setDiameter1Point(const DRW_Coord p){setPt5(p);}
    DRW_Coord getDiameter2Point() const {return getDefPoint();} /*!< Opposite point for diameter, code 10, 20 & 30 */
    void setDiameter2Point(const DRW_Coord p){setDefPoint(p);}
    double getLeaderLength() const {return getRa40();}          /*!< Leader length, code 40 */
    void setLeaderLength(const double d) {setRa40(d);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};

/*!
*  Class to handle angular dimension entity
*  @author Rallaz
*/
class DRW_DimAngular : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimAngular() {
        eType = DRW::DIMANGULAR;
    }
    DRW_DimAngular(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMANGULAR;
    }

    DRW_Coord getFirstLine1() const {return getPt3();}       /*!< Definition point line 1-1, code 13, 23 & 33 */
    void setFirstLine1(const DRW_Coord p) {setPt3(p);}
    DRW_Coord getFirstLine2() const {return getPt4();}       /*!< Definition point line 1-2, code 14, 24 & 34 */
    void setFirstLine2(const DRW_Coord p) {setPt4(p);}
    DRW_Coord getSecondLine1() const {return getPt5();}      /*!< Definition point line 2-1, code 15, 25 & 35 */
    void setSecondLine1(const DRW_Coord p) {setPt5(p);}
    DRW_Coord getSecondLine2() const {return getDefPoint();} /*!< Definition point line 2-2, code 10, 20 & 30 */
    void setSecondLine2(const DRW_Coord p){setDefPoint(p);}
    DRW_Coord getDimPoint() const {return getPt6();}         /*!< Dimension definition point, code 16, 26 & 36 */
    void setDimPoint(const DRW_Coord p) {setPt6(p);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};


/*!
*  Class to handle angular 3p dimension entity
*  @author Rallaz
*/
class DRW_DimAngular3p : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimAngular3p() {
        eType = DRW::DIMANGULAR3P;
    }
    DRW_DimAngular3p(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMANGULAR3P;
    }

    DRW_Coord getFirstLine() const {return getPt3();}       /*!< Definition point line 1, code 13, 23 & 33 */
    void setFirstLine(const DRW_Coord p) {setPt3(p);}
    DRW_Coord getSecondLine() const {return getPt4();}       /*!< Definition point line 2, code 14, 24 & 34 */
    void setSecondLine(const DRW_Coord p) {setPt4(p);}
    DRW_Coord getVertexPoint() const {return getPt5();}      /*!< Vertex point, code 15, 25 & 35 */
    void SetVertexPoint(const DRW_Coord p) {setPt5(p);}
    DRW_Coord getDimPoint() const {return getDefPoint();}    /*!< Dimension definition point, code 10, 20 & 30 */
    void setDimPoint(const DRW_Coord p) {setDefPoint(p);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};

/*!
*  Class to handle ordinate dimension entity
*  @author Rallaz
*/
class DRW_DimOrdinate : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimOrdinate() {
        eType = DRW::DIMORDINATE;
    }
    DRW_DimOrdinate(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMORDINATE;
    }

    DRW_Coord getOriginPoint() const {return getDefPoint();}   /*!< Origin definition point, code 10, 20 & 30 */
    void setOriginPoint(const DRW_Coord p) {setDefPoint(p);}
    DRW_Coord getFirstLine() const {return getPt3();}          /*!< Feature location point, code 13, 23 & 33 */
    void setFirstLine(const DRW_Coord p) {setPt3(p);}
    DRW_Coord getSecondLine() const {return getPt4();}         /*!< Leader end point, code 14, 24 & 34 */
    void setSecondLine(const DRW_Coord p) {setPt4(p);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);
};


/*!
*  Class to handle leader entity
*  @author Rallaz
*/
class DRW_Leader : public DRW_Entity {
    SETENTFRIENDS
  public:
    DRW_Leader( enum DRW::ETYPE type = DRW::LEADER )
      : DRW_Entity( type )
      , extrusionPoint( 0., 0., 1. )
    {
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    UTF8STRING style;          /*!< Dimension style name, code 3 */
    int arrow{1};                 /*!< Arrowhead flag, code 71, 0=Disabled; 1=Enabled */
    int leadertype{0};            /*!< Leader path type, code 72, 0=Straight line segments; 1=Spline */
    int flag{3};                  /*!< Leader creation flag, code 73, default 3 */
    int hookline{1};              /*!< Hook line direction flag, code 74, default 1 */
    int hookflag{0};              /*!< Hook line flag, code 75 */
    double textheight{0};         /*!< Text annotation height, code 40 */
    double textwidth{0};          /*!< Text annotation width, code 41 */
    int vertnum{0};               /*!< Number of vertices, code 76 */
    int coloruse{0};              /*!< Color to use if leader's DIMCLRD = BYBLOCK, code 77 */
    duint32 annotHandle{0};       /*!< Hard reference to associated annotation, code 340 */
    DRW_Coord extrusionPoint;  /*!< Normal vector, code 210, 220 & 230 */
    DRW_Coord horizdir;        /*!< "Horizontal" direction for leader, code 211, 221 & 231 */
    DRW_Coord offsetblock;     /*!< Offset of last leader vertex from block, code 212, 222 & 232 */
    DRW_Coord offsettext;      /*!< Offset of last leader vertex from annotation, code 213, 223 & 233 */

    std::vector<std::shared_ptr<DRW_Coord>> vertexlist;  /*!< vertex points list, code 10, 20 & 30 */

private:
    std::shared_ptr<DRW_Coord> vertexpoint;   /*!< current control point to add data */
    dwgHandle dimStyleH;
    dwgHandle AnnotH;
};

/*!
*  Class to handle viewport entity
*  @author Rallaz
*/
class DRW_Viewport : public DRW_Point {
    SETENTFRIENDS
  public:
    DRW_Viewport( enum DRW::ETYPE type = DRW::VIEWPORT )
      : DRW_Point( type )
    {
    }

    virtual void applyExtrusion(){}

protected:
    void parseCode(int code, dxfReader *reader);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0);

public:
    double pswidth{205};           /*!< Width in paper space units, code 40 */
    double psheight{156};          /*!< Height in paper space units, code 41 */
    int vpstatus{0};             /*!< Viewport status, code 68 */
    int vpID{0};                 /*!< Viewport ID, code 69 */
    double centerPX{128.5};          /*!< view center point X, code 12 */
    double centerPY{97.5};          /*!< view center point Y, code 22 */
    double snapPX{0};          /*!< Snap base point X, code 13 */
    double snapPY{0};          /*!< Snap base point Y, code 23 */
    double snapSpPX{0};          /*!< Snap spacing X, code 14 */
    double snapSpPY{0};          /*!< Snap spacing Y, code 24 */
    //TODO: complete in dxf
    DRW_Coord viewDir;        /*!< View direction vector, code 16, 26 & 36 */
    DRW_Coord viewTarget;     /*!< View target point, code 17, 27, 37 */
    double viewLength{0};        /*!< Perspective lens length, code 42 */
    double frontClip{0};         /*!< Front clip plane Z value, code 43 */
    double backClip{0};          /*!< Back clip plane Z value, code 44 */
    double viewHeight{0};        /*!< View height in model space units, code 45 */
    double snapAngle{0};         /*!< Snap angle, code 50 */
    double twistAngle{0};        /*!< view twist angle, code 51 */

private:
    duint32 frozenLyCount;
};//RLZ: missing 15,25, 72, 331, 90, 340, 1, 281, 71, 74, 110, 120, 130, 111, 121,131, 112,122, 132, 345,346, and more...

//used  //DRW_Coord basePoint;      /*!<  base point, code 10, 20 & 30 */

//double thickness;         /*!< thickness, code 39 */
//DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
//enum DRW::ETYPE eType;     /*!< enum: entity type, code 0 */
//duint32 handle;            /*!< entity identifier, code 5 */
//std::list<std::list<DRW_Variant> > appData; /*!< list of application data, code 102 */
//duint32 parentHandle;      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
//DRW::Space space;          /*!< space indicator, code 67*/
//UTF8STRING layer;          /*!< layer name, code 8 */
//UTF8STRING lineType;       /*!< line type, code 6 */
//duint32 material;          /*!< hard pointer id to material object, code 347 */
//int color;                 /*!< entity color, code 62 */
//enum DRW_LW_Conv::lineWidth lWeight; /*!< entity lineweight, code 370 */
//double ltypeScale;         /*!< linetype scale, code 48 */
//bool visible;              /*!< entity visibility, code 60 */
//int numProxyGraph;         /*!< Number of bytes in proxy graphics, code 92 */
//std::string proxyGraphics; /*!< proxy graphics bytes, code 310 */
//int color24;               /*!< 24-bit color, code 420 */
//std::string colorName;     /*!< color name, code 430 */
//int transparency;          /*!< transparency, code 440 */
//int plotStyle;             /*!< hard pointer id to plot style object, code 390 */
//DRW::ShadowMode shadow;    /*!< shadow mode, code 284 */
//bool haveExtrusion;        /*!< set to true if the entity have extrusion*/

#endif

// EOF
