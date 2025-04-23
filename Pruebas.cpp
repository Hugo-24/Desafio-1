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
void reconstruirImagenDesdeCaso(QString carpeta, int pasoMaximo);

//======== main ========
int main() {
    int opcion;
    cout << "Seleccione la opcion que desea (1) para desencriptar el Caso1, (2) para el Caso2 o (3) si desea hacerlo en una carpeta aparte: ";
    cin >> opcion;

    if (opcion == 1) {
        reconstruirImagenCaso1();
    } else if (opcion == 2) {
        reconstruirImagenCaso2();
    } else if (opcion == 3) {
        QString carpeta;
        int pasoMaximo;
        cout << "Ingrese el nombre de la carpeta donde se encuentra la imagen que desea desencriptar (por ejemplo: Caso1 o Caso2): ";
        string input;
        cin >> input;
        carpeta = QString::fromStdString(input);

        cout <<  "Ingrese el numero del ultimo enmascaramiento (ultimo archivo .txt, por ejemplo 2 o 6): ";
        cin >> pasoMaximo;

        reconstruirImagenDesdeCaso(carpeta, pasoMaximo);
    } else {
        cout << "Opcion invalida." << endl;
    }

    return 0;
}

//======== FUNCIONES ========
unsigned char* cargarPixeles(QString rutaEntrada, int &ancho, int &alto) {
  /*
  Carga una imagen BMP y extrae sus píxeles en formato RGB888 (sin canal alfa).

  Esta función abre un archivo de imagen BMP utilizando la clase QImage de Qt,
  convierte su formato a RGB888 (24 bits por píxel: 8 para R, 8 para G y 8 para B),
  y copia todos los píxeles en un arreglo lineal de tipo unsigned char.

  El arreglo resultante contiene los canales RGB de cada píxel de forma secuencial y sin padding.

  parametro: rutaEntrada Ruta del archivo BMP que se desea cargar (QString).
  parametro: ancho Referencia donde se guardará el ancho (en píxeles) de la imagen cargada.
  parametro: alto Referencia donde se guardará el alto (en píxeles) de la imagen cargada.
  return de un puntero a un arreglo dinámico de bytes con los datos RGB de la imagen cargada.
          Devuelve nullptr si la imagen no pudo cargarse.

  Es responsabilidad del usuario liberar la memoria con `delete[]`.
  */
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
        const uchar* srcLine = imagen.scanLine(y);               // Línea de la imagen original
        unsigned char* dstLine = datosPixeles + y * ancho * 3;   // Línea correspondiente en el arreglo
        memcpy(dstLine, srcLine, ancho * 3);                     // Copiar los 3 bytes por píxel (R, G, B)
    }

    return datosPixeles;
}

bool exportarImagen(unsigned char* datosPixeles, int ancho, int alto, QString rutaSalida) {
 /*
 Guarda una imagen BMP a partir de un arreglo de píxeles RGB.

  Esta función recibe un arreglo de bytes que representa una imagen en formato RGB888
  (3 bytes por píxel: R, G y B), construye un objeto QImage a partir de esos datos
  y guarda la imagen resultante en el disco en formato BMP.

  parametro: datosPixeles Puntero al arreglo de bytes RGB a exportar (RGB por píxel).
  parametro: ancho Ancho de la imagen (en píxeles).
  parametro: alto Alto de la imagen (en píxeles).
  parametro: rutaSalida Ruta del archivo BMP de salida (QString).
  return true si la imagen se guardó correctamente, false si ocurrió un error.

  Esta función no libera la memoria del arreglo datosPixeles.
 */
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
/*
  Carga desde un archivo .txt la semilla y los valores RGB del enmascaramiento.

  Esta función lee primero una semilla desde la primera línea del archivo,
  luego cuenta la cantidad de píxeles que aparecen (cada uno con 3 valores: R, G, B),
  y finalmente vuelve a abrir el archivo para cargar todos los datos RGB en un arreglo dinámico.

  parametro:rutaArchivo Ruta del archivo .txt que contiene la semilla y los valores RGB.
  parametro: semilla Variable de salida donde se guarda la semilla leída.
  parametro: num_pixeles Variable de salida que indica cuántos píxeles fueron leídos (tripletes RGB).
  return de un puntero a un arreglo dinámico de enteros que contiene todos los valores RGB esperados.
          Devuelve nullptr si hubo error al abrir o leer el archivo.

  El usuario debe liberar la memoria del arreglo con `delete[]`.
 */
    ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    archivo >> semilla;
    int r, g, b;
    num_pixeles = 0;
    while (archivo >> r >> g >> b) {
        num_pixeles++;  // Contamos cuántos tripletes de valores hay
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
        delete [] I_D;
        delete[] mascara;
        delete [] IM;
        delete[] datosM2;
        delete[] P1;
        delete[] datosM1;
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
        delete [] I_D;
        delete[] mascara;
        delete [] IM;
        delete[] datosM2;
        delete[] P2;
        delete[] P1;
        delete[] datosM1;
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
        return;
    }

    int tamaño = ancho * alto * 3;
    unsigned char* imgActual = ID;
    unsigned char* imgAnterior = nullptr;

    for (int i = 6; i >= 0; --i) {
        QString nombreArchivoTxt = QString("Caso2/M%1.txt").arg(i);
        int semilla, numpix;
        unsigned int* datos = cargarSemillaYEnmascaramiento(nombreArchivoTxt.toStdString().c_str(), semilla, numpix);
        if (!datos) {
            cout << "Error cargando " << nombreArchivoTxt.toStdString() << endl;
            if (imgAnterior != nullptr) {
                delete[] imgAnterior;
            }
            delete[] IM;
            delete[] mascara;
            delete[] imgActual;
            return;
        }

        unsigned char* imgSiguiente = identificarTransformaciones(imgActual, IM, mascara, datos, semilla, numpix, tamaño);
        if (!imgSiguiente) {
            cout << "Fallo al identificar transformación en paso " << i << endl;
            if (imgAnterior != nullptr) {
                delete[] imgAnterior;
            }
            delete[] IM;
            delete[] mascara;
            delete[] imgActual;
            delete[] datos;
            return;
        }

        QString nombreSalida = QString("Caso2/Posible_P%1.bmp").arg(i);
        if (i == 0) nombreSalida = "Caso2/Posible_I_O.bmp";
        exportarImagen(imgSiguiente, ancho, alto, nombreSalida);

        delete[] datos;

        if (imgAnterior != nullptr) {
            delete[] imgAnterior;
        }
        imgAnterior = imgActual;
        imgActual = imgSiguiente;
    }

    if (imgAnterior != nullptr) {
        delete[] imgAnterior;
    }
    delete[] IM;
    delete[] mascara;
    delete[] imgActual;

    return;
}
void reconstruirImagenDesdeCaso(QString carpeta, int ultimotxt) {
    QString rutaIM = carpeta + "/I_M.bmp";
    QString rutaMascara = carpeta + "/M.bmp";
    QString rutaID = carpeta + "/I_D.bmp";

    int ancho = 0, alto = 0;
    unsigned char* IM = cargarPixeles(rutaIM, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    unsigned char* ID = cargarPixeles(rutaID, ancho, alto);

    if (!IM || !mascara || !ID) {
        cout << "Error cargando imagenes base desde la carpeta " << carpeta.toStdString() << endl;
        delete[] IM;
        delete[] mascara;
        delete[] ID;
        return;
    }

    int tamaño = ancho * alto * 3;
    unsigned char* imgActual = ID;
    unsigned char* imgAnterior = nullptr;

    for (int i = ultimotxt; i >= 0; i--) {
        QString nombreArchivoTxt = carpeta + QString("/M%1.txt").arg(i);
        int semilla, numpixels;
        unsigned int* datos = cargarSemillaYEnmascaramiento(nombreArchivoTxt.toStdString().c_str(), semilla, numpixels);
        if (!datos) {
            cout << "Error cargando " << nombreArchivoTxt.toStdString() << endl;
            if (imgAnterior != nullptr)
            {
                delete [] imgAnterior;
            }
            delete[] IM;
            delete[] mascara;
            delete[] imgActual;
            return;
        }

        unsigned char* imgSiguiente = identificarTransformaciones(imgActual, IM, mascara, datos, semilla, numpixels, tamaño);
        if (!imgSiguiente) {
            cout << "Fallo al identificar transformacion en paso " << i << endl;
            if (imgAnterior != nullptr)
            {
                delete [] imgAnterior;
            }
            delete[] IM;
            delete[] mascara;
            delete[] imgActual;
            delete[] datos;
            return;
        }

        QString nombreSalida;
        if (i == 0)
            nombreSalida = carpeta + "/Posible_I_O.bmp";
        else
            nombreSalida = carpeta + QString("/Posible_P%1.bmp").arg(i);

        exportarImagen(imgSiguiente, ancho, alto, nombreSalida);

        delete[] datos;  //  Liberar siempre

        if (imgAnterior != nullptr)
        {
            delete [] imgAnterior;
        }
        imgAnterior = imgActual;
        imgActual = imgSiguiente; //  Siempre actualizar
    }
    if (imgAnterior != nullptr)
    {
        delete [] imgAnterior;
    }
    delete[] IM;
    delete[] mascara;
    delete[] imgActual;  // Última imagen generada
    return;
}
