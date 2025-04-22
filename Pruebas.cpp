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
void reconstruirImagenCaso1();
void reconstruirImagenCaso2();
int main()
{
    int opcion;
    cout << "Seleccione el caso a reconstruir (1 o 2): ";
    cin >> opcion;

    if (opcion == 1) {
        reconstruirImagenCaso1();
    } else if (opcion == 2) {
        reconstruirImagenCaso2();
    } else {
        cout << "Opcion invalida." << endl;
    }

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
            return false;
        }
    }
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
unsigned char* identificarTransformaciones(unsigned char* imgEncriptada, unsigned char* imgIM, unsigned char* mascara, unsigned int* datostxt, int semilla, int num_pixeles, int tamaño) {
    // === Intento 1: aplicar XOR ===
    unsigned char* copiaXOR = new unsigned char[tamaño];
    memcpy(copiaXOR, imgEncriptada, tamaño);
    aplicarXOR(copiaXOR, imgIM, tamaño);
    if (validarEnmascaramiento(copiaXOR, mascara, datostxt, semilla, num_pixeles)) {
        cout << "Transformacion correcta: XOR." << endl;
        return copiaXOR;
    }
    delete[] copiaXOR;
    // === Intento 2: probar rotaciones ===
    for (int bits = 1; bits <= 7; ++bits) {
        // Rotar a la izquierda
        unsigned char* copia = new unsigned char[tamaño];
        memcpy(copia, imgEncriptada, tamaño);
        rotarBitsIzquierda(copia, bits, tamaño);
        if (validarEnmascaramiento(copia, mascara, datostxt, semilla, num_pixeles)) {
            cout << "Transformacion correcta: rotacion a la izquierda de " << bits << " bits." << endl;
            return copia;
        }
        delete[] copia;

        // Rotar a la derecha
        copia = new unsigned char[tamaño];
        memcpy(copia, imgEncriptada, tamaño);
        rotarBitsDerecha(copia, bits, tamaño);
        if (validarEnmascaramiento(copia, mascara, datostxt, semilla, num_pixeles)) {
            cout << "Transformacion correcta: rotacion a la derecha de " << bits << " bits." << endl;
            return copia;
        }
        delete[] copia;
    }

    cout << "Ninguna rotacion coincidio con los datos del enmascaramiento." << endl;
    return nullptr;
}
void reconstruirImagenCaso1() {
    // === Paso 1: I_D → P2 ===
    QString rutaI_D = "Caso1/I_D.bmp";
    QString rutaIM = "Caso1/I_M.bmp";
    QString rutaMascara = "Caso1/M.bmp";
    QString rutaM2 = "Caso1/M2.txt";

    int ancho = 0, alto = 0;
    unsigned char* I_D = cargarPixeles(rutaI_D, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    unsigned char* IM = cargarPixeles(rutaIM, ancho, alto);
    int semilla2, numpix2;
    unsigned int* datosM2 = cargarSemillaYEnmascaramiento(rutaM2.toStdString().c_str(), semilla2, numpix2);

    if (!I_D || !mascara || !IM || !datosM2) {
        cout << "Error cargando archivos para paso I_D → P2" << endl;
        delete [] I_D;
        delete[] mascara;
        delete [] IM;
        delete[] datosM2;
        return;
    }

    int tamaño = ancho * alto * 3;
    unsigned char* P2 = identificarTransformaciones(I_D, IM, mascara, datosM2, semilla2, numpix2, tamaño);
    if (!P2) {
        cout << "Fallo al obtener P2 desde I_D." << endl;
        return;
    }
    exportarImagen(P2, ancho, alto, "Caso1/Posible_P2.bmp");

    // === Paso 2: P2 → P1 ===
    QString rutaM1 = "Caso1/M1.txt";
    int semilla1, numpix1;
    unsigned int* datosM1 = cargarSemillaYEnmascaramiento(rutaM1.toStdString().c_str(), semilla1, numpix1);

    if (!datosM1) {
        cout << "Error cargando M1.txt" << endl;
        delete [] I_D;
        delete[] mascara;
        delete [] IM;
        delete[] P2;
        delete[] datosM2;
        return ;
    }

    unsigned char* P1 = identificarTransformaciones(P2, IM, mascara, datosM1, semilla1, numpix1, tamaño);
    if (!P1) {
        cout << "Fallo al obtener P1 desde P2." << endl;
        delete[] P2;
        return;
    }
    exportarImagen(P1, ancho, alto, "Caso1/Posible_P1.bmp");

    // === Paso 3: P1 → I_O ===
    QString rutaM0 = "Caso1/M0.txt";
    int semilla0, numpix0;
    unsigned int* datosM0 = cargarSemillaYEnmascaramiento(rutaM0.toStdString().c_str(), semilla0, numpix0);

    if (!datosM0) {
        cout << "Error cargando M0.txt" << endl;
        delete [] I_D;
        delete[] mascara;
        delete [] IM;
        delete[] datosM2;
        delete[] P2;
        delete[] P1;
        delete[] datosM1;
        return;
    }

    unsigned char* IO = identificarTransformaciones(P1, IM, mascara, datosM0, semilla0, numpix0, tamaño);
    if (!IO) {
        cout << "Fallo al obtener I_O desde P1." << endl;
        delete[] P2;
        delete[] P1;
        return;
    }
    exportarImagen(IO, ancho, alto, "Caso1/Posible_I_O.bmp");

    // === Liberar memoria ===
    delete[] I_D;
    delete[] IM;
    delete[] mascara;
    delete[] datosM2;
    delete[] datosM1;
    delete[] datosM0;
    delete[] P2;
    delete[] P1;
    delete[] IO;

    return;
}

void reconstruirImagenCaso2() {
    QString rutaIM = "Caso2/I_M.bmp";
    QString rutaMascara = "Caso2/M.bmp";
    QString rutaID = "Caso2/I_D.bmp";

    int ancho = 0, alto = 0;
    unsigned char* IM = cargarPixeles(rutaIM, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    unsigned char* ID = cargarPixeles(rutaID, ancho, alto);

    if (!IM || !mascara || !ID) {
        cout << "Error cargando imágenes base del Caso 2." << endl;
        delete[] IM;
        delete[] mascara;
        delete[] ID;
        return ;
    }

    int tamaño = ancho * alto * 3;
    unsigned char* actual = ID;

    for (int i = 6; i >= 0; --i) {
        QString nombreArchivoTxt = QString("Caso2/M%1.txt").arg(i);
        int semilla, numpix;
        unsigned int* datos = cargarSemillaYEnmascaramiento(nombreArchivoTxt.toStdString().c_str(), semilla, numpix);
        if (!datos) {
            cout << "Error cargando " << nombreArchivoTxt.toStdString() << endl;
            delete[] IM;
            delete[] mascara;
            delete[] actual;
            return ;
        }

        unsigned char* siguiente = identificarTransformaciones(actual, IM, mascara, datos, semilla, numpix, tamaño);
        if (!siguiente) {
            cout << "Fallo al identificar transformación en paso " << i << endl;
            delete[] IM;
            delete[] mascara;
            delete[] actual;
            delete[] datos;
            return ;
        }

        QString nombreSalida = QString("Caso2/Posible_P%1.bmp").arg(i);
        if (i == 0) nombreSalida = "Caso2/Posible_I_O.bmp";
        exportarImagen(siguiente, ancho, alto, nombreSalida);

        // Liberar imagen anterior (excepto la primera que era ID)
        if (i < 6) {
            delete[] actual;
        }

        delete[] datos;
        actual = siguiente;
    }

    // === Liberar memoria final ===
    delete[] IM;
    delete[] mascara;
    delete[] actual;

    return ;
}
