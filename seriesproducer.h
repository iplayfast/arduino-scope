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


#ifndef SERIESPRODUCER_H
#define SERIESPRODUCER_H

//#include "line.h"
#include "TFuzzy.h"

using namespace Crystal;
using namespace Fuzzy;

class SeriesProducer 
{
  SeriesProducer() {} // we don't allow this
  SeriesProducer(SeriesProducer &) {} // we don't allow this either
ClassFuzzy *Channels;
int number_of_channels;
    public: 
	SeriesProducer(int numChannels); 
	~SeriesProducer();
      // update from whatever input this class has been used with
	virtual void readSeries() = 0;
	int GetNumChannels() const { return number_of_channels; } 
	void SetSample(int channel,int time,int value);
	int GetSample(int channel,int time) const;
	int GetEnd(int channel);
	void Clear() { for (int i=0;i<number_of_channels;i++) Channels[i].Clear(); }
	int SampleCount(int channel) const { return Channels[channel].Count(); }
	int SampleTime(int channel,int SampleIndex) { return Channels[channel].IndexAt(SampleIndex); }
	int SampleValue(int channel,int SampleIndex) { return Channels[channel].ValueAt(SampleIndex); }
};

#endif // SERIESPRODUCER_H
