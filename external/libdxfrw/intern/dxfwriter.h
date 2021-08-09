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

#ifndef DXFWRITER_H
#define DXFWRITER_H

#include "drw_textcodec.h"

class dxfWriter {
public:
    dxfWriter(std::ofstream *stream){filestr = stream; /*count =0;*/}
    virtual ~dxfWriter(){}
    virtual bool writeString(int code, std::string text) = 0;
    bool writeUtf8String(int code, std::string text);
    bool writeUtf8Caps(int code, std::string text);
    std::string fromUtf8String(std::string t) {return encoder.fromUtf8(t);}
    virtual bool writeInt16(int code, int data) = 0;
    virtual bool writeInt32(int code, int data) = 0;
    virtual bool writeInt64(int code, unsigned long long int data) = 0;
    virtual bool writeDouble(int code, double data) = 0;
    virtual bool writeBool(int code, bool data) = 0;
    void setVersion(std::string *v, bool dxfFormat){encoder.setVersion(v, dxfFormat);}
    void setCodePage(std::string *c){encoder.setCodePage(c, true);}
    std::string getCodePage(){return encoder.getCodePage();}
protected:
    std::ofstream *filestr;
private:
    DRW_TextCodec encoder;
};

class dxfWriterBinary : public dxfWriter {
public:
    dxfWriterBinary(std::ofstream *stream):dxfWriter(stream){}
    virtual ~dxfWriterBinary() {}
    virtual bool writeString(int code, std::string text);
    virtual bool writeInt16(int code, int data);
    virtual bool writeInt32(int code, int data);
    virtual bool writeInt64(int code, unsigned long long int data);
    virtual bool writeDouble(int code, double data);
    virtual bool writeBool(int code, bool data);
};

class dxfWriterAscii : public dxfWriter {
public:
    dxfWriterAscii(std::ofstream *stream);
    virtual ~dxfWriterAscii(){}
    virtual bool writeString(int code, std::string text);
    virtual bool writeInt16(int code, int data);
    virtual bool writeInt32(int code, int data);
    virtual bool writeInt64(int code, unsigned long long int data);
    virtual bool writeDouble(int code, double data);
    virtual bool writeBool(int code, bool data);
};

#endif // DXFWRITER_H
