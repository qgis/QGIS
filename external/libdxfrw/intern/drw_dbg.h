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

#ifndef DRW_DBG_H
#define DRW_DBG_H

#include <string>
#include <iostream>
#include <memory>
#include "../drw_base.h"
//#include <iomanip>

#define DRW_DBGSL(a) DRW_dbg::getInstance()->setLevel(a)
#define DRW_DBGGL DRW_dbg::getInstance()->getLevel()
#define DRW_DBG(a) DRW_dbg::getInstance()->print(a, __FILE__, __FUNCTION__, __LINE__)
#define DRW_DBGH(a) DRW_dbg::getInstance()->printH(a, __FILE__, __FUNCTION__, __LINE__)
#define DRW_DBGB(a) DRW_dbg::getInstance()->printB(a, __FILE__, __FUNCTION__, __LINE__)
#define DRW_DBGHL(a, b, c) DRW_dbg::getInstance()->printHL(a, b ,c, __FILE__, __FUNCTION__, __LINE__)
#define DRW_DBGPT(a, b, c) DRW_dbg::getInstance()->printPT(a, b, c, __FILE__, __FUNCTION__, __LINE__)

class DRW_dbg {
public:
    enum class Level {
        None,
        Debug
    };
    void setLevel(Level lvl);
    /**
     * Sets a custom debug printer to use when non-silent output
     * is required.
     */
    void setCustomDebugPrinter(std::unique_ptr<DRW::DebugPrinter> printer);
    Level getLevel();
    static DRW_dbg *getInstance();
    void print(const std::string& s, const char *file, const char *function, int line);
    void print(int i, const char *file, const char *function, int line);
    void print(unsigned int i, const char *file, const char *function, int line);
    void print(long long int i, const char *file, const char *function, int line);
    void print(long unsigned int i, const char *file, const char *function, int line);
    void print(long long unsigned int i, const char *file, const char *function, int line);
    void print(double d, const char *file, const char *function, int line);
    void printH(long long int i, const char *file, const char *function, int line);
    void printB(int i, const char *file, const char *function, int line);
    void printHL(int c, int s, int h, const char *file, const char *function, int line);
    void printPT(double x, double y, double z, const char *file, const char *function, int line);

private:
    DRW_dbg();
    static DRW_dbg *instance;
    Level level{Level::None};
    DRW::DebugPrinter silentDebug;
    std::unique_ptr< DRW::DebugPrinter > debugPrinter;
    DRW::DebugPrinter* currentPrinter{nullptr};
};


#endif // DRW_DBG_H
