#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

// Declaración de funciones
unsigned char* cargarPixeles(QString rutaEntrada, int &ancho, int &alto);
bool exportarImagen(unsigned char* datosPixeles, int ancho, int alto, QString rutaSalida);
unsigned int* cargarSemillaYEnmascaramiento(const char* rutaArchivo, int &semilla, int &num_pixeles);

int main()
{
    QString rutaImagenEntrada = "Caso1/I_M.bmp";
    QString rutaImagenSalida = "Caso1/I_D.bmp";

    int alto = 0;
    int ancho = 0;

    unsigned char *datosPixeles = cargarPixeles(rutaImagenEntrada, ancho, alto);

    for (int i = 0; i < ancho * alto * 3; i += 3) {
        datosPixeles[i] = i;       // Canal rojo
        datosPixeles[i + 1] = i;   // Canal verde
        datosPixeles[i + 2] = i;   // Canal azul
    }

    bool exportado = exportarImagen(datosPixeles, ancho, alto, rutaImagenSalida);

    cout << "Exportacion: " << (exportado ? "Exito" : "Fallo") << endl;

    delete[] datosPixeles;
    datosPixeles = nullptr;

    int semilla = 0;
    int num_pixeles = 0;

    unsigned int *datosEnmascaramiento = cargarSemillaYEnmascaramiento("Caso1/M1.txt", semilla, num_pixeles);

    if (datosEnmascaramiento != nullptr) {
        for (int i = 0; i < num_pixeles * 3; i += 3) {
            cout << "Pixel " << i / 3 << ": ("
                 << datosEnmascaramiento[i] << ", "
                 << datosEnmascaramiento[i + 1] << ", "
                 << datosEnmascaramiento[i + 2] << ")" << endl;
        }

        delete[] datosEnmascaramiento;
        datosEnmascaramiento = nullptr;
    }

    return 0;
}

// Cargar píxeles de imagen BMP
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
