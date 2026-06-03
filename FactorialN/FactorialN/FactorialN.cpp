// main.cpp
#include <iostream>
extern "C" int factorial(int n);  // declara la funcion del .asm

int main() {
    int n = 5;
    std::cout << n << "! = " << factorial(n) << std::endl;
    return 0;
}