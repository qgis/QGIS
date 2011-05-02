/***************************************************************************
 *   Copyright (C) 2007 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QGSTIPFACTORY
#define QGSTIPFACTORY

#include "qgstip.h"
#include <QList>

/** \ingroup app
* \brief A factory class to serve up tips to the user.
* Tips can be generic, in which case they make no mention of
* gui dialogs etc, or gui-specific in which case they may allude
* to features of the graphical user interface.
* @see also QgsTipOfTheDay, QgsTip
*/

class QgsTipFactory : public QObject
{
    Q_OBJECT //used for tr() so we don't need to do QObject::tr()
  public:
    /** Constructor */
    QgsTipFactory();
    /** Destructor */
    ~QgsTipFactory();
    /** Get a random tip (generic or gui-centric)
     * @return An QgsTip containing the tip
     */
    QgsTip getTip();
    /** Get a specific tip (generic or gui-centric).
     * @param thePosition The tip returned will be based on the
     *        number passed in as thePosition. If the
     *        position is invalid, an empty string will be
     *        returned.
     * @return An QgsTip containing the tip
     */
    QgsTip getTip( int thePosition );
    /** Get a random generic tip
     * @return An QgsTip containing the tip
     */
    QgsTip getGenericTip();
    /** Get a random gui-centric tip
     * @return An QgsTip  containing the tip
     */
    QgsTip getGuiTip();

    int position( QgsTip );
    int count();

  private:
    void addGenericTip( QgsTip );
    void addGuiTip( QgsTip );
    int randomNumber( int theMax );
    //@TODO move tipts into a sqlite db
    QList <QgsTip> mGenericTips;
    QList <QgsTip> mGuiTips;
    QList <QgsTip> mAllTips;
};
#endif //QGSTIPFACTORY

