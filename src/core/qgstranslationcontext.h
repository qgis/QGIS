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
#include "qgis.h"

class QgsProject;

/**
 * \ingroup core
 * \class QgsTranslationContext
 * \brief used for the collecting of strings of .qgs to be translated and writing of ts file
 *
 * \since QGIS 3.2
 */

class CORE_EXPORT QgsTranslationContext
{
    struct TranslatableObject
    {
      QString context;
      QString source;
    };

  public:

    /**
     * Constructor
     */
    QgsTranslationContext( );

    /**
     * Returns the project
     * \see setProject()
     */
    QgsProject *project() const;

    /**
     * Sets the \a project where the translation need to be done for
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
     * Sets the \a name of the TS file
     *
     * \see fileName()
     */
    void setFileName( const QString &fileName );

    /**
     * Registers the \a string to be translated
     *
     * \param translationString name and path of the object need to be translated
     * \param layerName the name of the layer
     */
    void registerTranslation( const QString &context, const QString &source );

    /**
     * Writes the Ts-file
     */
    void writeTsFile( const QString &locale );

  private:

    QgsProject *mProject = nullptr;
    QString mFileName;
    QList < TranslatableObject > mTranslatableObjects;

};

#endif // QGSTRANSLATIONCONTEXT_H
