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


#include "randomseriesproducer.h"

void RandomSeriesProducer::readSeries()
{
  
    srand((unsigned)time(0));
    for(int i=0; i<number_of_channels; i++){
        test_series->AddSample(time(0), channel_numbers[i], min+int((max-min+1)*rand()/(RAND_MAX + 1.0)));
    } 
}


RandomSeriesProducer::RandomSeriesProducer()
{

}

RandomSeriesProducer::RandomSeriesProducer(const int min_value, const int max_value, line *series_storage, int num_channels, const int *channels)
{
    setSeriesInfo(series_storage, num_channels, channels);
    min = min_value;
    max = max_value;
}


RandomSeriesProducer::RandomSeriesProducer(const RandomSeriesProducer& other)
{

}

RandomSeriesProducer::~RandomSeriesProducer()
{

}

RandomSeriesProducer& RandomSeriesProducer::operator=(const RandomSeriesProducer& other)
{
    return *this;
}

bool RandomSeriesProducer::operator==(const RandomSeriesProducer& other) const
{
///TODO: return ...;
}

