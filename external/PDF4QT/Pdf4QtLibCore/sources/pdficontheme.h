// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
