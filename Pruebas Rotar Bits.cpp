#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

// Declaración de funciones
unsigned char* cargarPixeles(QString rutaEntrada, int &ancho, int &alto);
bool exportarImagen(unsigned char* datosPixeles, int ancho, int alto, QString rutaSalida);
unsigned int* cargarSemillaYEnmascaramiento(const char* rutaArchivo, int &semilla, int &num_pixeles);
void rotarBitsDerecha(unsigned char* imagen, int bits, int tamaño);
void rotarBitsIzquierda(unsigned char* imagen, int bits, int tamaño);

int main()
{
    // 1. Cargar imagen original
    QString rutaImagenEntrada = "Caso1/I_O.bmp";
    QString rutaImagenRotada = "Caso1/I_Rotada.bmp";
    QString rutaImagenRestaurada = "Caso1/I_Restaurada.bmp";

    int alto = 0, ancho = 0;
    unsigned char* datosPixeles = cargarPixeles(rutaImagenEntrada, ancho, alto);

    if (datosPixeles == nullptr) {
        cout << "Error: No se pudo cargar la imagen." << endl;
        return 1;
    }

    // 2. Aplicar rotación de 3 bits a la derecha
    rotarBitsDerecha(datosPixeles, 3, ancho * alto * 3);
    if (!exportarImagen(datosPixeles, ancho, alto, rutaImagenRotada)) {
        cout << "Error al guardar imagen rotada." << endl;
    }

    // 3. Revertir rotación (3 bits a la izquierda)
    rotarBitsIzquierda(datosPixeles, 3, ancho * alto * 3);
    if (!exportarImagen(datosPixeles, ancho, alto, rutaImagenRestaurada)) {
        cout << "Error al guardar imagen restaurada." << endl;
    }

    // 4. Liberar memoria
    delete[] datosPixeles;

    // Opcional: Cargar datos de enmascaramiento para pruebas
    int semilla = 0, num_pixeles = 0;
    unsigned int* datosEnmascaramiento = cargarSemillaYEnmascaramiento("Caso1/M1.txt", semilla, num_pixeles);

    if (datosEnmascaramiento != nullptr) {
        cout << "\nDatos de enmascaramiento:" << endl;
        for (int i = 0; i < num_pixeles * 3; i += 3) {
            cout << "Pixel " << i/3 << ": ("
                 << datosEnmascaramiento[i] << ", "
                 << datosEnmascaramiento[i+1] << ", "
                 << datosEnmascaramiento[i+2] << ")" << endl;
        }
        delete[] datosEnmascaramiento;
    }

    return 0;
}

// Implementación de funciones
unsigned char* cargarPixeles(QString rutaEntrada, int &ancho, int &alto) {
    QImage imagen(rutaEntrada);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << endl;
        return nullptr;
    }

    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    ancho = imagen.width();
    alto = imagen.height();

    unsigned char* datosPixeles = new unsigned char[ancho * alto * 3];
    for (int y = 0; y < alto; ++y) {
        memcpy(datosPixeles + y * ancho * 3, imagen.scanLine(y), ancho * 3);
    }

    return datosPixeles;
}

bool exportarImagen(unsigned char* datosPixeles, int ancho, int alto, QString rutaSalida) {
    QImage imagenSalida(ancho, alto, QImage::Format_RGB888);
    for (int y = 0; y < alto; ++y) {
        memcpy(imagenSalida.scanLine(y), datosPixeles + y * ancho * 3, ancho * 3);
    }
    return imagenSalida.save(rutaSalida, "BMP");
}

unsigned int* cargarSemillaYEnmascaramiento(const char* rutaArchivo, int &semilla, int &num_pixeles) {
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        cout << "Error al abrir archivo de enmascaramiento." << endl;
        return nullptr;
    }

    // Contar número de píxeles
    num_pixeles = 0;
    int r, g, b;
    archivo >> semilla; // Leer semilla
    while (archivo >> r >> g >> b) num_pixeles++;

    // Leer datos
    archivo.clear();
    archivo.seekg(0);
    unsigned int* datos = new unsigned int[num_pixeles * 3];
    archivo >> semilla; // Leer semilla nuevamente
    for (int i = 0; i < num_pixeles * 3; i += 3) {
        archivo >> datos[i] >> datos[i+1] >> datos[i+2];
    }

    return datos;
}

void rotarBitsDerecha(unsigned char* imagen, int bits, int tamaño) {
    for (int i = 0; i < tamaño; i++) {
        imagen[i] = (imagen[i] >> bits) | (imagen[i] << (8 - bits));
    }
}

void rotarBitsIzquierda(unsigned char* imagen, int bits, int tamaño) {
    for (int i = 0; i < tamaño; i++) {
        imagen[i] = (imagen[i] << bits) | (imagen[i] >> (8 - bits));
    }
}
