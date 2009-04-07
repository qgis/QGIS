/* 
 gaiaaux.h -- Gaia common utility functions
  
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

#ifdef DLL_EXPORT
#define GAIAAUX_DECLARE __declspec(dllexport)
#else
#define GAIAAUX_DECLARE extern
#endif

#ifndef _GAIAAUX_H
#define _GAIAAUX_H

#ifdef __cplusplus
extern "C"
{
#endif

/* function prototipes */

    GAIAAUX_DECLARE const char *gaiaGetLocaleCharset ();
    GAIAAUX_DECLARE int gaiaConvertCharset (char **buf, const char *fromCs,
					    const char *toCs);
    GAIAAUX_DECLARE int gaiaToUTF8 (char **buf, const char *fromCs,
				    const char *toCs);
    GAIAAUX_DECLARE void *gaiaCreateUTF8Converter (const char *fromCS);
    GAIAAUX_DECLARE void gaiaFreeUTF8Converter (void *cvtCS);
    GAIAAUX_DECLARE char *gaiaConvertToUTF8 (void *cvtCS, const char *buf,
					     int len, int *err);
    GAIAAUX_DECLARE int gaiaIsReservedSqliteName (const char *name);
    GAIAAUX_DECLARE int gaiaIsReservedSqlName (const char *name);
    GAIAAUX_DECLARE int gaiaIllegalSqlName (const char *name);
    GAIAAUX_DECLARE void gaiaCleanSqlString (char *value);

#ifdef __cplusplus
}
#endif

#endif				/* _GAIAAUX_H */
