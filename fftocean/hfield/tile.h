#pragma once

#include <memory>
#include <vector>

#include "base/basictypes.h"


/**
 * TVertex: 顶点的结构体类型
 * TVertexInitPolicy 初始化顶点的 policy
 */
template<class TVertex, class TVertexInitPolicy>
class Tile : public TVertexInitPolicy {
  using TVertexInitPolicy::InitVertex;
 public:
  /**
   * tile 的 grid line 个数的计算公式是 2 ^ level + 1
   * 采用这种方法的好处是可以非常容易的计算中点位置 2 ^ (level - 1) + 1
   */
  Tile(const int level = 8, float width = 1.0f)
      : kGridLineNum((1 << level) + 1)
      , kLevel(level)
      , kVertexNum(kGridLineNum * kGridLineNum) {
  }

  class Pitch {
   public:
    int left;
    int right;
    int top;
    int bottom;

    Pitch(): left(-1), right(-1), top(-1), bottom(-1) {}
    Pitch(int l, int t, int r, int b): left(l), right(r), top(t), bottom(b) {}
    Pitch(const Pitch& pitch) {*this = pitch; }
    Pitch& operator = (const Pitch& pitch);
  };

  int32 grid_line_num() const { return kGridLineNum;}
  int32 vertices_num() const { return kVertexNum;}
  int32 indices_num() const { 
    return (grid_line_num() - 1) * (grid_line_num() - 1) * 6;
  }
  TVertex* InitVerticesData(TVertex* vertex) const;
  int32* CalcIndicesForPitch(int level, const Pitch& pitch, int32* start) const;
  int32* CalcIndices(int32* start) const;
 private:
  const int32 kGridLineNum;
  const int32 kLevel; 
  const int32 kVertexNum;
  DISALLOW_COPY_AND_ASSIGN(Tile);
};

template<class TVertex, class TVertexInitPolicy>
TVertex* Tile<TVertex, TVertexInitPolicy>::InitVerticesData(TVertex* start) const {
  TVertex* cur = start;
  for (int i = 0; i < kGridLineNum; ++i) {
    for (int j = 0; j < kGridLineNum; ++j) {
      int idx = i * kGridLineNum + j;
      InitVertex(cur, i, j, *this);
      cur++;
    }
  }
  return cur;
}

template<class TVertex, class TVertexInitPolicy>
int32* Tile<TVertex, TVertexInitPolicy>::CalcIndices(int32* start) const {
  Pitch pitch;
  pitch.left = 0;
  pitch.top = 0;
  pitch.right = kGridLineNum - 1;
  pitch.bottom = kGridLineNum - 1;
  return CalcIndicesForPitch(0, pitch, start);
}

template<class TVertex, class TVertexInitPolicy>
int32* Tile<TVertex, TVertexInitPolicy>::CalcIndicesForPitch(
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

template<class TVertex, class TVertexInitPolicy>
typename Tile<TVertex, TVertexInitPolicy>::Pitch& 
Tile<TVertex, TVertexInitPolicy>::Pitch::operator = (
    const typename Tile<TVertex, TVertexInitPolicy>::Pitch& pitch) {
  left = pitch.left;
  right = pitch.right;
  top = pitch.top;
  bottom = pitch.bottom;
  return *this;
}
