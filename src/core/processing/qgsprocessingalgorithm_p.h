/***************************************************************************
                         qgsprocessingalgorithm_p.h
                         ------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGALGORITHMP_H
#define QGSPROCESSINGALGORITHMP_H

#include <typeindex>
#include <unordered_map>

#include "qgis.h"
#include "qgis_core.h"

#include <QIcon>
#include <QString>
#include <QVariant>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


#ifndef CMAKE_SOURCE_DIR
#define CMAKE_SOURCE_DIR ""
#endif

///@cond PRIVATE

CORE_EXPORT std::unordered_map<std::type_index, QString> &algorithmSourceRegistry();

namespace QgsProcessingAlgorithmPrivate
{

  template<size_t N> struct QgsStringLiteral
  {
      constexpr QgsStringLiteral( const char ( &str )[N] )
      {
        for ( size_t i = 0; i < N; ++i )
          value[i] = str[i];
      }
      char value[N] = {};
  };

  constexpr size_t sFilePrefixLength = sizeof( CMAKE_SOURCE_DIR ) > 1 ? sizeof( CMAKE_SOURCE_DIR ) : 0;

  template<typename ClassType, int LineNum, QgsStringLiteral File> struct QgsSourceLocationRegistrar
  {
      static const bool registered;
      static bool doRegister()
      {
        std::unordered_map<std::type_index, QString> &registry = algorithmSourceRegistry();

        registry[std::type_index( typeid( ClassType ) )] = QString::fromUtf8( File.value ).mid( sFilePrefixLength ) + u":"_s + QString::number( LineNum );
        return true;
      }
  };

  // static instantiation triggers dynamic registration when the library loads
  template<typename ClassType, int LineNum, QgsStringLiteral File>
  const bool QgsSourceLocationRegistrar<ClassType, LineNum, File>::registered = QgsSourceLocationRegistrar<ClassType, LineNum, File>::doRegister();
} //namespace QgsProcessingAlgorithmPrivate

#define QGS_MARK_ALGORITHM_SOURCE ( void ) QgsProcessingAlgorithmPrivate::QgsSourceLocationRegistrar<std::remove_pointer_t<decltype( this )>, __LINE__, __FILE__>::registered;

///@endcond PRIVATE

#endif // QGSPROCESSINGALGORITHMP_H
