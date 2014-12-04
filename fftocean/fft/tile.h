#pragma once

#include <memory>
#include <vector>

#include "base/basictypes.h"


/**
 * class Tile
 * 作用：创建 tile 对应的 VertexData 和 IndicesData
 * TVertex: 顶点的结构体类型
 * TVertexInitPolicy 初始化顶点的 policy
 */
class Tile {
 public:
  /**
   * tile 的 grid line 个数的计算公式是 2 ^ level + 1
   * 采用这种方法的好处是可以非常容易的计算中点位置 2 ^ (level - 1) + 1
   */
  Tile(const int level = 8)
      : kGridLineNum(grid_line_num(level))
      , kLevel(level)
      , kVertexNum(kGridLineNum * kGridLineNum) {
  }

  static float grid_line_num(int level) {
    return (1 << level) + 1;
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

  int32 cell_num() const { return kGridLineNum -1 ;}
  int32 grid_line_num() const { return kGridLineNum;}
  int32 vertices_num() const { return kVertexNum;}
  int32 indices_num() const;

  template<class TVertex, class TVertexInitPolicy>
  TVertex* InitVerticesData(TVertex* vertex, const TVertexInitPolicy& polocy) const;

  int32* CalcIndicesForPitch(int level, const Pitch& pitch, int32* start) const;
  int32* CalcIndices(int32* start) const;
 private:
  const int32 kGridLineNum;
  const int32 kLevel; 
  const int32 kVertexNum;
  DISALLOW_COPY_AND_ASSIGN(Tile);
};


template<class TVertex, class TVertexInitPolicy>
TVertex* Tile::InitVerticesData(TVertex* start, const TVertexInitPolicy& policy) 
    const {
  TVertex* cur = start;
  for (int i = 0; i < kGridLineNum; ++i) {
    for (int j = 0; j < kGridLineNum; ++j) {
      int idx = i * kGridLineNum + j;
      policy(cur, i, j, *this);
      cur++;
    }
  }
  return cur;
}

inline int32* Tile::CalcIndices(int32* start) const {
  Pitch pitch;
  pitch.left = 0;
  pitch.top = 0;
  pitch.right = kGridLineNum - 1;
  pitch.bottom = kGridLineNum - 1;
  return CalcIndicesForPitch(0, pitch, start);
}

inline Tile::Pitch& Tile::Pitch::operator = (const Tile::Pitch& pitch) {
  left = pitch.left;
  right = pitch.right;
  top = pitch.top;
  bottom = pitch.bottom;
  return *this;
}

inline int32 Tile::indices_num() const { 
  return (grid_line_num() - 1) * (grid_line_num() - 1) * 6;
}
