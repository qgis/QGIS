/***************************************************************************
     testqgslabelpropertydialog.cpp
     ------------------------------
    Date                 : Feb 2020
    Copyright            : (C) 2020 by Paul Blottiere
    Email                : blottiere dot paul at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>

#include "qgsapplication.h"
#include "qgslabelpropertydialog.h"

class TestQgsLabelPropertyDialog : public QObject
{
    Q_OBJECT

  public:
    TestQgsLabelPropertyDialog() = default;

  private:

  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();

    }

    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }

    void test()
    {
      // TODO
    }
};

QGSTEST_MAIN( TestQgsLabelPropertyDialog )
#include "testqgslabelpropertydialog.moc"
