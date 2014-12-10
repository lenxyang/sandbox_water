#include "azer/util/sampleapp.h"
#include "azer/base/appinit.h"
#include "base/files/file_path.h"
#include "azer/sandbox/base/camera_control.h"
#include "azer/sandbox/water/fftocean/lighted/tile.h"
#include "azer/sandbox/water/fftocean/lighted/ocean.h"

#include <tchar.h>

#include "diffuse.afx.h"
#define SHADER_NAME "/diffuse.afx"

using base::FilePath;

class VertexInit {
 public:
  void operator() (DiffuseEffect::Vertex* vertex, int i, int j, const Tile& tile)
      const{
    vertex->position.x = ((float)j - tile.grid_line_num() / 2.0f) * 1.0f;
    vertex->position.z = ((float)i - tile.grid_line_num() / 2.0f) * 1.0f;
    vertex->position.y = 0.0f;

    vertex->normal = azer::Vector3(0.0f, 1.0f, 0.0f);
  }
};

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() 
      : tile_(6)
      , tick_(0.1f) {
  }
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit() {}
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  Tile tile_;
  float tick_;
  std::unique_ptr<Ocean> hfield_;
  std::unique_ptr<azer::VertexData> vdataptr_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<DiffuseEffect> effect_;

  DirLight light_;
  DiffuseEffect::WaterMaterial mtrl_;

  DISALLOW_COPY_AND_ASSIGN(MainDelegate);
};

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  // renderer->SetFillMode(azer::kWireFrame);
  renderer->EnableDepthTest(true);
  camera_.SetPosition(azer::Vector3(0.0f, 0.0f, 5.0f));

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  InitPhysicsBuffer(rs);

  light_.dir = azer::Vector4(0.0f, -0.6f, 0.75f, 0.0f).NormalizeCopy();
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  mtrl_.diffuse_color = azer::Vector4(0.0f, 0.65f, 0.75f, 1.0f);
  mtrl_.ambient_color = azer::Vector4(0.5f, 0.65f, 0.75f, 1.0f);
  mtrl_.specular_color = azer::Vector4(1.0f, .25f, 0.0f, 1.0f);
}


void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  vdataptr_.reset(new azer::VertexData(effect_->GetVertexDesc(), 
                                      tile_.vertices_num()));
  DiffuseEffect::Vertex* v = (DiffuseEffect::Vertex*)vdataptr_->pointer();
  DiffuseEffect::Vertex* vend = 
      tile_.InitVerticesData<DiffuseEffect::Vertex>(v, VertexInit());
  tick_ += 1.0f / 30.0f;
  hfield_.reset(new Ocean(&tile_, 1.0f));
  hfield_->SimulateWaveFFT<DiffuseEffect::Vertex>(tick_, v);
 
  DCHECK_EQ(vend - v, tile_.vertices_num());
 
  azer::IndicesData idata(tile_.indices_num(), azer::IndicesData::kUint32);
  tile_.CalcIndices((int32*)idata.pointer());
  azer::VertexBuffer::Options vbopt;
  vbopt.cpu_access = azer::kCPUWrite;
  vbopt.usage = azer::GraphicBuffer::kDynamic;
  vb_.reset(rs->CreateVertexBuffer(vbopt, vdataptr_.get()));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), &idata));
}


void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);

  if( ::GetAsyncKeyState('Z') & 0x8000f ) {
    uint32 size = tile_.vertices_num() * sizeof(DiffuseEffect::Vertex);
    DiffuseEffect::Vertex* v = (DiffuseEffect::Vertex*)vdataptr_->pointer();
    tick_ += 1.0f / 30.0f;
    hfield_->SimulateWaveFFT<DiffuseEffect::Vertex>(tick_, v);
    azer::HardwareBufferDataPtr hbptr(vb_->map(azer::kWriteDiscard));
    memcpy(hbptr->data_ptr(), v, size);
    vb_->unmap();
  }
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
  effect_->SetCameraPos(azer::Vector4(camera_.position(), 1.0f));
  effect_->SetWorld(world);
  effect_->SetDirLight(light_);
  effect_->SetMaterial(mtrl_);
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

