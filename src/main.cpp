#include "app.h"

int main() {
 App app;
 app.init_window();

 while(app.is_running()) {
   glfwPollEvents();

   app.update();

   glClearColor(0, 0, 0, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 } 

 app.terminate_window();
}
