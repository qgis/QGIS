/***************************************************************************
    qgsannotationregistry.h
    -----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONREGISTRY_H
#define QGSANNOTATIONREGISTRY_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsannotation.h"
#include "qgstextannotation.h"
#include "qgssvgannotation.h"
#include "qgshtmlannotation.h"
#include <QString>
#include <functional>

///@cond PRIVATE

// None of this is stable API!

//! Creates a new annotation object
typedef std::function < QgsAnnotation*() > QgsCreateAnnotationFunc;

/**
 * \class QgsAnnotationMetadata
 * \ingroup core
 * Metadata item for an annotation type within a QgsAnnotationRegistry.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAnnotationMetadata
{
  public:

    /**
     * Constructor for QgsAnnotationMetadata. \a typeName should be a unique string
     * identifying the annotation type.
     */
    QgsAnnotationMetadata( const QString &typeName, const QgsCreateAnnotationFunc &createFunc )
      : mTypeName( typeName )
      , mCreateFunc( createFunc )
    {}

    /**
     * Returns the annotation type.
     */
    QString type() const { return mTypeName; }

    /**
     * Creates a new annotation of the associated type.
     */
    QgsAnnotation *createAnnotation() const { return mCreateFunc ? mCreateFunc() : nullptr ; }

  private:

    QString mTypeName;
    QgsCreateAnnotationFunc mCreateFunc = nullptr;

    QgsAnnotationMetadata() = default;
    friend class QMap< QString, QgsAnnotationMetadata >;

};

/**
 * \class QgsAnnotationRegistry
 * \ingroup core
 * Handles registration and creation of annotation item types.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAnnotationRegistry
{

  public:

    /**
     * Constructor for QgsAnnotationRegistry. The registry is automatically populated
     * with several standard annotation types.
     */
    QgsAnnotationRegistry()
    {
      addAnnotationType( QgsAnnotationMetadata( QStringLiteral( "TextAnnotationItem" ), QgsTextAnnotation::create ) );
      addAnnotationType( QgsAnnotationMetadata( QStringLiteral( "HtmlAnnotationItem" ), QgsHtmlAnnotation::create ) );
      addAnnotationType( QgsAnnotationMetadata( QStringLiteral( "SVGAnnotationItem" ), QgsSvgAnnotation::create ) );
    }

    /**
     * Adds a new annotation type to the registry. Returns TRUE if adding the type
     * was successful, or FALSE if an annotation with duplicate type already exists
     * in the registry.
     */
    bool addAnnotationType( const QgsAnnotationMetadata &metadata )
    {
      if ( mMetadata.contains( metadata.type() ) )
        return false;

      mMetadata.insert( metadata.type(), metadata );
      return true;
    }

    /**
     * Creates a new annotation of the specified type. Returns NULLPTR if no
     * matching annotations types were found.
     */
    QgsAnnotation *create( const QString &typeName ) const
    {
      if ( !mMetadata.contains( typeName ) )
        return nullptr;

      return mMetadata.value( typeName ).createAnnotation();
    }

  private:

    QMap< QString, QgsAnnotationMetadata > mMetadata;

};

///@endcond

#endif // QGSANNOTATIONREGISTRY_H
