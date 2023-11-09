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

#ifndef DRW_BASE_H
#define DRW_BASE_H

#define DRW_VERSION "0.6.3"

#include <string>
#include <list>
#include <cmath>

#ifdef DRW_ASSERTS
# define drw_assert(a) assert(a)
#else
# define drw_assert(a)
#endif

#define UTF8STRING std::string
#define DRW_UNUSED(x) (void)x

#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define DRW_WIN
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#    define DRW_WIN
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define DRW_WIN
#else
#  define DRW_POSIX
#endif

#define M_PIx2 6.283185307179586 // 2*PI
#define ARAD 57.29577951308232

typedef signed char dint8;              /* 8 bit signed */
typedef signed short dint16;            /* 16 bit signed */
typedef signed int dint32;              /* 32 bit signed */
typedef long long int dint64;           /* 64 bit signed */

typedef unsigned char duint8;           /* 8 bit unsigned */
typedef unsigned short duint16;         /* 16 bit unsigned */
typedef unsigned int duint32;           /* 32 bit unsigned */
typedef unsigned long long int duint64; /* 64 bit unsigned */

typedef float dfloat32;                 /* 32 bit floating point */
typedef double ddouble64;               /* 64 bit floating point */
typedef long double ddouble80;          /* 80 bit floating point */


namespace DRW
{

//! Version numbers for the DXF Format.
  enum Version
  {
    UNKNOWNV,     //!< UNKNOWN VERSION.
    MC00,       //!< DWG Release 1.1
    AC12,         //!< DWG Release 1.2
    AC14,         //!< DWG Release 1.4
    AC150,        //!< DWG Release 2.0
    AC210,        //!< DWG Release 2.10
    AC1002,       //!< DWG Release 2.5
    AC1003,       //!< DWG Release 2.6
    AC1004,       //!< DWG Relase 9
    AC1006,       //!< DWG Release 10
    AC1009,       //!< DWG Release 11/12 (LT R1/R2)
    AC1012,       //!< DWG Release 13 (LT95)
    AC1014,       //!< DWG Release 14/14.01 (LT97/LT98)
    AC1015,       //!< AutoCAD 2000/2000i/2002
    AC1018,       //!< AutoCAD 2004/2005/2006
    AC1021,       //!< AutoCAD 2007/2008/2009
    AC1024,       //!< AutoCAD 2010/2011/2012
    AC1027,       //!< AutoCAD 2013/2014/2015/2016/2017
    AC1032,       //!< AutoCAD 2018/2019/2020
  };

  enum error
  {
    BAD_NONE,             //!< No error.
    BAD_UNKNOWN,          //!< UNKNOWN.
    BAD_OPEN,             //!< Error opening file.
    BAD_VERSION,          //!< Unsupported version.
    BAD_READ_METADATA,    //!< Error reading matadata.
    BAD_READ_FILE_HEADER, //!< Error in file header read process.
    BAD_READ_HEADER,      //!< Error in header vars read process.
    BAD_READ_HANDLES,     //!< Error in object map read process.
    BAD_READ_CLASSES,     //!< Error in classes read process.
    BAD_READ_TABLES,      //!< Error in tables read process.
    BAD_READ_BLOCKS,      //!< Error in block read process.
    BAD_READ_ENTITIES,    //!< Error in entities read process.
    BAD_READ_OBJECTS      //!< Error in objects read process.
  };

  enum class DebugLevel {
      None,
      Debug
  };

  /**
   * Interface for debug printers.
   *
   * The base class is silent and ignores all debugging.
   */
  class DebugPrinter {
  public:
      virtual void printS(const std::string &s, const char *file, const char *function, int line){(void)s;(void)file;(void)function;(void)line;}
      virtual void printI(long long int i, const char *file, const char *function, int line){(void)i;(void)file;(void)function;(void)line;}
      virtual void printUI(long long unsigned int i, const char *file, const char *function, int line){(void)i;(void)file;(void)function;(void)line;}
      virtual void printD(double d, const char *file, const char *function, int line){(void)d;(void)file;(void)function;(void)line;}
      virtual void printH(long long int i, const char *file, const char *function, int line){(void)i;(void)file;(void)function;(void)line;}
      virtual void printB(int i, const char *file, const char *function, int line){(void)i;(void)file;(void)function;(void)line;}
      virtual void printHL(int c, int s, int h, const char *file, const char *function, int line){(void)c;(void)s;(void)h;(void)file;(void)function;(void)line;}
      virtual void printPT(double x, double y, double z, const char *file, const char *function, int line){(void)x;(void)y;(void)z;(void)file;(void)function;(void)line;}
      DebugPrinter()=default;
      virtual ~DebugPrinter();
  };

  /**
   * Sets a custom debug printer to use when outputting debug messages.
   *
   * Ownership of `printer` is transferred.
   */
  void setCustomDebugPrinter( DebugPrinter* printer );

  //! Special codes for colors
  enum ColorCodes
  {
    ColorByLayer = 256,
    ColorByBlock = 0
  };

  //! Spaces
  enum Space
  {
    ModelSpace = 0,
    PaperSpace = 1
  };

  //! Special kinds of handles
  enum HandleCodes
  {
    NoHandle = 0
  };

  //! Shadow mode
  enum ShadowMode
  {
    CastAndReceiveShadows = 0,
    CastShadows = 1,
    ReceiveShadows = 2,
    IgnoreShadows = 3
  };

  //! Special kinds of materials
  enum MaterialCodes
  {
    MaterialByLayer = 0
  };

  //! Special kinds of plot styles
  enum PlotStyleCodes
  {
    DefaultPlotStyle = 0
  };

  //! Special kinds of transparencies
  enum TransparencyCodes
  {
    Opaque = 0,
    Transparent = -1
  };

} // namespace DRW

//! Class to handle 3D coordinate point
/*!
*  Class to handle 3D coordinate point
*  @author Rallaz
*/
class DRW_Coord
{
  public:
    DRW_Coord(): x( 0 ), y( 0 ), z( 0 ) {}
    DRW_Coord( double ix, double iy, double iz ): x( ix ), y( iy ), z( iz ) {}

    //! < convert to unitary vector
    void unitize()
    {
      double dist;
      dist = std::sqrt( x * x + y * y + z * z );
      if ( dist > 0.0 )
      {
        x = x / dist;
        y = y / dist;
        z = z / dist;
      }
    }

  public:
    double x;
    double y;
    double z;
};


//! Class to handle vertex
/*!
*  Class to handle vertex for lwpolyline entity
*  @author Rallaz
*/
class DRW_Vertex2D
{
  public:
    DRW_Vertex2D(): x( 0 ), y( 0 ), stawidth( 0 ), endwidth( 0 ), bulge( 0 ) {}

    DRW_Vertex2D( double sx, double sy, double b ): x( sx ), y( sy ), stawidth( 0 ), endwidth( 0 ), bulge( b ) {}

  public:
    double x;         //!< X coordinate, code 10
    double y;         //!< Y coordinate, code 20
    double stawidth;  //!< Start width, code 40
    double endwidth;  //!< End width, code 41
    double bulge;     //!< Bulge, code 42
};


//! Class to handle header vars
/*!
*  Class to handle header vars
*  @author Rallaz
*/
class DRW_Variant
{
  public:
    enum TYPE
    {
      STRING,
      INTEGER,
      DOUBLE,
      COORD,
      INVALID
    };
//TODO: add INT64 support
    DRW_Variant(): sdata( std::string() ), vdata(), content( 0 ), vType( INVALID ), vCode( 0 ) {}

    DRW_Variant( int c, dint32 i ): sdata( std::string() ), vdata(), content( i ), vType( INTEGER ), vCode( c ) {}

    DRW_Variant( int c, duint32 i ): sdata( std::string() ), vdata(), content( static_cast<dint32>( i ) ), vType( INTEGER ), vCode( c ) {}

    DRW_Variant( int c, double d ): sdata( std::string() ), vdata(), content( d ), vType( DOUBLE ), vCode( c ) {}

    DRW_Variant( int c, UTF8STRING s ): sdata( s ), vdata(), content( &sdata ), vType( STRING ), vCode( c ) {}

    DRW_Variant( int c, DRW_Coord crd ): sdata( std::string() ), vdata( crd ), content( &vdata ), vType( COORD ), vCode( c ) {}

    DRW_Variant( const DRW_Variant &d ): sdata( d.sdata ), vdata( d.vdata ), content( d.content ), vType( d.vType ), vCode( d.vCode )
    {
      if ( d.vType == COORD )
        content.v = &vdata;
      if ( d.vType == STRING )
        content.s = &sdata;
    }

    ~DRW_Variant()
    {
    }

    void addString( int c, UTF8STRING s ) {vType = STRING; sdata = s; content.s = &sdata; vCode = c;}
    void addInt( int c, int i ) {vType = INTEGER; content.i = i; vCode = c;}
    void addDouble( int c, double d ) {vType = DOUBLE; content.d = d; vCode = c;}
    void addCoord( int c, DRW_Coord v ) {vType = COORD; vdata = v; content.v = &vdata; vCode = c;}
    void setCoordX( double d ) { if ( vType == COORD ) vdata.x = d;}
    void setCoordY( double d ) { if ( vType == COORD ) vdata.y = d;}
    void setCoordZ( double d ) { if ( vType == COORD ) vdata.z = d;}
    enum TYPE type() { return vType;}
    int code() { return vCode;}            //!< Returns dxf code of this value

  private:
    std::string sdata;
    DRW_Coord vdata;

  private:
    union DRW_VarContent
    {
      UTF8STRING *s; //skip-init-check
      dint32 i;
      double d;
      DRW_Coord *v; //skip-init-check

      DRW_VarContent( UTF8STRING *sd ): s( sd ) {}
      DRW_VarContent( dint32 id ): i( id ) {}
      DRW_VarContent( double dd ): d( dd ) {}
      DRW_VarContent( DRW_Coord *vd ): v( vd ) {}
    };

  public:
    DRW_VarContent content;
  private:
    enum TYPE vType;
    int vCode;            //!< Dxf code of this value

};

//! Class to handle dwg handles
/*!
*  Class to handle dwg handles
*  @author Rallaz
*/
class dwgHandle
{
  public:
    dwgHandle(): code( 0 ), size( 0 ), ref( 0 ) {}

    ~dwgHandle() {}
    duint8 code;
    duint8 size;
    duint32 ref;
};

//! Class to convert between line width and integer
/*!
*  Class to convert between line width and integer
*  verifing valid values, if value is not valid
*  returns widthDefault.
*  @author Rallaz
*/
class DRW_LW_Conv
{
  public:
    enum lineWidth
    {
      width00 = 0,       //!< 0.00mm (dxf 0)
      width01 = 1,       //!< 0.05mm (dxf 5)
      width02 = 2,       //!< 0.09mm (dxf 9)
      width03 = 3,       //!< 0.13mm (dxf 13)
      width04 = 4,       //!< 0.15mm (dxf 15)
      width05 = 5,       //!< 0.18mm (dxf 18)
      width06 = 6,       //!< 0.20mm (dxf 20)
      width07 = 7,       //!< 0.25mm (dxf 25)
      width08 = 8,       //!< 0.30mm (dxf 30)
      width09 = 9,       //!< 0.35mm (dxf 35)
      width10 = 10,      //!< 0.40mm (dxf 40)
      width11 = 11,      //!< 0.50mm (dxf 50)
      width12 = 12,      //!< 0.53mm (dxf 53)
      width13 = 13,      //!< 0.60mm (dxf 60)
      width14 = 14,      //!< 0.70mm (dxf 70)
      width15 = 15,      //!< 0.80mm (dxf 80)
      width16 = 16,      //!< 0.90mm (dxf 90)
      width17 = 17,      //!< 1.00mm (dxf 100)
      width18 = 18,      //!< 1.06mm (dxf 106)
      width19 = 19,      //!< 1.20mm (dxf 120)
      width20 = 20,      //!< 1.40mm (dxf 140)
      width21 = 21,      //!< 1.58mm (dxf 158)
      width22 = 22,      //!< 2.00mm (dxf 200)
      width23 = 23,      //!< 2.11mm (dxf 211)
      widthByLayer = 29, //!< By layer (dxf -1)
      widthByBlock = 30, //!< By block (dxf -2)
      widthDefault = 31  //!< By default (dxf -3)
    };

    static int lineWidth2dxfInt( enum lineWidth lw )
    {
      switch ( lw )
      {
        case widthByLayer:
          return -1;
        case widthByBlock:
          return -2;
        case widthDefault:
          return -3;
        case width00:
          return 0;
        case width01:
          return 5;
        case width02:
          return 9;
        case width03:
          return 13;
        case width04:
          return 15;
        case width05:
          return 18;
        case width06:
          return 20;
        case width07:
          return 25;
        case width08:
          return 30;
        case width09:
          return 35;
        case width10:
          return 40;
        case width11:
          return 50;
        case width12:
          return 53;
        case width13:
          return 60;
        case width14:
          return 70;
        case width15:
          return 80;
        case width16:
          return 90;
        case width17:
          return 100;
        case width18:
          return 106;
        case width19:
          return 120;
        case width20:
          return 140;
        case width21:
          return 158;
        case width22:
          return 200;
        case width23:
          return 211;
        default:
          break;
      }
      return -3;
    }

    static int lineWidth2dwgInt( enum lineWidth lw )
    {
      return static_cast<int>( lw );
    }

    static enum lineWidth dxfInt2lineWidth( int i )
    {
      if ( i < 0 )
      {
        if ( i == -1 )
          return widthByLayer;
        else if ( i == -2 )
          return widthByBlock;
        else if ( i == -3 )
          return widthDefault;
      }
      else if ( i < 3 )
      {
        return width00;
      }
      else if ( i < 7 )
      {
        return width01;
      }
      else if ( i < 11 )
      {
        return width02;
      }
      else if ( i < 14 )
      {
        return width03;
      }
      else if ( i < 16 )
      {
        return width04;
      }
      else if ( i < 19 )
      {
        return width05;
      }
      else if ( i < 22 )
      {
        return width06;
      }
      else if ( i < 27 )
      {
        return width07;
      }
      else if ( i < 32 )
      {
        return width08;
      }
      else if ( i < 37 )
      {
        return width09;
      }
      else if ( i < 45 )
      {
        return width10;
      }
      else if ( i < 52 )
      {
        return width11;
      }
      else if ( i < 57 )
      {
        return width12;
      }
      else if ( i < 65 )
      {
        return width13;
      }
      else if ( i < 75 )
      {
        return width14;
      }
      else if ( i < 85 )
      {
        return width15;
      }
      else if ( i < 95 )
      {
        return width16;
      }
      else if ( i < 103 )
      {
        return width17;
      }
      else if ( i < 112 )
      {
        return width18;
      }
      else if ( i < 130 )
      {
        return width19;
      }
      else if ( i < 149 )
      {
        return width20;
      }
      else if ( i < 180 )
      {
        return width21;
      }
      else if ( i < 205 )
      {
        return width22;
      }
      else
      {
        return width23;
      }
      //default by default
      return widthDefault;
    }

    static enum lineWidth dwgInt2lineWidth( int i )
    {
      if ( ( i > -1 && i < 24 ) || ( i > 28 && i < 32 ) )
      {
        return static_cast<lineWidth>( i );
      }
      //default by default
      return widthDefault;
    }
};

#endif

// EOF
