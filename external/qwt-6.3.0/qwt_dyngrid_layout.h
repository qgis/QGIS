/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_DYNGRID_LAYOUT_H
#define QWT_DYNGRID_LAYOUT_H

#include "qwt_global.h"
#include <qlayout.h>

template< typename T > class QList;

/*!
   \brief The QwtDynGridLayout class lays out widgets in a grid,
         adjusting the number of columns and rows to the current size.

   QwtDynGridLayout takes the space it gets, divides it up into rows and
   columns, and puts each of the widgets it manages into the correct cell(s).
   It lays out as many number of columns as possible (limited by maxColumns()).
 */

class QWT_EXPORT QwtDynGridLayout : public QLayout
{
    Q_OBJECT
  public:
    explicit QwtDynGridLayout( QWidget*, int margin = 0, int spacing = -1 );
    explicit QwtDynGridLayout( int spacing = -1 );

    virtual ~QwtDynGridLayout();

    virtual void invalidate() QWT_OVERRIDE;

    void setMaxColumns( uint maxColumns );
    uint maxColumns() const;

    uint numRows () const;
    uint numColumns () const;

    virtual void addItem( QLayoutItem* ) QWT_OVERRIDE;

    virtual QLayoutItem* itemAt( int index ) const QWT_OVERRIDE;
    virtual QLayoutItem* takeAt( int index ) QWT_OVERRIDE;
    virtual int count() const QWT_OVERRIDE;

    void setExpandingDirections( Qt::Orientations );
    virtual Qt::Orientations expandingDirections() const QWT_OVERRIDE;
    QList< QRect > layoutItems( const QRect&, uint numColumns ) const;

    virtual int maxItemWidth() const;

    virtual void setGeometry( const QRect& ) QWT_OVERRIDE;

    virtual bool hasHeightForWidth() const QWT_OVERRIDE;
    virtual int heightForWidth( int ) const QWT_OVERRIDE;

    virtual QSize sizeHint() const QWT_OVERRIDE;

    virtual bool isEmpty() const QWT_OVERRIDE;
    uint itemCount() const;

    virtual uint columnsForWidth( int width ) const;

  protected:

    void layoutGrid( uint numColumns,
        QVector< int >& rowHeight, QVector< int >& colWidth ) const;

    void stretchGrid( const QRect& rect, uint numColumns,
        QVector< int >& rowHeight, QVector< int >& colWidth ) const;

  private:
    void init();
    int maxRowWidth( int numColumns ) const;

    class PrivateData;
    PrivateData* m_data;
};

#endif
