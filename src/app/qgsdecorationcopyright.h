/***************************************************************************
                          plugin.h
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOPYRIGHTLABELPLUGIN
#define QGSCOPYRIGHTLABELPLUGIN

#include "qgsdecorationitem.h"

#include <QColor>
#include <QFont>
#include <QObject>
class QAction;
class QPainter;

class QgsDecorationCopyrightDialog;

class APP_EXPORT QgsDecorationCopyright : public QgsDecorationItem
{
    Q_OBJECT
  public:

    //! Constructor
    QgsDecorationCopyright( QObject* parent = NULL );
    //! Destructor
    virtual ~QgsDecorationCopyright();

  public slots:
    //! set values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! render the copyright label
    void render( QPainter * ) override;

  private:
    //! This is the font that will be used for the copyright label
    QFont mQFont;
    //! This is the string that will be used for the copyright label
    QString mLabelQString;
    //! This is the color for the copyright label
    QColor mLabelQColor;
    //! Placement of the copyright label - index and translated label names
    int mPlacementIndex;
    QStringList mPlacementLabels;

    friend class QgsDecorationCopyrightDialog;
};

#endif
