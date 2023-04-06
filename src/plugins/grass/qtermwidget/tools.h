#ifndef TOOLS_H
#define TOOLS_H

#include <QString>
#include <QStringList>
#include <QLoggingCategory>

QString get_kb_layout_dir();
void add_custom_color_scheme_dir(const QString& custom_dir);
const QStringList get_color_schemes_dirs();

Q_DECLARE_LOGGING_CATEGORY(qtermwidgetLogger)

#endif
