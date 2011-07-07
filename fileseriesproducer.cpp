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


#include "fileseriesproducer.h"

void FileSeriesProducer::readSeries()
{
    int time;
    int value;
  
    if (input)
    {
	input >> time;
	int c = GetNumChannels();
	for(int i=0; i<c; i++) {
	    input >> value;
	    this->SetSample(i,time,value);
	};
    };
}

int FileSeriesProducer::openFile(const char* file_name)
{
    input.open(file_name, ios::in);
    if (input==0)
    {
      return 1;
    } 
    else
    {
      return 0;
    };
}

void FileSeriesProducer::closeFile()
{
    input.close();
}


FileSeriesProducer::FileSeriesProducer(const char *file_name, int num_channels) : SeriesProducer(num_channels)
{  
    if (openFile(file_name))
    {
      throw "could not open ";
    }
}



FileSeriesProducer::~FileSeriesProducer()
{
  closeFile();
}


