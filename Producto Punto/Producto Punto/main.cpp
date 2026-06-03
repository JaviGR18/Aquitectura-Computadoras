#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

extern "C" float DotProduct(float* A, float* B, int N);

// Versión C++ para verificar
float dotProductCPP(const vector<float>& A, const vector<float>& B) {
    float result = 0.0f;
    for (int i = 0; i < (int)A.size(); i++)
        result += A[i] * B[i];
    return result;
}

int main() {
    // A=[1,2,3,4,5]  B=[5,4,3,2,1]
    // Esperado: 1*5+2*4+3*3+4*2+5*1 = 35
    vector<float> A = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    vector<float> B = { 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
    int N = (int)A.size();

    float resultASM = DotProduct(A.data(), B.data(), N);
    float resultCPP = dotProductCPP(A, B);

    cout << "=== Producto Punto - FPU x86 ===" << endl;
    cout << "N             : " << N << endl;
    cout << "Resultado ASM : " << resultASM << endl;
    cout << "Resultado C++ : " << resultCPP << endl;
    cout << "Diferencia    : " << fabs(resultASM - resultCPP) << endl;

    // Segunda prueba
    int N2 = 8;
    vector<float> C = { 1.5f, 2.5f, 0.5f, 3.0f, -1.0f, 4.0f,  2.0f, -2.0f };
    vector<float> D = { 2.0f, 1.0f, 4.0f, 0.5f,  3.0f, 1.5f,  2.5f,  1.0f };

    float r2 = DotProduct(C.data(), D.data(), N2);
    cout << "\nPrueba 2 (N=8): " << r2 << endl;

    return 0;
}