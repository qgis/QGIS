/***************************************************************************
  qgis_sip - QGIS SIP Macros

 ---------------------
 begin                : 4.5.2017
 copyright            : (C) 2017 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGIS_SIP_H
#define QGIS_SIP_H

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=keepreference#function-annotation-Transfer
 *
 * Example QgsVectorLayer::setDiagramRenderer
 */
#define SIP_TRANSFER

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=keepreference#function-annotation-TransferBack
 */
#define SIP_TRANSFERBACK

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=keepreference#function-annotation-TransferThis
 */
#define SIP_TRANSFERTHIS

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-Out
 */
#define SIP_OUT

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-In
 */
#define SIP_IN

/*
 * Combination of
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-In
 * and
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-Out
 */
#define SIP_INOUT

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#function-annotation-Factory
 */
#define SIP_FACTORY

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#class-annotation-PyName
 */
#define SIP_PYNAME(name)

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-KeepReference
 */
#define SIP_KEEPREFERENCE

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-Array
 */
#define SIP_ARRAY

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#argument-annotation-ArraySize
 */
#define SIP_ARRAYSIZE

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html#class-annotation-NoDefaultCtors
 */
#define SIP_NODEFAULTCTORS

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=deprecated#function-annotation-Deprecated
 */
#define SIP_DEPRECATED

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=constrained#argument-annotation-Constrained
 */
#define SIP_CONSTRAINED

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=external#class-annotation-External
 */
#define SIP_EXTERNAL

/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=allownone
 */
#define SIP_ALLOWNONE

/*
  * discard line
  */
#define SIP_SKIP

/*
  * force a private line to be written
  */
#define SIP_FORCE

/*
  * specify an alternative type for SIP argument or return value
  */
#define SIP_PYALTERNATIVETYPE(type)

/*
  * specify an alternative default value for SIP argument
  */
#define SIP_PYARGDEFAULT(value)

/*
  * remove argument in SIP method
  */
#define SIP_PYARGREMOVE


/*
 * http://pyqt.sourceforge.net/Docs/sip4/annotations.html?highlight=keepreference#function-annotation-ReleaseGIL
 */
#define SIP_RELEASEGIL

/*
 * Will insert a `%Feature feature` directive in sip files
 */
#define SIP_FEATURE(feature)

/*
 * Will insert a `%If feature` directive in sip files
 */
#define SIP_IF_FEATURE(feature)

/*
 * Will place the current line with an `%If feature` directive in sip file
 */
#define SIP_WHEN_FEATURE(feature)

/*
 * Convert to subclass code
 */
#define SIP_CONVERT_TO_SUBCLASS_CODE(code)

/*
 * Virtual error handler (/VirtualErrorHandler/)
 */
#define SIP_VIRTUALERRORHANDLER(name)

/*
 * Will insert a `%End` directive in sip files
 */
#define SIP_END

/*
 * Class level annotation for abstract classes
 */
#define SIP_ABSTRACT


#endif // QGIS_SIP_H
