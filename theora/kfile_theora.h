/***************************************************************************
 *   Copyright (C) 2004 by Jean-Baptiste Mardelle                          *
 *   bj@altern.org                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
#ifndef __KFILE_THEORA_H__
#define __KFILE_THEORA_H__

/**
 * Note: For further information look into <$KDEDIR/include/kfilemetainfo.h>
 */
#include <kfilemetainfo.h>

class QStringList;

class theoraPlugin: public KFilePlugin
{
    Q_OBJECT
    
public:
    theoraPlugin( QObject *parent, const char *name, const QStringList& args );
    
    virtual bool readInfo( KFileMetaInfo& info, uint what);
};

#endif // __KFILE_THEORA_H__

