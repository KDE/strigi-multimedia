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

#ifndef __KFILE_OGG_H__
#define __KFILE_OGG_H__

#include <kfilemetainfo.h>

class QString;
class QStringList;

class KOggPlugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    KOggPlugin( QObject *parent, const char *name,
                const QStringList& preferredItems );
    
    virtual bool readInfo( KFileMetaInfo::Internal& info, int );
    virtual bool writeInfo( const KFileMetaInfo::Internal& info ) const;
    virtual QValidator* createValidator( const QString &key,
                                         QObject* parent, const char* name,
                                         const QString &group) const;
};


#endif
