#include "azer/util/sampleapp.h"
#include "azer/base/appinit.h"
#include "base/files/file_path.h"
#include "azer/sandbox/base/camera_control.h"
#include "azer/sandbox/water/fftocean/hfield/tile.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define SHADER_NAME "/diffuse.afx"

using base::FilePath;

class VertexInit {
 public:
  void InitVertex(DiffuseEffect::Vertex* vertex, int i, int j,
                  const Tile<DiffuseEffect::Vertex, VertexInit>& tile) const{
    vertex->position.x = ((float)j - tile.grid_line_num() / 2.0f) * 1.0f;
    vertex->position.z = ((float)i - tile.grid_line_num() / 2.0f) * 1.0f;
    vertex->position.y = 0.0f;
    vertex->position.w = 1.0f;
  }
};

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : tile_(4) {
  }
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  Tile<DiffuseEffect::Vertex, VertexInit> tile_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<DiffuseEffect> effect_;

  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  renderer->SetFillMode(azer::kWireFrame);
  renderer->EnableDepthTest(true);
  camera_.SetPosition(azer::Vector3(0.0f, 0.0f, 5.0f));

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);
}


void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexData vdata(effect_->GetVertexDesc(), tile_.vertices_num());
  DiffuseEffect::Vertex* v = (DiffuseEffect::Vertex*)vdata.pointer();
  DiffuseEffect::Vertex* vend = tile_.InitVerticesData(v);
  DCHECK_EQ(vend - v, tile_.vertices_num());
  azer::IndicesData idata(tile_.indices_num(), azer::IndicesData::kUint32);
  tile_.CalcIndices((int32*)idata.pointer());
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), &vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  azer::Matrix4 world = std::move(azer::Translate(0.0f, 0.0f, 0.0f));
  effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);
}

int main(int argc, char* argv[]) {
  ::azer::InitApp(&argc, &argv, "");
  
  MainDelegate delegate;
  azer::WindowHost win(azer::WindowHost::Options(), &delegate);
  win.Init();
  CHECK(azer::LoadRenderSystem(&win));
  LOG(ERROR) << "Current RenderSystem: " << azer::RenderSystem::Current()->name();
  delegate.Init();
  win.Show();
  azer::MainRenderLoop(&win);
  return 0;
}

