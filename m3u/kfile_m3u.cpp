/* This file is part of the KDE project
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
 * Copyright (C) 2007 Tim Beaulen <tbscope@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *  $Id$
 */

#include "kfile_m3u.h"

#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include <strigi/streamlineanalyzer.h>

#include <QString>
#include <kdebug.h>

// AnalyzerFactory
void M3uLineAnalyzerFactory::registerFields(Strigi::FieldRegister& reg) 
{
    tracksField = reg.registerField("tracks", Strigi::FieldRegister::integerType, 1, 0);
    trackPathField = reg.registerField("trackpath", Strigi::FieldRegister::stringType, 1, 0);
}

// Analyzer
void M3uLineAnalyzer::startAnalysis(Strigi::AnalysisResult* i) {
    analysisResult = i;
    ready = false;
    line = 0;
    count = 0;
}

void M3uLineAnalyzer::handleLine(const char* data, uint32_t length) 
{
    if (ready) 
        return;
    
    ++line;

    QString strLine(data);
    strLine = strLine.trimmed();

    if (strLine.isEmpty())
        return;

    if (!strLine.startsWith('#')) {
        ++count;
    }
    
#if 0
    if (line == 1 && (length < 9 || strncmp(data, "/* XPM */", 9))) {
        // this is not an xpm file
        ready = true;
        return;
    } else if (length == 0 || data[0] != '"') {
        return;
    }
    uint32_t i = 0;
    // we have found the line which should contain the information we want
    ready = true;
    // read the height
    uint32_t propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->setField(factory->heightField, propertyValue);

    // read the width
    propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->setField(factory->widthField, propertyValue);

    // read the number of colors
    propertyValue = 0;
    i++;
    while (i < length && isdigit(data[i])) {
        propertyValue = (propertyValue * 10) + data[i] - '0';
        i++;
    }

    if (i >= length || data[i] != ' ')
        return;

    analysisResult->setField(factory->numberOfColorsField, propertyValue);
#endif
}

bool M3uLineAnalyzer::isReadyWithStream() 
{
    return ready;
}

void M3uLineAnalyzer::endAnalysis()
{
    kDebug(7034) << "m3u - endAnalysis" << endl;

    analysisResult->setField(factory->tracksField, count);
}

#if 0

#include <kdebug.h>
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <q3cstring.h>
#include <QFile>
#include <qtextstream.h>
#include <QDateTime>
#include <q3dict.h>
#include <qvalidator.h>

typedef KGenericFactory<KM3uPlugin> M3uFactory;

K_EXPORT_COMPONENT_FACTORY( kfile_m3u, M3uFactory( "kfile_m3u" ) )

KM3uPlugin::KM3uPlugin( QObject *parent, 
                        const QStringList &preferredItems )
    : KFilePlugin( parent, preferredItems )
{
    kDebug(7034) << "m3u plugin\n";

    KFileMimeTypeInfo* info = addMimeTypeInfo( "audio/x-mpegurl" );

    KFileMimeTypeInfo::GroupInfo* group;

    // tracks group
    group = addGroupInfo(info, "Tracks", i18n("Tracks"));
    addVariableInfo(group, QVariant::String, 0);
}

bool KM3uPlugin::readInfo( KFileMetaInfo& info, uint )
{
    if ( info.path().isEmpty() ) // remote file
        return false;

    QFile f(info.path());
    if (!f.open(QIODevice::ReadOnly)) return false;
    QTextStream str(&f);
    str.setEncoding(QTextStream::Locale);
    
    
    KFileMetaInfoGroup group = appendGroup(info, "Tracks");
    
    // for now treat all lines that don't start with # like entries
    int num = 1;
    while (!str.atEnd())
    {
        QString s = str.readLine();
        if (!s.startsWith("#"))
        {
            if (s.endsWith("\n")) s.truncate(s.length()-1);

            if (!s.trimmed().isEmpty()) {
                appendItem(group, ki18n("Track %1").subs(num, 3).toString(), s);
                num++;
            }
        }
    }
    
    return true;
}

#include "kfile_m3u.moc"
#endif

