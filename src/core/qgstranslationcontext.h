/***************************************************************************
  qgstranslationcontext.h

 ---------------------
 begin                : 23.5.2018
 copyright            : (C) 2018 by David Signer
 email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTRANSLATIONCONTEXT_H
#define QGSTRANSLATIONCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QList>
#include <QString>


class QgsProject;

/**
 * \ingroup core
 * \class QgsTranslationContext
 * \brief Used for the collecting of strings from projects for translation and creation of ts files.
 *
 * \since QGIS 3.4
 */

class CORE_EXPORT QgsTranslationContext
{

    /**
     * Object that could be translated by the QTranslator with the qm file.
     */
    struct TranslatableObject
    {
      QString context; //!< In what context the object is used
      QString source; //!< The original text of the object
    };

  public:

    /**
     * Constructor
     */
    QgsTranslationContext() = default;

    /**
     * Returns the project
     * \see setProject()
     */
    QgsProject *project() const;

    /**
     * Sets the \a project being translated.
     *
     * \see project()
     */
    void setProject( QgsProject *project );

    /**
     * Returns the TS fileName
     * \see setFileName()
     */
    QString fileName() const;

    /**
     * Sets the \a fileName of the TS file
     *
     * \see fileName()
     */
    void setFileName( const QString &fileName );

    /**
     * Registers the \a source to be translated. It's the text of the object needed to be translated.
     * The \a context defines in what context the object is used. Means layer name and sub category of object needed to be translated.
     */
    void registerTranslation( const QString &context, const QString &source );

    /**
     * Writes the Ts-file
     */
    void writeTsFile( const QString &locale ) const;

  private:

    QgsProject *mProject = nullptr;
    QString mFileName;
    QList < TranslatableObject > mTranslatableObjects;

};

#endif // QGSTRANSLATIONCONTEXT_H
