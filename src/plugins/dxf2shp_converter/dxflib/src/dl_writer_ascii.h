/****************************************************************************
** $Id: dl_writer_ascii.h 2719 2005-09-24 20:41:23Z andrew $
**
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
** Copyright (C) 2001 Robert J. Campbell Jr.
**
** This file is part of the dxflib project.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
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

#ifndef DL_WRITER_ASCII_H
#define DL_WRITER_ASCII_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "dl_writer.h"
#include <fstream>
#include <string>
using std::string;

/**
 * Implements functions defined in DL_Writer for writing low
 *   level DXF constructs to an ASCII format DXF file.
 *
 * @para fname File name of the file to be created.
 * @para version DXF version. Defaults to VER_2002.
 *
 * @todo What if \c fname is NULL?  Or \c fname can't be opened for
 * another reason?
 */
class DL_WriterA : public DL_Writer
{
  public:
    DL_WriterA( const char* fname, DL_Codes::version version = VER_2000 )
        : DL_Writer( version ), m_ofile( fname ) {}
    virtual ~DL_WriterA() {}

    bool openFailed() const;
    void close() const;
    void dxfReal( int gc, double value ) const;
    void dxfInt( int gc, int value ) const;
    void dxfHex( int gc, int value ) const;
    void dxfString( int gc, const char* value ) const;
    void dxfString( int gc, const string& value ) const;

    static void strReplace( char* str, char src, char dest );

  private:
    /**
     * DXF file to be created.
     */
    mutable std::ofstream m_ofile;

};

#endif

