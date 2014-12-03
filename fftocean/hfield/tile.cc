#include "azer/sandbox/water/fftocean/hfield/tile.h"

int32* Tile::CalcIndicesForPitch(
    int level, const Pitch& pitch, int32* start) const {
  const int step = 1 << level;
  int32* cur = start;
  for (int i = pitch.top; i < pitch.bottom; i += step) {
    for (int j = pitch.left; j < pitch.right; j += step) {
      int cur_line = i * kGridLineNum;
      int next_line = (i + step) * kGridLineNum;
      *cur++ = cur_line  + j;
      *cur++ = next_line + j;
      *cur++ = next_line + j + step;
      *cur++ = cur_line  + j;
      *cur++ = next_line + j + step;
      *cur++ = cur_line  + j + step;
    }
  }
  return cur;
}
