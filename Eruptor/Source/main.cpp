#include "erupch.hpp"

#include "application/application.hpp"

int main()
{
   glfwInit();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

   try
   {
      eru::application{}.run();
   }
   catch (std::exception const exception)
   {
      std::cout << std::format("exception caught: {}\n", exception.what());
   }

   glfwTerminate();

   return 0;
}