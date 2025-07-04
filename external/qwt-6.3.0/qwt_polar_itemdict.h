/******************************************************************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_POLAR_ITEMDICT_H
#define QWT_POLAR_ITEMDICT_H

#include "qwt_global.h"
#include "qwt_polar_item.h"
#include <qlist.h>

typedef QList< QwtPolarItem* >::ConstIterator QwtPolarItemIterator;
typedef QList< QwtPolarItem* > QwtPolarItemList;

/*!
   \brief A dictionary for polar plot items

   QwtPolarItemDict organizes polar plot items in increasing z-order.
   If autoDelete() is enabled, all attached items will be deleted
   in the destructor of the dictionary.

   \sa QwtPolarItem::attach(), QwtPolarItem::detach(), QwtPolarItem::z()
 */
class QWT_EXPORT QwtPolarItemDict
{
  public:
    explicit QwtPolarItemDict();
    ~QwtPolarItemDict();

    void setAutoDelete( bool );
    bool autoDelete() const;

    const QwtPolarItemList& itemList() const;

    void detachItems( int rtti = QwtPolarItem::Rtti_PolarItem,
        bool autoDelete = true );

  protected:
    void insertItem( QwtPolarItem* );
    void removeItem( QwtPolarItem* );

  private:
    class PrivateData;
    PrivateData* m_data;
};

#endif
