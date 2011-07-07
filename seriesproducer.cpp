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

#include <assert.h>
#include "seriesproducer.h"

/*void SeriesProducer::setSeriesInfo(line* series_storage, int num_channels, const int* channels)
{
    test_series = series_storage;
    number_of_channels = num_channels;
    for(int i=0; i<num_channels; i++){
        channel_numbers[i] = channels[i];
    }
}*/

SeriesProducer::SeriesProducer(int numChannels)
{
   number_of_channels = numChannels; 
   Channels = new ClassFuzzy[numChannels];
}

SeriesProducer::~SeriesProducer()
{
  delete []Channels;
}

void SeriesProducer::SetSample ( int channel, int time, int value )
{
assert(channel>=0 && channel < number_of_channels);
  Channels[channel].TFuzzyAddPoint(time,value);
}

int SeriesProducer::GetSample ( int channel, int time ) const
{
assert(channel>=0 && channel < number_of_channels);
  return Channels[channel].Value(time);
}
int SeriesProducer::GetEnd ( int channel )
{
assert(channel>=0 && channel < number_of_channels);
  return Channels[channel].HighestRange();
}
