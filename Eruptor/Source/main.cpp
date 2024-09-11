#include "erupch.hpp"

#include "Application/Application.hpp"

int main()
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   try
   {
      eru::Application{}.run();
   }
   catch (std::exception const exception)
   {
      std::cout << std::format("Exception caught: {}\n", exception.what());
   }

   glfwTerminate();

   return 0;
}