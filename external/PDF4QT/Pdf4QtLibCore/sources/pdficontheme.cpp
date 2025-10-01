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

#include "pdficontheme.h"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

#include "pdfdbgheap.h"

namespace pdf
{

void PDFIconTheme::registerAction(QAction* action, QString iconFileName)
{
    m_actionInfos.emplace_back(action, std::move(iconFileName));
}

QString PDFIconTheme::getDirectory() const
{
    return m_directory;
}

void PDFIconTheme::setDirectory(const QString& directory)
{
    m_directory = directory;
}

QString PDFIconTheme::getPrefix() const
{
    return m_prefix;
}

void PDFIconTheme::setPrefix(const QString& prefix)
{
    m_prefix = prefix;
}

void PDFIconTheme::prepareTheme()
{
    QString directory = getThemeDirectory();
    QDir dir(directory);

    if (!dir.exists())
    {
        dir.mkpath(directory);

        QStringList infoList;

        for (const ActionInfo& actionInfo : m_actionInfos)
        {
            QString fileName = formatFileName(actionInfo, directory);
            if (!QFile::exists(fileName))
            {
                QFile::copy(actionInfo.fileName, fileName);
                QFile::setPermissions(fileName, QFile::Permissions(0xFFFF));
            }

            infoList << QString("%1;%2").arg(actionInfo.action->text(), fileName);
        }

        if (!infoList.isEmpty())
        {
            QFile file(directory + "/icons.txt");
            if (file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
            {
                {
                    QTextStream stream(&file);
                    stream.setEncoding(QStringConverter::Utf8);
                    stream.setGenerateByteOrderMark(true);
                    for (const QString& text : infoList)
                    {
                        stream << text << Qt::endl;
                    }
                }
                file.close();
            }
        }
    }
}

void PDFIconTheme::loadTheme()
{
    QString directory = getThemeDirectory();
    QDir dir(directory);

    if (dir.exists())
    {
        for (const ActionInfo& actionInfo : m_actionInfos)
        {
            QString fileName = formatFileName(actionInfo, directory);
            if (QFile::exists(fileName))
            {
                actionInfo.action->setIcon(QIcon(fileName));
            }
        }
    }
}

QString PDFIconTheme::getThemeDirectory() const
{
    return QCoreApplication::applicationDirPath() + "/" + m_directory;
}

QString PDFIconTheme::formatFileName(const ActionInfo& info, const QString& themeDirectory) const
{
    QString fileName = info.fileName;
    fileName.remove(m_prefix);
    return QString("%1/%2").arg(themeDirectory, fileName);
}

}   // namespace pdf
