#include "line.h"


  line::line() { for (int i=0; i< CHANNELNUM; i++) {visible[i] = true;  scale[i] = 1;}; end = 0; }
  void line::AddSample(int* val) { timemark[end]=*val++; for (int i=0; i< CHANNELNUM; i++){ value[i][end] = *val++; }; end++; if (end==TRACELENGTH) end--;  }
  void line::AddSample(int time, int channel, int value){};
  int line::GetChSample(int start, int ch) const { if (start<0) start=0; if (start>=end) return 0; else return value[ch][start] / scale[ch]; }
  int line::GetTime(int start) { return timemark[start]; }
  void line::IncScale(int ch) { if (scale[ch]>1) scale[ch]--; }
  void line::DecScale(int ch) { if (scale[ch]<512) scale[ch]++; }
  bool line::IsVisible(int ch) const { return visible[ch]; }
  void line::ToggleVisible(int ch) { visible[ch] = !visible[ch]; }
  int line::GetScale(int ch) const { return scale[ch]; }
  void line::Reset() { end=0; }
  int line::GetBaseTime() const { return timemark[0]; }
  int line::GetMaxTime() const { if (end>0) return timemark[end-1]; else return timemark[0]; }
  int line::GetEnd() const { return end; }

