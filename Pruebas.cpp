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
unsigned char* identificarTransformaciones(unsigned char* imgEncriptada, unsigned char* imgIM, unsigned char* mascara, unsigned int* datosTxt, int semilla, int num_pixeles, int tamaño);


int main()
{
    // Rutas de las imágenes y archivos
    QString rutaImgI_D = "Caso1/I_D.bmp";  // Imagen encriptada (I_D)
    QString rutaImgIM = "Caso1/I_M.bmp";
    QString rutaMascara = "Caso1/M.bmp";
    QString rutatxt = "Caso1/M2.txt";  // Para validar el enmascaramiento después de transformaciones

    int ancho = 0, alto = 0;

    // Cargar imágenes y otros datos necesarios
    unsigned char* imgI_D = cargarPixeles(rutaImgI_D, ancho, alto);  // I_D (antes P3)
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    int semilla = 0, num_pixeles = 0;
    unsigned int* datostxt = cargarSemillaYEnmascaramiento(rutatxt.toStdString().c_str(), semilla, num_pixeles);

    if (!imgI_D || !mascara || !datostxt) {
        cout << "Error cargando datos para la prueba." << endl;
        delete[] imgI_D;
        delete[] mascara;
        delete[] datostxt;
        return 1;
    }

    // Prueba de transformaciones sobre la imagen encriptada (I_D -> P2)
    unsigned char* imgIM = cargarPixeles(rutaImgIM, ancho, alto);

    if (!imgIM || !mascara || !datostxt) {
        cout << "Error cargando datos necesarios." << endl;
        delete[] imgIM;
        delete[] mascara;
        delete[] datostxt;
        return 1;
    }

    int tamaño = ancho * alto * 3;

    // Identificar transformaciones de I_D usando IM y verificar enmascaramiento con M2.txt
    unsigned char* posibleP2 = identificarTransformaciones(imgI_D, imgIM, mascara, datostxt, semilla, num_pixeles, tamaño);

    if (posibleP2) {
        exportarImagen(posibleP2, ancho, alto, "Caso1/Posible_P2.bmp");
        delete[] posibleP2;
    } else {
        cout << "No se encontró una transformación válida para P2." << endl;
    }

    delete[] imgI_D;
    delete[] imgIM;
    delete[] mascara;
    delete[] datostxt;

    // Comparar la imagen posible P2 con la imagen P2 original
    cout << "Comparando Posible_P2 con P2.bmp" << endl;
    int anchoP2 = 0, altoP2 = 0;
    unsigned char* realP2 = cargarPixeles("Caso1/P2.bmp", anchoP2, altoP2);
    unsigned char* posibleP2Comparar = cargarPixeles("Caso1/Posible_P2.bmp", anchoP2, altoP2);

    if (!realP2 || !posibleP2Comparar || anchoP2 != ancho || altoP2 != alto) {
        cout << "No se pudo cargar alguna imagen o no coinciden en dimensiones." << endl;
    } else {
        int total = anchoP2 * altoP2 * 3;
        bool iguales = true;
        for (int i = 0; i < total; ++i) {
            if (realP2[i] != posibleP2Comparar[i]) {
                cout << "Diferencia encontrada en el byte " << i << ": P2 = "
                     << (int)realP2[i] << ", Posible_P2 = " << (int)posibleP2Comparar[i] << endl;
                iguales = false;
                break;
            }
        }
        if (iguales) {
            cout << "Posible_P2 es igual a P2." << endl;
        } else {
            cout << "Posible_P2 NO es igual a P2." << endl;
        }
    }

    delete[] realP2;
    delete[] posibleP2Comparar;

    return 0;
}
//== FUNCIONES (sin modificar) ====
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
    cout << "Verificando enmascaramiento." << endl;
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
        cout << "El enmascaramiento coincide con los valores del txt." << endl;
    } else {
        cout << "El enmascaramiento no coincide con los valores del txt." << endl;
    }

    return correcto;
}
unsigned char* identificarTransformaciones(unsigned char* imgEncriptada, unsigned char* imgIM, unsigned char* mascara, unsigned int* datosTxt, int semilla, int num_pixeles, int tamaño) {
    // === Intento 1: aplicar XOR ===
    unsigned char* copiaXOR = new unsigned char[tamaño];
    memcpy(copiaXOR, imgEncriptada, tamaño);
    aplicarXOR(copiaXOR, imgIM, tamaño);
    cout << "Probando XOR..." << endl;
    if (validarEnmascaramiento(copiaXOR, mascara, datosTxt, semilla, num_pixeles)) {
        cout << "Transformacion correcta: XOR." << endl;
        return copiaXOR;
    }
    delete[] copiaXOR;
    cout << "XOR no valido. Probando rotaciones..." << endl;

    // === Intento 2: probar rotaciones ===
    for (int bits = 1; bits <= 7; ++bits) {
        // Rotar a la izquierda
        unsigned char* copia = new unsigned char[tamaño];
        memcpy(copia, imgEncriptada, tamaño);
        rotarBitsIzquierda(copia, bits, tamaño);
        cout << "Probando rotacion a la izquierda de " << bits << " bits..." << endl;
        if (validarEnmascaramiento(copia, mascara, datosTxt, semilla, num_pixeles)) {
            cout << "Transformacion correcta: rotacion a la izquierda de " << bits << " bits." << endl;
            return copia;
        }
        delete[] copia;

        // Rotar a la derecha
        copia = new unsigned char[tamaño];
        memcpy(copia, imgEncriptada, tamaño);
        rotarBitsDerecha(copia, bits, tamaño);
        cout << "Probando rotacion a la derecha de " << bits << " bits..." << endl;
        if (validarEnmascaramiento(copia, mascara, datosTxt, semilla, num_pixeles)) {
            cout << "Transformacion correcta: rotacion a la derecha de " << bits << " bits." << endl;
            return copia;
        }
        delete[] copia;
    }

    cout << "Ninguna rotacion coincidio con los datos del enmascaramiento." << endl;
    return nullptr;
}
