/***************************************************************************
 *   Copyright (C) 2007 by Tim Sutton   *
 *   tim@linfiniti.com   *
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

#ifndef OMGTIPFACTORY
#define OMGTIPFACTORY

#include "omgtip.h"
#include <QList>

/** \ingroup lib
* \brief A factory class to serve up tips to the user.
* Tips can be generic, in which case they make no mention of
* gui dialogs etc, or gui-secific in which case they may allude
* to features of the graphical user interface.
* @see also OmgTipOfTheDay, OmgTip
*/

class OMG_LIB_EXPORT OmgTipFactory : public QObject 
{
  Q_OBJECT; //used for tr() so we dont need to do QObject::tr()
  public:
    /** Constructor */
    OmgTipFactory();
    /** Destructor */
    ~OmgTipFactory();
    /** Get a random tip (generic or gui-centric)
     * @return An OmgTip containing the tip
     */
    OmgTip getTip();
    /** Get a specific tip (generic or gui-centric).
     * @param thePosition The tip returned will be based on the 
     *        number passed in as thePosition. If the
     *        position is invalid, an empty string will be 
     *        returned.
     * @return An OmgTip containing the tip
     */
    OmgTip getTip(int thePosition);
    /** Get a random generic tip
     * @return An OmgTip containing the tip
     */
    OmgTip getGenericTip();
    /** Get a random gui-centric tip
     * @return An OmgTip  containing the tip
     */
    OmgTip getGuiTip();

  private:
    void addGenericTip(OmgTip);
    void addGuiTip(OmgTip);
    int randomNumber(int theMax);
    //@TODO move tipts into a sqlite db
    QList <OmgTip> mGenericTips;
    QList <OmgTip> mGuiTips;
    QList <OmgTip> mAllTips;
};
#endif //OMGTIPFACTORY

