/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * QwtPolar Widget Library
 * Copyright (C) 2008   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_polar_itemdict.h"

class QwtPolarItemDict::PrivateData
{
public:
    class ItemList: public QList<QwtPolarItem *>
    {
    public:
        void insertItem( QwtPolarItem *item )
        {
            if ( item == NULL )
                return;

            // Unfortunately there is no inSort operation
            // for lists in Qt4. The implementation below
            // is slow, but there shouldn't be many plot items.

            QList<QwtPolarItem *>::Iterator it;
            for ( it = begin(); it != end(); ++it )
            {
                if ( *it == item )
                    return;

                if ( ( *it )->z() > item->z() )
                {
                    insert( it, item );
                    return;
                }
            }
            append( item );
        }

        void removeItem( QwtPolarItem *item )
        {
            if ( item == NULL )
                return;

            int i = 0;

            QList<QwtPolarItem *>::Iterator it;
            for ( it = begin(); it != end(); ++it )
            {
                if ( item == *it )
                {
                    removeAt( i );
                    return;
                }
                i++;
            }
        }
    };

    ItemList itemList;
    bool autoDelete;
};

/*!
   Constructor

   Auto deletion is enabled.
   \sa setAutoDelete, attachItem
*/
QwtPolarItemDict::QwtPolarItemDict()
{
    d_data = new QwtPolarItemDict::PrivateData;
    d_data->autoDelete = true;
}

/*!
   Destructor

   If autoDelete is on, all attached items will be deleted
   \sa setAutoDelete, autoDelete, attachItem
*/
QwtPolarItemDict::~QwtPolarItemDict()
{
    detachItems( QwtPolarItem::Rtti_PolarItem, d_data->autoDelete );
    delete d_data;
}

/*!
   En/Disable Auto deletion

   If Auto deletion is on all attached plot items will be deleted
   in the destructor of QwtPolarItemDict. The default value is on.

   \sa autoDelete, attachItem
*/
void QwtPolarItemDict::setAutoDelete( bool autoDelete )
{
    d_data->autoDelete = autoDelete;
}

/*!
   \return true if auto deletion is enabled
   \sa setAutoDelete, attachItem
*/
bool QwtPolarItemDict::autoDelete() const
{
    return d_data->autoDelete;
}

/*!
  Insert a plot item

  \param item PlotItem
  \sa removeItem()
 */
void QwtPolarItemDict::insertItem( QwtPolarItem *item )
{
    d_data->itemList.insertItem( item );
}

/*!
  Remove a plot item

  \param item PlotItem
  \sa insertItem()
 */
void QwtPolarItemDict::removeItem( QwtPolarItem *item )
{
    d_data->itemList.removeItem( item );
}

/*!
   Detach items from the dictionary

   \param rtti In case of QwtPolarItem::Rtti_PlotItem detach all items
               otherwise only those items of the type rtti.
   \param autoDelete If true, delete all detached items
*/
void QwtPolarItemDict::detachItems( int rtti, bool autoDelete )
{
    PrivateData::ItemList list = d_data->itemList;
    QwtPolarItemIterator it = list.begin();
    while ( it != list.end() )
    {
        QwtPolarItem *item = *it;

        ++it; // increment before removing item from the list

        if ( rtti == QwtPolarItem::Rtti_PolarItem || item->rtti() == rtti )
        {
            item->attach( NULL );
            if ( autoDelete )
                delete item;
        }
    }
}

/*!
  \brief A QwtPolarItemList of all attached plot items.

  \return List of all attached plot items.
  \note Use caution when iterating these lists, as removing/detaching
        an item will invalidate the iterator.
        Instead you can place pointers to objects to be
        removed in a removal list, and traverse that list later.
*/
const QwtPolarItemList &QwtPolarItemDict::itemList() const
{
    return d_data->itemList;
}
