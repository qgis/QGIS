/****************************************************************************
** $Id: dl_writer_ascii.cpp 242 2004-04-12 22:39:43Z andrew $
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

#include <cstdio>
#include <cstring>

#include "dl_writer_ascii.h"
#include "dl_exception.h"


/**
 * Closes the output file.
 */
void DL_WriterA::close() const
{
  m_ofile.close();
}


/**
 * @retval true Opening file has failed.
 * @retval false Otherwise.
 */
bool DL_WriterA::openFailed() const
{
  return m_ofile.fail();
}



/**
 * Writes a real (double) variable to the DXF file.
 *
 * @param gc Group code.
 * @param value Double value
 */
void DL_WriterA::dxfReal( int gc, double value ) const
{
  char str[256];
  sprintf( str, "%.16f", value );

  // fix for german locale:
  strReplace( str, ',', '.' );

  // Cut away those zeros at the end:
  bool dot = false;
  int end = -1;
  for ( unsigned int i = 0; i < strlen( str ); ++i )
  {
    if ( str[i] == '.' )
    {
      dot = true;
      end = i + 2;
      continue;
    }
    else if ( dot && str[i] != '0' )
    {
      end = i + 1;
    }
  }
  if ( end > 0 && end < ( int )strlen( str ) )
  {
    str[end] = '\0';
  }

  dxfString( gc, str );
  m_ofile.flush();
}



/**
 * Writes an int variable to the DXF file.
 *
 * @param gc Group code.
 * @param value Int value
 */
void DL_WriterA::dxfInt( int gc, int value ) const
{
  m_ofile << ( gc < 10 ? "  " : ( gc < 100 ? " " : "" ) ) << gc << "\n"
  << value << "\n";
}



/**
 * Writes a hex int variable to the DXF file.
 *
 * @param gc Group code.
 * @param value Int value
 */
void DL_WriterA::dxfHex( int gc, int value ) const
{
  char str[12];
  sprintf( str, "%0X", value );
  dxfString( gc, str );
}



/**
 * Writes a string variable to the DXF file.
 *
 * @param gc Group code.
 * @param value String
 */
void DL_WriterA::dxfString( int gc, const char* value ) const
{
  if ( value == NULL )
  {
#ifndef __GCC2x__
    throw DL_NullStrExc();
#endif
  }
  m_ofile << ( gc < 10 ? "  " : ( gc < 100 ? " " : "" ) ) << gc << "\n"
  << value << "\n";
}



void DL_WriterA::dxfString( int gc, const string& value ) const
{
  m_ofile << ( gc < 10 ? "  " : ( gc < 100 ? " " : "" ) ) << gc << "\n"
  << value << "\n";
}


/**
 * Replaces every occurence of src with dest in the null terminated str.
 */
void DL_WriterA::strReplace( char* str, char src, char dest )
{
  size_t i;
  for ( i = 0; i < strlen( str ); i++ )
  {
    if ( str[i] == src )
    {
      str[i] = dest;
    }
  }
}

