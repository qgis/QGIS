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

#ifndef DRW_CLASSES_H
#define DRW_CLASSES_H


#include "drw_base.h"
//#include "libdwgr.h"

class dxfReader;
class dxfWriter;
class dwgBuffer;

/** Class to handle classes table entries
 *  TODO: verify the dxf read/write part
 *  @author Rallaz
 */
class DRW_Class
{
  public:
    DRW_Class()
      : proxyFlag( 0 )
      , instanceCount( 0 )
      , wasaProxyFlag( 0 )
      , entityFlag( 0 )
      , classNum( 0 )
      , dwgType( 0 )
    {
    }
    ~DRW_Class()
    {
    }

    void parseCode( int code, dxfReader *reader );
    void write( dxfWriter *writer, DRW::Version ver );
    bool parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf );

  private:
    void toDwgType();
  public:
    UTF8STRING recName;      //!< Record name, code 1
    UTF8STRING className;    //!< C++ class name, code 2
    UTF8STRING appName;      //!< App name, code 3
    int proxyFlag;           //!< Proxy capabilities flag, code 90
    int instanceCount;       //!< Number of instances for a custom class, code 91
    int wasaProxyFlag;       //!< Proxy flag (app loaded on save), code 280
    int entityFlag;          //!< Entity flag, code 281 (0 object, 1 entity)
  public: //only for read dwg
    duint16 classNum;
    int dwgType;
};

#endif

// EOF
