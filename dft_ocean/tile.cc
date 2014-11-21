#include "azer/sandbox/water/dft_ocean/tile.h"

#include <limits>
#include "azer/render/render.h"

using azer::Vector3;
using azer::Vector2;

void Tile::Init() {
  min_x_ = std::numeric_limits<float>::max();
  max_x_ = -std::numeric_limits<float>::max();
  min_y_ = std::numeric_limits<float>::max();
  max_y_ = -std::numeric_limits<float>::max();
  min_z_ = std::numeric_limits<float>::max();
  max_z_ = -std::numeric_limits<float>::max();
  yspec_ = false;
  InitVertex();
  Pitch pitch;
  pitch.left = 0;
  pitch.top = 0;
  pitch.right = kGridLine - 1;
  pitch.bottom = kGridLine - 1;
  indices_.resize((kGridLine - 1) * (kGridLine - 1) * 6);
  InitPitchIndices(0, pitch, &(indices_[0]));
}

void Tile::InitVertex() {
  vertices_.clear();
  const int kVertexNum = kGridLine * kGridLine;
  vertices_.reserve(kVertexNum);
  for (int i = 0; i < kGridLine; ++i) {
    for (int j = 0; j < kGridLine; ++j) {
      int idx = i * kGridLine + j;
      float x = ((float)j - kGridLine / 2.0f) * kCellWidth;
      float z = ((float)i - kGridLine / 2.0f) * kCellWidth;
      Vector2 texcoord((float)j / (float)kGridLine, (float)i / (float)kGridLine);
      vertices_.push_back(azer::vec3(x, 0.0f, z));
      texcoord_.push_back(texcoord);
      if (x > max_x_) max_x_ = x;
      if (x < min_x_) min_x_ = x;
      if (z > max_z_) max_z_ = z;
      if (z < min_z_) min_z_ = z;
    }
  }
}

int32* Tile::InitPitchIndices(int level, const Tile::Pitch& pitch, int32* indices) {
  return ::InitPitchIndices(level, pitch, kGridLine, indices);
}

void Tile::CalcNormal() {
  normal_.resize(GetVertexNum());
  std::vector<float> used(GetVertexNum(), 0.0f);
  for (int i = 0; i < GetIndicesNum(); i+=3) {
    int index1 = indices_[i];
    int index2 = indices_[i + 1];
    int index3 = indices_[i + 2];
    const Vector3& p1 = vertices_[index1];
    const Vector3& p2 = vertices_[index2];
    const Vector3& p3 = vertices_[index3];
    used[index1] += 1.0f;
    used[index2] += 1.0f;
    used[index3] += 1.0f;

    Vector3 normal = CalcPlaneNormal(p1, p2, p3);

    normal_[index1] += normal;
    normal_[index2] += normal;
    normal_[index3] += normal;
  }

  for (int i = 0; i < GetVertexNum(); ++i) {
    normal_[i] = normal_[i] / used[i];
  }
}

void Tile::CalcTBN(float repeat, std::vector<azer::Vector3>* tangent,
                   std::vector<azer::Vector3>* binormal) {
  tangent->resize(GetVertexNum());
  binormal->resize(GetVertexNum());
  std::vector<float> used(GetVertexNum(), 0.0f);
  for (int i = 0; i < GetIndicesNum(); i+=3) {
    int index1 = indices_[i];
    int index2 = indices_[i + 1];
    int index3 = indices_[i + 2];
    const Vector3& p1 = vertices_[index1];
    const Vector3& p2 = vertices_[index2];
    const Vector3& p3 = vertices_[index3];
    const Vector2 t1 = texcoord_[index1] * repeat;
    const Vector2 t2 = texcoord_[index2] * repeat;
    const Vector2 t3 = texcoord_[index3] * repeat;
    used[index1] += 1.0f;
    used[index2] += 1.0f;
    used[index3] += 1.0f;

    azer::Vector3 tan, bin;
    azer::CalcTangentAndBinormal(p1, t1, p2, t2, p3, t3, &tan, &bin);

    (*tangent)[index1] += tan;
    (*tangent)[index2] += tan;
    (*tangent)[index3] += tan;
    (*binormal)[index1] += bin;
    (*binormal)[index2] += bin;
    (*binormal)[index3] += bin;
  }

  for (int i = 0; i < GetVertexNum(); ++i) {
    (*tangent)[i] = (*tangent)[i] / used[i];
    (*binormal)[i] = (*binormal)[i] / used[i];
  }
}

void Tile::SetHeight(int x, int z, float height) {
  vertices_[x * kGridLine + z].y = height;
  if (height < min_y_) min_y_ = height;
  if (height > max_y_) max_y_ = height;
  yspec_ = true;
}

int32* InitPitchIndices(int level, const Tile::Pitch& pitch, int kGridLine,
                        int32* indices) {
  const int step = 1 << level;
  int32* cur = indices;
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
