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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *  $Id$
 */

#include "kfile_m3u.h"

#include <kdebug.h>
#include <kurl.h>
#include <kprocess.h>
#include <klocale.h>

#include <qcstring.h>
#include <qfile.h>
#include <qdatetime.h>
#include <kgenericfactory.h>
#include <qdict.h>
#include <qvalidator.h>

typedef KGenericFactory<KM3uPlugin> M3uFactory;

K_EXPORT_COMPONENT_FACTORY( kfile_m3u, M3uFactory( "kfile_m3u" ) );

KM3uPlugin::KM3uPlugin( QObject *parent, const char *name,
                        const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
    kdDebug(7034) << "m3u plugin\n";
}

bool KM3uPlugin::readInfo( KFileMetaInfo::Internal& info, int )
{
    QFile f(info.path());
    f.open(IO_ReadOnly);
    
    // for now treat all lines like entries
    int num = 1;
    while (!f.atEnd())
    {
        QString s;
        f.readLine(s, 1000);
        if (!s.startsWith("#"))
        {
            QString key; key.sprintf("%04d", num);
            if (s.endsWith("\n")) s.truncate(s.length()-1);
            info.insert(KFileMetaInfoItem(key, i18n("Track %1").arg(num),
                                          QVariant(s)));
            num++;
        }
    }
    
    info.setPreferredKeys(m_preferred);

    QStringList supported(m_preferred);
    supported.sort();
    info.setSupportedKeys(supported);
    info.setSupportsVariableKeys(false);
    return true;
}

#include "kfile_m3u.moc"
