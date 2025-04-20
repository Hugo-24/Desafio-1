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

//main
int main()
{
    QString rutaImgEncriptada = "Caso1/P3.bmp";      // Imagen que queremos desencriptar
    QString rutaImgP2 = "Caso1/P2.bmp";
    QString rutaImgIM = "Caso1/I_M.bmp";             // Imagen IM para XOR
    QString rutaMascara = "Caso1/M.bmp";             // Mascara M
    QString rutaTxt = "Caso1/M2.txt";                // Archivo con semilla y valores esperados

    int ancho = 0, alto = 0;
    unsigned char* imgActual = cargarPixeles(rutaImgEncriptada, ancho, alto);
    unsigned char* imgIM = cargarPixeles(rutaImgIM, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);

    int semilla = 0, num_pixeles = 0;
    unsigned int* datostxt = cargarSemillaYEnmascaramiento(rutaTxt.toStdString().c_str(), semilla, num_pixeles);

    if (!imgActual || !imgIM || !mascara || !datostxt) {
        cout << "Error cargando datos necesarios." << endl;
        delete[] imgActual;
        delete[] imgIM;
        delete[] mascara;
        delete[] datostxt;
        return 1;
    }

    // === Intento 1: aplicar XOR ===
    aplicarXOR(imgActual, imgIM, ancho * alto * 3);
    cout << "Probando XOR..." << endl;
    if (validarEnmascaramiento(imgActual, mascara, datostxt, semilla, num_pixeles)) {
        cout << "Transformacion correcta: XOR." << endl;
    } else {
        cout << "XOR no valido. Probando rotaciones..." << endl;

        // === Intento 2: probar rotaciones ===
        bool encontrada = false;
        for (int bits = 1; bits <= 7; ++bits) {
            // Rotar a la izquierda
            unsigned char* copia = cargarPixeles(rutaImgEncriptada, ancho, alto);
            rotarBitsIzquierda(copia, bits, ancho * alto * 3);
            cout << "Probando rotacion a la izquierda de " << bits << " bits..." << endl;
            if (validarEnmascaramiento(copia, mascara, datostxt, semilla, num_pixeles)) {
                cout << "Transformacion correcta: rotacion a la izquierda de " << bits << " bits." << endl;
                encontrada = true;
                delete[] copia;
                break;
            }
            delete[] copia;

            // Rotar a la derecha
            copia = cargarPixeles(rutaImgEncriptada, ancho, alto);
            rotarBitsDerecha(copia, bits, ancho * alto * 3);
            cout << "Probando rotacion a la derecha de " << bits << " bits..." << endl;
            if (validarEnmascaramiento(copia, mascara, datostxt, semilla, num_pixeles)) {
                cout << "Transformacion correcta: rotacion a la derecha de " << bits << " bits." << endl;
                encontrada = true;
                delete[] copia;
                break;
            }
            delete[] copia;
        }

        if (!encontrada) {
            cout << "Ninguna rotacion coincidio con los datos del enmascaramiento." << endl;
        }
    }

    delete[] imgActual;
    delete[] imgIM;
    delete[] mascara;
    delete[] datostxt;
    // Verificar si P3 XOR IM da como resultado P2
    cout << "\nVerificando si P3 XOR IM es igual a P2..." << endl;

    int anchoP2 = 0, altoP2 = 0;
    unsigned char* imgP2 = cargarPixeles(rutaImgP2, anchoP2, altoP2);
    unsigned char* copiaP3 = cargarPixeles(rutaImgEncriptada, ancho, alto);
    unsigned char* copiaIM = cargarPixeles(rutaImgIM, ancho, alto);

    if (!imgP2 || !copiaP3 || !copiaIM || anchoP2 != ancho || altoP2 != alto) {
        cout << "Error cargando imagenes para verificacion final." << endl;
    } else {
        aplicarXOR(copiaP3, copiaIM, ancho * alto * 3); // P3 XOR IM -> deberia dar P2

        bool iguales = true;
        for (int i = 0; i < ancho * alto * 3; ++i) {
            if (copiaP3[i] != imgP2[i]) {
                iguales = false;
                cout << "Diferencia en byte " << i << ": P3^IM = " << (int)copiaP3[i]
                     << ", P2 = " << (int)imgP2[i] << endl;
                break;
            }
        }

        if (iguales) {
            cout << "P3 XOR IM es exactamente igual a P2." << endl;
        } else {
            cout << "P3 XOR IM no coincide con P2." << endl;
        }

        delete[] copiaP3;
        delete[] copiaIM;
    }

    delete[] imgP2;
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
