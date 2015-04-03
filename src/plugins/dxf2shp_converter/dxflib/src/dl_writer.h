/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
** Copyright (C) 2001 Robert J. Campbell Jr.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use 
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_WRITER_H
#define DL_WRITER_H

#include "dl_global.h"

#ifndef _WIN32
#include <strings.h>
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <algorithm>

#include "dl_attributes.h"
#include "dl_codes.h"



/**
 * Defines interface for writing low level DXF constructs to
 * a file. Implementation is defined in derived classes that write
 * to binary or ASCII files.
 * 
 * Implements functions that write higher level constructs in terms of
 * the low level ones.
 *
 * @todo Add error checking for string/entry length.
 */
class DXFLIB_EXPORT DL_Writer {
public:
    /**
     * @param version DXF version. Defaults to DL_VERSION_2002.
     */
    DL_Writer(DL_Codes::version version) : m_handle(0x30) {
        this->version = version;
        modelSpaceHandle = 0;
        paperSpaceHandle = 0;
        paperSpace0Handle = 0;
    }

    virtual ~DL_Writer() {}
    ;

    /** Generic section for section 'name'.
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  name
     * </pre>
     */
    void section(const char* name) const {
        dxfString(0, "SECTION");
        dxfString(2, name);
    }

    /**
     * Section HEADER
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  HEADER
     * </pre>
     */
    void sectionHeader() const {
        section("HEADER");
    }

    /**
     * Section TABLES
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  TABLES
     * </pre>
     */
    void sectionTables() const {
        section("TABLES");
    }

    /**
     * Section BLOCKS
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  BLOCKS
     * </pre>
     */
    void sectionBlocks() const {
        section("BLOCKS");
    }

    /**
     * Section ENTITIES
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  ENTITIES
     * </pre>
     */
    void sectionEntities() const {
        section("ENTITIES");
    }

    /**
     * Section CLASSES
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  CLASSES
     * </pre>
     */
    void sectionClasses() const {
        section("CLASSES");
    }

    /**
     * Section OBJECTS
     *
     * <pre>
     *   0
     *  SECTION
     *   2
     *  OBJECTS
     * </pre>
     */
    void sectionObjects() const {
        section("OBJECTS");
    }

    /**
     * End of a section.
     *
     * <pre>
     *   0
     *  ENDSEC
     * </pre>
     */
    void sectionEnd() const {
        dxfString(0, "ENDSEC");
    }

    /**
     * Generic table for table 'name' with 'num' entries:
     *
     * <pre>
     *   0
     *  TABLE
     *   2
     *  name
     *  70
     *   num
     * </pre>
     */
    void table(const char* name, int num, int h=0) const {
        dxfString(0, "TABLE");
        dxfString(2, name);
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            }
            else {
                dxfHex(5, h);
            }
            dxfString(100, "AcDbSymbolTable");
        }
        dxfInt(70, num);
    }

    /** Table for layers.
     *
     * @param num Number of layers in total.
     *
     * <pre>
     *   0
     *  TABLE
     *   2
     *  LAYER
     *   70
     *      num
     * </pre>
     */
    void tableLayers(int num) const {
        table("LAYER", num, 2);
    }

    /** Table for line types.
     *
     * @param num Number of line types in total.
     *
     * <pre>
     *   0
     *  TABLE
     *   2
     *  LTYPE
     *   70
     *      num
     * </pre>
     */
    void tableLinetypes(int num) const {
        //linetypeHandle = 5;
        table("LTYPE", num, 5);
    }

    /** Table for application id.
     *
     * @param num Number of registered applications in total.
     *
     * <pre>
     *   0
     *  TABLE
     *   2
     *  APPID
     *   70
     *      num
     * </pre>
     */
    void tableAppid(int num) const {
        table("APPID", num, 9);
    }

    /** Table for text style.
     *
     * @param num Number of text styles.
     *
     * <pre>
     *   0
     *  TABLE
     *   2
     *  STYLE
     *   70
     *      num
     * </pre>
     */
    void tableStyle(int num) const {
        table("STYLE", num, 3);
    }

    /**
     * End of a table.
     *
     * <pre>
     *   0
     *  ENDTAB
     * </pre>
     */
    void tableEnd() const {
        dxfString(0, "ENDTAB");
    }

    /**
     * End of the DXF file.
     *
     * <pre>
     *   0
     *  EOF
     * </pre>
     */
    void dxfEOF() const {
        dxfString(0, "EOF");
    }

    /**
     * Comment.
     *
     * <pre>
     *  999
     *  text
     * </pre>
     */
    void comment(const char* text) const {
        dxfString(999, text);
    }

    /**
     * Entity.
     *
     * <pre>
     *   0
     *  entTypeName
     * </pre>
     *
     * @return Unique handle or 0.
     */
    void entity(const char* entTypeName) const {
        dxfString(0, entTypeName);
        if (version>=DL_VERSION_2000) {
            handle();
        }
    }

    /**
     * Attributes of an entity.
     *
     * <pre>
     *   8
     *  layer
     *  62
     *  color
     *  39
     *  width
     *   6
     *  linetype
     * </pre>
     */
    void entityAttributes(const DL_Attributes& attrib) const {
    
        // layer name:
        dxfString(8, attrib.getLayer());
        
        // R12 doesn't accept BYLAYER values. The value has to be missing
        //   in that case.
        if (version>=DL_VERSION_2000 || attrib.getColor()!=256) {
            dxfInt(62, attrib.getColor());
        }
        if (version>=DL_VERSION_2000 && attrib.getColor24()!=-1) {
            dxfInt(420, attrib.getColor24());
        }
        if (version>=DL_VERSION_2000) {
            dxfInt(370, attrib.getWidth());
        }
        if (version>=DL_VERSION_2000) {
            dxfReal(48, attrib.getLinetypeScale());
        }
        std::string linetype = attrib.getLinetype();
        std::transform(linetype.begin(), linetype.end(), linetype.begin(), ::toupper);
        if (version>=DL_VERSION_2000 || linetype=="BYLAYER") {
            dxfString(6, attrib.getLinetype());
        }
    }

    /**
     * Subclass.
     */
    void subClass(const char* sub) const {
        dxfString(100, sub);
    }

    /**
     * Layer (must be in the TABLES section LAYER).
     *
     * <pre>
     *   0
     *  LAYER
     * </pre>
     */
    void tableLayerEntry(unsigned long int h=0)  const {
        dxfString(0, "LAYER");
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            } else {
                dxfHex(5, h);
            }
            dxfString(100, "AcDbSymbolTableRecord");
            dxfString(100, "AcDbLayerTableRecord");
        }
    }

    /**
     * Line type (must be in the TABLES section LTYPE).
     *
     * <pre>
     *   0
     *  LTYPE
     * </pre>
     */
    void tableLinetypeEntry(unsigned long int h=0)  const {
        dxfString(0, "LTYPE");
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            } else {
                dxfHex(5, h);
            }
            //dxfHex(330, 0x5);
            dxfString(100, "AcDbSymbolTableRecord");
            dxfString(100, "AcDbLinetypeTableRecord");
        }
    }

    /**
     * Appid (must be in the TABLES section APPID).
     *
     * <pre>
     *   0
     *  APPID
     * </pre>
     */
    void tableAppidEntry(unsigned long int h=0)  const {
        dxfString(0, "APPID");
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            } else {
                dxfHex(5, h);
            }
            //dxfHex(330, 0x9);
            dxfString(100, "AcDbSymbolTableRecord");
            dxfString(100, "AcDbRegAppTableRecord");
        }
    }

    /**
     * Block (must be in the section BLOCKS).
     *
     * <pre>
     *   0
     *  BLOCK
     * </pre>
     */
    void sectionBlockEntry(unsigned long int h=0)  const {
        dxfString(0, "BLOCK");
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            } else {
                dxfHex(5, h);
            }
            //dxfHex(330, blockHandle);
            dxfString(100, "AcDbEntity");
            if (h==0x1C) {
                dxfInt(67, 1);
            }
            dxfString(8, "0");                 // TODO: Layer for block
            dxfString(100, "AcDbBlockBegin");
        }
    }

    /**
     * End of Block (must be in the section BLOCKS).
     *
     * <pre>
     *   0
     *  ENDBLK
     * </pre>
     */
    void sectionBlockEntryEnd(unsigned long int h=0)  const {
        dxfString(0, "ENDBLK");
        if (version>=DL_VERSION_2000) {
            if (h==0) {
                handle();
            } else {
                dxfHex(5, h);
            }
            //dxfHex(330, blockHandle);
            dxfString(100, "AcDbEntity");
            if (h==0x1D) {
                dxfInt(67, 1);
            }
            dxfString(8, "0");                 // TODO: Layer for block
            dxfString(100, "AcDbBlockEnd");
        }
    }

    void color(int col=256) const {
        dxfInt(62, col);
    }
    void linetype(const char *lt) const {
        dxfString(6, lt);
    }
    void linetypeScale(double scale) const {
        dxfReal(48, scale);
    }
    void lineWeight(int lw) const {
        dxfInt(370, lw);
    }

    void coord(int gc, double x, double y, double z=0) const {
        dxfReal(gc, x);
        dxfReal(gc+10, y);
        dxfReal(gc+20, z);
    }

    void coordTriplet(int gc, const double* value) const {
        if (value) {
            dxfReal(gc, *value++);
            dxfReal(gc+10, *value++);
            dxfReal(gc+20, *value++);
        }
    }

    void resetHandle() const {
        m_handle = 1;
    }

    /**
     * Writes a unique handle and returns it.
     */
    unsigned long handle(int gc=5) const {
        // handle has to be hex
        dxfHex(gc, m_handle);
        return m_handle++;
    }

    /**
     * @return Next handle that will be written.
     */
    unsigned long getNextHandle() const {
        return m_handle;
    }
    
    /**
     * Increases handle, so that the handle returned remains available.
     */
    unsigned long incHandle() const {
        return m_handle++;
    }

    /**
     * Sets the handle of the model space. Entities refer to 
     * this handle.
     */
    void setModelSpaceHandle(unsigned long h) {
        modelSpaceHandle = h;
    }

    unsigned long getModelSpaceHandle() {
        return modelSpaceHandle;
    }

    /**
     * Sets the handle of the paper space. Some special blocks refer to 
     * this handle.
     */
    void setPaperSpaceHandle(unsigned long h) {
        paperSpaceHandle = h;
    }

    unsigned long getPaperSpaceHandle() {
        return paperSpaceHandle;
    }

    /**
     * Sets the handle of the paper space 0. Some special blocks refer to 
     * this handle.
     */
    void setPaperSpace0Handle(unsigned long h) {
        paperSpace0Handle = h;
    }

    unsigned long getPaperSpace0Handle() {
        return paperSpace0Handle;
    }

    /**
     * Must be overwritten by the implementing class to write a
     * real value to the file.
     *
     * @param gc Group code.
     * @param value The real value.
     */
    virtual void dxfReal(int gc, double value) const = 0;

    /**
     * Must be overwritten by the implementing class to write an
     * int value to the file.
     *
     * @param gc Group code.
     * @param value The int value.
     */
    virtual void dxfInt(int gc, int value) const = 0;

    /**
     * Can be overwritten by the implementing class to write a
     * bool value to the file.
     *
     * @param gc Group code.
     * @param value The bool value.
     */
    virtual void dxfBool(int gc, bool value) const {
        dxfInt(gc, (int)value);
    }

    /**
     * Must be overwritten by the implementing class to write an
     * int value (hex) to the file.
     *
     * @param gc Group code.
     * @param value The int value.
     */
    virtual void dxfHex(int gc, int value) const = 0;

    /**
     * Must be overwritten by the implementing class to write a
     * string to the file.
     *
     * @param gc Group code.
     * @param value The string.
     */
    virtual void dxfString(int gc, const char* value) const = 0;

    /**
     * Must be overwritten by the implementing class to write a
     * string to the file.
     *
     * @param gc Group code.
     * @param value The string.
     */
    virtual void dxfString(int gc, const std::string& value) const = 0;

protected:
    mutable unsigned long m_handle;
    mutable unsigned long modelSpaceHandle;
    mutable unsigned long paperSpaceHandle;
    mutable unsigned long paperSpace0Handle;

    /**
     * DXF version to be created.
     */
    DL_Codes::version version;
private:
};

#endif
