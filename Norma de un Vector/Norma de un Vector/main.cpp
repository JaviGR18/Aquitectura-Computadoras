#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

extern "C" float Norma(float* A, int N);

// Versión C++ para verificar
float normaCPP(const vector<float>& A) {
    float sum = 0.0f;
    for (int i = 0; i < (int)A.size(); i++)
        sum += A[i] * A[i];
    return sqrt(sum);
}

int main() {
    // Prueba 1: A = [3, 4]
    // Norma esperada: sqrt(3^2 + 4^2) = sqrt(9+16) = sqrt(25) = 5.0
    vector<float> A = { 3.0f, 4.0f };
    int N = (int)A.size();

    float resultASM = Norma(A.data(), N);
    float resultCPP = normaCPP(A);

    cout << "=== Norma de Vector - FPU x86 ===" << endl;
    cout << "Vector A      : [3, 4]" << endl;
    cout << "Norma ASM     : " << resultASM << endl;
    cout << "Norma C++     : " << resultCPP << endl;
    cout << "Diferencia    : " << fabs(resultASM - resultCPP) << endl;

    // Prueba 2: A = [1, 2, 3, 4, 5]
    // Norma esperada: sqrt(1+4+9+16+25) = sqrt(55) ≈ 7.416
    vector<float> B = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    int N2 = (int)B.size();

    float r2ASM = Norma(B.data(), N2);
    float r2CPP = normaCPP(B);

    cout << "\nVector B      : [1, 2, 3, 4, 5]" << endl;
    cout << "Norma ASM     : " << r2ASM << endl;
    cout << "Norma C++     : " << r2CPP << endl;
    cout << "Diferencia    : " << fabs(r2ASM - r2CPP) << endl;

    return 0;
}