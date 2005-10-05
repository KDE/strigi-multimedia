/* This file is part of the KDE project
 * Copyright (C) 2001, 2002 Rolf Magnus <ramagnus@kde.org>
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

#include <kdebug.h>
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <q3cstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <q3dict.h>
#include <qvalidator.h>

typedef KGenericFactory<KM3uPlugin> M3uFactory;

K_EXPORT_COMPONENT_FACTORY( kfile_m3u, M3uFactory( "kfile_m3u" ) )

KM3uPlugin::KM3uPlugin( QObject *parent, const char *name,
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
    kdDebug(7034) << "m3u plugin\n";

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

            if (!s.stripWhiteSpace().isEmpty()) {
                appendItem(group, i18n("Track %1").arg(num, 3), s);
                num++;
            }
        }
    }
    
    return true;
}

#include "kfile_m3u.moc"
