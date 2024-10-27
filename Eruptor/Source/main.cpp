#include "erupch.hpp"

#include "application/application.hpp"
#include "shader_compiler/shader_compiler.hpp"

int main()
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   try
   {
      std::cout << eru::compile_shader("resources/shaders/shader.vert").size();
      eru::application{}.run();
   }
   catch (std::exception const exception)
   {
      std::cout << std::format("exception caught: {}\n", exception.what());
   }

   glfwTerminate();

   return 0;
}