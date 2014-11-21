#pragma once

#include <string>
#include <vector>
#include <queue>
#include <cmath>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "azer/render/render.h"
#include "azer/render/render_system_enum.h"

class Tile {
 public:
  /**
   * tile 的 grid line 个数的计算公式是 2 ^ level + 1
   * 采用这种方法的好处是可以非常容易的计算中点位置 2 ^ (level - 1) + 1
   */
  Tile(const int level = 8, float width = 1.0f)
      : kGridLine((1 << level) + 1)
      , kLevel_(level)
      , kCellWidth(width) {
  }

  struct Pitch {
    int left;
    int right;
    int top;
    int bottom;

    Pitch(): left(-1), right(-1), top(-1), bottom(-1) {}
    Pitch(int l, int t, int r, int b): left(l), right(r), top(t), bottom(b) {}
    Pitch(const Pitch& pitch) {*this = pitch; }
    Pitch& operator = (const Pitch& pitch);
  };

  void Init();
  const std::vector<azer::Vector3>& vertices() { return vertices_;}
  const std::vector<azer::Vector3>& normal() { return normal_;}
  const std::vector<azer::Vector2>& texcoord() { return texcoord_;}
  const std::vector<int32>& indices() { return indices_;}

  int32* InitPitchIndices(int level, const Tile::Pitch& pitch, int32* indices);

  /**
   * 注意： 以下三个函数， x, z 是以 grid line 为坐标，而非实际的 3D 世界坐标
   * 3D 世界坐标与 kCellWidth 相关
   */
  void SetHeight(int x, int z, float height);
  azer::Vector3& vertex(int x, int z);
  const azer::Vector3& vertex(int x, int z) const;
  const azer::Vector2& texcoord(int x, int z) const;

  int GetGridLineNum() const { return kGridLine;}
  int GetVertexNum() const { return vertices_.size();}
  int GetIndicesNum() const { return indices_.size();}
  float cell_width() const { return kCellWidth;}
  void CalcNormal();
  /**
   * 计算 TBN 空间（主要用户 BUMPMAP)
   * 参数 repeat: 用作计算 texcoord, 默认情况下，每一个单元格的坐标是 1.0f
   * 通过 repeat 参数可以对 texcoord 进行缩放
   */
  void CalcTBN(float repeat, std::vector<azer::Vector3>* tangent,
               std::vector<azer::Vector3>* binormal);

  float minx() const;
  float maxx() const;
  float x_range() const { return maxx() - minx();}

  float miny() const;
  float maxy() const;
  float y_range() const { return maxy() - miny();}
  
  float minz() const;
  float maxz() const;
  float z_range() const { return maxz() - minz();}

  int level() const { return kLevel_;}

 private:
  void InitVertex();
  void InitIndices();
  std::vector<azer::Vector3> vertices_;
  std::vector<azer::Vector3> normal_;
  std::vector<azer::Vector2> texcoord_;
  std::vector<int32> indices_;
  const int kGridLine;
  const int kLevel_;
  const float kCellWidth;
  float min_x_, max_x_, min_y_, max_y_, min_z_, max_z_;
  bool yspec_;
  DISALLOW_COPY_AND_ASSIGN(Tile);
};

int32* InitPitchIndices(int level, const Tile::Pitch& pitch, int kGridLine,
                        int32* indices);

inline Tile::Pitch& Tile::Pitch::operator = (const Pitch& pitch) {
  left = pitch.left;
  right = pitch.right;
  top = pitch.top;
  bottom = pitch.bottom;
  return *this;
}

inline float Tile::minx() const {
  DCHECK_GT(vertices_.size(), 0u);
  return min_x_;
}
inline float Tile::maxx() const {
  DCHECK_GT(vertices_.size(), 0u);
  return max_x_;
}
inline float Tile::miny() const {
  DCHECK(yspec_);
  DCHECK_GT(vertices_.size(), 0u);
  return min_y_;
}
inline float Tile::maxy() const {
  DCHECK(yspec_);
  DCHECK_GT(vertices_.size(), 0u);
  return max_y_;
}
inline float Tile::minz() const {
  DCHECK_GT(vertices_.size(), 0u);
  return min_z_;
}
inline float Tile::maxz() const {
  DCHECK_GT(vertices_.size(), 0u);
  return max_z_;
}

inline azer::Vector3& Tile::vertex(int x, int z) {
  return vertices_[z * kGridLine + x];
}

inline const azer::Vector3& Tile::vertex(int x, int z) const {
  return vertices_[z * kGridLine + x];
}
