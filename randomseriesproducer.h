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


#ifndef RANDOMSERIESPRODUCER_H
#define RANDOMSERIESPRODUCER_H


#include "seriesproducer.h"
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include "line.h"

using namespace std;

class RandomSeriesProducer : public SeriesProducer
{
private:
  int min;
  int max;
public:
    virtual void readSeries();
    RandomSeriesProducer(int Min,int Max,int numChannels) : SeriesProducer(numChannels) {min = Min; max = Max; }
   // RandomSeriesProducer( int min_value, int max_value, line *series_storage, int num_channels, const int* channels);
   // RandomSeriesProducer(const RandomSeriesProducer& other);
   // virtual ~RandomSeriesProducer();
   // virtual RandomSeriesProducer& operator=(const RandomSeriesProducer& other);
    //virtual bool operator==(const RandomSeriesProducer& other) const;
};

#endif // RANDOMSERIESPRODUCER_H
