/*  KFile MPC-plugin

    Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef __KFILE_MPC_H__
#define __KFILE_MPC_H__

#include <kfilemetainfo.h>

class QString;
class QStringList;

class KMpcPlugin: public KFilePlugin
{
    Q_OBJECT

public:
    KMpcPlugin( QObject *parent, const char *name, const QStringList& args );

    virtual bool readInfo( KFileMetaInfo& info, uint what);
    virtual bool writeInfo( const KFileMetaInfo& info ) const;
    virtual QValidator* createValidator( const QString& mimetype,
                                         const QString &group,
                                         const QString &key,
                                         QObject* parent, const char* name) const;
};


#endif
