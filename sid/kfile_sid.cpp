/* This file is part of the KDE project
 * Copyright (C) 2003 Rolf Magnus <ramagnus@kde.org>
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
 */

#include "kfile_sid.h"

#include <klocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <qfile.h>

typedef KGenericFactory<KSidPlugin> SidFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_sid, SidFactory("kfile_sid"))

KSidPlugin::KSidPlugin(QObject *parent, const char *name,
                       const QStringList &args)
    
    : KFilePlugin(parent, name, args)
{
    kdDebug(7034) << "sid plugin\n";
    
    KFileMimeTypeInfo* info = addMimeTypeInfo("audio/prs.sid");

    KFileMimeTypeInfo::GroupInfo* group = 0L;

    // General group
    group = addGroupInfo(info, "General", i18n("General"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Title", i18n("Title"), QVariant::String);
//    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Name);

    item = addItemInfo(group, "Artist", i18n("Artist"), QVariant::String);
//    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Author);

    item = addItemInfo(group, "Copyright", i18n("Copyright"), QVariant::String);
//    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Description);

    // technical group
    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    item = addItemInfo(group, "Version", i18n("Version"), QVariant::Int);
    setPrefix(item,  i18n("PSID v"));

    item = addItemInfo(group, "Number of Songs", i18n("Number of Songs"), QVariant::Int);
}

bool KSidPlugin::readInfo(KFileMetaInfo& info, uint what)
{
    QFile file(info.path());
    file.open(IO_ReadOnly);

    int version;
    int num_songs;
    QString name;
    QString artist;
    QString copyright;
    
    char buf[64] = { 0 };
    
    if (4 != file.readBlock(buf, 4))
        return false;
    if (strncmp(buf, "PSID", 4))
        return false;

    //read version
    int ch;
    if (0 > (ch = file.getch()))
        return false;
    version = ch << 8;
    if (0 > (ch = file.getch()))
        return false;
    version+= ch;

    //read number of songs
    file.at(0xE);
    if (0 > (ch = file.getch()))
        return false;
    num_songs = ch << 8;
    if (0 > (ch = file.getch()))
        return false;
    num_songs += ch;

    //name
    file.at(0x16);
    if (32 != file.readBlock(buf, 32))
        return false;
    name = buf;

    //artist
    if (32 != file.readBlock(buf, 32))
        return false;
    artist = buf;

    //copyright
    if (32 != file.readBlock(buf, 32))
        return false;
    copyright = buf;

    QString TODO("TODO");
    kdDebug(7034) << "sid plugin readInfo\n";
    
    KFileMetaInfoGroup general = appendGroup(info, "General");

    appendItem(general, "Title",     name);
    appendItem(general, "Artist",    artist);
    appendItem(general, "Copyright", copyright);

    KFileMetaInfoGroup tech = appendGroup(info, "Technical");

    appendItem(tech, "Version",         version);
    appendItem(tech, "Number of Songs", num_songs);

    kdDebug(7034) << "reading finished\n";
    return true;
}
#include "kfile_sid.moc"
