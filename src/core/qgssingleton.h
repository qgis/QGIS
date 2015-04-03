/***************************************************************************
    qgssingleton.h
     --------------------------------------
    Date                 : 24.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLETON_H
#define QGSSINGLETON_H

template <typename T>
class QgsSingleton
{
  public:
    static T* instance()
    {
      if ( sInstance == 0 )
      {
        sInstance = createInstance();
      }
      return sInstance;
    }

    static void cleanup()
    {
      delete sInstance;
      sInstance = 0;
    }

  protected:
    virtual ~QgsSingleton() {}

    explicit QgsSingleton()
    {
    }

  private:
    static T* sInstance;
    static T* createInstance()
    {
      return new T;
    }
};

template <typename T> T* QgsSingleton<T>::sInstance = 0;

#endif // QGSSINGLETON_H
