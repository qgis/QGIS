//    Copyright (C) 2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT. If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFICONTHEME_H
#define PDFICONTHEME_H

#include "pdfglobal.h"

#include <QAction>

namespace pdf
{

/// Theme icon provider. Can provide icons from custom directory,
/// so user can use their own icon theme.
class PDF4QTLIBCORESHARED_EXPORT PDFIconTheme
{
public:
    explicit PDFIconTheme() = default;

    void registerAction(QAction* action, QString iconFileName);

    QString getDirectory() const;
    void setDirectory(const QString& directory);

    QString getPrefix() const;
    void setPrefix(const QString& prefix);

    void prepareTheme();
    void loadTheme();

    QString getThemeDirectory() const;

private:
    struct ActionInfo
    {
        ActionInfo(QAction* action = nullptr, QString fileName = QString()) :
            action(action),
            fileName(std::move(fileName))
        {

        }

        QAction* action = nullptr;
        QString fileName;
    };

    QString m_directory;
    QString m_prefix;

    QString formatFileName(const ActionInfo& info, const QString& themeDirectory) const;

    std::vector<ActionInfo> m_actionInfos;
};

}   // namespace pdf

#endif // PDFICONTHEME_H
