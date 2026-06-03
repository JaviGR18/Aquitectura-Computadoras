#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

extern "C" void convertir_a_grises(
    unsigned char* src,
    unsigned char* dst,
    int total_pixeles
);

struct EncabezadoRAW {
    int renglones;
    int columnas;
    int bytes_por_pixel;
};

int calcular_padding(int total_bytes_datos) {
    int resto = (12 + total_bytes_datos) % 4;
    if (resto == 0) return 0;
    return 4 - resto;
}

bool leer_raw(const string& nombre_archivo, EncabezadoRAW& enc, vector<unsigned char>& datos) {
    ifstream archivo(nombre_archivo, ios::binary);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo: " << nombre_archivo << endl;
        return false;
    }
    archivo.read(reinterpret_cast<char*>(&enc), sizeof(EncabezadoRAW));
    cout << "Imagen leida: " << enc.renglones << " renglones x "
        << enc.columnas << " columnas, "
        << enc.bytes_por_pixel << " bytes/pixel" << endl;
    int total_bytes = enc.renglones * enc.columnas * enc.bytes_por_pixel;
    datos.resize(total_bytes);
    archivo.read(reinterpret_cast<char*>(datos.data()), total_bytes);
    archivo.close();
    return true;
}

bool guardar_raw(const string& nombre_archivo, const EncabezadoRAW& enc, const vector<unsigned char>& datos) {
    ofstream archivo(nombre_archivo, ios::binary);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo crear el archivo: " << nombre_archivo << endl;
        return false;
    }
    archivo.write(reinterpret_cast<const char*>(&enc), sizeof(EncabezadoRAW));
    archivo.write(reinterpret_cast<const char*>(datos.data()), datos.size());
    int padding = calcular_padding((int)datos.size());
    for (int i = 0; i < padding; i++) {
        char cero = 0;
        archivo.write(&cero, 1);
    }
    archivo.close();
    cout << "Archivo guardado: " << nombre_archivo << " (padding: " << padding << " bytes)" << endl;
    return true;
}

int main() {
    string archivo_entrada, archivo_salida;

    cout << "============================================" << endl;
    cout << "  Proyecto 1 - RGB a Escala de Grises (ASM)" << endl;
    cout << "============================================" << endl;
    cout << endl;

    cout << "Ingresa el nombre del archivo RAW de entrada: ";
    cin >> archivo_entrada;

    size_t punto = archivo_entrada.rfind('.');
    if (punto != string::npos)
        archivo_salida = archivo_entrada.substr(0, punto) + "_grises.raw";
    else
        archivo_salida = archivo_entrada + "_grises.raw";

    cout << "Archivo de salida: " << archivo_salida << endl << endl;

    EncabezadoRAW enc_entrada;
    vector<unsigned char> datos_rgb;

    if (!leer_raw(archivo_entrada, enc_entrada, datos_rgb))
        return 1;

    if (enc_entrada.bytes_por_pixel != 3) {
        cerr << "Error: El archivo debe ser RGB (3 bytes/pixel)." << endl;
        return 1;
    }

    int total_pixeles = enc_entrada.renglones * enc_entrada.columnas;
    vector<unsigned char> datos_grises(total_pixeles, 0);

    cout << "Convirtiendo " << total_pixeles << " pixeles..." << endl;
    convertir_a_grises(datos_rgb.data(), datos_grises.data(), total_pixeles);
    cout << "Conversion terminada." << endl;

    EncabezadoRAW enc_salida;
    enc_salida.renglones = enc_entrada.renglones;
    enc_salida.columnas = enc_entrada.columnas;
    enc_salida.bytes_por_pixel = 1;

    if (!guardar_raw(archivo_salida, enc_salida, datos_grises))
        return 1;

    cout << endl << "Proceso completado exitosamente." << endl;
    return 0;
}