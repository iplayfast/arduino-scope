/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Ralph Janke <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef FILESERIESPRODUCER_H
#define FILESERIESPRODUCER_H


#include "seriesproducer.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "line.h"

using namespace std;

class FileSeriesProducer : public SeriesProducer
{
protected:
    ifstream input;
    int openFile(const char* file_name);
    void closeFile();
public:
    virtual void readSeries();
    FileSeriesProducer(const char *file_name, int num_channels); 
    virtual ~FileSeriesProducer();
};

#endif // FILESERIESPRODUCER_H
