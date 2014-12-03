#pragma once

#include <memory>
#include <vector>

#include "base/basictypes.h"

template<class TVertex>
class Tile {
 public:
  /**
   * tile 的 grid line 个数的计算公式是 2 ^ level + 1
   * 采用这种方法的好处是可以非常容易的计算中点位置 2 ^ (level - 1) + 1
   */
  Tile(const int level = 8, float width = 1.0f)
      : kGridLineNum((1 << level) + 1)
      , kLevel_(level)
      , kCellWidth(width) {
  }

  class Pitch {
    int left;
    int right;
    int top;
    int bottom;

    Pitch(): left(-1), right(-1), top(-1), bottom(-1) {}
    Pitch(int l, int t, int r, int b): left(l), right(r), top(t), bottom(b) {}
    Pitch(const Pitch& pitch) {*this = pitch; }
    Pitch& operator = (const Pitch& pitch);
  };

  TVertex* first();
  TVertex* next(TVertex* cur);
  const TVertex* first() const;
  const TVertex* next(TVertex* cur) const;

  int32 grid_line_num() const { return kGridLineNum;}
  int32 vertices_num() const { return vertices_.size();}
  float cell_width() const { return kCellWidth;}

  int32* CalcIndices(int level, const Pitch& pitch, int32* start);
 private:
  std::vector<TVertex> vertices_;
  const int32 kGridLineNum;
  const int32 kLevel; 
  const float kCellWidth;
  DISALLOW_COPY_AND_ASSIGN(Tile);
};

template<class TVertex>
TVertex* Tile<TVertex>::first() {
  return &(vertices_[0]);
}

template<class TVertex>
TVertex* Tile<TVertex>::next(TVertex* cur) {
  if (cur - &(vertices_[0]) < vertices_.size()) {
    return cur+1;
  } else {
    return NULL;
  }
}

template<class TVertex>
const TVertex* Tile<TVertex>::first() const {
  return &(vertices_[0]);
}

template<class TVertex>
const TVertex* Tile<TVertex>::next(TVertex* cur) const {
  if (cur - &(vertices_[0]) < vertices_.size()) {
    return cur+1;
  } else {
    return NULL;
  }
}

template<class TVertex>
int32* Tile<TVertex>::CalcIndices(int level, const Pitch& pitch, 
                                  int32* start) {
  const int step = 1 << level;
  int32* cur = start;
  for (int i = pitch.top; i < pitch.bottom; i += step) {
    for (int j = pitch.left; j < pitch.right; j += step) {
      int cur_line = i * kGridLine;
      int next_line = (i + step) * kGridLine;
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

template<class TVertex>
typename Tile<TVertex>::Pitch& Tile<TVertex>::Pitch::operator = (
    const typename Tile<TVertex>::Pitch& pitch) {
  left = pitch.left;
  right = pitch.right;
  top = pitch.top;
  bottom = pitch.bottom;
  return *this;
}
