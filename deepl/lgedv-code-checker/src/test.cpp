#include <iostream>

int multipleReturns() {
  int * ptr = new int(5);
  if (ptr == nullptr) {
    return -1; // First return
  }
  if (*ptr < 0) {
    return 0; // Second return
  }
  int a = 1;
  if (a > 0) {
    return a; // First return
  }
  if (a < 0) {
    return -a; // Second return
  }
  // No return here, but function type is int, so this is fine.
}

int main() {
  int x = 2;
  switch (x) {
    case 1:
      std::cout << "Case 1" << std::endl;
      break;
    case 2: {
      std::cout << "Case 2" << std::endl; // Missing break
      break;
    }
    case 3:
      std::cout << "Case 3" << std::endl;
      break;
    case 4:
    std::cout << "Case 4" << std::endl;
    default:
      std::cout << "Default" << std::endl;
  }
  return 0;
}