/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "drw_base.h"
#include "intern/drw_dbg.h"

void DRW::setCustomDebugPrinter(DebugPrinter *printer)
{
  DRW_dbg::getInstance()->setCustomDebugPrinter(std::unique_ptr<DebugPrinter>(printer));
}

DRW::DebugPrinter::~DebugPrinter() = default;
