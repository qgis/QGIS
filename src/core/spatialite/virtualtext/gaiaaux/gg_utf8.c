/*

 gg_utf8.c -- locale charset handling

 version 2.3, 2008 October 13

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------

 Version: MPL 1.1/GPL 2.0/LGPL 2.1

 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri

Portions created by the Initial Developer are Copyright (C) 2008
the Initial Developer. All Rights Reserved.

Contributor(s):

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef __MINGW32__
#define LIBICONV_STATIC
#include <iconv.h>
#define LIBCHARSET_STATIC
#include <localcharset.h>
#else /* not MINGW32 */
#ifdef __APPLE__
#include <iconv.h>
#include <localcharset.h>
#else /* not Mac OsX */
#include <iconv.h>
#ifndef _MSC_VER
#include <langinfo.h>
#else
#include <locale.h>
#endif
#endif
#endif
#include <spatialite/gaiaaux.h>

#ifndef _MSC_VER
GAIAAUX_DECLARE const char *
gaiaGetLocaleCharset()
{
  /* indentifies the locale charset */
#if defined(__MINGW32__)
  return locale_charset();
#else /* not MINGW32 */
#ifdef __APPLE__
  return locale_charset();
#else /* not Mac OsX */
  return nl_langinfo( CODESET );
#endif
#endif
}
#endif

GAIAAUX_DECLARE
int
gaiaConvertCharset( char **buf, const char *fromCs, const char *toCs )
{
  /* converting a string from a charset to another "by-the-fly" */
  char utf8buf[65536];
#if defined(__MINGW32__)
  const char *pBuf;
  int len;
  int utf8len;
#else /* not MINGW32 */
  char *pBuf;
  size_t len;
  size_t utf8len;
#endif
  char *pUtf8buf;
  iconv_t cvt = iconv_open( toCs, fromCs );
  if ( cvt == ( iconv_t ) - 1 )
    goto unsupported;
  len = strlen( *buf );
  utf8len = 65536;
  pBuf = *buf;
  pUtf8buf = utf8buf;
  if ( iconv( cvt, &pBuf, &len, &pUtf8buf, &utf8len ) < 0 )
    goto error;
  utf8buf[65536 - utf8len] = '\0';
  memcpy( *buf, utf8buf, ( 65536 - utf8len ) + 1 );
  iconv_close( cvt );
  return 1;
error:
  iconv_close( cvt );
unsupported:
  return 0;
}

GAIAAUX_DECLARE void *
gaiaCreateUTF8Converter( const char *fromCS )
{
  /* creating an UTF8 converter and returning on opaque reference to it */
  iconv_t cvt = iconv_open( "UTF-8", fromCS );
  if ( cvt == ( iconv_t ) - 1 )
    return NULL;
  return cvt;
}

GAIAAUX_DECLARE void
gaiaFreeUTF8Converter( void *cvtCS )
{
  /* destroyng an UTF8 converter */
  if ( cvtCS )
    iconv_close( cvtCS );
}

GAIAAUX_DECLARE char *
gaiaConvertToUTF8( void *cvtCS, const char *buf, int buflen, int *err )
{
  /* converting a string to UTF8 */
  char *utf8buf = 0;
#if defined(__MINGW32__) || defined(_MSC_VER)
  const char *pBuf;
  int len;
  int utf8len;
#else
  char *pBuf;
  size_t len;
  size_t utf8len;
#endif
  int maxlen = buflen * 4;
  char *pUtf8buf;
  *err = 0;
  if ( !cvtCS )
  {
    *err = 1;
    return NULL;
  }
  utf8buf = malloc( maxlen );
  len = buflen;
  utf8len = maxlen;
  pBuf = ( char * ) buf;
  pUtf8buf = utf8buf;
  if ( iconv( cvtCS, &pBuf, &len, &pUtf8buf, &utf8len ) < 0 )
  {
    free( utf8buf );
    *err = 1;
    return NULL;
  }
  utf8buf[maxlen - utf8len] = '\0';
  return utf8buf;
}
