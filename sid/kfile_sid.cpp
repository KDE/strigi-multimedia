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
#include <qvalidator.h>
#include <qwidget.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Name);

    item = addItemInfo(group, "Artist", i18n("Artist"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Author);

    item = addItemInfo(group, "Copyright", i18n("Copyright"), QVariant::String);
    setAttributes(item, KFileMimeTypeInfo::Modifiable);
    setHint(item,  KFileMimeTypeInfo::Description);

    // technical group
    group = addGroupInfo(info, "Technical", i18n("Technical Details"));

    item = addItemInfo(group, "Version", i18n("Version"), QVariant::Int);
    setPrefix(item,  i18n("PSID v"));

    addItemInfo(group, "Number of Songs", i18n("Number of Songs"), QVariant::Int);
    item = addItemInfo(group, "Start Song", i18n("Start Song"), QVariant::Int);
}

bool KSidPlugin::readInfo(KFileMetaInfo& info, uint /*what*/)
{
    if ( info.path().isEmpty() ) // remote file
        return false;
    QFile file(info.path());
    if ( !file.open(IO_ReadOnly) )
        return false;

    int version;
    int num_songs;
    int start_song;
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

    //start song
    if (0 > (ch = file.getch()))
        return false;
    start_song = ch << 8;
    if (0 > (ch = file.getch()))
        return false;
    start_song += ch;

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
    appendItem(tech, "Start Song", start_song);

    kdDebug(7034) << "reading finished\n";
    return true;
}

bool KSidPlugin::writeInfo(const KFileMetaInfo& info) const
{
    kdDebug(7034) << k_funcinfo << endl;

    char name[32] = {0};
    char artist[32] = {0};
    char copyright[32] = {0};

    int file = 0;
    QString s;
        
    KFileMetaInfoGroup group = info.group("General");
    if (!group.isValid())
        goto failure;

    s = group.item("Title").value().toString();
    if (s.isNull()) goto failure;
    strncpy(name, s.local8Bit(), 31);
    
    s = group.item("Artist").value().toString();
    if (s.isNull()) goto failure;
    strncpy(artist, s.local8Bit(), 31);
    
    s = group.item("Copyright").value().toString();
    if (s.isNull()) goto failure;
    strncpy(copyright, s.local8Bit(), 31);
    
    kdDebug(7034) << "Opening sid file " << info.path() << endl;
    file = ::open(QFile::encodeName(info.path()), O_WRONLY);
    //name
    if (-1 == ::lseek(file, 0x16, SEEK_SET))
        goto failure;
    if (32 != ::write(file, name, 32))
        goto failure;

    //artist
    if (32 != ::write(file, artist, 32))
        goto failure;

    //copyright
    if (32 != write(file, copyright, 32))
        goto failure;

    close(file);
    return true;

failure:
    if (file) close(file);
    kdDebug(7034) << "something went wrong writing to sid file\n";
    return false;
}

QValidator*
KSidPlugin::createValidator(const QString& /*mimetype*/, const QString& group,
                            const QString& /*key*/, QObject* parent,
                            const char* name) const
{
    kdDebug(7034) << k_funcinfo << endl;
    // all items in "General" group are strings of max lenght 31
    if (group == "General")
        return new QRegExpValidator(QRegExp(".{,31}"), parent, name);
    // all others are read-only
    return 0;
}



#include "kfile_sid.moc"
