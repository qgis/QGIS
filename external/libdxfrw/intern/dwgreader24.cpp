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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "drw_dbg.h"
#include "dwgreader24.h"
#include "drw_textcodec.h"
#include "../libdwgr.h"


bool dwgReader24::readFileHeader() {
    DRW_DBG("dwgReader24::readFileHeader\n");
    bool ret = dwgReader18::readFileHeader();
    DRW_DBG("dwgReader24::readFileHeader END\n");
    return ret;
}

bool dwgReader24::readDwgHeader(DRW_Header& hdr){
    DRW_DBG("dwgReader24::readDwgHeader\n");
    bool ret = dwgReader18::readDwgHeader(hdr);
    DRW_DBG("dwgReader24::readDwgHeader END\n");
    return ret;
}

bool dwgReader24::readDwgClasses(){
    DRW_DBG("\ndwgReader24::readDwgClasses");
    bool ret = dwgReader18::readDwgClasses();
    DRW_DBG("\ndwgReader24::readDwgClasses END\n");
    return ret;
}
