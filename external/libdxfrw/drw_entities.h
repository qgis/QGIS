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
#include "drw_base.h"

class dxfReader;
class dwgBuffer;
class DRW_Polyline;

namespace DRW
{

  //! Entity's type.
  enum ETYPE
  {
    E3DFACE,
    E3DSOLID, // encrypted proprietary data
    ACAD_PROXY_ENTITY,
    ARC,
    ATTDEF,
    ATTRIB,
    BLOCK, // and ENDBLK
    BODY, // encrypted proprietary data
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
    HELIX,
    IMAGE,
    INSERT,
    LEADER,
    LIGHT,
    LINE,
    LWPOLYLINE,
    MESH,
    MLINE,
    MLEADERSTYLE,
    MLEADER,
    MTEXT,
    OLEFRAME,
    OLE2FRAME,
    POINT,
    POLYLINE,
    RAY,
    REGION, // encrypted proprietary data
    SECTION,
    SEQEND, // not needed?? used in polyline and insert/attrib and dwg
    SHAPE,
    SOLID,
    SPLINE,
    SUN,
    SURFACE, // encrypted proprietary data can be four types
    TABLE,
    TEXT,
    TOLERANCE,
    TRACE,
    UNDERLAY,
    VERTEX,
    VIEWPORT,
    WIPEOUT, // WIPEOUTVARIABLE
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
      , parentHandle( DRW::NoHandle )
      , appData( 0 )
      , space( DRW::ModelSpace )
      , layer( aLayer )
      , lineType( "BYLAYER" )
      , material( DRW::MaterialByLayer )
      , color( DRW::ColorByLayer )
      , lWeight( DRW_LW_Conv::widthByLayer )
      , ltypeScale( 1.0 )
      , visible( true )
      , numProxyGraph( 0 )
      , proxyGraphics( std::string() )
      , color24( -1 )
      , colorName( std::string() )
      , transparency( DRW::Opaque )
      , plotStyle( DRW::DefaultPlotStyle )
      , shadow( DRW::CastAndReceiveShadows )
      , haveExtrusion( false )
      , extData()
      , haveNextLinks( 0 )
      , plotFlags( 0 )
      , ltFlags( 0 )
      , materialFlag( 0 )
      , shadowFlag( 0 )
      , lTypeH( dwgHandle() )
      , layerH( dwgHandle() )
      , nextEntLink( 0 )
      , prevEntLink( 0 )
      , ownerHandle( false )
      , xDictFlag( 0 )
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
      for ( std::vector<DRW_Variant *>::const_iterator it = e.extData.begin(); it != e.extData.end(); ++it )
      {
        extData.push_back( new DRW_Variant( *( *it ) ) );
      }
    }

    virtual ~DRW_Entity()
    {
      for ( std::vector<DRW_Variant *>::iterator it = extData.begin(); it != extData.end(); ++it )
        delete *it;

      extData.clear();
    }

    void reset()
    {
      for ( std::vector<DRW_Variant *>::iterator it = extData.begin(); it != extData.end(); ++it )
        delete *it;
      extData.clear();
    }

    virtual void applyExtrusion() = 0;

  protected:
    //parses dxf pair to read entity
    bool parseCode( int code, dxfReader *reader );
    //calculates extrusion axis (normal vector)
    void calculateAxis( const DRW_Coord &extPoint );
    //apply extrusion to @extPoint and return data in @point
    void extrudePoint( const DRW_Coord &extPoint, DRW_Coord *point );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 ) = 0;
    //parses dwg common start part to read entity
    bool parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf, duint32 bs = 0 );
    //parses dwg common handles part to read entity
    bool parseDwgEntHandle( DRW::Version version, dwgBuffer *buf );

    //parses dxf 102 groups to read entity
    bool parseDxfGroups( int code, dxfReader *reader );

  public:
    enum DRW::ETYPE eType;                      //!< Enum: entity type, code 0
    duint32 handle;                             //!< Entity identifier, code 5
    duint32 parentHandle;                       //!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330
    std::list<std::list<DRW_Variant> > appData; //!< List of application data, code 102
    DRW::Space space;                           //!< Space indicator, code 67
    UTF8STRING layer;                           //!< Layer name, code 8
    UTF8STRING lineType;                        //!< Line type, code 6
    duint32 material;                           //!< Hard pointer id to material object, code 347
    int color;                                  //!< Entity color, code 62
    enum DRW_LW_Conv::lineWidth lWeight;        //!< Entity lineweight, code 370
    double ltypeScale;                          //!< Linetype scale, code 48
    bool visible;                               //!< Entity visibility, code 60
    int numProxyGraph;                          //!< Number of bytes in proxy graphics, code 92
    std::string proxyGraphics;                  //!< Proxy graphics bytes, code 310
    int color24;                                //!< 24-bit color, code 420
    std::string colorName;                      //!< Color name, code 430
    int transparency;                           //!< Transparency, code 440
    int plotStyle;                              //!< Hard pointer id to plot style object, code 390
    DRW::ShadowMode shadow;                     //!< Shadow mode, code 284
    bool haveExtrusion;                         //!< Set to true if the entity have extrusion
    std::vector<DRW_Variant *> extData;         //!< FIFO list of extended data, codes 1000 to 1071

  protected: //only for read dwg
    duint8 haveNextLinks; // aka nolinks //B
    duint8 plotFlags; // presence of plot style //BB
    duint8 ltFlags; // presence of linetype handle //BB
    duint8 materialFlag; // presence of material handle //BB
    duint8 shadowFlag; // presence of shadow handle ?? (in dwg may be plotflag)//RC
    dwgHandle lTypeH;
    dwgHandle layerH;
    duint32 nextEntLink;
    duint32 prevEntLink;
    bool ownerHandle;

    duint8 xDictFlag;
    dint32 numReactors;
    duint32 objSize;  // RL 32bits object data size in bits
    dint16 oType;

  private:
    DRW_Coord extAxisX;
    DRW_Coord extAxisY;
    DRW_Variant *curr = nullptr;
};


/**
 * Class to handle point entity
 *  @author Rallaz
 */
class DRW_Point : public DRW_Entity
{
    SETENTFRIENDS
  public:
    DRW_Point( enum DRW::ETYPE type = DRW::POINT, double sx = 0., double sy = 0., double sz = 0. )
      : DRW_Entity( type )
      , basePoint( sx, sy, sz )
      , thickness( 0 )
      , extPoint( 0., 0., 1. )
    {
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    DRW_Coord basePoint;      //!< Base point, code 10, 20 & 30
    double thickness;         //!< Thickness, code 39
    DRW_Coord extPoint;       //!< Dir extrusion normal vector, code 210, 220 & 230
    // TNick: we're not handling code 50 - Angle of the X axis for
    // the UCS in effect when the point was drawn
};

/**
 * Class to handle line entity
 *  @author Rallaz
 */
class DRW_Line : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Line( enum DRW::ETYPE type = DRW::LINE )
      : DRW_Point( type )
      , secPoint( 0., 0., 0. )
    {
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    DRW_Coord secPoint;        //!< Second point, code 11, 21 & 31
};

/**
 * Class to handle ray entity
 *  @author Rallaz
 */
class DRW_Ray : public DRW_Line
{
    SETENTFRIENDS
  public:
    DRW_Ray( enum DRW::ETYPE type = DRW::RAY )
      : DRW_Line( type )
    {
    }
  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

/**
 * Class to handle xline entity
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

/**
 * Class to handle circle entity
 *  @author Rallaz
 */
class DRW_Circle : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Circle( enum DRW::ETYPE type = DRW::CIRCLE )
      : DRW_Point( type )
      , mRadius( 0. )
    {
    }

    virtual void applyExtrusion();

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double mRadius;  //!< Radius, code 40
};

/**
 * Class to handle arc entity
 *  @author Rallaz
 */
class DRW_Arc : public DRW_Circle
{
    SETENTFRIENDS
  public:
    DRW_Arc( enum DRW::ETYPE type = DRW::ARC )
      : DRW_Circle( type )
      , staangle( 0 )
      , endangle( M_PIx2 )
      , isccw( 1 )
    {
    }

    virtual void applyExtrusion();

    //! center point in OCS
    const DRW_Coord &center() { return basePoint; }
    //! the radius of the circle
    double radius() { return mRadius; }
    //! start angle in radians
    double startAngle() { return staangle; }
    //! end angle in radians
    double endAngle() { return endangle; }
    //! thickness
    double thick() { return thickness; }
    //! extrusion
    const DRW_Coord &extrusion() { return extPoint; }

  protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode( int code, dxfReader *reader );
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double staangle;            //!< Start angle, code 50 in radians
    double endangle;            //!< End angle, code 51 in radians
    int isccw;                  //!< Is counter clockwise arc?, only used in hatch, code 73
};

/**
 * Class to handle ellipse and elliptic arc entity
 *
 *  Note: start/end parameter are in radians for ellipse entity but
 *  for hatch boundary are in degrees
 *  @author Rallaz
 */
class DRW_Ellipse : public DRW_Line
{
    SETENTFRIENDS
  public:
    DRW_Ellipse( enum DRW::ETYPE type = DRW::ELLIPSE )
      : DRW_Line( type )
      , ratio( 1. )
      , staparam( 0. )
      , endparam( M_PIx2 )
      , isccw( 1 )
    {
    }

    void toPolyline( DRW_Polyline *pol, int parts = 128 ) const;

    virtual void applyExtrusion();

  protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode( int code, dxfReader *reader );
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  private:
    void correctAxis();

  public:
    double ratio;        //!< Ratio, code 40
    double staparam;     //!< Start parameter, code 41, 0.0 for full ellipse
    double endparam;     //!< End parameter, code 42, 2*PI for full ellipse
    int isccw;           //!< Is counter clockwise arc?, only used in hatch, code 73
};

/**
 * Class to handle trace entity
 *
 *  @author Rallaz
 */
class DRW_Trace : public DRW_Line
{
    SETENTFRIENDS
  public:
    DRW_Trace( enum DRW::ETYPE type = DRW::TRACE )
      : DRW_Line( type )
      , thirdPoint( 0., 0., 0. )
      , fourthPoint( 0., 0., 0. )
    {
    }

    virtual void applyExtrusion();

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    DRW_Coord thirdPoint;       //!< Third point, code 12, 22 & 32
    DRW_Coord fourthPoint;      //!< Four point, code 13, 23 & 33
};

/**
 * Class to handle solid entity
 *
 *  @author Rallaz
 */
class DRW_Solid : public DRW_Trace
{
    SETENTFRIENDS
  public:
    DRW_Solid( enum DRW::ETYPE type = DRW::SOLID )
      : DRW_Trace( type )
    {
    }

  protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode( int code, dxfReader *reader );
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    //! first corner (2D)
    const DRW_Coord &firstCorner() { return basePoint; }
    //! second corner (2D)
    const DRW_Coord &secondCorner() { return secPoint; }
    //! third corner (2D)
    const DRW_Coord &thirdCorner() { return thirdPoint; }
    //! fourth corner (2D)
    const DRW_Coord &fourthCorner() { return thirdPoint; }
    //! thickness
    double thick() { return thickness; }
    //! elevation
    double elevation() { return basePoint.z; }
    //! extrusion
    const DRW_Coord &extrusion() { return extPoint; }

};

/**
 * Class to handle 3dface entity
 *  @author Rallaz
 */
class DRW_3Dface : public DRW_Trace
{
    SETENTFRIENDS
  public:
    enum InvisibleEdgeFlags
    {
      NoEdge = 0x00,
      FirstEdge = 0x01,
      SecondEdge = 0x02,
      ThirdEdge = 0x04,
      FourthEdge = 0x08,
      AllEdges = 0x0F  //#spellok
    };

    DRW_3Dface( enum DRW::ETYPE type = DRW::E3DFACE )
      : DRW_Trace( type )
      , invisibleflag( 0 )
    {
    }

    virtual void applyExtrusion() {}

    //! first corner in WCS
    const DRW_Coord &firstCorner() { return basePoint; }
    //! second corner in WCS
    const DRW_Coord &secondCorner() { return secPoint; }
    //! third corner in WCS
    const DRW_Coord &thirdCorner() { return thirdPoint; }
    //! fourth corner in WCS
    const DRW_Coord &fourthCorner() { return fourthPoint; }
    //! edge visibility flags
    InvisibleEdgeFlags edgeFlags() { return ( InvisibleEdgeFlags )invisibleflag; }

  protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    void parseCode( int code, dxfReader *reader );
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    int invisibleflag;       //!< Invisible edge flag, code 70

};

/**
 * Class to handle block entries
 *  @author Rallaz
 */
class DRW_Block : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Block( enum DRW::ETYPE type = DRW::BLOCK )
      : DRW_Point( type )
      , name( "*U0" )
      , flags( 0 )
      , isEnd( false )
    {
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    UTF8STRING name;             //!< Block name, code 2
    int flags;                   //!< Block type, code 70
  private:
    bool isEnd; //for dwg parsing
};


/**
 * Class to handle insert entries
 *  @author Rallaz
 */
class DRW_Insert : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Insert( enum DRW::ETYPE type = DRW::INSERT )
      : DRW_Point( type )
      , xscale( 1 )
      , yscale( 1 )
      , zscale( 1 )
      , angle( 0 )
      , colcount( 1 )
      , rowcount( 1 )
      , colspace( 0 )
      , rowspace( 0 )
    {
    }

    virtual void applyExtrusion() {DRW_Point::applyExtrusion();}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    UTF8STRING name;         //!< Block name, code 2
    double xscale;           //!< X scale factor, code 41
    double yscale;           //!< Y scale factor, code 42
    double zscale;           //!< Z scale factor, code 43
    double angle;            //!< Rotation angle in radians, code 50
    int colcount;            //!< Column count, code 70
    int rowcount;            //!< Row count, code 71
    double colspace;         //!< Column space, code 44
    double rowspace;         //!< Row space, code 45
  public: //only for read dwg
    dwgHandle blockRecH;
    dwgHandle seqendH; //RLZ: on implement attrib remove this handle from obj list (see pline/vertex code)
};

/**
 * Class to handle lwpolyline entity
 *  @author Rallaz
 */
class DRW_LWPolyline : public DRW_Entity
{
    SETENTFRIENDS
  public:
    DRW_LWPolyline( enum DRW::ETYPE type = DRW::LWPOLYLINE )
      : DRW_Entity( type )
      , vertexnum( 0 )
      , flags( 0 )
      , width( 0. )
      , elevation( 0. )
      , thickness( 0. )
      , extPoint( 0., 0., 1. )
      , vertex( nullptr )
    {
    }

    DRW_LWPolyline( const DRW_LWPolyline &p )
      : DRW_Entity( p )
      , vertexnum( 0 )
      , flags( p.flags )
      , width( p.width )
      , elevation( p.elevation )
      , thickness( p.thickness )
      , extPoint( p.extPoint )
      , vertex( nullptr )
    {
      for ( unsigned i = 0; i < p.vertlist.size(); i++ )// RLZ ok or new
        vertlist.push_back( new DRW_Vertex2D( *( p.vertlist.at( i ) ) ) );
    }

    ~DRW_LWPolyline()
    {
      while ( !vertlist.empty() )
      {
        vertlist.pop_back();
      }
    }
    virtual void applyExtrusion();
    void addVertex( DRW_Vertex2D v )
    {
      DRW_Vertex2D *vert = new DRW_Vertex2D();
      vert->x = v.x;
      vert->y = v.y;
      vert->stawidth = v.stawidth;
      vert->endwidth = v.endwidth;
      vert->bulge = v.bulge;
      vertlist.push_back( vert );
    }
    DRW_Vertex2D *addVertex()
    {
      DRW_Vertex2D *vert = new DRW_Vertex2D();
      vert->stawidth = 0;
      vert->endwidth = 0;
      vert->bulge = 0;
      vertlist.push_back( vert );
      return vert;
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version v, dwgBuffer *buf, duint32 bs = 0 );

  public:
    std::vector<DRW_Vertex2D *>::size_type vertexnum; //!< Number of vertices, code 90
    int flags;                                        //!< Polyline flag, code 70, default 0
    double width;                                     //!< Constant width, code 43
    double elevation;                                 //!< Elevation, code 38
    double thickness;                                 //!< Thickness, code 39
    DRW_Coord extPoint;                               //!< Dir extrusion normal vector, code 210, 220 & 230
    DRW_Vertex2D *vertex;                             //!< Current vertex to add data
    std::vector<DRW_Vertex2D *> vertlist;             //!< Vertex list
};

/**
 * Class to handle insert entries
 *  @author Rallaz
 */
class DRW_Text : public DRW_Line
{
    SETENTFRIENDS
  public:
    //! Vertical alignments.
    enum VAlign
    {
      VBaseLine = 0,  //!< Top = 0
      VBottom,        //!< Bottom = 1
      VMiddle,        //!< Middle = 2
      VTop            //!< Top = 3
    };

    //! Horizontal alignments.
    enum HAlign
    {
      HLeft = 0,     //!< Left = 0
      HCenter,       //!< Centered = 1
      HRight,        //!< Right = 2
      HAligned,      //!< Aligned = 3 (if VAlign==0)
      HMiddle,       //!< Middle = 4 (if VAlign==0)
      HFit           //!< Fit into point = 5 (if VAlign==0)
    };

    DRW_Text( enum DRW::ETYPE type = DRW::TEXT )
      : DRW_Line( type )
      , height( 0. )
      , angle( 0. )
      , widthscale( 1. )
      , oblique( 0. )
      , style( "STANDARD" )
      , textgen( 0 )
      , alignH( HLeft )
      , alignV( VBaseLine )
    {
    }

    virtual void applyExtrusion() {} //RLZ TODO

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double height;             //!< Height text, code 40
    UTF8STRING text;           //!< Text string, code 1
    double angle;              //!< Rotation angle in degrees (360), code 50
    double widthscale;         //!< Width factor, code 41
    double oblique;            //!< Oblique angle, code 51
    UTF8STRING style;          //!< Style name, code 7
    int textgen;               //!< Text generation, code 71
    enum HAlign alignH;        //!< Horizontal align, code 72
    enum VAlign alignV;        //!< Vertical align, code 73
    dwgHandle styleH;          //!< Handle for text style
};

/**
 * Class to handle insert entries
 *  @author Rallaz
 */
class DRW_MText : public DRW_Text
{
    SETENTFRIENDS
  public:
    //! Attachments.
    enum Attach
    {
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
      interlin = 1;
      alignV = ( VAlign )TopLeft;
      textgen = 1;
      haveXAxis = false;    //if true needed to recalculate angle
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    void updateAngle();    //recalculate angle if 'haveXAxis' is true
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double interlin;     //!< Width factor, code 44
  private:
    bool haveXAxis;
};

/**
 * Class to handle vertex for polyline entity
 *  @author Rallaz
 */
class DRW_Vertex : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Vertex( enum DRW::ETYPE type = DRW::VERTEX )
      : DRW_Point( type )
      , stawidth( 0. )
      , endwidth( 0. )
      , bulge( 0. )
      , flags( 0 )
      , tgdir( 0. )
      , vindex1( 0 )
      , vindex2( 0 )
      , vindex3( 0 )
      , vindex4( 0 )
      , identifier( 0 )
    {
    }

    DRW_Vertex( double sx, double sy, double sz, double b )
      : DRW_Point( DRW::VERTEX, sx, sy, sz )
      , stawidth( 0. )
      , endwidth( 0. )
      , bulge( b )
      , flags( 0 )
      , tgdir( 0. )
      , vindex1( 0 )
      , vindex2( 0 )
      , vindex3( 0 )
      , vindex4( 0 )
      , identifier( 0 )
    {
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0, double el = 0 );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 )
    {
      DRW_UNUSED( version );
      DRW_UNUSED( buf );
      DRW_UNUSED( bs );
      return true;
    }

  public:
    double stawidth;          //!< Start width, code 40
    double endwidth;          //!< End width, code 41
    double bulge;             //!< Bulge, code 42

    int flags;                //!< Vertex flag, code 70, default 0
    double tgdir;             //!< Curve fit tangent direction, code 50
    int vindex1;              //!< Polyface mesh vertex index, code 71, default 0
    int vindex2;              //!< Polyface mesh vertex index, code 72, default 0
    int vindex3;              //!< Polyface mesh vertex index, code 73, default 0
    int vindex4;              //!< Polyface mesh vertex index, code 74, default 0
    int identifier;           //!< Vertex identifier, code 91, default 0
};

/**
 * Class to handle polyline entity
 *  @author Rallaz
 */
class DRW_Polyline : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Polyline( enum DRW::ETYPE type = DRW::POLYLINE )
      : DRW_Point( type )
      , flags( 0 )
      , defstawidth( 0. )
      , defendwidth( 0. )
      , vertexcount( 0 )
      , facecount( 0 )
      , smoothM( 0 )
      , smoothN( 0 )
      , curvetype( 0 )
      , firstEH( 0 )
      , lastEH( 0 )
    {
    }

    ~DRW_Polyline()
    {
      while ( !vertlist.empty() )
      {
        vertlist.pop_back();
      }
    }
    void addVertex( DRW_Vertex v )
    {
      DRW_Vertex *vert = new DRW_Vertex();
      vert->basePoint = v.basePoint;
      vert->stawidth = v.stawidth;
      vert->endwidth = v.endwidth;
      vert->bulge = v.bulge;
      vertlist.push_back( vert );
    }
    void appendVertex( DRW_Vertex *v )
    {
      vertlist.push_back( v );
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    int flags;               //!< Polyline flag, code 70, default 0
    double defstawidth;      //!< Start width, code 40, default 0
    double defendwidth;      //!< End width, code 41, default 0
    int vertexcount;         //!< Polygon mesh M vertex or  polyface vertex num, code 71, default 0
    int facecount;           //!< Polygon mesh N vertex or  polyface face num, code 72, default 0
    int smoothM;             //!< Smooth surface M density, code 73, default 0
    int smoothN;             //!< Smooth surface M density, code 74, default 0
    int curvetype;           //!< Curves & smooth surface type, code 75, default 0

    std::vector<DRW_Vertex *> vertlist;  //!< Vertex list

  private:
    std::list<duint32> handleList;  // list of handles, only in 2004+
    duint32 firstEH;                // handle of first entity, only in pre-2004
    duint32 lastEH;                 // handle of last entity, only in pre-2004
    dwgHandle seqEndH;              // handle of SEQEND entity
};


/**
 * Class to handle spline entity
 *  @author Rallaz
 */
class DRW_Spline : public DRW_Entity
{
    SETENTFRIENDS
  public:
    DRW_Spline()
      : DRW_Entity( DRW::SPLINE )
      , flags( 0 )
      , degree( 0 )
      , nknots( 0 )
      , ncontrol( 0 )
      , nfit( 0 )
      , tolknot( 0.0000001 )
      , tolcontrol( 0.0000001 )
      , tolfit( 0.0000001 )
      , controlpoint( nullptr )
      , fitpoint( nullptr )
    {
    }
    ~DRW_Spline()
    {
      while ( !controllist.empty() )
      {
        controllist.pop_back();
      }
      while ( !fitlist.empty() )
      {
        fitlist.pop_back();
      }
    }
    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
//  double ex;                //!< Normal vector x coordinate, code 210
//  double ey;                //!< Normal vector y coordinate, code 220
//  double ez;                //!< Normal vector z coordinate, code 230
    DRW_Coord normalVec;      //!< Normal vector, code 210, 220, 230
    DRW_Coord tgStart;        //!< Start tangent, code 12, 22, 32
//  double tgsx;              //!< Start tangent x coordinate, code 12
//  double tgsy;              //!< Start tangent y coordinate, code 22
//  double tgsz;              //!< Start tangent z coordinate, code 32
    DRW_Coord tgEnd;          //!< End tangent, code 13, 23, 33
//  double tgex;              //!< End tangent x coordinate, code 13
//  double tgey;              //!< End tangent y coordinate, code 23
//  double tgez;              //!< End tangent z coordinate, code 33
    int flags;                //!< Spline flag, code 70
    int degree;               //!< Degree of the spline, code 71
    dint32 nknots;            //!< Number of knots, code 72, default 0
    dint32 ncontrol;          //!< Number of control points, code 73, default 0
    dint32 nfit;              //!< Number of fit points, code 74, default 0
    double tolknot;           //!< Knot tolerance, code 42, default 0.0000001
    double tolcontrol;        //!< Control point tolerance, code 43, default 0.0000001
    double tolfit;            //!< Fit point tolerance, code 44, default 0.0000001

    std::vector<double> knotslist;         //!< Knots list, code 40
    std::vector<DRW_Coord *> controllist;  //!< Control points list, code 10, 20 & 30
    std::vector<DRW_Coord *> fitlist;      //!< Fit points list, code 11, 21 & 31

  private:
    DRW_Coord *controlpoint;   //!< Current control point to add data
    DRW_Coord *fitpoint;       //!< Current fit point to add data
};

/**
 * Class to handle hatch loop
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

    ~DRW_HatchLoop()
    {
#if 0
      while ( !pollist.empty() )
      {
        pollist.pop_back();
      }
#endif
      while ( !objlist.empty() )
      {
        objlist.pop_back();
      }
    }

    void update()
    {
      numedges = objlist.size();
    }

  public:
    int type;                                       //!< Boundary path type, code 92, polyline=2, default=0
    std::vector<DRW_Entity *>::size_type numedges;  //!< Number of edges (if not a polyline), code 93
//TODO: store lwpolylines as entities
//  std::vector<DRW_LWPolyline *> pollist;          //!< Polyline list
    std::vector<DRW_Entity *> objlist;              //!< Entities list
};

/**
 * Class to handle hatch entity
 *  @author Rallaz
 */
//TODO: handle lwpolylines, splines and ellipses
class DRW_Hatch : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Hatch( enum DRW::ETYPE type = DRW::HATCH )
      : DRW_Point( type )
      , solid( 1 )
      , associative( 0 )
      , hstyle( 0 )
      , hpattern( 1 )
      , doubleflag( 0 )
      , loopsnum( 0 )
      , angle( 0. )
      , scale( 0. )
      , deflines( 0 )
      , loop( nullptr )
    {
      clearEntities();
    }

    ~DRW_Hatch()
    {
      while ( !looplist.empty() )
      {
        looplist.pop_back();
      }
    }

    void appendLoop( DRW_HatchLoop *v )
    {
      looplist.push_back( v );
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    UTF8STRING name;                                   //!< Hatch pattern name, code 2
    int solid;                                         //!< Solid fill flag, code 70, solid=1, pattern=0
    int associative;                                   //!< Associativity, code 71, associatve=1, non-assoc.=0
    int hstyle;                                        //!< Hatch style, code 75
    int hpattern;                                      //!< Hatch pattern type, code 76
    int doubleflag;                                    //!< Hatch pattern double flag, code 77, double=1, single=0
    std::vector<DRW_HatchLoop *>::size_type loopsnum;  //!< Number of boundary paths (loops), code 91
    double angle;                                      //!< Hatch pattern angle, code 52
    double scale;                                      //!< Hatch pattern scale, code 41
    int deflines;                                      //!< Number of pattern definition lines, code 78

    std::vector<DRW_HatchLoop *> looplist;             //!< Polyline list

  private:
    void clearEntities()
    {
      pt = line = nullptr;
      pline = nullptr;
      arc = nullptr;
      ellipse = nullptr;
      spline = nullptr;
      plvert = nullptr;
    }

    void addLine()
    {
      clearEntities();
      if ( loop )
      {
        pt = line = new DRW_Line;
        loop->objlist.push_back( line );
      }
    }

    void addArc()
    {
      clearEntities();
      if ( loop )
      {
        pt = arc = new DRW_Arc;
        loop->objlist.push_back( arc );
      }
    }

    void addEllipse()
    {
      clearEntities();
      if ( loop )
      {
        pt = ellipse = new DRW_Ellipse;
        loop->objlist.push_back( ellipse );
      }
    }

    void addSpline()
    {
      clearEntities();
      if ( loop )
      {
        pt = nullptr;
        spline = new DRW_Spline;
        loop->objlist.push_back( spline );
      }
    }

    DRW_HatchLoop *loop;       //!< Current loop to add data
    DRW_Line *line = nullptr;
    DRW_Arc *arc = nullptr;
    DRW_Ellipse *ellipse = nullptr;
    DRW_Spline *spline = nullptr;
    DRW_LWPolyline *pline = nullptr;
    DRW_Point *pt = nullptr;
    DRW_Vertex2D *plvert = nullptr;
    bool ispol;
};

/**
 * Class to handle image entity
 *  @author Rallaz
 */
class DRW_Image : public DRW_Line
{
    SETENTFRIENDS
  public:
    DRW_Image( enum DRW::ETYPE type = DRW::IMAGE )
      : DRW_Line( type )
      , ref( 0 )
      , sizeu( 0. )
      , sizev( 0. )
      , dz( 0. )
      , clip( 0 )
      , brightness( 50 )
      , contrast( 50 )
      , fade( 0 )
    {
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    duint32 ref;               //!< Hard reference to imagedef object, code 340
    DRW_Coord vVector;         //!< V-vector of single pixel, x coordinate, code 12, 22 & 32
    double sizeu;              //!< Image size in pixels, U value, code 13
    double sizev;              //!< Image size in pixels, V value, code 23
    double dz;                 //!< Z coordinate, code 33
    int clip;                  //!< Clipping state, code 280, 0=off 1=on
    int brightness;            //!< Brightness value, code 281, (0-100) default 50
    int contrast;              //!< Brightness value, code 282, (0-100) default 50
    int fade;                  //!< Brightness value, code 283, (0-100) default 0

};


/**
 * Base class for dimension entity
 *  @author Rallaz
 */
class DRW_Dimension : public DRW_Entity
{
    SETENTFRIENDS
  public:
    DRW_Dimension( enum DRW::ETYPE type = DRW::DIMENSION )
      : DRW_Entity( type )
      , type( 0 )
      , style( "STANDARD" )
      , align( 5 )
      , linesty( 1 )
      , linefactor( 1. )
      , rot( 0. )
      , extPoint( 0., 0., 1. )
      , hdir( 0 )   // correct default?
      , angle( 0. )
      , oblique( 0. )
      , length( 0. )
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

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *sBuf );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 )
    {
      DRW_UNUSED( version );
      DRW_UNUSED( buf );
      DRW_UNUSED( bs );
      return true;
    }

  public:
    DRW_Coord getDefPoint() const {return defPoint;}      //!< Definition point, code 10, 20 & 30
    void setDefPoint( const DRW_Coord &p ) {defPoint = p;}
    DRW_Coord getTextPoint() const {return textPoint;}    //!< Middle point of text, code 11, 21 & 31
    void setTextPoint( const DRW_Coord &p ) {textPoint = p;}
    std::string getStyle() const {return style;}          //!< Dimension style, code 3
    void setStyle( const std::string &s ) {style = s;}
    int getAlign() const { return align;}                 //!< Attachment point, code 71
    void setAlign( const int a ) { align = a;}
    int getTextLineStyle() const { return linesty;}       //!< Dimension text line spacing style, code 72, default 1
    void setTextLineStyle( const int l ) { linesty = l;}
    std::string getText() const {return text;}            //!< Dimension text explicitly entered by the user, code 1
    void setText( const std::string &t ) {text = t;}
    double getTextLineFactor() const { return linefactor;} //!< Dimension text line spacing factor, code 41, default 1?
    void setTextLineFactor( const double l ) { linefactor = l;}
    double getDir() const { return rot;}                  //!< Rotation angle of the dimension text, code 53 (optional) default 0
    void setDir( const double d ) { rot = d;}

    DRW_Coord getExtrusion() {return extPoint;}           //!< Extrusion, code 210, 220 & 230
    void setExtrusion( const DRW_Coord &p ) {extPoint = p;}
    std::string getName() {return name;}                  //!< Name of the block that contains the entities, code 2
    void setName( const std::string &s ) {name = s;}
//  int getType(){ return type;}                          //!< Dimension type, code 70

  protected:
    DRW_Coord getPt2() const {return clonePoint;}
    void setPt2( const DRW_Coord &p ) {clonePoint = p;}
    DRW_Coord getPt3() const {return def1;}
    void setPt3( const DRW_Coord &p ) {def1 = p;}
    DRW_Coord getPt4() const {return def2;}
    void setPt4( const DRW_Coord &p ) {def2 = p;}
    DRW_Coord getPt5() const {return circlePoint;}
    void setPt5( const DRW_Coord &p ) {circlePoint = p;}
    DRW_Coord getPt6() const {return arcPoint;}
    void setPt6( const DRW_Coord &p ) {arcPoint = p;}
    double getAn50() const {return angle;}      //!< Angle of rotated, horizontal, or vertical dimensions, code 50
    void setAn50( const double d ) {angle = d;}
    double getOb52() const {return oblique;}    //!< Oblique angle, code 52
    void setOb52( const double d ) {oblique = d;}
    double getRa40() const {return length;}    //!< Leader length, code 40
    void setRa40( const double d ) {length = d;}
  public:
    int type;                  //!< Dimension type, code 70
  private:
    std::string name;          //!< Name of the block that contains the entities, code 2
    DRW_Coord defPoint;        //!< Definition point, code 10, 20 & 30 (WCS)
    DRW_Coord textPoint;       //!< Middle point of text, code 11, 21 & 31 (OCS)
    UTF8STRING text;           //!< Dimension text explicitly entered by the user, code 1
    UTF8STRING style;          //!< Dimension style, code 3
    int align;                 //!< Attachment point, code 71
    int linesty;               //!< Dimension text line spacing style, code 72, default 1
    double linefactor;         //!< Dimension text line spacing factor, code 41, default 1? (value range 0.25 to 4.00)
    double rot;                //!< Rotation angle of the dimension text, code 53
    DRW_Coord extPoint;        //!< Extrusion normal vector, code 210, 220 & 230

    double hdir;               //!< Horizontal direction for the dimension, code 51, default ?
    DRW_Coord clonePoint;      //!< Insertion point for clones (Baseline & Continue), code 12, 22 & 32 (OCS)
    DRW_Coord def1;            //!< Definition point 1for linear & angular, code 13, 23 & 33 (WCS)
    DRW_Coord def2;            //!< Definition point 2, code 14, 24 & 34 (WCS)
    double angle;              //!< Angle of rotated, horizontal, or vertical dimensions, code 50
    double oblique;            //!< Oblique angle, code 52

    DRW_Coord circlePoint;     //!< Definition point for diameter, radius & angular dims code 15, 25 & 35 (WCS)
    DRW_Coord arcPoint;        //!< Point defining dimension arc, x coordinate, code 16, 26 & 36 (OCS)
    double length;             //!< Leader length, code 40

  protected:
    dwgHandle dimStyleH;
    dwgHandle blockH;
};


/**
 * Class to handle aligned dimension entity
 *  @author Rallaz
 */
class DRW_DimAligned : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimAligned() : DRW_Dimension()
    {
      eType = DRW::DIMALIGNED;
    }

    explicit DRW_DimAligned( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMALIGNED;
    }

    DRW_Coord getClonepoint() const {return getPt2();}      //!< Insertion for clones (Baseline & Continue), 12, 22 & 32
    void setClonePoint( DRW_Coord &c ) {setPt2( c );}

    DRW_Coord getDimPoint() const {return getDefPoint();}   //!< Dim line location point, code 10, 20 & 30
    void setDimPoint( const DRW_Coord &p ) {setDefPoint( p );}
    DRW_Coord getDef1Point() const {return getPt3();}       //!< Definition point 1, code 13, 23 & 33
    void setDef1Point( const DRW_Coord &p ) {setPt3( p );}
    DRW_Coord getDef2Point() const {return getPt4();}       //!< Definition point 2, code 14, 24 & 34
    void setDef2Point( const DRW_Coord &p ) {setPt4( p );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

/**
 * Class to handle linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimLinear : public DRW_DimAligned
{
  public:
    DRW_DimLinear() : DRW_DimAligned()
    {
      eType = DRW::DIMLINEAR;
    }

    explicit DRW_DimLinear( const DRW_Dimension &d ) : DRW_DimAligned( d )
    {
      eType = DRW::DIMLINEAR;
    }

    double getAngle() const {return getAn50();}        //!< Angle of rotated, horizontal, or vertical dimensions, code 50
    void setAngle( const double d ) {setAn50( d );}
    double getOblique() const {return getOb52();}      //!< Oblique angle, code 52
    void setOblique( const double d ) {setOb52( d );}
};

/**
 * Class to handle aligned, linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimRadial : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimRadial() : DRW_Dimension()
    {
      eType = DRW::DIMRADIAL;
    }

    explicit DRW_DimRadial( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMRADIAL;
    }

    DRW_Coord getCenterPoint() const {return getDefPoint();}   //!< Center point, code 10, 20 & 30
    void setCenterPoint( const DRW_Coord &p ) {setDefPoint( p );}
    DRW_Coord getDiameterPoint() const {return getPt5();}      //!< Definition point for radius, code 15, 25 & 35
    void setDiameterPoint( const DRW_Coord &p ) {setPt5( p );}
    double getLeaderLength() const {return getRa40();}         //!< Leader length, code 40
    void setLeaderLength( const double d ) {setRa40( d );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

/**
 * Class to handle aligned, linear or rotated dimension entity
 *  @author Rallaz
 */
class DRW_DimDiametric : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimDiametric() : DRW_Dimension()
    {
      eType = DRW::DIMDIAMETRIC;
    }

    DRW_DimDiametric( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMDIAMETRIC;
    }

    DRW_Coord getDiameter1Point() const {return getPt5();}      //!< First definition point for diameter, code 15, 25 & 35
    void setDiameter1Point( const DRW_Coord &p ) {setPt5( p );}
    DRW_Coord getDiameter2Point() const {return getDefPoint();} //!< Opposite point for diameter, code 10, 20 & 30
    void setDiameter2Point( const DRW_Coord &p ) {setDefPoint( p );}
    double getLeaderLength() const {return getRa40();}          //!< Leader length, code 40
    void setLeaderLength( const double d ) {setRa40( d );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

/**
 * Class to handle angular dimension entity
 *  @author Rallaz
 */
class DRW_DimAngular : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimAngular() : DRW_Dimension()
    {
      eType = DRW::DIMANGULAR;
    }

    DRW_DimAngular( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMANGULAR;
    }

    DRW_Coord getFirstLine1() const {return getPt3();}       //!< Definition point line 1-1, code 13, 23 & 33
    void setFirstLine1( const DRW_Coord &p ) {setPt3( p );}
    DRW_Coord getFirstLine2() const {return getPt4();}       //!< Definition point line 1-2, code 14, 24 & 34
    void setFirstLine2( const DRW_Coord &p ) {setPt4( p );}
    DRW_Coord getSecondLine1() const {return getPt5();}      //!< Definition point line 2-1, code 15, 25 & 35
    void setSecondLine1( const DRW_Coord &p ) {setPt5( p );}
    DRW_Coord getSecondLine2() const {return getDefPoint();} //!< Definition point line 2-2, code 10, 20 & 30
    void setSecondLine2( const DRW_Coord &p ) {setDefPoint( p );}
    DRW_Coord getDimPoint() const {return getPt6();}         //!< Dimension definition point, code 16, 26 & 36
    void setDimPoint( const DRW_Coord &p ) {setPt6( p );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};


/**
 * Class to handle angular 3p dimension entity
 *  @author Rallaz
 */
class DRW_DimAngular3p : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimAngular3p() : DRW_Dimension()
    {
      eType = DRW::DIMANGULAR3P;
    }

    DRW_DimAngular3p( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMANGULAR3P;
    }

    DRW_Coord getFirstLine() const {return getPt3();}        //!< Definition point line 1, code 13, 23 & 33
    void setFirstLine( const DRW_Coord &p ) {setPt3( p );}
    DRW_Coord getSecondLine() const {return getPt4();}       //!< Definition point line 2, code 14, 24 & 34
    void setSecondLine( const DRW_Coord &p ) {setPt4( p );}
    DRW_Coord getVertexPoint() const {return getPt5();}      //!< Vertex point, code 15, 25 & 35
    void SetVertexPoint( const DRW_Coord &p ) {setPt5( p );}
    DRW_Coord getDimPoint() const {return getDefPoint();}    //!< Dimension definition point, code 10, 20 & 30
    void setDimPoint( const DRW_Coord &p ) {setDefPoint( p );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

/**
 * Class to handle ordinate dimension entity
 *  @author Rallaz
 */
class DRW_DimOrdinate : public DRW_Dimension
{
    SETENTFRIENDS
  public:
    DRW_DimOrdinate() : DRW_Dimension()
    {
      eType = DRW::DIMORDINATE;
    }

    DRW_DimOrdinate( const DRW_Dimension &d ) : DRW_Dimension( d )
    {
      eType = DRW::DIMORDINATE;
    }

    DRW_Coord getOriginPoint() const {return getDefPoint();}   //!< Origin definition point, code 10, 20 & 30
    void setOriginPoint( const DRW_Coord &p ) {setDefPoint( p );}
    DRW_Coord getFirstLine() const {return getPt3();}          //!< Feature location point, code 13, 23 & 33
    void setFirstLine( const DRW_Coord &p ) {setPt3( p );}
    DRW_Coord getSecondLine() const {return getPt4();}         //!< Leader end point, code 14, 24 & 34
    void setSecondLine( const DRW_Coord &p ) {setPt4( p );}

  protected:
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};


/**
 * Class to handle leader entity
 *  @author Rallaz
 */
class DRW_Leader : public DRW_Entity
{
    SETENTFRIENDS
  public:
    DRW_Leader( enum DRW::ETYPE type = DRW::LEADER )
      : DRW_Entity( type )
      , arrow( 1 )
      , leadertype( 0 )
      , flag( 3 )
      , hookline( 1 )
      , hookflag( 0 )
      , textheight( 0. )
      , textwidth( 0. )
      , vertnum( 0 )
      , coloruse( 0 )
      , annotHandle( 0 )
      , extrusionPoint( 0., 0., 1. )
      , vertexpoint( nullptr )
    {
    }

    ~DRW_Leader()
    {
      while ( !vertexlist.empty() )
      {
        vertexlist.pop_back();
      }
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    UTF8STRING style;          //!< Dimension style name, code 3
    int arrow;                 //!< Arrowhead flag, code 71, 0=Disabled; 1=Enabled
    int leadertype;            //!< Leader path type, code 72, 0=Straight line segments; 1=Spline
    int flag;                  //!< Leader creation flag, code 73, default 3
    int hookline;              //!< Hook line direction flag, code 74, default 1
    int hookflag;              //!< Hook line flag, code 75
    double textheight;         //!< Text annotation height, code 40
    double textwidth;          //!< Text annotation width, code 41
    int vertnum;               //!< Number of vertices, code 76
    int coloruse;              //!< Color to use if leader's DIMCLRD = BYBLOCK, code 77
    duint32 annotHandle;       //!< Hard reference to associated annotation, code 340
    DRW_Coord extrusionPoint;  //!< Normal vector, code 210, 220 & 230
    DRW_Coord horizdir;        //!< "Horizontal" direction for leader, code 211, 221 & 231
    DRW_Coord offsetblock;     //!< Offset of last leader vertex from block, code 212, 222 & 232
    DRW_Coord offsettext;      //!< Offset of last leader vertex from annotation, code 213, 223 & 233

    std::vector<DRW_Coord *> vertexlist;  //!< Vertex points list, code 10, 20 & 30

  private:
    DRW_Coord *vertexpoint;   //!< Current control point to add data
    dwgHandle dimStyleH;
    dwgHandle AnnotH;
};

/**
 * Class to handle viewport entity
 *  @author Rallaz
 */
class DRW_Viewport : public DRW_Point
{
    SETENTFRIENDS
  public:
    DRW_Viewport( enum DRW::ETYPE type = DRW::VIEWPORT )
      : DRW_Point( type )
      , pswidth( 205 )
      , psheight( 156 )
      , vpstatus( 0 )
      , vpID( 0 )
      , centerPX( 128.5 )
      , centerPY( 97.5 )
      , snapPX( 0. )
      , snapPY( 0. )
      , snapSpPX( 0. )
      , snapSpPY( 0. )
      , viewLength( 0. )
      , frontClip( 0. )
      , backClip( 0. )
      , viewHeight( 0. )
      , snapAngle( 0. )
      , twistAngle( 0. )
      , frozenLyCount( 0 )
    {
    }

    virtual void applyExtrusion() {}

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double pswidth;           //!< Width in paper space units, code 40
    double psheight;          //!< Height in paper space units, code 41
    int vpstatus;             //!< Viewport status, code 68
    int vpID;                 //!< Viewport ID, code 69
    double centerPX;          //!< View center point X, code 12
    double centerPY;          //!< View center point Y, code 22
    double snapPX;            //!< Snap base point X, code 13
    double snapPY;            //!< Snap base point Y, code 23
    double snapSpPX;          //!< Snap spacing X, code 14
    double snapSpPY;          //!< Snap spacing Y, code 24
    //TODO: complete in dxf
    DRW_Coord viewDir;        //!< View direction vector, code 16, 26 & 36
    DRW_Coord viewTarget;     //!< View target point, code 17, 27, 37
    double viewLength;        //!< Perspective lens length, code 42
    double frontClip;         //!< Front clip plane Z value, code 43
    double backClip;          //!< Back clip plane Z value, code 44
    double viewHeight;        //!< View height in model space units, code 45
    double snapAngle;         //!< Snap angle, code 50
    double twistAngle;        //!< View twist angle, code 51

  private:
    duint32 frozenLyCount;
}; //RLZ: missing 15,25, 72, 331, 90, 340, 1, 281, 71, 74, 110, 120, 130, 111, 121,131, 112,122, 132, 345,346, and more...

#if 0
//used
DRW_Coord basePoint;      //!< Base point, code 10, 20 & 30

double thickness;          //!< Thickness, code 39
DRW_Coord extPoint;        //!< Dir extrusion normal vector, code 210, 220 & 230
enum DRW::ETYPE eType;     //!< Enum: entity type, code 0
duint32 handle;            //!< Entity identifier, code 5
std::list<std::list<DRW_Variant> > appData; //!< List of application data, code 102
duint32 parentHandle;      //!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330
DRW::Space space;          //!< Space indicator, code 67
UTF8STRING layer;          //!< Layer name, code 8
UTF8STRING lineType;       //!< Line type, code 6
duint32 material;          //!< Hard pointer id to material object, code 347
int color;                 //!< Entity color, code 62
enum DRW_LW_Conv::lineWidth lWeight; //!< Entity lineweight, code 370
double ltypeScale;         //!< Linetype scale, code 48
bool visible;              //!< Entity visibility, code 60
int numProxyGraph;         //!< Number of bytes in proxy graphics, code 92
std::string proxyGraphics; //!< Proxy graphics bytes, code 310
int color24;               //!< 24-bit color, code 420
std::string colorName;     //!< Color name, code 430
int transparency;          //!< Transparency, code 440
int plotStyle;             //!< Hard pointer id to plot style object, code 390
DRW::ShadowMode shadow;    //!< Shadow mode, code 284
bool haveExtrusion;        //!< Set to true if the entity have extrusion
#endif

#endif

// EOF
