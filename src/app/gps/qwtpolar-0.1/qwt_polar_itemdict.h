/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEMDICT_H
#define QWT_POLAR_ITEMDICT_H

/*! \file !*/

#include "qwt_polar_global.h"
#include "qwt_polar_item.h"

#if QT_VERSION < 0x040000
#include <qvaluelist.h>
typedef QValueListConstIterator<QwtPolarItem *> QwtPolarItemIterator;
/// \var typedef QValueList< QwtPolarItem *> QwtPolarItemList
/// \brief See QT 3.x assistant documentation for QValueList
typedef QValueList<QwtPolarItem *> QwtPolarItemList;
#else
#include <qlist.h>
typedef QList<QwtPolarItem *>::ConstIterator QwtPolarItemIterator;
/// \var typedef QList< QwtPolarItem *> QwtPolarItemList
/// \brief See QT 4.x assistant documentation for QList
typedef QList<QwtPolarItem *> QwtPolarItemList;
#endif

/*!
  \brief A dictionary for polar plot items

  QwtPolarItemDict organizes polar plot items in increasing z-order.
  If autoDelete() is enabled, all attached items will be deleted
  in the destructor of the dictionary.

  \sa QwtPolarItem::attach(), QwtPolarItem::detach(), QwtPolarItem::z()
*/
class QWT_POLAR_EXPORT QwtPolarItemDict
{
  public:
    explicit QwtPolarItemDict();
    ~QwtPolarItemDict();

    void setAutoDelete( bool );
    bool autoDelete() const;

    const QwtPolarItemList& itemList() const;

    void detachItems( int rtti = QwtPolarItem::Rtti_PolarItem,
                      bool autoDelete = true );

  private:
    friend class QwtPolarItem;

    void attachItem( QwtPolarItem *, bool );

    class PrivateData;
    PrivateData *d_data;
};

#endif
