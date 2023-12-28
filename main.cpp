#include "rayner.hpp"
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    rn::Rayner ray;
    ray.run();
  } catch (vk::SystemError &err) {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit(-1);
  } catch (std::exception &err) {
    std::cout << "std::exception: " << err.what() << std::endl;
    exit(-1);
  } catch (...) {
    std::cout << "unknown errow\n";
    exit(-1);
  }

  return 0;
}