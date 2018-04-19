/***************************************************************************
  qgsopenclutils.h - QgsOpenClUtils

 ---------------------
 begin                : 11.4.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPENCLUTILS_H
#define QGSOPENCLUTILS_H

#define SIP_NO_FILE

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#include <CL/cl2.hpp>


#include "qgis_core.h"
#include "qgis.h"

#include "cpl_conv.h"

/**
 * \ingroup core
 * \class QgsOpenClUtils
 * \brief The QgsOpenClUtils class is responsible for common OpenCL operations such as
 * - enable/disable opencl
 * - check opencl device availability and automatically choose the first GPU (TODO: let the user choose & override!)
 * - creating contexts
 * - loading program sources from standard locations
 * - build programs and log errors
 * \since QGIS 3.4
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsOpenClUtils
{
  public:

    enum ExceptionBehavior
    {
      Catch,  // Write errors in the message log and silently fail
      Throw   // Write errors in the message log and re-throw exceptions
    };

    static bool enabled();
    static bool available();
    static void setEnabled( bool enabled );
    static QString buildLog( cl::BuildError &e );
    static QString sourceFromPath( const QString &path );
    static QString sourceFromBaseName( const QString &baseName );
    static QLatin1String LOGMESSAGE_TAG;
    static QString errorText( const int errorCode );
    static cl::Program buildProgram( const cl::Context &context, const QString &source, ExceptionBehavior exceptionBehavior = Catch );
    static cl::Context context();
    static QString sourcePath();
    static void setSourcePath( const QString &value );

    /**
     * Tiny smart-pointer-like wrapper around CPLMalloc and CPLFree: this is needed because
     * OpenCL C++ API may throw exceptions
     */
    template <typename T>
    struct CPLAllocator
    {

      public:

        explicit CPLAllocator( unsigned long size ): mMem( ( T * )CPLMalloc( sizeof( T ) * size ) ) { }

        ~CPLAllocator()
        {
          CPLFree( ( void * )mMem );
        }

        void reset( T *newData )
        {
          if ( mMem )
            CPLFree( ( void * )mMem );
          mMem = newData;
        }

        void reset( unsigned long size )
        {
          reset( ( T * )CPLMalloc( sizeof( T ) *size ) );
        }

        T &operator* ()
        {
          return &mMem[0];
        }

        T *release()
        {
          T *tmpMem = mMem;
          mMem = nullptr;
          return tmpMem;
        }

        T &operator[]( const int index )
        {
          return mMem[index];
        }

        T *get()
        {
          return mMem;
        }

      private:

        T *mMem = nullptr;
    };


  private:
    QgsOpenClUtils();
    static void init();
    static bool sAvailable;
    static cl::Device sDevice;
    static cl::Platform sPlatform;
    static QLatin1String SETTINGS_KEY;
    static QString sSourcePath;
};



#endif // QGSOPENCLUTILS_H
