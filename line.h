#ifndef LINES_H
#define LINES_H
#include "TFuzzy.h"
#define TRACELENGTH 10000
#define CHANNELNUM 2

using namespace Crystal;
using namespace Fuzzy;

class line
{
  int timemark[TRACELENGTH];
  int value[CHANNELNUM][TRACELENGTH];
  int end;
  bool visible[CHANNELNUM];
  int scale[CHANNELNUM];
  ClassFuzzy channel[CHANNELNUM];
public:
  line();
  void AddSample(int* val);
  int GetChSample(int start, int ch) const;
  int GetTime(int start);
  int GetY(int ch,int time);
  void IncScale(int ch);
  void DecScale(int ch);
  bool IsVisible(int ch) const;
  void ToggleVisible(int ch);
  int GetScale(int ch) const;
  void Reset();
  int GetBaseTime() const;
  int GetMaxTime() const;
  int GetEnd() const;
};

#endif