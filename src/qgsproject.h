/***************************************************************************
                                  qgsproject.h

                      Implements persistent project state.
 
                              -------------------
  begin                : July 23, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSPROJECT_H
#define QGSPROJECT_H

#include <memory>

#include <qmap.h>
#include <qvaluelist.h>
#include <qvariant.h>

#include <qgsscalecalculator.h>


class QFileInfo;


/** Reads and writes project states.
   

  @note

  Has two general kinds of state to make persistent.  (I.e., to read and
  write.)  First, Qgis proprietary information.  Second plug-in information.

  A singleton since there shall only be one active project at a time; and
  provides canonical location for plug-ins and main app to find/set
  properties.

  Might want to consider moving from Singleton; i.e., allowing more than one
  project.  Just as the GIMP can have simultaneous multiple images, perhaps
  qgis can one day have simultaneous multiple projects.

*/
class QgsProject
{
public:

    /**
       @todo XXX Should have semantics for saving project if dirty as last gasp?
    */
    ~QgsProject();

    /// access to canonical QgsProject instance
    static QgsProject * instance();

    /**
       Every project has an associated title string

       @todo However current dialogs don't allow setting of it yet
     */
    //@{
    void title( QString const & title );

    /** returns title */
    QString const & title() const;
    //@}

    /**
     * Gets the currently select map units
     * @return int which matches a value in the units enum in QgsScaleCalculator::units
     */
    QgsScaleCalculator::units mapUnits() const;


    /**
     * Set the map units
     * @param new units type
     */
    void mapUnits(QgsScaleCalculator::units u);


    /**
       the dirty flag is true if the project has been modified since the last
       write()
     */
    //@{
    bool QgsProject::dirty() const;

    void QgsProject::dirty( bool b );
    //@}


    /**
       Every project has an associated file that contains its XML
     */
    //@{
    void filename( QString const & name );

    /** returns file name */
    QString filename() const;
    //@}


    /** read project file

       @note Any current plug-in state is erased

       @note dirty set to false after successful invocation

       @note file name argument implicitly sets file

       (Is that really desirable behavior?  Maybe prompt to save before
       reading new one?)

       Should we presume the file is opened elsewhere?  Or do we open it
       ourselves?

       XXX How to best get modify access to Qgis state?  Actually we can finagle
       that by searching for qgisapp in object hiearchy.

       @note

       - Gets the extents
       - Creates maplayers
       - Registers maplayers

       @note it's presumed that the caller has already reset the map canvas, map registry, and legend

       @exception

     */
    //@{
    bool read( QFileInfo const & file );
    bool read( );
    //@}

    /** write project file

       XXX How to best get read access to Qgis state?  Actually we can finagle
       that by searching for qgisapp in object hiearchy.

       @note file name argument implicitly sets file

       @note dirty set to false after successful invocation
     */
    //@{
    bool write( QFileInfo const & file );
    bool write( );
    //@}


    /// syntactic sugar for property lists
    typedef QMap< QString, QValueList<QVariant> > Properties;

    /** extra properties, typically added by plug-ins

       @note 

       Use a QMap because the key string is a sort of "name space".  Each
       plug-in, provider, or whatever would presumably store its respective
       properties under a name well known and specific to itself.  E.g., open
       modeller might use <code>"QgsProject::instance()->properties()["openmodeller"]</code>.

       @todo "properties" is, overall, a good name; but that might imply that
       the qgis specific state properites are different since they aren't
       accessible here.  Actually, what if we make "qgis" yet another
       namespace that stores its state in the properties list?  E.g., 
       QgsProject::instance()->properties()["qgis"]  ?
     */
    Properties & properties();

private:

    QgsProject(); // private 'cause it's a singleton

    QgsProject( QgsProject const & ); // private 'cause it's a singleton

    struct Imp;

    /// implementation handle
    std::auto_ptr<Imp> imp_;

    static QgsProject * theProject_;

}; // QgsProject

#endif
