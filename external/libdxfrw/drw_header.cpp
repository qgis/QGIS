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

#include "drw_header.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include "intern/drw_dbg.h"
#include "intern/dwgbuffer.h"

DRW_Header::DRW_Header() {
    linetypeCtrl = layerCtrl = styleCtrl = dimstyleCtrl = appidCtrl = 0;
    blockCtrl = viewCtrl = ucsCtrl = vportCtrl = vpEntHeaderCtrl = 0;
    version = DRW::AC1021;
}

void DRW_Header::addComment(std::string c){
    if (!comments.empty())
        comments += '\n';
    comments += c;
}

void DRW_Header::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 9:
        curr = new DRW_Variant();
        name = reader->getString();
        if (version < DRW::AC1015 && name == "$DIMUNIT")
            name="$DIMLUNIT";
        vars[name]=curr;
        break;
    case 1:
        curr->addString(code, reader->getUtf8String());
        if (name =="$ACADVER") {
            reader->setVersion(*curr->content.s, true);
            version = reader->getVersion();
        }
        break;
    case 2:
        curr->addString(code, reader->getUtf8String());
        break;
    case 3:
        curr->addString(code, reader->getUtf8String());
        if (name =="$DWGCODEPAGE") {
            reader->setCodePage(*curr->content.s);
            curr->addString(code, reader->getCodePage());
        }
        break;
    case 6:
        curr->addString(code, reader->getUtf8String());
        break;
    case 7:
        curr->addString(code, reader->getUtf8String());
        break;
    case 8:
        curr->addString(code, reader->getUtf8String());
        break;
    case 10:
        curr->addCoord(code, DRW_Coord(reader->getDouble(), 0.0, 0.0));
        break;
    case 20:
        curr->setCoordY(reader->getDouble());
        break;
    case 30:
        curr->setCoordZ(reader->getDouble());
        break;
    case 40:
        curr->addDouble(code, reader->getDouble());
        break;
    case 50:
        curr->addDouble(code, reader->getDouble());
        break;
    case 62:
        curr->addInt(code, reader->getInt32());
        break;
    case 70:
        curr->addInt(code, reader->getInt32());
        break;
    case 280:
        curr->addInt(code, reader->getInt32());
        break;
    case 290:
        curr->addInt(code, reader->getInt32());
        break;
    case 370:
        curr->addInt(code, reader->getInt32());
        break;
    case 380:
        curr->addInt(code, reader->getInt32());
        break;
    case 390:
        curr->addString(code, reader->getUtf8String());
        break;
    default:
        break;
    }
}

void DRW_Header::write(dxfWriter *writer, DRW::Version ver){
/*RLZ: TODO complete all vars to AC1024*/
    double varDouble;
    int varInt;
    std::string varStr;
    DRW_Coord varCoord;
    writer->writeString(2, "HEADER");
    writer->writeString(9, "$ACADVER");
    switch (ver) {
    case DRW::AC1006: //unsupported version acad 10
    case DRW::AC1009: //acad 11 & 12
        varStr = "AC1009";
        break;
    case DRW::AC1012: //unsupported version acad 13
    case DRW::AC1014: //acad 14
        varStr = "AC1014";
        break;
    case DRW::AC1015: //acad 2000
        varStr = "AC1015";
        break;
    case DRW::AC1018: //acad 2004
        varStr = "AC1018";
        break;
/*    case DRW::AC1021: //acad 2007
        varStr = "AC1021";
        break;*/
    case DRW::AC1024: //acad 2010
        varStr = "AC1024";
        break;
    case DRW::AC1027: //acad 2013
        varStr = "AC1027";
        break;
    default: //acad 2007 default version
        varStr = "AC1021";
        break;
    }
    writer->writeString(1, varStr);
    writer->setVersion(varStr, true);

    getStr("$ACADVER", &varStr);
    getStr("$ACADMAINTVER", &varStr);

    if (!getStr("$DWGCODEPAGE", &varStr)) {
        varStr = "ANSI_1252";
    }
    writer->writeString(9, "$DWGCODEPAGE");
    writer->setCodePage(varStr);
    writer->writeString(3, writer->getCodePage() );
    writer->writeString(9, "$INSBASE");
    if (getCoord("$INSBASE", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$EXTMIN");
    if (getCoord("$EXTMIN", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 1.0000000000000000E+020);
        writer->writeDouble(20, 1.0000000000000000E+020);
        writer->writeDouble(30, 1.0000000000000000E+020);
    }
    writer->writeString(9, "$EXTMAX");
    if (getCoord("$EXTMAX", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, -1.0000000000000000E+020);
        writer->writeDouble(20, -1.0000000000000000E+020);
        writer->writeDouble(30, -1.0000000000000000E+020);
    }
    writer->writeString(9, "$LIMMIN");
    if (getCoord("$LIMMIN", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
    }
    writer->writeString(9, "$LIMMAX");
    if (getCoord("$LIMMAX", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    } else {
        writer->writeDouble(10, 420.0);
        writer->writeDouble(20, 297.0);
    }
    writer->writeString(9, "$ORTHOMODE");
    if (getInt("$ORTHOMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$REGENMODE");
    if (getInt("$REGENMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$FILLMODE");
    if (getInt("$FILLMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$QTEXTMODE");
    if (getInt("$QTEXTMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$MIRRTEXT");
    if (getInt("$MIRRTEXT", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    if (ver == DRW::AC1009){
        writer->writeString(9, "$DRAGMODE");
        if (getInt("$DRAGMODE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 2);
    }
    writer->writeString(9, "$LTSCALE");
    if (getDouble("$LTSCALE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 1.0);
    if (ver == DRW::AC1009){
        writer->writeString(9, "$OSMODE");
        if (getInt("$OSMODE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
    }
    writer->writeString(9, "$ATTMODE");
    if (getInt("$ATTMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$TEXTSIZE");
    if (getDouble("$TEXTSIZE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 2.5);
    writer->writeString(9, "$TRACEWID");
    if (getDouble("$TRACEWID", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 15.68);
    writer->writeString(9, "$TEXTSTYLE");
    if (getStr("$TEXTSTYLE", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(7, varStr);
        else
            writer->writeUtf8String(7, varStr);
    else
        writer->writeString(7, "STANDARD");
    writer->writeString(9, "$CLAYER");
    if (getStr("$CLAYER", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(8, varStr);
        else
            writer->writeUtf8String(8, varStr);
    else
        writer->writeString(8, "0");
    writer->writeString(9, "$CELTYPE");
    if (getStr("$CELTYPE", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(6, varStr);
        else
            writer->writeUtf8String(6, varStr);
    else
        writer->writeString(6, "BYLAYER");
    writer->writeString(9, "$CECOLOR");
    if (getInt("$CECOLOR", &varInt))
        writer->writeInt16(62, varInt);
    else
        writer->writeInt16(62, 256);
    if (ver > DRW::AC1009){
        writer->writeString(9, "$CELTSCALE");
        if (getDouble("$CELTSCALE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 1.0);
        writer->writeString(9, "$DISPSILH");
        if (getInt("$DISPSILH", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
    }

    writer->writeString(9, "$DIMSCALE");
    if (getDouble("$DIMSCALE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 2.5);
    writer->writeString(9, "$DIMASZ");
    if (getDouble("$DIMASZ", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 2.5);
    writer->writeString(9, "$DIMEXO");
    if (getDouble("$DIMEXO", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.625);
    writer->writeString(9, "$DIMDLI");
    if (getDouble("$DIMDLI", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 3.75);
    writer->writeString(9, "$DIMRND");
    if (getDouble("$DIMRND", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMDLE");
    if (getDouble("$DIMDLE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMEXE");
    if (getDouble("$DIMEXE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 1.25);
    writer->writeString(9, "$DIMTP");
    if (getDouble("$DIMTP", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMTM");
    if (getDouble("$DIMTM", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMTXT");
    if (getDouble("$DIMTXT", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 2.5);
    writer->writeString(9, "$DIMCEN");
    if (getDouble("$DIMCEN", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 2.5);
    writer->writeString(9, "$DIMTSZ");
    if (getDouble("$DIMTSZ", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMTOL");
    if (getInt("$DIMTOL", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMLIM");
    if (getInt("$DIMLIM", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMTIH");
    if (getInt("$DIMTIH", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMTOH");
    if (getInt("$DIMTOH", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMSE1");
    if (getInt("$DIMSE1", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMSE2");
    if (getInt("$DIMSE2", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMTAD");
    if (getInt("$DIMTAD", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$DIMZIN");
    if (getInt("$DIMZIN", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 8);
    writer->writeString(9, "$DIMBLK");
    if (getStr("$DIMBLK", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, "");
    writer->writeString(9, "$DIMASO");
    if (getInt("$DIMASO", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$DIMSHO");
    if (getInt("$DIMSHO", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$DIMPOST");
    if (getStr("$DIMPOST", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, "");
    writer->writeString(9, "$DIMAPOST");
    if (getStr("$DIMAPOST", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, "");
    writer->writeString(9, "$DIMALT");
    if (getInt("$DIMALT", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMALTD");
    if (getInt("$DIMALTD", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 3);
    writer->writeString(9, "$DIMALTF");
    if (getDouble("$DIMALTF", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.03937);
    writer->writeString(9, "$DIMLFAC");
    if (getDouble("$DIMLFAC", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 1.0);
    writer->writeString(9, "$DIMTOFL");
    if (getInt("$DIMTOFL", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$DIMTVP");
    if (getDouble("$DIMTVP", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$DIMTIX");
    if (getInt("$DIMTIX", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMSOXD");
    if (getInt("$DIMSOXD", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMSAH");
    if (getInt("$DIMSAH", &varInt))
    writer->writeInt16(70, varInt);
    else
    writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMBLK1");
    if (getStr("$DIMBLK1", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, "");
    writer->writeString(9, "$DIMBLK2");
    if (getStr("$DIMBLK2", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, "");
    writer->writeString(9, "$DIMSTYLE");
    if (getStr("$DIMSTYLE", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(2, varStr);
        else
            writer->writeUtf8String(2, varStr);
    else
        writer->writeString(2, "STANDARD");
    writer->writeString(9, "$DIMCLRD");
    if (getInt("$DIMCLRD", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMCLRE");
    if (getInt("$DIMCLRE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMCLRT");
    if (getInt("$DIMCLRT", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$DIMTFAC");
    if (getDouble("$DIMTFAC", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 1.0);
    writer->writeString(9, "$DIMGAP");
    if (getDouble("$DIMGAP", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.625);
    //post r12 dim vars
    if (ver > DRW::AC1009) {
        writer->writeString(9, "$DIMJUST");
        if (getInt("$DIMJUST", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMSD1");
        if (getInt("$DIMSD1", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMSD2");
        if (getInt("$DIMSD2", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMTOLJ");
        if (getInt("$DIMTOLJ", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMTZIN");
        if (getInt("$DIMTZIN", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 8);
        writer->writeString(9, "$DIMALTZ");
        if (getInt("$DIMALTZ", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMALTTZ");
        if (getInt("$DIMALTTZ", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMUPT");
        if (getInt("$DIMUPT", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMDEC");
        if (getInt("$DIMDEC", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 2);
        writer->writeString(9, "$DIMTDEC");
        if (getInt("$DIMTDEC", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 2);
        writer->writeString(9, "$DIMALTU");
        if (getInt("$DIMALTU", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 2);
        writer->writeString(9, "$DIMALTTD");
        if (getInt("$DIMALTTD", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 3);
        writer->writeString(9, "$DIMTXSTY");
        if (getStr("$DIMTXSTY", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(7, varStr);
            else
                writer->writeUtf8String(7, varStr);
        else
            writer->writeString(7, "STANDARD");
        writer->writeString(9, "$DIMAUNIT");
        if (getInt("$DIMAUNIT", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMADEC");
        if (getInt("$DIMADEC", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMALTRND");
        if (getDouble("$DIMALTRND", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$DIMAZIN");
        if (getInt("$DIMAZIN", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMDSEP");
        if (getInt("$DIMDSEP", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 44);
        writer->writeString(9, "$DIMATFIT");
        if (getInt("$DIMATFIT", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 3);
        writer->writeString(9, "$DIMFRAC");
        if (getInt("$DIMFRAC", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$DIMLDRBLK");
        if (getStr("$DIMLDRBLK", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(1, varStr);
            else
                writer->writeUtf8String(1, varStr);
        else
            writer->writeString(1, "STANDARD");
    //verify if exist "$DIMLUNIT" or obsolete "$DIMUNIT" (pre v2000)
        if ( !getInt("$DIMLUNIT", &varInt) ){
            if (!getInt("$DIMUNIT", &varInt))
                varInt = 2;
        }
        //verify valid values from 1 to 6
        if (varInt<1 || varInt>6)
            varInt = 2;
        if (ver > DRW::AC1014) {
            writer->writeString(9, "$DIMLUNIT");
            writer->writeInt16(70, varInt);
        } else {
            writer->writeString(9, "$DIMUNIT");
            writer->writeInt16(70, varInt);
        }
        writer->writeString(9, "$DIMLWD");
        if (getInt("$DIMLWD", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, -2);
        writer->writeString(9, "$DIMLWE");
        if (getInt("$DIMLWE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, -2);
        writer->writeString(9, "$DIMTMOVE");
        if (getInt("$DIMTMOVE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);

        if (ver > DRW::AC1018) {// and post v2004 dim vars
            writer->writeString(9, "$DIMFXL");
            if (getDouble("$DIMFXL", &varDouble))
                writer->writeDouble(40, varDouble);
            else
                writer->writeDouble(40, 1.0);
            writer->writeString(9, "$DIMFXLON");
            if (getInt("$DIMFXLON", &varInt))
                writer->writeInt16(70, varInt);
            else
                writer->writeInt16(70, 0);
            writer->writeString(9, "$DIMJOGANG");
            if (getDouble("$DIMJOGANG", &varDouble))
                writer->writeDouble(40, varDouble);
            else
                writer->writeDouble(40, 0.7854);
            writer->writeString(9, "$DIMTFILL");
            if (getInt("$DIMTFILL", &varInt))
                writer->writeInt16(70, varInt);
            else
                writer->writeInt16(70, 0);
            writer->writeString(9, "$DIMTFILLCLR");
            if (getInt("$DIMTFILLCLR", &varInt))
                writer->writeInt16(70, varInt);
            else
                writer->writeInt16(70, 0);
            writer->writeString(9, "$DIMARCSYM");
            if (getInt("$DIMARCSYM", &varInt))
                writer->writeInt16(70, varInt);
            else
                writer->writeInt16(70, 0);
            writer->writeString(9, "$DIMLTYPE");
            if (getStr("$DIMLTYPE", &varStr))
                if (ver == DRW::AC1009)
                    writer->writeUtf8Caps(6, varStr);
                else
                    writer->writeUtf8String(6, varStr);
            else
                writer->writeString(6, "");
            writer->writeString(9, "$DIMLTEX1");
            if (getStr("$DIMLTEX1", &varStr))
                if (ver == DRW::AC1009)
                    writer->writeUtf8Caps(6, varStr);
                else
                    writer->writeUtf8String(6, varStr);
            else
                writer->writeString(6, "");
            writer->writeString(9, "$DIMLTEX2");
            if (getStr("$DIMLTEX2", &varStr))
                if (ver == DRW::AC1009)
                    writer->writeUtf8Caps(6, varStr);
                else
                    writer->writeUtf8String(6, varStr);
            else
                writer->writeString(6, "");
            if (ver > DRW::AC1021) {// and post v2007 dim vars
                writer->writeString(9, "$DIMTXTDIRECTION");
                if (getInt("$DIMTXTDIRECTION", &varInt))
                    writer->writeInt16(70, varInt);
                else
                    writer->writeInt16(70, 0);
            }
        }// end post v2004 dim vars
    }//end post r12 dim vars

    writer->writeString(9, "$LUNITS");
    if (getInt("$LUNITS", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 2);
    writer->writeString(9, "$LUPREC");
    if (getInt("$LUPREC", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 4);
    writer->writeString(9, "$SKETCHINC");
    if (getDouble("$SKETCHINC", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 1.0);
    writer->writeString(9, "$FILLETRAD");
    if (getDouble("$FILLETRAD", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$AUNITS");
    if (getInt("$AUNITS", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$AUPREC");
    if (getInt("$AUPREC", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 2);
    writer->writeString(9, "$MENU");
    if (getStr("$MENU", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(1, varStr);
        else
            writer->writeUtf8String(1, varStr);
    else
        writer->writeString(1, ".");
    writer->writeString(9, "$ELEVATION");
    if (getDouble("$ELEVATION", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$PELEVATION");
    if (getDouble("$PELEVATION", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$THICKNESS");
    if (getDouble("$THICKNESS", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$LIMCHECK");
    if (getInt("$LIMCHECK", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    if (ver < DRW::AC1015) {
        writer->writeString(9, "$BLIPMODE");
        if (getInt("$BLIPMODE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
    }
    writer->writeString(9, "$CHAMFERA");
    if (getDouble("$CHAMFERA", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$CHAMFERB");
    if (getDouble("$CHAMFERB", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    if (ver > DRW::AC1009) {
        writer->writeString(9, "$CHAMFERC");
        if (getDouble("$CHAMFERC", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$CHAMFERD");
        if (getDouble("$CHAMFERD", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
    }
    writer->writeString(9, "$SKPOLY");
    if (getInt("$SKPOLY", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 0);
    //rlz: todo, times
    writer->writeString(9, "$USRTIMER");
    if (getInt("$USRTIMER", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$ANGBASE");
    if (getDouble("$ANGBASE", &varDouble))
        writer->writeDouble(50, varDouble);
    else
        writer->writeDouble(50, 0.0);
    writer->writeString(9, "$ANGDIR");
    if (getInt("$ANGDIR", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$PDMODE");
    if (getInt("$PDMODE", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 34);
    writer->writeString(9, "$PDSIZE");
    if (getDouble("$PDSIZE", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$PLINEWID");
    if (getDouble("$PLINEWID", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    if (ver < DRW::AC1012) {
        writer->writeString(9, "$COORDS");
        if (getInt("$COORDS", &varInt)) {
            writer->writeInt16(70, varInt);
        } else
            writer->writeInt16(70, 2);
    }
    writer->writeString(9, "$SPLFRAME");
    if (getInt("$SPLFRAME", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$SPLINETYPE");
    if (getInt("$SPLINETYPE", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 2);
    writer->writeString(9, "$SPLINESEGS");
    if (getInt("$SPLINESEGS", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 8);
    if (ver < DRW::AC1012) {
        writer->writeString(9, "$ATTDIA");
        if (getInt("$ATTDIA", &varInt)) {
            writer->writeInt16(70, varInt);
        } else
            writer->writeInt16(70, 1);
        writer->writeString(9, "$ATTREQ");
        if (getInt("$ATTREQ", &varInt)) {
            writer->writeInt16(70, varInt);
        } else
            writer->writeInt16(70, 1);
        writer->writeString(9, "$HANDLING");
        if (getInt("$HANDLING", &varInt)) {
            writer->writeInt16(70, varInt);
        } else
            writer->writeInt16(70, 1);
    }
    writer->writeString(9, "$HANDSEED");
    //RLZ        dxfHex(5, 0xFFFF);
    writer->writeString(5, "20000");
    writer->writeString(9, "$SURFTAB1");
    if (getInt("$SURFTAB1", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 6);
    writer->writeString(9, "$SURFTAB2");
    if (getInt("$SURFTAB2", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 6);
    writer->writeString(9, "$SURFTYPE");
    if (getInt("$SURFTYPE", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 6);
    writer->writeString(9, "$SURFU");
    if (getInt("$SURFU", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 6);
    writer->writeString(9, "$SURFV");
    if (getInt("$SURFV", &varInt)) {
        writer->writeInt16(70, varInt);
    } else
        writer->writeInt16(70, 6);
    if (ver > DRW::AC1009) {
    writer->writeString(9, "$UCSBASE");
    if (getStr("$UCSBASE", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(2, varStr);
        else
            writer->writeUtf8String(2, varStr);
    else
        writer->writeString(2, "");
}
    writer->writeString(9, "$UCSNAME");
    if (getStr("$UCSNAME", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(2, varStr);
        else
            writer->writeUtf8String(2, varStr);
    else
        writer->writeString(2, "");
    writer->writeString(9, "$UCSORG");
    if (getCoord("$UCSORG", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$UCSXDIR");
    if (getCoord("$UCSXDIR", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 1.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$UCSYDIR");
    if (getCoord("$UCSYDIR", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 1.0);
        writer->writeDouble(30, 0.0);
    }
    if (ver > DRW::AC1009) { //begin post r12 UCS vars
        writer->writeString(9, "$UCSORTHOREF");
        if (getStr("$UCSORTHOREF", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(2, varStr);
            else
                writer->writeUtf8String(2, varStr);
        else
            writer->writeString(2, "");
        writer->writeString(9, "$UCSORTHOVIEW");
        if (getInt("$UCSORTHOVIEW", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$UCSORGTOP");
        if (getCoord("$UCSORGTOP", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$UCSORGBOTTOM");
        if (getCoord("$UCSORGBOTTOM", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$UCSORGLEFT");
        if (getCoord("$UCSORGLEFT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$UCSORGRIGHT");
        if (getCoord("$UCSORGRIGHT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$UCSORGFRONT");
        if (getCoord("$UCSORGFRONT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$UCSORGBACK");
        if (getCoord("$UCSORGBACK", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSBASE");
        if (getStr("$PUCSBASE", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(2, varStr);
            else
                writer->writeUtf8String(2, varStr);
        else
            writer->writeString(2, "");
    } //end post r12 UCS vars
    writer->writeString(9, "$PUCSNAME");
    if (getStr("$PUCSNAME", &varStr))
        if (ver == DRW::AC1009)
            writer->writeUtf8Caps(2, varStr);
        else
            writer->writeUtf8String(2, varStr);
    else
        writer->writeString(2, "");
    writer->writeString(9, "$PUCSORG");
    if (getCoord("$PUCSORG", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$PUCSXDIR");
    if (getCoord("$PUCSXDIR", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 1.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$PUCSYDIR");
    if (getCoord("$PUCSYDIR", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 1.0);
        writer->writeDouble(30, 0.0);
    }
    if (ver > DRW::AC1009) { //begin post r12 PUCS vars
        writer->writeString(9, "$PUCSORTHOREF");
        if (getStr("$PUCSORTHOREF", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(2, varStr);
            else
                writer->writeUtf8String(2, varStr);
        else
            writer->writeString(2, "");
        writer->writeString(9, "$PUCSORTHOVIEW");
        if (getInt("$PUCSORTHOVIEW", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$PUCSORGTOP");
        if (getCoord("$PUCSORGTOP", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSORGBOTTOM");
        if (getCoord("$PUCSORGBOTTOM", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSORGLEFT");
        if (getCoord("$PUCSORGLEFT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSORGRIGHT");
        if (getCoord("$PUCSORGRIGHT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSORGFRONT");
        if (getCoord("$PUCSORGFRONT", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
        writer->writeString(9, "$PUCSORGBACK");
        if (getCoord("$PUCSORGBACK", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
    } //end post r12 PUCS vars

    writer->writeString(9, "$USERI1");
    if (getInt("$USERI1", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$USERI2");
    if (getInt("$USERI2", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$USERI3");
    if (getInt("$USERI3", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$USERI4");
    if (getInt("$USERI4", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$USERI5");
    if (getInt("$USERI5", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$USERR1");
    if (getDouble("$USERR1", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$USERR2");
    if (getDouble("$USERR2", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$USERR3");
    if (getDouble("$USERR3", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$USERR4");
    if (getDouble("$USERR4", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$USERR5");
    if (getDouble("$USERR5", &varDouble))
        writer->writeDouble(40, varDouble);
    else
        writer->writeDouble(40, 0.0);
    writer->writeString(9, "$WORLDVIEW");
    if (getInt("$WORLDVIEW", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$SHADEDGE");
    if (getInt("$SHADEDGE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 3);
    writer->writeString(9, "$SHADEDIF");
    if (getInt("$SHADEDIF", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 70);
    writer->writeString(9, "$TILEMODE");
    if (getInt("$TILEMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$MAXACTVP");
    if (getInt("$MAXACTVP", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 64);
    if (ver > DRW::AC1009) { //begin post r12 PUCS vars
        writer->writeString(9, "$PINSBASE");
        if (getCoord("$PINSBASE", &varCoord)) {
            writer->writeDouble(10, varCoord.x);
            writer->writeDouble(20, varCoord.y);
            writer->writeDouble(30, varCoord.z);
        } else {
            writer->writeDouble(10, 0.0);
            writer->writeDouble(20, 0.0);
            writer->writeDouble(30, 0.0);
        }
    }
    writer->writeString(9, "$PLIMCHECK");
    if (getInt("$PLIMCHECK", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$PEXTMIN");
    if (getCoord("$PEXTMIN", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }
    writer->writeString(9, "$PEXTMAX");
    if (getCoord("$PEXTMAX", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
        writer->writeDouble(30, varCoord.z);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
        writer->writeDouble(30, 0.0);
    }

/* RLZ: moved to active VPORT, but can write in header if present*/
    if (getInt("$GRIDMODE", &varInt)) {
        writer->writeString(9, "$GRIDMODE");
        writer->writeInt16(70, varInt);
    }
    if (getInt("$SNAPSTYLE", &varInt)) {
        writer->writeString(9, "$SNAPSTYLE");
        writer->writeInt16(70, varInt);
    }
    if (getCoord("$GRIDUNIT", &varCoord)) {
        writer->writeString(9, "$GRIDUNIT");
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    }
    if (getCoord("$VIEWCTR", &varCoord)) {
        writer->writeString(9, "$VIEWCTR");
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    }
/* RLZ: moved to active VPORT, but can write in header if present*/

    writer->writeString(9, "$PLIMMIN");
    if (getCoord("$PLIMMIN", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    } else {
        writer->writeDouble(10, 0.0);
        writer->writeDouble(20, 0.0);
    }
    writer->writeString(9, "$PLIMMAX");
    if (getCoord("$PLIMMAX", &varCoord)) {
        writer->writeDouble(10, varCoord.x);
        writer->writeDouble(20, varCoord.y);
    } else {
        writer->writeDouble(10, 297.0);
        writer->writeDouble(20, 210.0);
    }
    writer->writeString(9, "$UNITMODE");
    if (getInt("$UNITMODE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$VISRETAIN");
    if (getInt("$VISRETAIN", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    writer->writeString(9, "$PLINEGEN");
    if (getInt("$PLINEGEN", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 0);
    writer->writeString(9, "$PSLTSCALE");
    if (getInt("$PSLTSCALE", &varInt))
        writer->writeInt16(70, varInt);
    else
        writer->writeInt16(70, 1);
    if (ver > DRW::AC1009){//start port r12 vars
        writer->writeString(9, "$TREEDEPTH");
        if (getInt("$TREEDEPTH", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 3020);
        writer->writeString(9, "$CMLSTYLE");
        if (getStr("$CMLSTYLE", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(2, varStr);
            else
                writer->writeUtf8String(2, varStr);
        else
            writer->writeString(2, "Standard");
        writer->writeString(9, "$CMLJUST");
        if (getInt("$CMLJUST", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 0);
        writer->writeString(9, "$CMLSCALE");
        if (getDouble("$CMLSCALE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 20.0);
        writer->writeString(9, "$PROXYGRAPHICS");
        if (getInt("$PROXYGRAPHICS", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 1);
        writer->writeString(9, "$MEASUREMENT");
        if (getInt("$MEASUREMENT", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 1);
        writer->writeString(9, "$CELWEIGHT");
        if (getInt("$CELWEIGHT", &varInt))
            writer->writeInt16(370, varInt);
        else
            writer->writeInt16(370, -1);
        writer->writeString(9, "$ENDCAPS");
        if (getInt("$ENDCAPS", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$JOINSTYLE");
        if (getInt("$JOINSTYLE", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$LWDISPLAY"); //RLZ bool flag, verify in bin version
        if (getInt("$LWDISPLAY", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 0);
        if (ver > DRW::AC1014) {
            writer->writeString(9, "$INSUNITS");
            if (getInt("$INSUNITS", &varInt))
                writer->writeInt16(70, varInt);
            else
                writer->writeInt16(70, 0);
        }
        writer->writeString(9, "$HYPERLINKBASE");
        if (getStr("$HYPERLINKBASE", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(1, varStr);
            else
                writer->writeUtf8String(1, varStr);
        else
            writer->writeString(1, "");
        writer->writeString(9, "$STYLESHEET");
        if (getStr("$STYLESHEET", &varStr))
            if (ver == DRW::AC1009)
                writer->writeUtf8Caps(1, varStr);
            else
                writer->writeUtf8String(1, varStr);
        else
            writer->writeString(1, "");
        writer->writeString(9, "$XEDIT"); //RLZ bool flag, verify in bin version
        if (getInt("$XEDIT", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 1);
        writer->writeString(9, "$CEPSNTYPE");
        if (getInt("$CEPSNTYPE", &varInt))
            writer->writeInt16(380, varInt);
        else
            writer->writeInt16(380, 0);
        writer->writeString(9, "$PSTYLEMODE"); //RLZ bool flag, verify in bin version
        if (getInt("$PSTYLEMODE", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 1);
//RLZ: here $FINGERPRINTGUID and $VERSIONGUID, do not add?
        writer->writeString(9, "$EXTNAMES"); //RLZ bool flag, verify in bin version
        if (getInt("$EXTNAMES", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 1);
        writer->writeString(9, "$PSVPSCALE");
        if (getDouble("$PSVPSCALE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$OLESTARTUP"); //RLZ bool flag, verify in bin version
        if (getInt("$OLESTARTUP", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 0);
    }
    if (ver > DRW::AC1015) {// and post v2004 vars
        writer->writeString(9, "$SORTENTS");
        if (getInt("$SORTENTS", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 127);
        writer->writeString(9, "$INDEXCTL");
        if (getInt("$INDEXCTL", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$HIDETEXT");
        if (getInt("$HIDETEXT", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$XCLIPFRAME"); //RLZ bool flag, verify in bin version
        if (ver > DRW::AC1021) {
            if (getInt("$XCLIPFRAME", &varInt))
                writer->writeInt16(280, varInt);
            else
                writer->writeInt16(280, 0);
        } else {
            if (getInt("$XCLIPFRAME", &varInt))
                writer->writeInt16(290, varInt);
            else
                writer->writeInt16(290, 0);
        }
        writer->writeString(9, "$HALOGAP");
        if (getInt("$HALOGAP", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$OBSCOLOR");
        if (getInt("$OBSCOLOR", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 257);
        writer->writeString(9, "$OBSLTYPE");
        if (getInt("$OBSLTYPE", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$INTERSECTIONDISPLAY");
        if (getInt("$INTERSECTIONDISPLAY", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$INTERSECTIONCOLOR");
        if (getInt("$INTERSECTIONCOLOR", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 257);
        writer->writeString(9, "$DIMASSOC");
        if (getInt("$DIMASSOC", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$PROJECTNAME");
        if (getStr("$PROJECTNAME", &varStr))
            writer->writeUtf8String(1, varStr);
        else
            writer->writeString(1, "");
    }
    if (ver > DRW::AC1018) {// and post v2007 vars
        writer->writeString(9, "$CAMERADISPLAY"); //RLZ bool flag, verify in bin version
        if (getInt("$CAMERADISPLAY", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 0);
        writer->writeString(9, "$LENSLENGTH");
        if (getDouble("$LENSLENGTH", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 50.0);
        writer->writeString(9, "$CAMERAHEIGHT");
        if (getDouble("$CAMERAHEIGHT", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$STEPSPERSEC");
        if (getDouble("$STEPSPERSEC", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 2.0);
        writer->writeString(9, "$STEPSIZE");
        if (getDouble("$STEPSIZE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 50.0);
        writer->writeString(9, "$3DDWFPREC");
        if (getDouble("$3DDWFPREC", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 2.0);
        writer->writeString(9, "$PSOLWIDTH");
        if (getDouble("$PSOLWIDTH", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 5.0);
        writer->writeString(9, "$PSOLHEIGHT");
        if (getDouble("$PSOLHEIGHT", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 80.0);
        writer->writeString(9, "$LOFTANG1");
        if (getDouble("$LOFTANG1", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, M_PI_2);
        writer->writeString(9, "$LOFTANG2");
        if (getDouble("$LOFTANG2", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, M_PI_2);
        writer->writeString(9, "$LOFTMAG1");
        if (getDouble("$LOFTMAG1", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$LOFTMAG2");
        if (getDouble("$LOFTMAG2", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$LOFTPARAM");
        if (getInt("$LOFTPARAM", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, 7);
        writer->writeString(9, "$LOFTNORMALS");
        if (getInt("$LOFTNORMALS", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$LATITUDE");
        if (getDouble("$LATITUDE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 1.0);
        writer->writeString(9, "$LONGITUDE");
        if (getDouble("$LONGITUDE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 1.0);
        writer->writeString(9, "$NORTHDIRECTION");
        if (getDouble("$LONGITUDE", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
        writer->writeString(9, "$TIMEZONE");
        if (getInt("$TIMEZONE", &varInt))
            writer->writeInt16(70, varInt);
        else
            writer->writeInt16(70, -8000);
        writer->writeString(9, "$LIGHTGLYPHDISPLAY");
        if (getInt("$LIGHTGLYPHDISPLAY", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$TILEMODELIGHTSYNCH");
        if (getInt("$TILEMODELIGHTSYNCH", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
    //$CMATERIAL is a handle
        writer->writeString(9, "$SOLIDHIST");
        if (getInt("$SOLIDHIST", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$SHOWHIST");
        if (getInt("$SHOWHIST", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 1);
        writer->writeString(9, "$DWFFRAME");
        if (getInt("$DWFFRAME", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 2);
        writer->writeString(9, "$DGNFRAME");
        if (getInt("$DGNFRAME", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$REALWORLDSCALE"); //RLZ bool flag, verify in bin version
        if (getInt("$REALWORLDSCALE", &varInt))
            writer->writeInt16(290, varInt);
        else
            writer->writeInt16(290, 1);
        writer->writeString(9, "$INTERFERECOLOR");
        if (getInt("$INTERFERECOLOR", &varInt))
            writer->writeInt16(62, varInt);
        else
            writer->writeInt16(62, 1);
        //$INTERFEREOBJVS is a handle
        //$INTERFEREVPVS is a handle
        writer->writeString(9, "$CSHADOW");
        if (getInt("$CSHADOW", &varInt))
            writer->writeInt16(280, varInt);
        else
            writer->writeInt16(280, 0);
        writer->writeString(9, "$SHADOWPLANELOCATION");
        if (getDouble("$SHADOWPLANELOCATION", &varDouble))
            writer->writeDouble(40, varDouble);
        else
            writer->writeDouble(40, 0.0);
    }

#ifdef DRW_DBG
    std::map<std::string,DRW_Variant *>::const_iterator it;
    for ( it=vars.begin() ; it != vars.end(); ++it ){
        DRW_DBG((*it).first); DRW_DBG("\n");
    }
#endif
}

void DRW_Header::addDouble(std::string key, double value, int code){
    curr = new DRW_Variant(code, value);
    vars[key] =curr;
}

void DRW_Header::addInt(std::string key, int value, int code){
    curr = new DRW_Variant(code, value);
    vars[key] =curr;
}

void DRW_Header::addStr(std::string key, std::string value, int code){
    curr = new DRW_Variant(code, value);
    vars[key] =curr;
}

void DRW_Header::addCoord(std::string key, DRW_Coord value, int code){
    curr = new DRW_Variant(code, value);
    vars[key] =curr;
}

bool DRW_Header::getDouble(std::string key, double *varDouble){
    bool result = false;
    std::map<std::string,DRW_Variant *>::iterator it;
    it=vars.find( key);
    if (it != vars.end()) {
        DRW_Variant *var = (*it).second;
        if (var->type() == DRW_Variant::DOUBLE) {
            *varDouble = var->content.d;
            result = true;
        }
        delete var;
        vars.erase (it);
    }
    return result;
}

bool DRW_Header::getInt(std::string key, int *varInt){
    bool result = false;
    std::map<std::string,DRW_Variant *>::iterator it;
    it=vars.find( key);
    if (it != vars.end()) {
        DRW_Variant *var = (*it).second;
        if (var->type() == DRW_Variant::INTEGER) {
            *varInt = var->content.i;
            result = true;
        }
        delete var;
        vars.erase (it);
    }
    return result;
}

bool DRW_Header::getStr(std::string key, std::string *varStr){
    bool result = false;
    std::map<std::string,DRW_Variant *>::iterator it;
    it=vars.find( key);
    if (it != vars.end()) {
        DRW_Variant *var = (*it).second;
        if (var->type() == DRW_Variant::STRING) {
            *varStr = *var->content.s;
            result = true;
        }
        delete var;
        vars.erase (it);
    }
    return result;
}

bool DRW_Header::getCoord(std::string key, DRW_Coord *varCoord){
    bool result = false;
    std::map<std::string,DRW_Variant *>::iterator it;
    it=vars.find( key);
    if (it != vars.end()) {
        DRW_Variant *var = (*it).second;
        if (var->type() == DRW_Variant::COORD) {
            *varCoord = *var->content.v;
            result = true;
        }
        delete var;
        vars.erase (it);
    }
    return result;
}

bool DRW_Header::parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *hBbuf, duint8 mv){
    bool result = true;
    duint32 size = buf->getRawLong32();
    duint32 bitSize = 0;
    duint32 endBitPos = 160; //start bit: 16 sentinel + 4 size
    DRW_DBG("\nbyte size of data: "); DRW_DBG(size);
    if (version > DRW::AC1021 && mv > 3) { //2010+
        duint32 hSize = buf->getRawLong32();
        endBitPos += 32; //start bit: + 4 height size
        DRW_DBG("\n2010+ & MV> 3, height 32b: "); DRW_DBG(hSize);
    }
//RLZ TODO add $ACADVER var & $DWGCODEPAGE & $MEASUREMENT
//RLZ TODO EN 2000 falta $CELWEIGHT, $ENDCAPS, $EXTNAMES $JOINSTYLE $LWDISPLAY $PSTYLEMODE $TDUCREATE  $TDUUPDATE $XEDIT

    //bit size of data needed to locate start of string stream in 2007+
    //and mark the start of handle stream
    //header is one object reads data and continue read strings ???
    if (version > DRW::AC1018) {//2007+
        bitSize = buf->getRawLong32();
        DRW_DBG("\nsize in bits: "); DRW_DBG(bitSize);
        endBitPos += bitSize;
        hBbuf->setPosition(endBitPos >>3);
        hBbuf->setBitPos(endBitPos&7);
    }

    if (version > DRW::AC1024) {//2013+
        duint64 requiredVersions = buf->getBitLongLong();
        DRW_DBG("\nREQUIREDVERSIONS var: "); DRW_DBG(requiredVersions);
    }
    DRW_DBG("\nUnknown1: "); DRW_DBG(buf->getBitDouble());
    DRW_DBG("\nUnknown2: "); DRW_DBG(buf->getBitDouble());
    DRW_DBG("\nUnknown3: "); DRW_DBG(buf->getBitDouble());
    DRW_DBG("\nUnknown4: "); DRW_DBG(buf->getBitDouble());
    if (version < DRW::AC1021) {//2007-
        DRW_DBG("\nUnknown text1: "); DRW_DBG(buf->getCP8Text());
        DRW_DBG("\nUnknown text2: "); DRW_DBG(buf->getCP8Text());
        DRW_DBG("\nUnknown text3: "); DRW_DBG(buf->getCP8Text());
        DRW_DBG("\nUnknown text4: "); DRW_DBG(buf->getCP8Text());
    }
    DRW_DBG("\nUnknown long1 (24L): "); DRW_DBG(buf->getBitLong());
    DRW_DBG("\nUnknown long2 (0L): "); DRW_DBG(buf->getBitLong());
    if (version < DRW::AC1015) {//pre 2000
        DRW_DBG("\nUnknown short (0): "); DRW_DBG(buf->getBitShort());
    }
    if (version < DRW::AC1018) {//pre 2004
        dwgHandle hcv = hBbuf->getHandle();
        DRW_DBG("\nhandle of current view: "); DRW_DBGHL(hcv.code, hcv.size, hcv.ref);
    }
    vars["DIMASO"]=new DRW_Variant(70, buf->getBit());
    vars["DIMSHO"]=new DRW_Variant(70, buf->getBit());
    if (version < DRW::AC1015) {//pre 2000
        vars["DIMSAV"]=new DRW_Variant(70, buf->getBit());
    }
    vars["PLINEGEN"]=new DRW_Variant(70, buf->getBit());
    vars["ORTHOMODE"]=new DRW_Variant(70, buf->getBit());
    vars["REGENMODE"]=new DRW_Variant(70, buf->getBit());
    vars["FILLMODE"]=new DRW_Variant(70, buf->getBit());
    vars["QTEXTMODE"]=new DRW_Variant(70, buf->getBit());
    vars["PSLTSCALE"]=new DRW_Variant(70, buf->getBit());
    vars["LIMCHECK"]=new DRW_Variant(70, buf->getBit());
    if (version < DRW::AC1015) {//pre 2000
        vars["BLIPMODE"]=new DRW_Variant(70, buf->getBit());
    }
    if (version > DRW::AC1015) {//2004+
         DRW_DBG("\nUndocumented: "); DRW_DBG(buf->getBit());
    }
    vars["USRTIMER"]=new DRW_Variant(70, buf->getBit());
    vars["SKPOLY"]=new DRW_Variant(70, buf->getBit());
    vars["ANGDIR"]=new DRW_Variant(70, buf->getBit());
    vars["SPLFRAME"]=new DRW_Variant(70, buf->getBit());
    if (version < DRW::AC1015) {//pre 2000
        vars["ATTREQ"]=new DRW_Variant(70, buf->getBit());
        vars["ATTDIA"]=new DRW_Variant(70, buf->getBit());
    }
    vars["MIRRTEXT"]=new DRW_Variant(70, buf->getBit());
    vars["WORLDVIEW"]=new DRW_Variant(70, buf->getBit());
    if (version < DRW::AC1015) {//pre 2000
        vars["WIREFRAME"]=new DRW_Variant(70, buf->getBit());
    }
    vars["TILEMODE"]=new DRW_Variant(70, buf->getBit());
    vars["PLIMCHECK"]=new DRW_Variant(70, buf->getBit());
    vars["VISRETAIN"]=new DRW_Variant(70, buf->getBit());
    if (version < DRW::AC1015) {//pre 2000
        vars["DELOBJ"]=new DRW_Variant(70, buf->getBit());
    }
    vars["DISPSILH"]=new DRW_Variant(70, buf->getBit());
    vars["PELLIPSE"]=new DRW_Variant(70, buf->getBit());
    vars["PROXIGRAPHICS"]=new DRW_Variant(70, buf->getBitShort());//RLZ short or bit??
    if (version < DRW::AC1015) {//pre 2000
        vars["DRAGMODE"]=new DRW_Variant(70, buf->getBitShort());//RLZ short or bit??
    }
    vars["TREEDEPTH"]=new DRW_Variant(70, buf->getBitShort());//RLZ short or bit??
    vars["LUNITS"]=new DRW_Variant(70, buf->getBitShort());
    vars["LUPREC"]=new DRW_Variant(70, buf->getBitShort());
    vars["AUNITS"]=new DRW_Variant(70, buf->getBitShort());
    vars["AUPREC"]=new DRW_Variant(70, buf->getBitShort());
    if (version < DRW::AC1015) {//pre 2000
        vars["OSMODE"]=new DRW_Variant(70, buf->getBitShort());
    }
    vars["ATTMODE"]=new DRW_Variant(70, buf->getBitShort());
    if (version < DRW::AC1015) {//pre 2000
        vars["COORDS"]=new DRW_Variant(70, buf->getBitShort());
    }
    vars["PDMODE"]=new DRW_Variant(70, buf->getBitShort());
    if (version < DRW::AC1015) {//pre 2000
        vars["PICKSTYLE"]=new DRW_Variant(70, buf->getBitShort());
    }
    if (version > DRW::AC1015) {//2004+
         DRW_DBG("\nUnknown long 1: "); DRW_DBG(buf->getBitLong());
         DRW_DBG("\nUnknown long 2: "); DRW_DBG(buf->getBitLong());
         DRW_DBG("\nUnknown long 3: "); DRW_DBG(buf->getBitLong());
    }
    vars["USERI1"]=new DRW_Variant(70, buf->getBitShort());
    vars["USERI2"]=new DRW_Variant(70, buf->getBitShort());
    vars["USERI3"]=new DRW_Variant(70, buf->getBitShort());
    vars["USERI4"]=new DRW_Variant(70, buf->getBitShort());
    vars["USERI5"]=new DRW_Variant(70, buf->getBitShort());
    vars["SPLINESEGS"]=new DRW_Variant(70, buf->getBitShort());
    vars["SURFU"]=new DRW_Variant(70, buf->getBitShort());
    vars["SURFV"]=new DRW_Variant(70, buf->getBitShort());
    vars["SURFTYPE"]=new DRW_Variant(70, buf->getBitShort());
    vars["SURFTAB1"]=new DRW_Variant(70, buf->getBitShort());
    vars["SURFTAB2"]=new DRW_Variant(70, buf->getBitShort());
    vars["SPLINETYPE"]=new DRW_Variant(70, buf->getBitShort());
    vars["SHADEDGE"]=new DRW_Variant(70, buf->getBitShort());
    vars["SHADEDIF"]=new DRW_Variant(70, buf->getBitShort());
    vars["UNITMODE"]=new DRW_Variant(70, buf->getBitShort());
    vars["MAXACTVP"]=new DRW_Variant(70, buf->getBitShort());
    vars["ISOLINES"]=new DRW_Variant(70, buf->getBitShort());//////////////////
    vars["CMLJUST"]=new DRW_Variant(70, buf->getBitShort());
    vars["TEXTQLTY"]=new DRW_Variant(70, buf->getBitShort());/////////////////////
    vars["LTSCALE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["TEXTSIZE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["TRACEWID"]=new DRW_Variant(40, buf->getBitDouble());
    vars["SKETCHINC"]=new DRW_Variant(40, buf->getBitDouble());
    vars["FILLETRAD"]=new DRW_Variant(40, buf->getBitDouble());
    vars["THICKNESS"]=new DRW_Variant(40, buf->getBitDouble());
    vars["ANGBASE"]=new DRW_Variant(50, buf->getBitDouble());
    vars["PDSIZE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["PLINEWID"]=new DRW_Variant(40, buf->getBitDouble());
    vars["USERR1"]=new DRW_Variant(40, buf->getBitDouble());
    vars["USERR2"]=new DRW_Variant(40, buf->getBitDouble());
    vars["USERR3"]=new DRW_Variant(40, buf->getBitDouble());
    vars["USERR4"]=new DRW_Variant(40, buf->getBitDouble());
    vars["USERR5"]=new DRW_Variant(40, buf->getBitDouble());
    vars["CHAMFERA"]=new DRW_Variant(40, buf->getBitDouble());
    vars["CHAMFERB"]=new DRW_Variant(40, buf->getBitDouble());
    vars["CHAMFERC"]=new DRW_Variant(40, buf->getBitDouble());
    vars["CHAMFERD"]=new DRW_Variant(40, buf->getBitDouble());
    vars["FACETRES"]=new DRW_Variant(40, buf->getBitDouble());/////////////////////////
    vars["CMLSCALE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["CELTSCALE"]=new DRW_Variant(40, buf->getBitDouble());
    if (version < DRW::AC1021) {//2004-
        vars["MENU"]=new DRW_Variant(1, buf->getCP8Text());
    }
    ddouble64 msec, day;
    day = buf->getBitLong();
    msec = buf->getBitLong();
    while (msec > 0)
        msec /=10;
    vars["TDCREATE"]=new DRW_Variant(40, day+msec);//RLZ: TODO convert to day.msec
//    vars["TDCREATE"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
//    vars["TDCREATE"]=new DRW_Variant(40, buf->getBitLong());
    day = buf->getBitLong();
    msec = buf->getBitLong();
    while (msec > 0)
        msec /=10;
    vars["TDUPDATE"]=new DRW_Variant(40, day+msec);//RLZ: TODO convert to day.msec
//    vars["TDUPDATE"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
//    vars["TDUPDATE"]=new DRW_Variant(40, buf->getBitLong());
    if (version > DRW::AC1015) {//2004+
         DRW_DBG("\nUnknown long 4: "); DRW_DBG(buf->getBitLong());
         DRW_DBG("\nUnknown long 5: "); DRW_DBG(buf->getBitLong());
         DRW_DBG("\nUnknown long 6: "); DRW_DBG(buf->getBitLong());
    }
    day = buf->getBitLong();
    msec = buf->getBitLong();
    while (msec > 0)
        msec /=10;
    vars["TDINDWG"]=new DRW_Variant(40, day+msec);//RLZ: TODO convert to day.msec
//    vars["TDINDWG"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
//    vars["TDINDWG"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
    day = buf->getBitLong();
    msec = buf->getBitLong();
    while (msec > 0)
        msec /=10;
    vars["TDUSRTIMER"]=new DRW_Variant(40, day+msec);//RLZ: TODO convert to day.msec
//    vars["TDUSRTIMER"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
//    vars["TDUSRTIMER"]=new DRW_Variant(40, buf->getBitLong());//RLZ: TODO convert to day.msec
    vars["CECOLOR"]=new DRW_Variant(62, buf->getCmColor(version));//RLZ: TODO read CMC or EMC color
    dwgHandle HANDSEED = buf->getHandle();//always present in data stream
    DRW_DBG("\nHANDSEED: "); DRW_DBGHL(HANDSEED.code, HANDSEED.size, HANDSEED.ref);
    dwgHandle CLAYER = hBbuf->getHandle();
    DRW_DBG("\nCLAYER: "); DRW_DBGHL(CLAYER.code, CLAYER.size, CLAYER.ref);
    dwgHandle TEXTSTYLE = hBbuf->getHandle();
    DRW_DBG("\nTEXTSTYLE: "); DRW_DBGHL(TEXTSTYLE.code, TEXTSTYLE.size, TEXTSTYLE.ref);
    dwgHandle CELTYPE = hBbuf->getHandle();
    DRW_DBG("\nCELTYPE: "); DRW_DBGHL(CELTYPE.code, CELTYPE.size, CELTYPE.ref);
    if (version > DRW::AC1018) {//2007+
        dwgHandle CMATERIAL = hBbuf->getHandle();
        DRW_DBG("\nCMATERIAL: "); DRW_DBGHL(CMATERIAL.code, CMATERIAL.size, CMATERIAL.ref);
    }
    dwgHandle DIMSTYLE = hBbuf->getHandle();
    DRW_DBG("\nDIMSTYLE: "); DRW_DBGHL(DIMSTYLE.code, DIMSTYLE.size, DIMSTYLE.ref);
    dwgHandle CMLSTYLE = hBbuf->getHandle();
    DRW_DBG("\nCMLSTYLE: "); DRW_DBGHL(CMLSTYLE.code, CMLSTYLE.size, CMLSTYLE.ref);
    if (version > DRW::AC1014) {//2000+
        vars["PSVPSCALE"]=new DRW_Variant(40, buf->getBitDouble());
    }
    vars["PINSBASE"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["PEXTMIN"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["PEXTMAX"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["PLIMMIN"]=new DRW_Variant(10, buf->get2RawDouble());
    vars["PLIMMAX"]=new DRW_Variant(10, buf->get2RawDouble());
    vars["PELEVATION"]=new DRW_Variant(40, buf->getBitDouble());
    vars["PUCSORG"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["PUCSXDIR"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["PUCSYDIR"]=new DRW_Variant(10, buf->get3BitDouble());
    dwgHandle PUCSNAME = hBbuf->getHandle();
    DRW_DBG("\nPUCSNAME: "); DRW_DBGHL(PUCSNAME.code, PUCSNAME.size, PUCSNAME.ref);
    if (version > DRW::AC1014) {//2000+
        dwgHandle PUCSORTHOREF = hBbuf->getHandle();
        DRW_DBG("\nPUCSORTHOREF: "); DRW_DBGHL(PUCSORTHOREF.code, PUCSORTHOREF.size, PUCSORTHOREF.ref);
        vars["PUCSORTHOVIEW"]=new DRW_Variant(70, buf->getBitShort());
        dwgHandle PUCSBASE = hBbuf->getHandle();
        DRW_DBG("\nPUCSBASE: "); DRW_DBGHL(PUCSBASE.code, PUCSBASE.size, PUCSBASE.ref);
        vars["PUCSORGTOP"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["PUCSORGBOTTOM"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["PUCSORGLEFT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["PUCSORGRIGHT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["PUCSORGFRONT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["PUCSORGBACK"]=new DRW_Variant(10, buf->get3BitDouble());
    }
    vars["INSBASE"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["EXTMIN"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["EXTMAX"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["LIMMIN"]=new DRW_Variant(10, buf->get2RawDouble());
    vars["LIMMAX"]=new DRW_Variant(10, buf->get2RawDouble());
    vars["ELEVATION"]=new DRW_Variant(40, buf->getBitDouble());
    vars["UCSORG"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["UCSXDIR"]=new DRW_Variant(10, buf->get3BitDouble());
    vars["UCSYDIR"]=new DRW_Variant(10, buf->get3BitDouble());
    dwgHandle UCSNAME = hBbuf->getHandle();
    DRW_DBG("\nUCSNAME: "); DRW_DBGHL(UCSNAME.code, UCSNAME.size, UCSNAME.ref);
    if (version > DRW::AC1014) {//2000+
        dwgHandle UCSORTHOREF = hBbuf->getHandle();
        DRW_DBG("\nUCSORTHOREF: "); DRW_DBGHL(UCSORTHOREF.code, UCSORTHOREF.size, UCSORTHOREF.ref);
        vars["UCSORTHOVIEW"]=new DRW_Variant(70, buf->getBitShort());
        dwgHandle UCSBASE = hBbuf->getHandle();
        DRW_DBG("\nUCSBASE: "); DRW_DBGHL(UCSBASE.code, UCSBASE.size, UCSBASE.ref);
        vars["UCSORGTOP"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["UCSORGBOTTOM"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["UCSORGLEFT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["UCSORGRIGHT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["UCSORGFRONT"]=new DRW_Variant(10, buf->get3BitDouble());
        vars["UCSORGBACK"]=new DRW_Variant(10, buf->get3BitDouble());
        if (version < DRW::AC1021) {//2004-
            vars["DIMPOST"]=new DRW_Variant(1, buf->getCP8Text());
            vars["DIMAPOST"]=new DRW_Variant(1, buf->getCP8Text());
        }
    }
    if (version < DRW::AC1015) {//r14-
        vars["DIMTOL"]=new DRW_Variant(70, buf->getBit());
        vars["DIMLIM"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTIH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTOH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSE1"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSE2"]=new DRW_Variant(70, buf->getBit());
        vars["DIMALT"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTOFL"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSAH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTIX"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSOXD"]=new DRW_Variant(70, buf->getBit());
        vars["DIMALTD"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMZIN"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMSD1"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSD2"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTOLJ"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMJUST"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMFIT"]=new DRW_Variant(70, buf->getRawChar8());///////////
        vars["DIMUPT"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTZIN"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMALTZ"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMALTTZ"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMTAD"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMUNIT"]=new DRW_Variant(70, buf->getBitShort());///////////
        vars["DIMAUNIT"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMDEC"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTDEC"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTU"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTTD"]=new DRW_Variant(70, buf->getBitShort());
        dwgHandle DIMTXSTY = hBbuf->getHandle();
        DRW_DBG("\nDIMTXSTY: "); DRW_DBGHL(DIMTXSTY.code, DIMTXSTY.size, DIMTXSTY.ref);
    }
    vars["DIMSCALE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMASZ"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMEXO"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMDLI"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMEXE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMRND"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMDLE"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMTP"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMTM"]=new DRW_Variant(40, buf->getBitDouble());
    if (version > DRW::AC1018) {//2007+
        vars["DIMFXL"]=new DRW_Variant(40, buf->getBitDouble());//////////////////
        vars["DIMJOGANG"]=new DRW_Variant(40, buf->getBitDouble());///////////////
        vars["DIMTFILL"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTFILLCLR"]=new DRW_Variant(62, buf->getCmColor(version));
    }
    if (version > DRW::AC1014) {//2000+
        vars["DIMTOL"]=new DRW_Variant(70, buf->getBit());
        vars["DIMLIM"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTIH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTOH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSE1"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSE2"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTAD"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMZIN"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMAZIN"]=new DRW_Variant(70, buf->getBitShort());
    }
    if (version > DRW::AC1018) {//2007+
        vars["DIMARCSYM"]=new DRW_Variant(70, buf->getBitShort());
    }
    vars["DIMTXT"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMCEN"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMTSZ"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMALTF"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMLFAC"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMTVP"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMTFAC"]=new DRW_Variant(40, buf->getBitDouble());
    vars["DIMGAP"]=new DRW_Variant(40, buf->getBitDouble());
    if (version < DRW::AC1015) {//r14-
        vars["DIMPOST"]=new DRW_Variant(1, buf->getCP8Text());
        vars["DIMAPOST"]=new DRW_Variant(1, buf->getCP8Text());
        vars["DIMBLK"]=new DRW_Variant(1, buf->getCP8Text());
        vars["DIMBLK1"]=new DRW_Variant(1, buf->getCP8Text());
        vars["DIMBLK2"]=new DRW_Variant(1, buf->getCP8Text());
    }
    if (version > DRW::AC1014) {//2000+
        vars["DIMALTRND"]=new DRW_Variant(40, buf->getBitDouble());
        vars["DIMALT"]=new DRW_Variant(70, buf->getBit());
        vars["DIMALTD"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTOFL"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSAH"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTIX"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSOXD"]=new DRW_Variant(70, buf->getBit());
    }
    vars["DIMCLRD"]=new DRW_Variant(70, buf->getCmColor(version));//RLZ: TODO read CMC or EMC color
    vars["DIMCLRE"]=new DRW_Variant(70, buf->getCmColor(version));//RLZ: TODO read CMC or EMC color
    vars["DIMCLRT"]=new DRW_Variant(70, buf->getCmColor(version));//RLZ: TODO read CMC or EMC color
    if (version > DRW::AC1014) {//2000+
        vars["DIAMDEC"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMDEC"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTDEC"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTU"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTTD"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMAUNIT"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMFAC"]=new DRW_Variant(70, buf->getBitShort());///////////////// DIMFAC O DIMFRAC
        vars["DIMLUNIT"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMDSEP"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTMOVE"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMJUST"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMSD1"]=new DRW_Variant(70, buf->getBit());
        vars["DIMSD2"]=new DRW_Variant(70, buf->getBit());
        vars["DIMTOLJ"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMTZIN"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTZ"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMALTTZ"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMUPT"]=new DRW_Variant(70, buf->getBit());
        vars["DIMATFIT"]=new DRW_Variant(70, buf->getBitShort());
    }
    if (version > DRW::AC1018) {//2007+
        vars["DIMFXLON"]=new DRW_Variant(70, buf->getBit());////////////////
    }
    if (version > DRW::AC1021) {//2010+
        vars["DIMTXTDIRECTION"]=new DRW_Variant(70, buf->getBit());////////////////
        vars["DIMALTMZF"]=new DRW_Variant(40, buf->getBitDouble());////////////////
        vars["DIMMZF"]=new DRW_Variant(40, buf->getBitDouble());////////////////
    }
    if (version > DRW::AC1014) {//2000+
        dwgHandle DIMTXSTY = hBbuf->getHandle();
        DRW_DBG("\nDIMTXSTY: "); DRW_DBGHL(DIMTXSTY.code, DIMTXSTY.size, DIMTXSTY.ref);
        dwgHandle DIMLDRBLK = hBbuf->getHandle();
        DRW_DBG("\nDIMLDRBLK: "); DRW_DBGHL(DIMLDRBLK.code, DIMLDRBLK.size, DIMLDRBLK.ref);
        dwgHandle DIMBLK = hBbuf->getHandle();
        DRW_DBG("\nDIMBLK: "); DRW_DBGHL(DIMBLK.code, DIMBLK.size, DIMBLK.ref);
        dwgHandle DIMBLK1 = hBbuf->getHandle();
        DRW_DBG("\nDIMBLK1: "); DRW_DBGHL(DIMBLK1.code, DIMBLK1.size, DIMBLK1.ref);
        dwgHandle DIMBLK2 = hBbuf->getHandle();
        DRW_DBG("\nDIMBLK2: "); DRW_DBGHL(DIMBLK2.code, DIMBLK2.size, DIMBLK2.ref);
    }
    if (version > DRW::AC1018) {//2007+
        dwgHandle DIMLTYPE = hBbuf->getHandle();
        DRW_DBG("\nDIMLTYPE: "); DRW_DBGHL(DIMLTYPE.code, DIMLTYPE.size, DIMLTYPE.ref);
        dwgHandle DIMLTEX1 = hBbuf->getHandle();
        DRW_DBG("\nDIMLTEX1: "); DRW_DBGHL(DIMLTEX1.code, DIMLTEX1.size, DIMLTEX1.ref);
        dwgHandle DIMLTEX2 = hBbuf->getHandle();
        DRW_DBG("\nDIMLTEX2: "); DRW_DBGHL(DIMLTEX2.code, DIMLTEX2.size, DIMLTEX2.ref);
    }
    if (version > DRW::AC1014) {//2000+
        vars["DIMLWD"]=new DRW_Variant(70, buf->getBitShort());
        vars["DIMLWE"]=new DRW_Variant(70, buf->getBitShort());
    }
    dwgHandle CONTROL = hBbuf->getHandle();
    DRW_DBG("\nBLOCK CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    blockCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nLAYER CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    layerCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nSTYLE CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    styleCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nLINETYPE CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    linetypeCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nVIEW CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    viewCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nUCS CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    ucsCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nVPORT CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    vportCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nAPPID CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    appidCtrl = CONTROL.ref;
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nDIMSTYLE CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    dimstyleCtrl = CONTROL.ref;
    if (version < DRW::AC1018) {//r2000-
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nVIEWPORT ENTITY HEADER CONTROL: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        vpEntHeaderCtrl = CONTROL.ref; //RLZ: only in R13-R15 ????
    }
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nDICT ACAD_GROUP: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nDICT ACAD_MLINESTYLE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nDICT NAMED OBJS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);

    if (version > DRW::AC1014) {//2000+
        vars["TSTACKALIGN"]=new DRW_Variant(70, buf->getBitShort());
        vars["TSTACKSIZE"]=new DRW_Variant(70, buf->getBitShort());
        if (version < DRW::AC1021) {//2004-
            vars["HYPERLINKBASE"]=new DRW_Variant(1, buf->getCP8Text());
            vars["STYLESHEET"]=new DRW_Variant(1, buf->getCP8Text());
        }
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT LAYOUTS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT PLOTSETTINGS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT PLOTSTYLES: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    }
    if (version > DRW::AC1015) {//2004+
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT MATERIALS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT COLORS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    }
    if (version > DRW::AC1018) {//2007+
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDICT VISUALSTYLE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    }
    if (version > DRW::AC1024) {//2013+
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nUNKNOWN HANDLE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    }
    if (version > DRW::AC1014) {//2000+
        DRW_DBG("\nFlags: "); DRW_DBGH(buf->getBitLong());//RLZ TODO change to 8 vars
        vars["INSUNITS"]=new DRW_Variant(70, buf->getBitShort());
        duint16 cepsntype = buf->getBitShort();
        vars["CEPSNTYPE"]=new DRW_Variant(70, cepsntype);
        if (cepsntype == 3){
            CONTROL = hBbuf->getHandle();
            DRW_DBG("\nCPSNID HANDLE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        }
        if (version < DRW::AC1021) {//2004-
            vars["FINGERPRINTGUID"]=new DRW_Variant(1, buf->getCP8Text());
            vars["VERSIONGUID"]=new DRW_Variant(1, buf->getCP8Text());
        }
    }
    if (version > DRW::AC1015) {//2004+
        vars["SORTENTS"]=new DRW_Variant(70, buf->getRawChar8());
        vars["INDEXCTL"]=new DRW_Variant(70, buf->getRawChar8());
        vars["HIDETEXT"]=new DRW_Variant(70, buf->getRawChar8());
        vars["XCLIPFRAME"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DIMASSOC"]=new DRW_Variant(70, buf->getRawChar8());
        vars["HALOGAP"]=new DRW_Variant(70, buf->getRawChar8());
        vars["OBSCUREDCOLOR"]=new DRW_Variant(70, buf->getBitShort());
        vars["INTERSECTIONCOLOR"]=new DRW_Variant(70, buf->getBitShort());
        vars["OBSCUREDLTYPE"]=new DRW_Variant(70, buf->getRawChar8());
        vars["INTERSECTIONDISPLAY"]=new DRW_Variant(70, buf->getRawChar8());
        if (version < DRW::AC1021) {//2004-
            vars["PROJECTNAME"]=new DRW_Variant(1, buf->getCP8Text());
        }
    }
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nBLOCK PAPER_SPACE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nBLOCK MODEL_SPACE: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nLTYPE BYLAYER: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nLTYPE BYBLOCK: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    CONTROL = hBbuf->getHandle();
    DRW_DBG("\nLTYPE CONTINUOUS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
    if (version > DRW::AC1018) {//2007+
        vars["CAMERADISPLAY"]=new DRW_Variant(70, buf->getBit());
        DRW_DBG("\nUnknown 2007+ long1: "); DRW_DBG(buf->getBitLong());
        DRW_DBG("\nUnknown 2007+ long2: "); DRW_DBG(buf->getBitLong());
        DRW_DBG("\nUnknown 2007+ double2: "); DRW_DBG(buf->getBitDouble());
        vars["STEPSPERSEC"]=new DRW_Variant(40, buf->getBitDouble());
        vars["STEPSIZE"]=new DRW_Variant(40, buf->getBitDouble());
        vars["3DDWFPREC"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LENSLENGTH"]=new DRW_Variant(40, buf->getBitDouble());
        vars["CAMERAHEIGHT"]=new DRW_Variant(40, buf->getBitDouble());
        vars["SOLIDHIST"]=new DRW_Variant(70, buf->getRawChar8());
        vars["SHOWHIST"]=new DRW_Variant(70, buf->getRawChar8());
        vars["PSOLWIDTH"]=new DRW_Variant(40, buf->getBitDouble());
        vars["PSOLHEIGHT"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LOFTANG1"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LOFTANG2"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LOFTMAG1"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LOFTMAG2"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LOFTPARAM"]=new DRW_Variant(70, buf->getBitShort());
        vars["LOFTNORMALS"]=new DRW_Variant(40, buf->getRawChar8());
        vars["LATITUDE"]=new DRW_Variant(40, buf->getBitDouble());
        vars["LONGITUDE"]=new DRW_Variant(40, buf->getBitDouble());
        vars["NORTHDIRECTION"]=new DRW_Variant(40, buf->getBitDouble());
        vars["TIMEZONE"]=new DRW_Variant(70, buf->getBitLong());
        vars["LIGHTGLYPHDISPLAY"]=new DRW_Variant(70, buf->getRawChar8());
        vars["TILEMODELIGHTSYNCH"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DWFFRAME"]=new DRW_Variant(70, buf->getRawChar8());
        vars["DGNFRAME"]=new DRW_Variant(70, buf->getRawChar8());
        DRW_DBG("\nUnknown 2007+ BIT: "); DRW_DBG(buf->getBit());
        vars["INTERFERECOLOR"]=new DRW_Variant(70, buf->getCmColor(version));
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nINTERFEREOBJVS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nINTERFEREVPVS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        CONTROL = hBbuf->getHandle();
        DRW_DBG("\nDRAGVS: "); DRW_DBGHL(CONTROL.code, CONTROL.size, CONTROL.ref);
        vars["CSHADOW"]=new DRW_Variant(70, buf->getRawChar8());
        DRW_DBG("\nUnknown 2007+ double2: "); DRW_DBG(buf->getBitDouble());
    }
    if (version > DRW::AC1012) {//R14+
        DRW_DBG("\nUnknown R14+ short1: "); DRW_DBG(buf->getBitShort());
        DRW_DBG("\nUnknown R14+ short2: "); DRW_DBG(buf->getBitShort());
        DRW_DBG("\nUnknown R14+ short3: "); DRW_DBG(buf->getBitShort());
        DRW_DBG("\nUnknown R14+ short4: "); DRW_DBG(buf->getBitShort());
    }

    DRW_DBG("\nbuf position: "); DRW_DBG(buf->getPosition());
    DRW_DBG("  buf bit position: "); DRW_DBG(buf->getBitPos());


    /**** RLZ: disabled, pending to read all data ***/
    //Start reading string stream for 2007 and further
    if (version > DRW::AC1018) {//2007+
        duint32 strStartPos = endBitPos -1;
        buf->setPosition(strStartPos >>3);
        buf->setBitPos(strStartPos&7);
        if (buf->getBit() == 1){
            strStartPos -= 16;
            buf->setPosition(strStartPos >>3);
            buf->setBitPos(strStartPos&7);
            duint32 strDataSize = buf->getRawShort16();
            if (strDataSize & 0x8000) {
                strStartPos -= 16;//decrement 16 bits
                strDataSize &= 0x7FFF; //strip 0x8000;
                buf->setPosition(strStartPos >> 3);
                buf->setBitPos(strStartPos & 7);
                duint32 hiSize = buf->getRawShort16();
                strDataSize |= (hiSize << 15);
            }
            strStartPos -= strDataSize;
            buf->setPosition(strStartPos >> 3);
            buf->setBitPos(strStartPos & 7);

            DRW_DBG("\nstring buf position: "); DRW_DBG(buf->getPosition());
            DRW_DBG("\nstring buf bit position: "); DRW_DBG(buf->getBitPos());
        }
        DRW_DBG("\nUnknown text1: "); DRW_DBG(buf->getUCSText(false));
        DRW_DBG("\nUnknown text2: "); DRW_DBG(buf->getUCSText(false));
        DRW_DBG("\nUnknown text3: "); DRW_DBG(buf->getUCSText(false));
        DRW_DBG("\nUnknown text4: "); DRW_DBG(buf->getUCSText(false));
        vars["MENU"]=new DRW_Variant(1, buf->getUCSText(false));
        vars["DIMPOST"]=new DRW_Variant(1, buf->getUCSText(false));
        vars["DIMAPOST"]=new DRW_Variant(1, buf->getUCSText(false));
        if (version > DRW::AC1021) {//2010+
            vars["DIMALTMZS"]=new DRW_Variant(70, buf->getUCSText(false));//RLZ: pending to verify//////////////
            vars["DIMMZS"]=new DRW_Variant(70, buf->getUCSText(false));//RLZ: pending to verify//////////////
        }
        vars["HYPERLINKBASE"]=new DRW_Variant(1, buf->getUCSText(false));
        vars["STYLESHEET"]=new DRW_Variant(1, buf->getUCSText(false));
        vars["FINGERPRINTGUID"]=new DRW_Variant(1, buf->getUCSText(false));
        DRW_DBG("\nstring buf position: "); DRW_DBG(buf->getPosition());
        DRW_DBG("  string buf bit position: "); DRW_DBG(buf->getBitPos());
        vars["VERSIONGUID"]=new DRW_Variant(1, buf->getUCSText(false));
        DRW_DBG("\nstring buf position: "); DRW_DBG(buf->getPosition());
        DRW_DBG("  string buf bit position: "); DRW_DBG(buf->getBitPos());
        vars["PROJECTNAME"]=new DRW_Variant(1, buf->getUCSText(false));
    }
/***    ****/
    DRW_DBG("\nstring buf position: "); DRW_DBG(buf->getPosition());
    DRW_DBG("  string buf bit position: "); DRW_DBG(buf->getBitPos());

    if (DRW_DBGGL == DRW_dbg::Level::Debug){
        for (std::map<std::string,DRW_Variant*>::iterator it=vars.begin(); it!=vars.end(); ++it){
            DRW_DBG("\n"); DRW_DBG(it->first); DRW_DBG(": ");
            switch (it->second->type()){
            case DRW_Variant::INTEGER:
                DRW_DBG(it->second->content.i);
                break;
            case DRW_Variant::DOUBLE:
                DRW_DBG(it->second->content.d);
                break;
            case DRW_Variant::STRING:
                DRW_DBG(it->second->content.s->c_str());
                break;
            case DRW_Variant::COORD:
                 DRW_DBG("x= "); DRW_DBG(it->second->content.v->x);
                 DRW_DBG(", y= "); DRW_DBG(it->second->content.v->y);
                 DRW_DBG(", z= "); DRW_DBG(it->second->content.v->z);
                break;
            default:
                break;
            }
             DRW_DBG(" code: ");DRW_DBG(it->second->code());
        }
    }

    buf->setPosition(size+16+4); //read size +16 start sentinel + 4 size
    if (version > DRW::AC1021 && mv > 3) { //2010+
        buf->getRawLong32();//advance 4 bytes (hisize)
    }
    DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
    DRW_DBG("\nHeader CRC: "); DRW_DBGH(buf->getRawShort16());
    DRW_DBG("\nbuf position: "); DRW_DBG(buf->getPosition());
    DRW_DBG("\ndwg header end sentinel= ");
    for (int i=0; i<16;i++) {
        DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
    }

    //temporary code to show header end sentinel
    duint64 sz= buf->size()-1;
    if (version < DRW::AC1018) {//pre 2004
        sz= buf->size()-16;
        buf->setPosition(sz);
        DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
        DRW_DBG("\ndwg header end sentinel= ");
        for (int i=0; i<16;i++) {
            DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
        }
    } else if (version == DRW::AC1018) {//2004
//        sz= buf->size()-132;
//        buf->setPosition(sz);
        buf->moveBitPos(-128);
        DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
        DRW_DBG("\ndwg header end sentinel= ");
        for (int i=0; i<16;i++) {
            DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
        }
    } else if (version == DRW::AC1021) {//2007
        sz= buf->size()-16;
        buf->setPosition(sz);
        DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
        DRW_DBG("\ndwg header end sentinel= ");
        for (int i=0; i<16;i++) {
            DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
        }
    } else if (version == DRW::AC1024) {//2010
//        sz= buf->size()-93;
//        buf->setPosition(sz);
        buf->moveBitPos(-128);
        DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
        DRW_DBG("\ndwg header end sentinel= ");
        for (int i=0; i<16;i++) {
            DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
        }
    } else if (version == DRW::AC1027) {//2013
//        sz= buf->size()-76;
//        buf->setPosition(sz);
        buf->moveBitPos(-128);
        DRW_DBG("\nsetting position to: "); DRW_DBG(buf->getPosition());
        DRW_DBG("\ndwg header end sentinel= ");
        for (int i=0; i<16;i++) {
            DRW_DBGH(buf->getRawChar8()); DRW_DBG(" ");
        }
    }

    return result;
}

