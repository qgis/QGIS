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

#ifndef LIBDWGR_H
#define LIBDWGR_H

#include <string>
#include <memory>
#include <unordered_map>
//#include <deque>
#include "drw_entities.h"
#include "drw_objects.h"
#include "drw_classes.h"
#include "drw_interface.h"

class dwgReader;

class dwgR {
public:
    dwgR(const char* name);
    ~dwgR();
    //read: return true if all ok
    bool read(DRW_Interface *interface_, bool ext);
    bool getPreview();
    DRW::Version getVersion(){return version;}
    DRW::error getError(){return error;}
bool testReader();
    void setDebug(DRW::DebugLevel lvl);

private:
    bool openFile(std::ifstream *filestr);
    bool processDwg();
    static std::unique_ptr< dwgReader > createReaderForVersion(DRW::Version version, std::ifstream *stream, dwgR *p);

private:
    DRW::Version version { DRW::UNKNOWNV };
    DRW::error error { DRW::BAD_NONE };
    std::string fileName;
    bool applyExt { false }; /*apply extrusion in entities to conv in 2D?*/
    std::string codePage;
    DRW_Interface *iface { nullptr };
    std::unique_ptr< dwgReader > reader;

    static std::unordered_map< const char*, DRW::Version > DRW_dwgVersionStrings;

};

#endif // LIBDWGR_H
