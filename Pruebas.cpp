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
void aplicarXOR(unsigned char* imgA, unsigned char* imgB, int tamaño);
bool validarEnmascaramiento(unsigned char* imagen, unsigned char* mascara, unsigned int* valoresEsperados, int semilla, int num_pixeles);
bool generarYCompararEnmascaramiento(unsigned char* imagen, unsigned char* mascara, unsigned int* valoresEsperados, int semilla, int num_pixeles);

int main() {
    QString rutaP2 = "Caso1/P2.bmp";
    QString rutaMascara = "Caso1/M.bmp";
    QString rutatxt = "Caso1/M2.txt";

    int ancho = 0, alto = 0;
    unsigned char* imgP2 = cargarPixeles(rutaP2, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    int semilla = 0, num_pixeles = 0;
    unsigned int* valores = cargarSemillaYEnmascaramiento(rutatxt.toStdString().c_str(), semilla, num_pixeles);

    if (!imgP2 || !mascara || !valores) {
        cout << "Error cargando datos para la prueba." << endl;
        delete[] imgP2;
        delete[] mascara;
        delete[] valores;
        return 1;
    }
    generarYCompararEnmascaramiento(imgP2, mascara, valores, semilla, num_pixeles);

    delete[] imgP2;
    delete[] mascara;
    delete[] valores;

    return 0;
}
// ==== FUNCIONES (sin modificar) ====
unsigned char* cargarPixeles(QString rutaEntrada, int &ancho, int &alto) {
    QImage imagen(rutaEntrada);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << endl;
        return nullptr;
    }
    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    ancho = imagen.width();
    alto = imagen.height();
    int dataSize = ancho * alto * 3;
    unsigned char* datosPixeles = new unsigned char[dataSize];
    for (int y = 0; y < alto; ++y) {
        const uchar* srcLine = imagen.scanLine(y);
        unsigned char* dstLine = datosPixeles + y * ancho * 3;
        memcpy(dstLine, srcLine, ancho * 3);
    }
    return datosPixeles;
}

bool exportarImagen(unsigned char* datosPixeles, int ancho, int alto, QString rutaSalida) {
    QImage imagenSalida(ancho, alto, QImage::Format_RGB888);
    for (int y = 0; y < alto; ++y) {
        memcpy(imagenSalida.scanLine(y), datosPixeles + y * ancho * 3, ancho * 3);
    }
    if (!imagenSalida.save(rutaSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP modificada.";
        return false;
    } else {
        cout << "Imagen BMP modificada guardada como " << rutaSalida.toStdString() << endl;
        return true;
    }
}

unsigned int* cargarSemillaYEnmascaramiento(const char* rutaArchivo, int &semilla, int &num_pixeles) {
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }
    archivo >> semilla;
    int r, g, b;
    num_pixeles = 0;
    while (archivo >> r >> g >> b) {
        num_pixeles++;
    }
    archivo.close();
    archivo.open(rutaArchivo);
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }
    archivo >> semilla;
    unsigned int* rgb = new unsigned int[num_pixeles * 3];
    for (int i = 0; i < num_pixeles * 3; i += 3) {
        archivo >> r >> g >> b;
        rgb[i] = r;
        rgb[i + 1] = g;
        rgb[i + 2] = b;
    }
    archivo.close();

    cout << "Semilla: " << semilla << endl;
    cout << "Cantidad de pixeles leidos: " << num_pixeles << endl;

    return rgb;
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

void aplicarXOR(unsigned char* imgA, unsigned char* imgB, int tamaño) {
    for (int i = 0; i < tamaño; ++i) {
        imgA[i] = imgA[i] ^ imgB[i];
    }
}

bool validarEnmascaramiento(unsigned char* imagen, unsigned char* mascara, unsigned int* valorestxt, int semilla, int num_pixeles) {
    for (int k = 0; k < num_pixeles * 3; ++k) {
        int pos = k + semilla;
        unsigned int suma = imagen[pos] + mascara[k];
        if (suma != valorestxt[k]) {
            cout << "Diferencia encontrada en k = " << k << ", suma = " << suma << ", esperado = " << valorestxt[k] << endl;
            return false;
        }
    }
    cout << "Todos los valores del enmascaramiento coinciden." << endl;
    return true;
}
bool generarYCompararEnmascaramiento(unsigned char* imagen, unsigned char* mascara, unsigned int* valoresEsperados, int semilla, int num_pixeles) {
    cout << "\n== Verificando enmascaramiento desde P2 con M para comparar con M2.txt ==" << endl;

    bool correcto = true;
    for (int k = 0; k < num_pixeles * 3; ++k) {
        int pos = k + semilla;
        unsigned int suma = imagen[pos] + mascara[k];
        if (suma != valoresEsperados[k]) {
            cout << "Diferencia encontrada: imagen[" << pos << "] = " << (int)imagen[pos]
                 << ", mascara[" << k << "] = " << (int)mascara[k]
                 << ", suma = " << suma << ", esperado = " << valoresEsperados[k] << endl;
            correcto = false;
            break;
        }
    }

    if (correcto) {
        cout << "El enmascaramiento desde P2 coincide con los valores de M2.txt." << endl;
    } else {
        cout << "El enmascaramiento desde P2 NO coincide con los valores de M2.txt." << endl;
    }

    return correcto;
}
