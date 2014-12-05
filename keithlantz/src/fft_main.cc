#include "ocean.h"

int _tmain(int argc, char *argv[]) {

  // constants
  const int WIDTH  = 1280, HEIGHT = 720;
	
  // buffer for grabs
  cBuffer buffer(WIDTH, HEIGHT);

  // controls
  // cKeyboard kb; int key_up, key_down, key_left, key_right, keyx, keyz;
  // cJoystick js; joystick_position jp[2];
  // cMouse    ms; mouse_state mst;

  // timers
  cTimer t0; double elapsed0; cTimer t1; double elapsed1; cTimer t2; double elapsed2; cTimer video; double elapsed_video = 0.1;

  // application is active.. fullscreen flag.. screen grab.. video grab..
  bool active = true, fullscreen = false, screen_grab = false, video_grab = false;

  // setup an opengl context.. initialize extension wrangler
  SDL_Surface *screen = my_SDL_init(WIDTH, HEIGHT, fullscreen);
  SDL_Event event;

  // ocean simulator
  cOcean ocean(64, 0.0005f, vector2(0.0f,32.0f), 64, false);

  // model view projection matrices and light position
  glm::mat4 Projection = glm::perspective(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f); 
  glm::mat4 View       = glm::mat4(1.0f);
  glm::mat4 Model      = glm::mat4(1.0f);
  glm::vec3 light_position;

  // rotation angles and viewpoint
  float alpha =   0.0f, beta =   0.0f, gamma =   0.0f,
      pitch =   0.0f, yaw  =   0.0f, roll  =   0.0f,
      x     =   0.0f, y    =   0.0f, z     = -20.0f;

  while(active) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          active = false;
          break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_g: screen_grab  = true; break;
            case SDLK_v: video_grab  ^= true; elapsed_video = 0.0; break;
            case SDLK_f:
              fullscreen ^= true;
              screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, (fullscreen ? SDL_FULLSCREEN : 0) | SDL_HWSURFACE | SDL_OPENGL);
              break;
          }
          break;
      }
    }

    // time elapsed since last frame
    elapsed0 = t0.elapsed(true);

    // update frame based on input state
    /*if (kb.getKeyState(KEY_UP))    alpha += 180.0f*elapsed0;
      if (kb.getKeyState(KEY_DOWN))  alpha -= 180.0f*elapsed0;
      if (kb.getKeyState(KEY_LEFT))  beta  -= 180.0f*elapsed0;
      if (kb.getKeyState(KEY_RIGHT)) beta  += 180.0f*elapsed0;
      jp[0] = js.joystickPosition(0);
      jp[1] = js.joystickPosition(1);
      yaw   += jp[1].x*elapsed0*90;
      pitch += jp[1].y*elapsed0*90;
      x     += -cos(-yaw*M_PI/180.0f)*jp[0].x*elapsed0*30 + sin(-yaw*M_PI/180.0f)*jp[0].y*elapsed0*30;
      z     +=  cos(-yaw*M_PI/180.0f)*jp[0].y*elapsed0*30 + sin(-yaw*M_PI/180.0f)*jp[0].x*elapsed0*30;
    */
    /*
    mst = ms.getMouseState();
    yaw   +=  mst.axis[0]*elapsed0*20;
    pitch += -mst.axis[1]*elapsed0*20;

    key_up    = kb.getKeyState(KEY_W);
    key_down  = kb.getKeyState(KEY_S);
    key_left  = kb.getKeyState(KEY_A);
    key_right = kb.getKeyState(KEY_D);
    keyx = -key_left +  key_right;
    keyz =  key_up   + -key_down;
    x     += -cos(-yaw*M_PI/180.0f)*keyx*elapsed0*30 + sin(-yaw*M_PI/180.0f)*keyz*elapsed0*30;
    z     +=  cos(-yaw*M_PI/180.0f)*keyz*elapsed0*30 + sin(-yaw*M_PI/180.0f)*keyx*elapsed0*30;
    */

    // rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // viewing and light position
    View  = glm::mat4(1.0f);
    View  = glm::rotate(View, pitch, glm::vec3(-1.0f, 0.0f, 0.0f));
    View  = glm::rotate(View, yaw,   glm::vec3(0.0f, 1.0f, 0.0f));
    View  = glm::translate(View, glm::vec3(x, -50, z));
    light_position = glm::vec3(1000.0f, 100.0f, -1000.0f);
    if (video_grab) {
      elapsed_video += 1.0f/30.0f;
      ocean.render(elapsed_video, light_position, Projection, View, Model, true);
      SDL_GL_SwapBuffers();
      buffer.save(true);
    } else {
      // ocean.render(t1.elapsed(false), light_position, Projection, View, Model, true);
      elapsed_video += 1.0f / 30.0f;
      ocean.render(elapsed_video, light_position, Projection, View, Model, true);
      SDL_GL_SwapBuffers();
    }

    if (screen_grab) { screen_grab = false; buffer.save(false); }
    //if (video_grab)  { elapsed_video += video.elapsed(true); if (elapsed_video >= 1/30.0) { buffer.save(true); elapsed_video = 0.0; } }
  }

  ocean.release();

  SDL_Quit();

  return 0;
}
