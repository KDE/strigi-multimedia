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

// AnalyzerFactory
void M3uLineAnalyzerFactory::registerFields(Strigi::FieldRegister& reg) 
{
    tracksField = reg.registerField("tracks", Strigi::FieldRegister::integerType, 1, 0);
    trackPathField = reg.registerField("trackpath", Strigi::FieldRegister::stringType, 1, 0);
}

// Analyzer
void M3uLineAnalyzer::startAnalysis(Strigi::AnalysisResult* i) 
{
    if (i->extension() != "m3u")
        extensionOk = false;
    else
        extensionOk = true;

    analysisResult = i;
    ready = false;
    line = 0;
    count = 0;
}

void M3uLineAnalyzer::handleLine(const char* data, uint32_t length) 
{
    if (ready || !extensionOk) 
        return;
    
    ++line;

    if (length == 0)
        return;

    if (*data != '#') {

        // TODO: Check for a valid url with QUrl
        // TODO: Add the url to the trackPathField

        ++count;
    }
}

bool M3uLineAnalyzer::isReadyWithStream() 
{
    return ready;
}

void M3uLineAnalyzer::endAnalysis()
{
    if (extensionOk)
        analysisResult->setField(factory->tracksField, count);
}

