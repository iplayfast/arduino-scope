/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

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


#ifndef ARDUINOPRODUCER_H
#define ARDUINOPRODUCER_H
// This producer has 2 channels input with the sequence of 
// timevalue channel1 channel2
// the timevalue is simply a counter of ms from the program start
#include <fileseriesproducer.h>


class ArduinoProducer : public FileSeriesProducer
{

public:
    ArduinoProducer(char *file_name)
    {
      
    }

    virtual void readSeries();
};

#endif // ARDUINOPRODUCER_H
