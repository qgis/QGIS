/***************************************************************************
    qgsgrassattributes.h  -  Edit vector attributes
                             -------------------
    begin                : July, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSATTRIBUTES_H
#define QGSGRASSATTRIBUTES_H

/* First attribute in the table is always field, second attribute is category */

#include "ui_qgsgrassattributesbase.h"
#include "qgsgrassedit.h"

class QgsGrassProvider;
class QgsGrassEdit;

class QEvent;
class QTableWidget;

class QgsGrassAttributesKeyPress : public QObject
{
    Q_OBJECT

  public:
    QgsGrassAttributesKeyPress( QTableWidget *tab );
    ~QgsGrassAttributesKeyPress();

  protected:
    bool eventFilter( QObject *o, QEvent *e );

  private:
    QTableWidget *mTable;
};


/*! \class QgsGrassAttributes
 *  \brief GRASS attributes.
 *
 */
class QgsGrassAttributes: public QDialog, private Ui::QgsGrassAttributesBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassAttributes( QgsGrassEdit *edit, QgsGrassProvider *provider, int line,
                        QWidget * parent = 0, const char * name = 0,
                        Qt::WFlags f = Qt::Window );
    //Qt::WFlags f = Qt::WStyle_Customize | Qt::WStyle_DialogBorder | Qt::WStyle_Title | Qt::WType_Dialog | Qt::WStyle_Tool);

    //! Destructor
    ~QgsGrassAttributes();

    void setRowReadOnly( QTableWidget* table, int col, bool ro );

    //! Add tab for one field:cat pair, returns tab id
    int addTab( const QString & label );

    //! Set field
    void setField( int tab, int field );

    //! Set Category
    void setCat( int tab, const QString & name, int cat );

    //! Add attribute (but no category)
    void addAttribute( int tab, const QString &name, const QString &value, const QString &type );

    //! Add text in one row usually warning
    void addTextRow( int tab, const QString &text );

    //! Set Line (must be used if the line is rewritten)
    void setLine( int line );

    //! Reset buttons
    void resetButtons();

  public slots:
    //! Update DB for current tab
    void on_updateButton_clicked() { updateAttributes(); }
    void updateAttributes();

    //! Add new category
    void on_newButton_clicked() { addCat(); }
    void addCat();

    //! Add new category
    void on_deleteButton_clicked() { deleteCat(); }
    void deleteCat();

    //! Called if tab is changed
    void tabChanged( int index );

    //! Column size changed
    void columnSizeChanged( int section, int oldSize, int newSize );

    //! Remove all tabs
    void clear();

    //! Enable/disable buttons depending on the category mode
    void setCategoryMode( QgsGrassEdit::CatMode mode, const QString &cat );

  private:
    //! Pointer to vector provider
    QgsGrassProvider *mProvider;

    QgsGrassEdit *mEdit;

    int mLine;

    void restorePosition( void );

    void saveWindowLocation( void );
};

#endif // QGSGRASSATTRIBUTES_H
