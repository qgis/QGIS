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

#include "../../src/qgsidentifyresults.h"

class QgsGrassProvider;
class QgsGrassEdit;
#include "qgsgrassattributesbase.h"

/*! \class QgsGrassAttributes
 *  \brief GRASS attributes.
 *
 */
class QgsGrassAttributes: public QgsGrassAttributesBase
{
    Q_OBJECT;

public:
    //! Constructor
    QgsGrassAttributes ( QgsGrassEdit *edit, QgsGrassProvider *provider, int line, 
	                 QWidget * parent = 0, const char * name = 0, 
			 WFlags f = Qt::WType_Dialog | Qt::WStyle_Customize | Qt::WStyle_Tool );

    //! Destructor
    ~QgsGrassAttributes();

    //! Add tab for one field:cat pair, returns tab id
    int addTab ( const QString & label );

    //! Set field
    void setField ( int tab, int field );

    //! Set Category
    void setCat ( int tab, const QString & name, int cat );

    //! Add attribute (but no category)
    void addAttribute ( int tab, const QString &name, const QString &value, const QString &type );
    
    //! Add text in one row usually warning
    void addTextRow ( int tab, const QString &text );
    
    //! Set Line (must be used if the line is rewritten)
    void setLine ( int line );

public slots:
    //! Update DB for current tab
    void updateAttributes ( void );

    //! Add new category
    void addCat ( void );

    //! Add new category
    void deleteCat ( void );

    //! Called if tab is changed
    void tabChanged ( QWidget *widget );

private:
    //! Pointer to vector provider 
    QgsGrassProvider *mProvider;

    QgsGrassEdit *mEdit;

    int mLine;

    void restorePosition(void);
    
    void saveWindowLocation(void);
};

#endif // QGSGRASSATTRIBUTES_H
