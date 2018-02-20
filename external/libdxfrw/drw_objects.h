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

#ifndef DRW_OBJECTS_H
#define DRW_OBJECTS_H


#include <string>
#include <vector>
#include <map>
#include "drw_base.h"

class dxfReader;
class dxfWriter;
class dwgBuffer;

namespace DRW
{

//! Table entries type.
  enum TTYPE
  {
    UNKNOWNT,
    LTYPE,
    LAYER,
    STYLE,
    DIMSTYLE,
    VPORT,
    BLOCK_RECORD,
    APPID,
    IMAGEDEF
  };

//@todo VIEW, UCS, APPID, VP_ENT_HDR, GROUP, MLINESTYLE, LONG_TRANSACTION, XRECORD,
// ACDBPLACEHOLDER, VBA_PROJECT, ACAD_TABLE, CELLSTYLEMAP, DBCOLOR, DICTIONARYVAR,
// DICTIONARYWDFLT, FIELD, IDBUFFER, IMAGEDEF, IMAGEDEFREACTOR, LAYER_INDEX, LAYOUT,
// MATERIAL, PLACEHOLDER, PLOTSETTINGS, RASTERVARIABLES, SCALE, SORTENTSTABLE,
// SPATIAL_INDEX, SPATIAL_FILTER, TABLEGEOMETRY, TABLESTYLES, VISUALSTYLE
}

#define SETOBJFRIENDS  friend class dxfRW; \
  friend class dwgReader;

/** Base class for tables entries
 *  @author Rallaz
 */
class DRW_TableEntry
{
  public:
    //initializes default values
    DRW_TableEntry()
      : tType( DRW::UNKNOWNT )
      , parentHandle( 0 )
      , flags( 0 )
      , curr( nullptr )
      , xDictFlag( 0 )
      , numReactors( 0 )
      , objSize( 0 )
    {
    }

    virtual ~DRW_TableEntry()
    {
      for ( std::vector<DRW_Variant *>::iterator it = extData.begin(); it != extData.end(); ++it )
        delete *it;

      extData.clear();
    }

    DRW_TableEntry( const DRW_TableEntry &e )
      : tType( e.tType )
      , handle( e.handle )
      , parentHandle( e.parentHandle )
      , name( e.name )
      , flags( e.flags )
      , curr( e.curr )
      , xDictFlag( e.xDictFlag )
      , numReactors( e.numReactors )
      , objSize( e.objSize )
    {
      for ( std::vector<DRW_Variant *>::const_iterator it = e.extData.begin(); it != e.extData.end(); ++it )
      {
        extData.push_back( new DRW_Variant( *( *it ) ) );
      }
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    virtual bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 ) = 0;
    bool parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf, duint32 bs = 0 );
    void reset()
    {
      flags = 0;
      for ( std::vector<DRW_Variant *>::iterator it = extData.begin(); it != extData.end(); ++it )
        delete *it;
      extData.clear();
    }

  public:
    enum DRW::TTYPE tType;              //!< Enum: entity type, code 0
    duint32 handle;                     //!< Entity identifier, code 5
    int parentHandle;                   //!< Soft-pointer ID/handle to owner object, code 330
    UTF8STRING name;                    //!< Entry name, code 2
    int flags;                          //!< Flags relevant to entry, code 70
    std::vector<DRW_Variant *> extData; //!< FIFO list of extended data, codes 1000 to 1071

  private:
    DRW_Variant *curr = nullptr;

    //***** dwg parse ********/
  protected:
    dint16 oType;
    duint8 xDictFlag;
    dint32 numReactors; //
    duint32 objSize;  //RL 32bits object data size in bits
};


/** Class to handle dim style symbol table entries
 *  @author Rallaz
 */
class DRW_Dimstyle : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_Dimstyle() { reset();}

    void reset()
    {
      tType = DRW::DIMSTYLE;
      dimasz = dimtxt = dimexe = 0.18;
      dimexo = 0.0625;
      dimgap = dimcen = 0.09;
      dimtxsty = "Standard";
      dimscale = dimlfac = dimtfac = dimfxl = 1.0;
      dimdli = 0.38;
      dimrnd = dimdle = dimtp = dimtm = dimtsz = dimtvp = 0.0;
      dimaltf = 25.4;
      dimtol = dimlim = dimse1 = dimse2 = dimtad = dimzin = 0;
      dimtoh = dimtolj = 1;
      dimalt = dimtofl = dimsah = dimtix = dimsoxd = dimfxlon = 0;
      dimaltd = dimunit = dimaltu = dimalttd = dimlunit = 2;
      dimclrd = dimclre = dimclrt = dimjust = dimupt = 0;
      dimazin = dimaltz = dimaltttz = dimtzin = dimfrac = 0;
      dimtih = dimadec = dimaunit = dimsd1 = dimsd2 = dimtmove = 0;
      dimaltrnd = 0.0;
      dimdec = dimtdec = 4;
      dimfit = dimatfit = 3;
      dimdsep = '.';
      dimlwd = dimlwe = -2;
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    //V12
    UTF8STRING dimpost;       //!< Code 3
    UTF8STRING dimapost;      //!< Code 4
    /* handle are code 105 */
    UTF8STRING dimblk;        //!< Code 5, code 342 V2000+
    UTF8STRING dimblk1;       //!< Code 6, code 343 V2000+
    UTF8STRING dimblk2;       //!< Code 7, code 344 V2000+
    double dimscale;          //!< Code 40
    double dimasz;            //!< Code 41
    double dimexo;            //!< Code 42
    double dimdli;            //!< Code 43
    double dimexe;            //!< Code 44
    double dimrnd;            //!< Code 45
    double dimdle;            //!< Code 46
    double dimtp;             //!< Code 47
    double dimtm;             //!< Code 48
    double dimfxl;            //!< Code 49 V2007+
    double dimtxt;            //!< Code 140
    double dimcen;            //!< Code 141
    double dimtsz;            //!< Code 142
    double dimaltf;           //!< Code 143
    double dimlfac;           //!< Code 144
    double dimtvp;            //!< Code 145
    double dimtfac;           //!< Code 146
    double dimgap;            //!< Code 147
    double dimaltrnd;         //!< Code 148 V2000+
    int dimtol;               //!< Code 71
    int dimlim;               //!< Code 72
    int dimtih;               //!< Code 73
    int dimtoh;               //!< Code 74
    int dimse1;               //!< Code 75
    int dimse2;               //!< Code 76
    int dimtad;               //!< Code 77
    int dimzin;               //!< Code 78
    int dimazin;              //!< Code 79 V2000+
    int dimalt;               //!< Code 170
    int dimaltd;              //!< Code 171
    int dimtofl;              //!< Code 172
    int dimsah;               //!< Code 173
    int dimtix;               //!< Code 174
    int dimsoxd;              //!< Code 175
    int dimclrd;              //!< Code 176
    int dimclre;              //!< Code 177
    int dimclrt;              //!< Code 178
    int dimadec;              //!< Code 179 V2000+
    int dimunit;              //!< Code 270 R13+ (obsolete 2000+, use dimlunit & dimfrac)
    int dimdec;               //!< Code 271 R13+
    int dimtdec;              //!< Code 272 R13+
    int dimaltu;              //!< Code 273 R13+
    int dimalttd;             //!< Code 274 R13+
    int dimaunit;             //!< Code 275 R13+
    int dimfrac;              //!< Code 276 V2000+
    int dimlunit;             //!< Code 277 V2000+
    int dimdsep;              //!< Code 278 V2000+
    int dimtmove;             //!< Code 279 V2000+
    int dimjust;              //!< Code 280 R13+
    int dimsd1;               //!< Code 281 R13+
    int dimsd2;               //!< Code 282 R13+
    int dimtolj;              //!< Code 283 R13+
    int dimtzin;              //!< Code 284 R13+
    int dimaltz;              //!< Code 285 R13+
    int dimaltttz;            //!< Code 286 R13+
    int dimfit;               //!< Code 287 R13+  (obsolete 2000+, use dimatfit & dimtmove)
    int dimupt;               //!< Code 288 R13+
    int dimatfit;             //!< Code 289 V2000+
    int dimfxlon;             //!< Code 290 V2007+
    UTF8STRING dimtxsty;      //!< Code 340 R13+
    UTF8STRING dimldrblk;     //!< Code 341 V2000+
    int dimlwd;               //!< Code 371 V2000+
    int dimlwe;               //!< Code 372 V2000+
};


/** Class to handle line type symbol table entries
 *  @author Rallaz
 * @todo handle complex lineType
 */
class DRW_LType : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_LType() { reset();}

    void reset()
    {
      tType = DRW::LTYPE;
      desc = "";
      size = 0;
      length = 0.0;
      pathIdx = 0;
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
    void update();

  public:
    UTF8STRING desc;                       //!< Descriptive string, code 3
//  int align;                             //!< Align code, always 65 ('A') code 72
    std::vector<double>::size_type size;   //!< Element number, code 73
    double length;                         //!< Total length of pattern, code 40
//  int haveShape;                         //!< Complex linetype type, code 74
    std::vector<double> path;              //!< Trace, point or space length sequence, code 49
  private:
    int pathIdx;
};


/** Class to handle layer symbol table entries
 *  @author Rallaz
 */
class DRW_Layer : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_Layer() { reset(); }

    void reset()
    {
      tType = DRW::LAYER;
      lineType = "CONTINUOUS";
      color = 7; // default BYLAYER (256)
      plotF = true; // default TRUE (plot yes)
      lWeight = DRW_LW_Conv::widthDefault; // default BYDEFAULT (dxf -3, dwg 31)
      color24 = -1; //default -1 not set
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    UTF8STRING lineType;                 //!< Line type, code 6
    int color;                           //!< Layer color, code 62
    int color24;                         //!< 24-bit color, code 420
    int transparency;                    //!< Transparency, code 440
    bool plotF;                          //!< Plot flag, code 290
    enum DRW_LW_Conv::lineWidth lWeight; //!< Layer lineweight, code 370
    std::string handlePlotS;             //!< Hard-pointer ID/handle of plotstyle, code 390
    std::string handleMaterialS;         //!< Hard-pointer ID/handle of materialstyle, code 347
    /*only used for read dwg*/
    dwgHandle lTypeH;
};

/** Class to handle block record table entries
 *  @author Rallaz
 */
class DRW_Block_Record : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_Block_Record() { reset();}
    void reset()
    {
      tType = DRW::BLOCK_RECORD;
      flags = 0;
      firstEH = lastEH = DRW::NoHandle;
      DRW_TableEntry::reset();
    }

  protected:
//  void parseCode(int code, dxfReader *reader);
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
//Note:    int DRW_TableEntry::flags; contains code 70 of block
    int insUnits;             //!< Block insertion units, code 70 of block_record
    DRW_Coord basePoint;      //!< Block insertion base point dwg only
  protected:
    //dwg parser
  private:
    duint32 block;            // handle for block entity
    duint32 endBlock;         // handle for end block entity
    duint32 firstEH;          // handle of first entity, only in pre-2004
    duint32 lastEH;           // handle of last entity, only in pre-2004
    std::vector<duint32>entMap;
};

/** Class to handle text style symbol table entries
 *  @author Rallaz
 */
class DRW_Textstyle : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_Textstyle() { reset();}

    void reset()
    {
      tType = DRW::STYLE;
      height = oblique = 0.0;
      width = lastHeight = 1.0;
      font = "txt";
      genFlag = 0; //2= X mirror, 4= Y mirror
      fontFamily = 0;
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    double height;          //!< Fixed text height (0 not set), code 40
    double width;           //!< Width factor, code 41
    double oblique;         //!< Oblique angle, code 50
    int genFlag;            //!< Text generation flags, code 71
    double lastHeight;      //!< Last height used, code 42
    UTF8STRING font;        //!< Primary font file name, code 3
    UTF8STRING bigFont;     //!< Bigfont file name or blank if none, code 4
    int fontFamily;         //!< Ttf font family, italic and bold flags, code 1071
};

/** Class to handle vport symbol table entries
 *  @author Rallaz
 */
class DRW_Vport : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_Vport() { reset();}

    void reset()
    {
      tType = DRW::VPORT;
      UpperRight.x = UpperRight.y = 1.0;
      snapSpacing.x = snapSpacing.y = 10.0;
      gridSpacing = snapSpacing;
      center.x = 0.651828;
      center.y = -0.16;
      viewDir.z = 1;
      height = 5.13732;
      ratio = 2.4426877;
      lensHeight = 50;
      frontClip = backClip = snapAngle = twistAngle = 0.0;
      viewMode = snap = grid = snapStyle = snapIsopair = 0;
      fastZoom = 1;
      circleZoom = 100;
      ucsIcon = 3;
      gridBehavior = 7;
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
    DRW_Coord lowerLeft;     //!< Lower left corner, code 10 & 20
    DRW_Coord UpperRight;    //!< Upper right corner, code 11 & 21
    DRW_Coord center;        //!< Center point in WCS, code 12 & 22
    DRW_Coord snapBase;      //!< Snap base point in DCS, code 13 & 23
    DRW_Coord snapSpacing;   //!< Snap Spacing, code 14 & 24
    DRW_Coord gridSpacing;   //!< Grid Spacing, code 15 & 25
    DRW_Coord viewDir;       //!< View direction from target point, code 16, 26 & 36
    DRW_Coord viewTarget;    //!< View target point, code 17, 27 & 37
    double height;           //!< View height, code 40
    double ratio;            //!< Viewport aspect ratio, code 41
    double lensHeight;       //!< Lens height, code 42
    double frontClip;        //!< Front clipping plane, code 43
    double backClip;         //!< Back clipping plane, code 44
    double snapAngle;        //!< Snap rotation angle, code 50
    double twistAngle;       //!< View twist angle, code 51
    int viewMode;            //!< View mode, code 71
    int circleZoom;          //!< Circle zoom percent, code 72
    int fastZoom;            //!< Fast zoom setting, code 73
    int ucsIcon;             //!< UCSICON setting, code 74
    int snap;                //!< Snap on/off, code 75
    int grid;                //!< Grid on/off, code 76
    int snapStyle;           //!< Snap style, code 77
    int snapIsopair;         //!< Snap isopair, code 78
    int gridBehavior;        //!< Grid behavior, code 60, undocumented

    /** Code 60, bit coded possible value are
     * bit 1 (1) show out of limits
     * bit 2 (2) adaptive grid
     * bit 3 (4) allow subdivision
     * bit 4 (8) follow dynamic SCP
     **/
};


/** Class to handle image definitions object entries
 *  @author Rallaz
 */
class DRW_ImageDef : public DRW_TableEntry  //
{
    SETOBJFRIENDS
  public:
    DRW_ImageDef()
    {
      reset();
    }

    void reset()
    {
      tType = DRW::IMAGEDEF;
      imgVersion = 0;
      DRW_TableEntry::reset();
    }

  protected:
    void parseCode( int code, dxfReader *reader );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );

  public:
//  std::string handle;       //!< Entity identifier, code 5
    UTF8STRING name;          //!< File name of image, code 1
    int imgVersion;           //!< Class version, code 90, 0=R14 version
    double u;                 //!< Image size in pixels U value, code 10
    double v;                 //!< Image size in pixels V value, code 20
    double up;                //!< Default size of one pixel U value, code 11
    double vp;                //!< Default size of one pixel V value, code 12 really is 21
    int loaded;               //!< Image is loaded flag, code 280, 0=unloaded, 1=loaded
    int resolution;           //!< Resolution units, code 281, 0=no, 2=centimeters, 5=inch

    std::map<std::string, std::string> reactors;
};

/** Class to handle AppId symbol table entries
 *  @author Rallaz
 */
class DRW_AppId : public DRW_TableEntry
{
    SETOBJFRIENDS
  public:
    DRW_AppId() { reset();}

    void reset()
    {
      tType = DRW::APPID;
      flags = 0;
      name = "";
    }

  protected:
    void parseCode( int code, dxfReader *reader ) {DRW_TableEntry::parseCode( code, reader );}
    bool parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs = 0 );
};

namespace DRW
{
  // Extended color palette:
  // The first entry is only for direct indexing starting with [1]
  // Color 1 is red (1,0,0)
  const unsigned char dxfColors[][3] =
  {
    {  0,  0,  0}, // unused
    {255,  0,  0}, // 1 red
    {255, 255,  0}, // 2 yellow
    {  0, 255,  0}, // 3 green
    {  0, 255, 255}, // 4 cyan
    {  0,  0, 255}, // 5 blue
    {255,  0, 255}, // 6 magenta
    {  0,  0,  0}, // 7 black or white
    {128, 128, 128}, // 8 50% gray
    {192, 192, 192}, // 9 75% gray
    {255,  0,  0}, // 10
    {255, 127, 127},
    {204,  0,  0},
    {204, 102, 102},
    {153,  0,  0},
    {153, 76, 76}, // 15
    {127,  0,  0},
    {127, 63, 63},
    { 76,  0,  0},
    { 76, 38, 38},
    {255, 63,  0}, // 20
    {255, 159, 127},
    {204, 51,  0},
    {204, 127, 102},
    {153, 38,  0},
    {153, 95, 76}, // 25
    {127, 31,  0},
    {127, 79, 63},
    { 76, 19,  0},
    { 76, 47, 38},
    {255, 127,  0}, // 30
    {255, 191, 127},
    {204, 102,  0},
    {204, 153, 102},
    {153, 76,  0},
    {153, 114, 76}, // 35
    {127, 63,  0},
    {127, 95, 63},
    { 76, 38,  0},
    { 76, 57, 38},
    {255, 191,  0}, // 40
    {255, 223, 127},
    {204, 153,  0},
    {204, 178, 102},
    {153, 114,  0},
    {153, 133, 76}, // 45
    {127, 95,  0},
    {127, 111, 63},
    { 76, 57,  0},
    { 76, 66, 38},
    {255, 255,  0}, // 50
    {255, 255, 127},
    {204, 204,  0},
    {204, 204, 102},
    {153, 153,  0},
    {153, 153, 76}, // 55
    {127, 127,  0},
    {127, 127, 63},
    { 76, 76,  0},
    { 76, 76, 38},
    {191, 255,  0}, // 60
    {223, 255, 127},
    {153, 204,  0},
    {178, 204, 102},
    {114, 153,  0},
    {133, 153, 76}, // 65
    { 95, 127,  0},
    {111, 127, 63},
    { 57, 76,  0},
    { 66, 76, 38},
    {127, 255,  0}, // 70
    {191, 255, 127},
    {102, 204,  0},
    {153, 204, 102},
    { 76, 153,  0},
    {114, 153, 76}, // 75
    { 63, 127,  0},
    { 95, 127, 63},
    { 38, 76,  0},
    { 57, 76, 38},
    { 63, 255,  0}, // 80
    {159, 255, 127},
    { 51, 204,  0},
    {127, 204, 102},
    { 38, 153,  0},
    { 95, 153, 76}, // 85
    { 31, 127,  0},
    { 79, 127, 63},
    { 19, 76,  0},
    { 47, 76, 38},
    {  0, 255,  0}, // 90
    {127, 255, 127},
    {  0, 204,  0},
    {102, 204, 102},
    {  0, 153,  0},
    { 76, 153, 76}, // 95
    {  0, 127,  0},
    { 63, 127, 63},
    {  0, 76,  0},
    { 38, 76, 38},
    {  0, 255, 63}, // 100
    {127, 255, 159},
    {  0, 204, 51},
    {102, 204, 127},
    {  0, 153, 38},
    { 76, 153, 95}, // 105
    {  0, 127, 31},
    { 63, 127, 79},
    {  0, 76, 19},
    { 38, 76, 47},
    {  0, 255, 127}, // 110
    {127, 255, 191},
    {  0, 204, 102},
    {102, 204, 153},
    {  0, 153, 76},
    { 76, 153, 114}, // 115
    {  0, 127, 63},
    { 63, 127, 95},
    {  0, 76, 38},
    { 38, 76, 57},
    {  0, 255, 191}, // 120
    {127, 255, 223},
    {  0, 204, 153},
    {102, 204, 178},
    {  0, 153, 114},
    { 76, 153, 133}, // 125
    {  0, 127, 95},
    { 63, 127, 111},
    {  0, 76, 57},
    { 38, 76, 66},
    {  0, 255, 255}, // 130
    {127, 255, 255},
    {  0, 204, 204},
    {102, 204, 204},
    {  0, 153, 153},
    { 76, 153, 153}, // 135
    {  0, 127, 127},
    { 63, 127, 127},
    {  0, 76, 76},
    { 38, 76, 76},
    {  0, 191, 255}, // 140
    {127, 223, 255},
    {  0, 153, 204},
    {102, 178, 204},
    {  0, 114, 153},
    { 76, 133, 153}, // 145
    {  0, 95, 127},
    { 63, 111, 127},
    {  0, 57, 76},
    { 38, 66, 76},
    {  0, 127, 255}, // 150
    {127, 191, 255},
    {  0, 102, 204},
    {102, 153, 204},
    {  0, 76, 153},
    { 76, 114, 153}, // 155
    {  0, 63, 127},
    { 63, 95, 127},
    {  0, 38, 76},
    { 38, 57, 76},
    {  0, 66, 255}, // 160
    {127, 159, 255},
    {  0, 51, 204},
    {102, 127, 204},
    {  0, 38, 153},
    { 76, 95, 153}, // 165
    {  0, 31, 127},
    { 63, 79, 127},
    {  0, 19, 76},
    { 38, 47, 76},
    {  0,  0, 255}, // 170
    {127, 127, 255},
    {  0,  0, 204},
    {102, 102, 204},
    {  0,  0, 153},
    { 76, 76, 153}, // 175
    {  0,  0, 127},
    { 63, 63, 127},
    {  0,  0, 76},
    { 38, 38, 76},
    { 63,  0, 255}, // 180
    {159, 127, 255},
    { 50,  0, 204},
    {127, 102, 204},
    { 38,  0, 153},
    { 95, 76, 153}, // 185
    { 31,  0, 127},
    { 79, 63, 127},
    { 19,  0, 76},
    { 47, 38, 76},
    {127,  0, 255}, // 190
    {191, 127, 255},
    {102,  0, 204},
    {153, 102, 204},
    { 76,  0, 153},
    {114, 76, 153}, // 195
    { 63,  0, 127},
    { 95, 63, 127},
    { 38,  0, 76},
    { 57, 38, 76},
    {191,  0, 255}, // 200
    {223, 127, 255},
    {153,  0, 204},
    {178, 102, 204},
    {114,  0, 153},
    {133, 76, 153}, // 205
    { 95,  0, 127},
    {111, 63, 127},
    { 57,  0, 76},
    { 66, 38, 76},
    {255,  0, 255}, // 210
    {255, 127, 255},
    {204,  0, 204},
    {204, 102, 204},
    {153,  0, 153},
    {153, 76, 153}, // 215
    {127,  0, 127},
    {127, 63, 127},
    { 76,  0, 76},
    { 76, 38, 76},
    {255,  0, 191}, // 220
    {255, 127, 223},
    {204,  0, 153},
    {204, 102, 178},
    {153,  0, 114},
    {153, 76, 133}, // 225
    {127,  0, 95},
    {127, 63, 11},
    { 76,  0, 57},
    { 76, 38, 66},
    {255,  0, 127}, // 230
    {255, 127, 191},
    {204,  0, 102},
    {204, 102, 153},
    {153,  0, 76},
    {153, 76, 114}, // 235
    {127,  0, 63},
    {127, 63, 95},
    { 76,  0, 38},
    { 76, 38, 57},
    {255,  0, 63}, // 240
    {255, 127, 159},
    {204,  0, 51},
    {204, 102, 127},
    {153,  0, 38},
    {153, 76, 95}, // 245
    {127,  0, 31},
    {127, 63, 79},
    { 76,  0, 19},
    { 76, 38, 47},
    { 51, 51, 51}, // 250
    { 91, 91, 91},
    {132, 132, 132},
    {173, 173, 173},
    {214, 214, 214},
    {255, 255, 255}  // 255
  };

}

#endif

// EOF
