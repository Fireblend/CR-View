#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

//Vertices para la lista de vertices
typedef struct {
	int x;
	int y;
} VERTEX;

//Poligonos para la lista de poligonos
typedef struct {
	int *pixels;
	int color[3];
	int imgId;
	char *img;
	int imgW;
	int imgH;
	int fileFlag;
	int pixCount; // Numero de elementos en la lista de pixeles
} POLY;

//Display callback para la ventana a dibujar.
void drawingMachine ();
	//Dibujo y relleno
void drawPolys(int id); 

//Bresenham
int analizaLinea(int x0,int y0,int x1,int y1);
void Bresenham(int x0, int y0, int x1, int y1);
void Bresenham_aux(int X0, int Y0, int X1, int Y1,int id);

//Funciones de inicializacion
void loadMap();
void readImgs();

//Funciones de estructura de datos
	//Agrega objetos
void addNewVertex(int a, int b, int id);
void addNewPoly(int id);
void addNewNum(int a);
	//Verifica estado de array
void ChangePixArray(VERTEX **array, int *numElements, int *numAllocated);
void ChangePidArray(int **array, int *numElements, int *numAllocated);
	//Traslada datos
void resetToOriginal();
void actToTemp();
void tempToAct();

//Sutherland
void sutherland(int bordes[]);
void tranformarUniversalaFrameBuffer();
int ubicacion(int x, int y,int id,int idValue);

//Zoom, Pan y Rotate
void zoom(int x);
void pan(int x);
void rotar(double grados,int xRotate, int yRotate);
void rotarTodos(long double grados);
void transladarTodos(int xRotate, int yRotate);
void rotarRefresh();
void translate(int xMat,int yMat,int xIn,int yIn);

//Callbacks de teclado
void keyboard(unsigned char k, int x, int y);
void sKeyboard(int k, int x, int y);

//Calculo de intersecciones
int calculaInterseccionRelleno(int x0,int y0,int x1,int y1,int bordeValor);
int calculaInterseccion(int x0,int y0,int x1,int y1,int id,int bordeValor);	


