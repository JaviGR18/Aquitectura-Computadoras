// ============================================================
//  main.cpp  -  Producto Punto: FPU vs SSE vs AVX
//  Visual Studio Community 2022  |  x64  |  Release o Debug
//  Llama funciones ASM: dot_fpu, dot_sse, dot_avx
//  Mide ciclos con rdtsc_start / rdtsc_end
//  Imprime resultados + volcado de memoria (registros XMM/YMM)
// ============================================================

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>      // uint64_t  (estandar C++11, no necesita windows.h)
#include <intrin.h>     // para __cpuid, _mm_*, _mm256_*
#include <immintrin.h>  // AVX intrinsics (para el volcado manual)

// ── Prototipos de funciones ASM ─────────────────────────────
extern "C" {
    float    dot_fpu(const float* A, const float* B, int N);
    float    dot_sse(const float* A, const float* B, int N);
    float    dot_avx(const float* A, const float* B, int N);
    uint64_t rdtsc_start();   // devuelve TSC serializado antes de la funcion
    uint64_t rdtsc_end();     // devuelve TSC serializado despues de la funcion
}

// ── Constante de prueba ──────────────────────────────────────
static const int N = 1000;

// ============================================================
//  Inicializa vectores con valores conocidos:
//  A[i] = (i+1) * 0.001f   B[i] = (i+1) * 0.002f
//  => producto punto exacto = sum_{i=1}^{N} i^2 * 2e-6
// ============================================================
static void init_vectors(float* A, float* B, int n) {
    for (int i = 0; i < n; ++i) {
        A[i] = (float)(i + 1) * 0.001f;
        B[i] = (float)(i + 1) * 0.002f;
    }
}

// ============================================================
//  Referencia en C (doble precision) para validar resultados
// ============================================================
static double dot_ref(const float* A, const float* B, int n) {
    double acc = 0.0;
    for (int i = 0; i < n; ++i)
        acc += (double)A[i] * (double)B[i];
    return acc;
}

// ============================================================
//  Volcado de memoria de un bloque de floats
//  Muestra la representacion hexadecimal de cada float (4 bytes)
//  como se verian en memoria (little-endian)
// ============================================================
static void dump_mem(const char* label, const float* ptr, int count) {
    std::cout << "\n--- Volcado de memoria: " << label
        << " [" << count << " flotantes] ---\n";
    std::cout << std::hex << std::uppercase;

    for (int i = 0; i < count; ++i) {
        // Alias type-safe via memcpy
        unsigned int bits = 0;
        std::memcpy(&bits, ptr + i, sizeof(bits));

        // Direccion de memoria del elemento
        std::cout << "  [" << std::setw(3) << std::dec << i << "]  "
            << "addr=0x" << std::hex << std::setw(16)
            << std::setfill('0')
            << reinterpret_cast<uintptr_t>(ptr + i)
            << "  bytes=";

        // Bytes en memoria (little-endian: byte 0 en la direccion mas baja)
        const unsigned char* b = reinterpret_cast<const unsigned char*>(ptr + i);
        for (int j = 0; j < 4; ++j)
            std::cout << std::setw(2) << std::setfill('0')
            << (unsigned int)b[j] << " ";

        // Valor float
        std::cout << std::dec << std::setfill(' ')
            << " = " << std::setw(10) << std::fixed
            << std::setprecision(6) << ptr[i] << "\n";
    }
    std::cout << std::dec << std::nouppercase << std::defaultfloat;
}

// ============================================================
//  Muestra como se veran los primeros 8 elementos de A y B
//  tal como los cargaria un registro YMM (AVX, 256 bits)
// ============================================================
static void show_ymm_load(const float* A, const float* B) {
    std::cout << "\n--- Simulacion registro YMM (256 bits = 8 x float32) ---\n";
    std::cout << "  YMM_A = [ ";
    for (int i = 7; i >= 0; --i)  // los YMM muestran lane alta a la izquierda
        std::cout << std::fixed << std::setprecision(4) << A[i]
        << (i ? " | " : "");
    std::cout << " ]  (lane7..lane0)\n";

    std::cout << "  YMM_B = [ ";
    for (int i = 7; i >= 0; --i)
        std::cout << std::fixed << std::setprecision(4) << B[i]
        << (i ? " | " : "");
    std::cout << " ]  (lane7..lane0)\n";

    // Producto lane a lane usando intrinsics reales
    __m256 va = _mm256_loadu_ps(A);
    __m256 vb = _mm256_loadu_ps(B);
    __m256 vr = _mm256_mul_ps(va, vb);

    alignas(32) float res[8];
    _mm256_store_ps(res, vr);

    std::cout << "  YMM_R = [ ";
    for (int i = 7; i >= 0; --i)
        std::cout << std::fixed << std::setprecision(6) << res[i]
        << (i ? " | " : "");
    std::cout << " ]  (A*B lane a lane)\n";
    _mm256_zeroupper();
}

// ============================================================
//  Muestra como se ve un registro XMM (SSE, 128 bits)
// ============================================================
static void show_xmm_load(const float* A, const float* B) {
    std::cout << "\n--- Simulacion registro XMM (128 bits = 4 x float32) ---\n";
    std::cout << "  XMM_A = [ ";
    for (int i = 3; i >= 0; --i)
        std::cout << std::fixed << std::setprecision(4) << A[i]
        << (i ? " | " : "");
    std::cout << " ]  (lane3..lane0)\n";

    __m128 va = _mm_loadu_ps(A);
    __m128 vb = _mm_loadu_ps(B);
    __m128 vr = _mm_mul_ps(va, vb);

    alignas(16) float res[4];
    _mm_store_ps(res, vr);

    std::cout << "  XMM_R = [ ";
    for (int i = 3; i >= 0; --i)
        std::cout << std::fixed << std::setprecision(6) << res[i]
        << (i ? " | " : "");
    std::cout << " ]  (A*B lane a lane)\n";
}

// ============================================================
//  Mide ciclos ejecutando la funcion varias veces y tomando
//  la mediana para reducir el ruido del sistema operativo
// ============================================================
static uint64_t measure_cycles(
    float (*fn)(const float*, const float*, int),
    const float* A, const float* B, int n,
    int reps = 9)
{
    uint64_t samples[16];
    for (int r = 0; r < reps; ++r) {
        uint64_t t0 = rdtsc_start();
        volatile float sink = fn(A, B, n);   // volatile impide eliminacion
        uint64_t t1 = rdtsc_end();
        (void)sink;
        samples[r] = t1 - t0;
    }
    // ordenar (burbuja simple, reps pequeno)
    for (int i = 0; i < reps - 1; ++i)
        for (int j = i + 1; j < reps; ++j)
            if (samples[j] < samples[i]) {
                uint64_t tmp = samples[i];
                samples[i] = samples[j];
                samples[j] = tmp;
            }
    return samples[reps / 2];  // mediana
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    // ── Asignar vectores alineados a 32 bytes (requerido por AVX) ──
    float* A = static_cast<float*>(_aligned_malloc(N * sizeof(float), 32));
    float* B = static_cast<float*>(_aligned_malloc(N * sizeof(float), 32));
    if (!A || !B) {
        std::cerr << "Error al asignar memoria alineada.\n";
        return 1;
    }

    init_vectors(A, B, N);

    // ── Referencia de precision ──────────────────────────────────
    double ref = dot_ref(A, B, N);

    // ============================================================
    //  VOLCADO DE MEMORIA  (primeros 10 elementos de A y B)
    // ============================================================
    std::cout << "========================================================\n";
    std::cout << "  VOLCADO DE MEMORIA  (primeros 10 elementos)\n";
    std::cout << "========================================================\n";
    dump_mem("Vector A", A, 10);
    dump_mem("Vector B", B, 10);

    // ── Visualizacion de registros SIMD ─────────────────────────
    show_xmm_load(A, B);
    show_ymm_load(A, B);

    // ============================================================
    //  CALCULO DEL PRODUCTO PUNTO
    // ============================================================
    float r_fpu = dot_fpu(A, B, N);
    float r_sse = dot_sse(A, B, N);
    float r_avx = dot_avx(A, B, N);

    // ============================================================
    //  MEDICION DE CICLOS (mediana de 9 corridas)
    // ============================================================
    uint64_t c_fpu = measure_cycles(dot_fpu, A, B, N);
    uint64_t c_sse = measure_cycles(dot_sse, A, B, N);
    uint64_t c_avx = measure_cycles(dot_avx, A, B, N);

    // ── Primera medicion raw (una sola llamada) ──────────────────
    uint64_t t0, t1;

    t0 = rdtsc_start(); dot_fpu(A, B, N); t1 = rdtsc_end();
    uint64_t raw_fpu = t1 - t0;

    t0 = rdtsc_start(); dot_sse(A, B, N); t1 = rdtsc_end();
    uint64_t raw_sse = t1 - t0;

    t0 = rdtsc_start(); dot_avx(A, B, N); t1 = rdtsc_end();
    uint64_t raw_avx = t1 - t0;

    // ============================================================
    //  RESULTADOS
    // ============================================================
    std::cout << "\n========================================================\n";
    std::cout << "  RESULTADOS  -  Producto Punto  (N = " << N << ")\n";
    std::cout << "========================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    std::cout << "\n  Referencia (C double): " << ref << "\n\n";

    auto err = [&](float r) {
        return std::fabs((double)r - ref) / std::fabs(ref) * 100.0;
        };

    std::cout << "  Unidad   | Resultado       | Error rel. | Ciclos (raw) | Ciclos (mediana)\n";
    std::cout << "  ---------+-----------------+------------+--------------+-----------------\n";
    std::cout << "  FPU      | " << std::setw(15) << r_fpu
        << " | " << std::setw(9) << std::setprecision(4) << err(r_fpu) << "%"
        << " | " << std::setw(12) << raw_fpu
        << " | " << std::setw(15) << c_fpu << "\n";
    std::cout << "  SSE      | " << std::setw(15) << std::setprecision(6) << r_sse
        << " | " << std::setw(9) << std::setprecision(4) << err(r_sse) << "%"
        << " | " << std::setw(12) << raw_sse
        << " | " << std::setw(15) << c_sse << "\n";
    std::cout << "  AVX      | " << std::setw(15) << std::setprecision(6) << r_avx
        << " | " << std::setw(9) << std::setprecision(4) << err(r_avx) << "%"
        << " | " << std::setw(12) << raw_avx
        << " | " << std::setw(15) << c_avx << "\n";

    // ── Speedup relativo ─────────────────────────────────────────
    std::cout << "\n  Speedup SSE vs FPU (mediana): "
        << std::setprecision(2) << (double)c_fpu / c_sse << "x\n";
    std::cout << "  Speedup AVX vs FPU (mediana): "
        << std::setprecision(2) << (double)c_fpu / c_avx << "x\n";
    std::cout << "  Speedup AVX vs SSE (mediana): "
        << std::setprecision(2) << (double)c_sse / c_avx << "x\n";

    // ── Volcado final del ultimo resultado en memoria ─────────────
    std::cout << "\n--- Volcado de resultados finales en memoria ---\n";
    float results[3] = { r_fpu, r_sse, r_avx };
    const char* labels[3] = { "FPU", "SSE", "AVX" };
    std::cout << std::hex << std::uppercase;
    for (int i = 0; i < 3; ++i) {
        unsigned int bits = 0;
        std::memcpy(&bits, &results[i], 4);
        std::cout << "  " << labels[i] << " resultado: "
            << "addr=0x" << std::setw(16) << std::setfill('0')
            << reinterpret_cast<uintptr_t>(&results[i])
            << "  hex=0x" << std::setw(8) << bits
            << "  float=" << std::dec << std::setfill(' ')
            << std::fixed << std::setprecision(6) << results[i] << "\n";
    }
    std::cout << std::dec << std::nouppercase << std::defaultfloat;

    std::cout << "\n========================================================\n";
    std::cout << "  Presiona Enter para salir...\n";
    std::cin.get();

    _aligned_free(A);
    _aligned_free(B);
    return 0;
}