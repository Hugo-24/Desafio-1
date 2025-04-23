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
unsigned char* identificarTransformaciones(unsigned char* imgEncriptada, unsigned char* imgIM, unsigned char* mascara, unsigned int* datosTxt, int semilla, int num_pixeles, int tamaño);
void reconstruirImagenDesdeCaso(QString carpeta, int pasoMaximo);

//======== main ========
int main() {
    int opcion;
    cout << "Seleccione la opcion que desea (1) para desencriptar el Caso1, (2) para el Caso2 o (3) si desea hacerlo en una carpeta aparte: ";
    cin >> opcion;

    // Opción 1: desencriptar automáticamente el Caso1 (2 pasos)
    if (opcion == 1) {
       reconstruirImagenDesdeCaso("Caso1", 2);

    // Opción 2: desencriptar automáticamente el Caso2 (6 pasos)
    } else if (opcion == 2) {
       reconstruirImagenDesdeCaso("Caso2", 6);

    // Opción 3: ingresar carpeta y número de pasos manualmente
    } else if (opcion == 3) {
        QString carpeta;
        int pasoMaximo;

        cout << "Ingrese el nombre de la carpeta donde se encuentra la imagen que desea desencriptar (por ejemplo: Caso1 o Caso2): ";
        string input;
        cin >> input;
        carpeta = QString::fromStdString(input);

        cout <<  "Ingrese el numero del ultimo enmascaramiento (ultimo archivo .txt, por ejemplo 2 o 6): ";
        cin >> pasoMaximo;

        if (pasoMaximo < 0) {
            cout << "Numero de paso invalido." << endl;
            return 1;
        }
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
        cout << "Imagen BMP modificada guardada en " << rutaSalida.toStdString() << endl;
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
// Verifica si una imagen transformada cumple con los valores registrados en un archivo de enmascaramiento (.txt).
    for (int k = 0; k < num_pixeles * 3; ++k) {
        int pos = k + semilla; // Posición dentro del arreglo de la imagen desde donde se aplica el enmascaramiento
        unsigned int suma = imagen[pos] + mascara[k]; // Se calcula la suma con la máscara
        if (suma != valorestxt[k]) {
            return false; // Si al menos un valor no coincide, se retorna falso
        }
    }
    return true; // Si todas las sumas coinciden, el enmascaramiento es válido
}

unsigned char* identificarTransformaciones(unsigned char* imgEncriptada, unsigned char* imgIM, unsigned char* mascara, unsigned int* datostxt, int semilla, int num_pixeles, int tamaño) {
// Intenta determinar qué transformación se aplicó a una imagen (XOR o rotaciones).
// Devuelve una nueva imagen si la transformación fue identificada correctamente.

    // === Intento 1: aplicar XOR ===
    unsigned char* copiaXOR = new unsigned char[tamaño];
    memcpy(copiaXOR, imgEncriptada, tamaño);
    aplicarXOR(copiaXOR, imgIM, tamaño);
    if (validarEnmascaramiento(copiaXOR, mascara, datostxt, semilla, num_pixeles)) {
        cout << "Transformacion correcta: XOR." << endl;
        return copiaXOR; // Retorna la imagen desencriptada con XOR
    }
    delete[] copiaXOR;

    // === Intento 2: probar rotaciones (1 a 7 bits, en ambas direcciones) ===
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

    // Si ninguna transformación fue válida
    cout << "Ninguna rotacion coincidio con los datos del enmascaramiento." << endl;
    return nullptr;
}

void liberar(unsigned char*& ptr) {
// Función para liberar memoria de un arreglo dinámico de tipo unsigned char y evitar punteros colgantes
    if (ptr != nullptr) {     // Verifica que el puntero tenga memoria asignada
        delete[] ptr;         // Libera la memoria dinámica del arreglo
        ptr = nullptr;        // Evita un puntero colgante al ponerlo en nulo
    }
}

void liberar(unsigned int*& ptr) {
// Libera memoria de un arreglo dinámico de tipo unsigned int
    if (ptr != nullptr) {     // Verifica que el puntero tenga memoria asignada
        delete[] ptr;         // Libera la memoria dinámica del arreglo
        ptr = nullptr;        // Evita un puntero colgante al ponerlo en nulo
    }
}

void reconstruirImagenDesdeCaso(QString carpeta, int ultimotxt) {
// Reconstruye la imagen original a partir de los archivos en la carpeta indicada.
// Aplica transformaciones inversas paso a paso usando los archivos .txt de enmascaramiento.
    QString rutaIM = carpeta + "/I_M.bmp";
    QString rutaMascara = carpeta + "/M.bmp";
    QString rutaID = carpeta + "/I_D.bmp";

    int ancho = 0, alto = 0;
    unsigned char* IM = cargarPixeles(rutaIM, ancho, alto);
    unsigned char* mascara = cargarPixeles(rutaMascara, ancho, alto);
    unsigned char* ID = cargarPixeles(rutaID, ancho, alto);

    // Validar si todas las imágenes se cargaron correctamente
    if (!IM || !mascara || !ID) {
        cout << "Error cargando imagenes base desde la carpeta " << carpeta.toStdString() << endl;
        liberar(IM);
        liberar(mascara);
        liberar(ID);
        return;
    }

    int tamaño = ancho * alto * 3;
    unsigned char* imgActual = ID;
    unsigned char* imgAnterior = nullptr;

    // Itera desde el último archivo .txt hasta M0.txt
    for (int i = ultimotxt; i >= 0; i--) {
        QString nombreArchivoTxt = carpeta + QString("/M%1.txt").arg(i);
        int semilla, numpixels;
        unsigned int* datos = cargarSemillaYEnmascaramiento(nombreArchivoTxt.toStdString().c_str(), semilla, numpixels);
        if (!datos) {
            cout << "Error cargando " << nombreArchivoTxt.toStdString() << endl;
            liberar(imgAnterior);
            liberar(IM);
            liberar(mascara);
            liberar(imgActual);
            return;
        }

        // Intentar identificar la transformación aplicada en este paso
        unsigned char* imgSiguiente = identificarTransformaciones(imgActual, IM, mascara, datos, semilla, numpixels, tamaño);
        if (!imgSiguiente) {
            cout << "Fallo al identificar transformacion en paso " << i << endl;
            liberar(imgAnterior);
            liberar(IM);
            liberar(mascara);
            liberar(imgActual);
            liberar(datos);
            return;
        }

        // Definir el nombre del archivo de salida BMP
        QString nombreSalida = (i == 0) ? carpeta + "/Imagen_Original.bmp" : carpeta + QString("/Posible_P%1.bmp").arg(i);
        exportarImagen(imgSiguiente, ancho, alto, nombreSalida);

        liberar(datos);
        if (imgAnterior != nullptr) delete[] imgAnterior;
        imgAnterior = imgActual;
        imgActual = imgSiguiente;
    }

    // Liberar memoria
    liberar(imgAnterior);
    liberar(IM);
    liberar(mascara);
    liberar(imgActual);
    cout << "Proceso de desencriptacion finalizado con exito." << endl;
    return;
}
