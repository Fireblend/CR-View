/*
||=================================================||
||	Instituto Tecnologico de Costa Rica        ||
|| Ingeniera en Computacion - Computer Graphics    ||
|| 	               Proyecto 1 		   ||
||                                                 ||
|| Sergio Morales - 2007 30434                     ||
||=================================================||

*/
#include "cr-view.h"
#include "string.h"

//#include "memory.h"

//Datos originales
VERTEX 	*OriPixelsArray;
int OriPixElements = 0; // elementos en el array
int	OriPixAllocated = 0; // tamano del array

POLY OriPolys[8];
int oriPolyElem = 0;

//Datos de dibujo
VERTEX 	*ActPixelsArray;
int ActPixElements = 0; // elementos en el array
int	ActPixAllocated = 0; // tamano del array

POLY ActPolys[8];
int actPolyElem = 0;

//Datos extra (sutherland)
VERTEX 	*TempPixelsArray;
int TempPixElements = 0; // elementos en el array
int	TempPixAllocated = 0; // tamano del array

//Lista temporal de vertices
int *tempIDs;
int idsElem = 0;
int idsAlloc = 0;

//Flag de uso de drawMachine
int kflag = 0;

//Modo de dibujo (0 = nada, 1 = colores, 2 = texturas)
int modoID = 0;

//Valores de resolucion de la ventana
int hRes = 800;
int vRes = 800;

//Bordes del visor actual
int bordes[4] = {0, 800, 0, 800};

//Acumulado de rotacion
long double rotarAcum = 0;

//Centro para la rotacion
int origin[2];

/*Zoom & Pan*/

int zoomInOut=40;
int aumento=500; //pan
long double porcentaje =50.0f;//zoom

//Imagenes para poligonos
double ****ImgsArray;

//Macro que invierte los bytes de un int, para leer el header del archivo .avs
#define SWAP(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )
#define FIX(x) (*(unsigned *)&(x) = \
         SWAP(*(unsigned *)&(x)))

void readImgs(){
	FILE *fptr;
	int poly, height, width, pixH, pixW;
	double red, green, blue;
	
	for(poly = 0; poly < oriPolyElem; poly++){
		//Intenta abrir el archivo
        
		if ((fptr = fopen(OriPolys[poly].img,"r")) != NULL){
			
			OriPolys[poly].fileFlag = 0;
			OriPolys[poly].imgId = poly;
			int alpha;
					
			//Determina width y height
			fread(&width,sizeof(int),1,fptr);
			width = FIX(width);
							
			fread(&height,sizeof(int),1,fptr);
			height = FIX(height);
			
			OriPolys[poly].imgH = height;
			OriPolys[poly].imgW = width;
			
			ImgsArray[poly] = (double***) calloc(height, sizeof(double**));
			for (pixH=0;pixH<height;pixH++) {
				ImgsArray[poly][pixH] = (double**) calloc(width, sizeof(double*));
				for (pixW=0;pixW<width;pixW++) {
					ImgsArray[poly][pixH][pixW] = (double*) calloc(3, sizeof(double));
					//Lee el pixel actual
					//Alpha se ignora
					alpha = fgetc(fptr);	
					red =(double)fgetc(fptr)/255;
					green =(double)fgetc(fptr)/255;
					blue =(double)fgetc(fptr)/255;
					ImgsArray[poly][pixH][pixW][0] = red;
					ImgsArray[poly][pixH][pixW][1] = green;
					ImgsArray[poly][pixH][pixW][2] = blue;
				}
			}
		}
		else{
			printf("No se encontro el archivo de imagen %s\n", ActPolys[poly].img);
			OriPolys[poly].fileFlag = 1;
		}				
	}
}

void sKeyboard(int key, int x, int y){
	if(kflag == 0){
		if(key == GLUT_KEY_LEFT) { pan(4); drawingMachine(); }
		else if(key == GLUT_KEY_RIGHT) { pan(3); drawingMachine(); }
		else if(key == GLUT_KEY_DOWN) { pan(2); drawingMachine(); }
		else if(key == GLUT_KEY_UP) { pan(1); drawingMachine(); }
	}
}

/*Main :)*/
int main(int argc, char** argv) {
	
	origin[0] = hRes/2;
	origin[1] = vRes/2;
	
	loadMap();
	actToTemp();
	
	ImgsArray = (double****) calloc(oriPolyElem, sizeof(double***));
	readImgs();
	
	resetToOriginal();
	
	printf("\nCRI-View (C) 2009 Laura Vasquez & Sergio Morales\n");

	//Inicializa Glut y llama a la funcion de dibujo de las lineas	
	glutInit(&argc, argv);
	glutInitWindowSize(hRes, vRes); 
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
	glutCreateWindow("~CRI-View~"); 
	
	glClearColor(0.717, 0.831, 1, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);  /*Clears buffer to preset values - buffer currently enabled for color writing */
	gluOrtho2D(-0.5, hRes-0.5, -0.5, vRes-0.5); /*Defines a 2D orthographic projection matrix*/
	glutKeyboardFunc(keyboard);
    glutSpecialFunc(sKeyboard);
	glutDisplayFunc(drawingMachine); /*Sets the display callback for the  current window */
	glutMainLoop();
	return 1;
}

void loadMap(){
	FILE *file;
	char line[30];
	char *temp;
	long x;
	long y;
	long r;
	long g;
	long b;
	char *imgFile;
	char buf[25];
	int cont = 1;
	file = fopen ("map.txt", "rt");
	fgets(line, 25, file);
	sscanf (line, "%ld", &r);
	fgets(line, 25, file);
	sscanf (line, "%ld", &g);
	fgets(line, 25, file);
	sscanf (line, "%ld", &b);
	fgets(line, 25, file);
	
	while(fgets(line, 25, file) != NULL)
	{
		if(strlen(line) == 1){	
			addNewPoly(0);
			OriPolys[oriPolyElem-1].color[0] = r;
			OriPolys[oriPolyElem-1].color[1] = g;
			OriPolys[oriPolyElem-1].color[2] = b;
			sprintf(buf, "%d", cont);
			imgFile = (char *)calloc(strlen("imgs/") + strlen(buf) + strlen(".avs") + 2, 
                        sizeof(char));
            strcat(imgFile, "imgs/");
            strcat(imgFile, buf);
            strcat(imgFile, ".avs");
			cont++;
			OriPolys[oriPolyElem-1].img = imgFile;
			
			if(fgets(line, 25, file) != NULL){
				sscanf (line, "%ld", &r);
				fgets(line, 25, file);
				sscanf (line, "%ld", &g);
				fgets(line, 25, file);
				sscanf (line, "%ld", &b);
				fgets(line, 25, file);
			}
			
		}else{
			temp = strtok(line, ",");
			sscanf (temp, "%ld", &x);
			temp = strtok(NULL, ",");
			sscanf (temp, "%ld", &y);
			addNewVertex(x+200, y+200, 0);
			addNewNum(OriPixElements-1);
		}
	}
	fclose(file);
}

void keyboard(unsigned char k, int x, int y)
{
	if(kflag == 0){
	switch (k)
	{	//zoomOut
		case 43: // +
    		zoom(0);
    		drawingMachine();
    		break;
    	//zoomIn
   		case 45: // -
    		zoom(1);
    		drawingMachine();
    		break;
    	case 97: // a
    		pan(4);
    		drawingMachine();
      		break;
    	case 115: // s
    		pan(2);
    		drawingMachine();
    		break;
    	case 100: // d
    		pan(3);
    		drawingMachine();
    		break;
    	case 119: // w
    		pan(1);
    		drawingMachine();
    		break;
    	case 114: // r
    		origin[0] = bordes[0]+((bordes[1]-bordes[0])/2);
    		origin[1] = bordes[2]+((bordes[3]-bordes[2])/2);
    		rotarAcum = rotarAcum - 50.0f;			    		
    		sutherland(bordes); 
    		drawingMachine(); 
    		break;
    	case 116: // t
    		origin[0] = bordes[0]+((bordes[1]-bordes[0])/2);
    		origin[1] = bordes[2]+((bordes[3]-bordes[2])/2);
    		rotarAcum = rotarAcum + 50.0f;			    		
    		sutherland(bordes); 
    		drawingMachine(); 
    		break;
    	case 49: // 1
    		modoID = 0;
    		drawingMachine();
    		break;
    	case 50: // 2
    		modoID = 1;
    		drawingMachine();
    		break;
    	case 51: // 3
    		modoID = 2;
    		drawingMachine();
    		break;
    	case 112: // p
    		resetToOriginal();
			rotarAcum = 0;
			bordes[0] = 0;
			bordes[1] = 800;
			bordes[2] = 0;
			bordes[3] = 800;
			origin[0] = hRes/2;
			origin[1] = vRes/2;
			zoom(0);
			zoomInOut = 40;
    		drawingMachine();
    		break;
		}
	}
}

void resetToOriginal(){
	int i;
	for(i = 0; i<OriPixElements; i++){
		ChangePixArray(&ActPixelsArray, &ActPixElements, &ActPixAllocated);
		ActPixelsArray[i] = OriPixelsArray[i];
	}
	for(i = 0; i < oriPolyElem; i++)
	ActPolys[i] = OriPolys[i];
	
	actPolyElem = oriPolyElem;
	ActPixElements = OriPixElements; // elementos en el array
	ActPixAllocated = OriPixAllocated;
}


void actToTemp(){
	int i;
	TempPixelsArray = (VERTEX*) calloc(ActPixAllocated, sizeof(VERTEX));
	for(i = 0; i<ActPixElements; i++){
		TempPixelsArray[i] = ActPixelsArray[i];
	}
	TempPixElements = ActPixElements; // elementos en el array
	TempPixAllocated = ActPixAllocated;
}


void tempToAct(){
	
	int i;
	for(i = 0; i<TempPixElements; i++){
		
		ChangePixArray(&ActPixelsArray, &ActPixElements, &ActPixAllocated);
		ActPixelsArray[i] = TempPixelsArray[i];
	}
	ActPixElements = TempPixElements; // elementos en el array
	ActPixAllocated = TempPixAllocated;
}

/*Draw! :D*/
void drawingMachine() 
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin (GL_POINTS);
	glColor3f (0.702, 0.925, 1);
	
	glColor3f (0, 0, 0);
	drawPolys(modoID);	
	glEnd();
	glFlush();

}

int calculaInterseccionRelleno(int x0,int y0,int x1,int y1,int bordeValor){
	long double m;
	long double b;
	
	if(x0 == x1 || y0 == y1)
		return x0;
		
	else
	m = (long double)(y1-y0)/(x1-x0);
	b = (long double)y0 - m*x0;
	
	return (int)((bordeValor-b)/m+0.5);
}

void drawPolys(int id){
	kflag = 1;
	int poly, vertex;
	//Si se va a usar relleno...
	
		int width, height;
		int x_min, x_max, y_min, y_max;
		double ***array;
		
		//Por cada poligono...
		for(poly = 0; poly < actPolyElem; poly++){
			//Carga el color del poligono, si es relleno con colores.
			if(id == 1){
				glColor3f((double)ActPolys[poly].color[0]/255, (double)ActPolys[poly].color[1]/255, (double)ActPolys[poly].color[2]/255);
			}
			
			//Carga la imagen .avs del poligono, si es relleno con texturas
			if(id == 2){
				array = ImgsArray[ActPolys[poly].imgId];
				width = ActPolys[poly].imgW;
				height = ActPolys[poly].imgH;
			}
		
			y_min = ActPixelsArray[ActPolys[poly].pixels[0]].y;
			y_max = ActPixelsArray[ActPolys[poly].pixels[0]].y;
			x_min = ActPixelsArray[ActPolys[poly].pixels[0]].x;
			x_max = ActPixelsArray[ActPolys[poly].pixels[0]].x;
			
			//obtiene X_min, X_max, Y_min y Y_max del poligono
			for(vertex = 0; vertex < ActPolys[poly].pixCount; vertex++){
				if(ActPixelsArray[ActPolys[poly].pixels[vertex]].y < y_min)
					y_min = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
				if(ActPixelsArray[ActPolys[poly].pixels[vertex]].y > y_max)
					y_max = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
				if(ActPixelsArray[ActPolys[poly].pixels[vertex]].x < x_min)
					x_min = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
				if(ActPixelsArray[ActPolys[poly].pixels[vertex]].x > x_max)
					x_max = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
			}
			
			if(x_max - x_min > 5){
			int scanline;
			if(id > 0){
			//POR CADA SCANLINE
			for(scanline = y_min+1; scanline < y_max; scanline++){
				
				int listaX[ActPolys[poly].pixCount+10];
				int contX = 0;
				int lastY;
				int firstY;
				
				//Recorre las lineas del poligono:	
				for(vertex = 0; vertex < ActPolys[poly].pixCount; vertex++){
					
					if(vertex == ActPolys[poly].pixCount-1){
						if((scanline >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && scanline <= ActPixelsArray[ActPolys[poly].pixels[0]].y)||
						(scanline <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && scanline >= ActPixelsArray[ActPolys[poly].pixels[0]].y)){
							if(ActPixelsArray[ActPolys[poly].pixels[vertex]].y != ActPixelsArray[ActPolys[poly].pixels[0]].y){							
								//Se agrega la interseccion a la lista de Xs
								listaX[contX] = calculaInterseccionRelleno(ActPixelsArray[ActPolys[poly].pixels[vertex]].x,ActPixelsArray[ActPolys[poly].pixels[vertex]].y,ActPixelsArray[ActPolys[poly].pixels[0]].x,ActPixelsArray[ActPolys[poly].pixels[0]].y,scanline);//calcula interseccion
							
								//Si ya estaba el X en la lista...
								if(listaX[0] == listaX[contX]){
									if(!((firstY >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && 
									ActPixelsArray[ActPolys[poly].pixels[0]].y >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y)||
									(firstY <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && 
									ActPixelsArray[ActPolys[poly].pixels[0]].y <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y))){
										contX--;	//se revisa lastYs para determinar si es un
											//minimo/maximo local o si se debe eliminar una entrada en la lista
									}
								}
								contX++;
								lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
							}				
							else{
								if((ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y < ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y < ActPixelsArray[ActPolys[poly].pixels[1]].y) || 
									(ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y > ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y > ActPixelsArray[ActPolys[poly].pixels[1]].y) ){
										listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
										contX++;
										lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
								}
								else {
									listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
									listaX[contX+1] = ActPixelsArray[ActPolys[poly].pixels[0]].x;
									contX += 2;
								}
							}		
						}
					}
					else{
						if((scanline >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && scanline <= ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y)||
						(scanline <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && scanline >= ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y)){
							
							if(ActPixelsArray[ActPolys[poly].pixels[vertex]].y != ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y){
								//Se agrega la interseccion a la lista de Xs
								listaX[contX] = calculaInterseccionRelleno(ActPixelsArray[ActPolys[poly].pixels[vertex]].x,ActPixelsArray[ActPolys[poly].pixels[vertex]].y,ActPixelsArray[ActPolys[poly].pixels[vertex+1]].x,ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y,scanline);//calcula interseccion
								
								//Si ya estaba el X en la lista...
								if(contX!=0){
								if(listaX[contX] == listaX[contX-1]){
									if(!((lastY >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && 
									ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y >= ActPixelsArray[ActPolys[poly].pixels[vertex]].y)||
									(lastY <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y && 
									ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y <= ActPixelsArray[ActPolys[poly].pixels[vertex]].y)))
									contX--;	//se revisa lastYs para determinar si es un
												//minimo/maximo local o si se debe eliminar una entrada en la lista
									
									}
								}
								else firstY = lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
								contX++;
								lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
							}
							else{
								if(vertex != 0){
									if(vertex != ActPolys[poly].pixCount-2){
									//if it's a slope, add point
										if((ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y < ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y < ActPixelsArray[ActPolys[poly].pixels[vertex+2]].y) || 
										(ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y > ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y > ActPixelsArray[ActPolys[poly].pixels[vertex+2]].y) ){
											listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
											contX++;
											lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
										}
										else {
											listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
											listaX[contX+1] = ActPixelsArray[ActPolys[poly].pixels[vertex+1]].x;
											contX += 2;
										}
									}
									else{
										if((ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y < ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y < ActPixelsArray[ActPolys[poly].pixels[0]].y) || 
										(ActPixelsArray[ActPolys[poly].pixels[vertex-1]].y > ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y > ActPixelsArray[ActPolys[poly].pixels[0]].y) ){
											listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
											contX++;
											lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
										}
										else {
											listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
											listaX[contX+1] = ActPixelsArray[ActPolys[poly].pixels[vertex+1]].x;
											contX += 2;
										}
									}
								//if not, nothing
								}
								else{
									if((ActPixelsArray[ActPolys[poly].pixels[ActPolys[poly].pixCount-1]].y < ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y < ActPixelsArray[ActPolys[poly].pixels[vertex+2]].y) || 
									(ActPixelsArray[ActPolys[poly].pixels[ActPolys[poly].pixCount-1]].y > ActPixelsArray[ActPolys[poly].pixels[vertex]].y && ActPixelsArray[ActPolys[poly].pixels[vertex]].y > ActPixelsArray[ActPolys[poly].pixels[vertex+2]].y) ){
										listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
										contX++;
										lastY = ActPixelsArray[ActPolys[poly].pixels[vertex]].y;
									}
									else {
										listaX[contX] = ActPixelsArray[ActPolys[poly].pixels[vertex]].x;
										listaX[contX+1] = ActPixelsArray[ActPolys[poly].pixels[vertex+1]].x;
										contX += 2;
									}
								//handle first vertex
								}
							}
						}
					}
				}
				int i, j, temp;
				//ORDENA Xs
				//Ordena la lista de Xs
				if(contX%2 > 0){ 
						contX--;
					}
				for (i = (contX - 1); i >= 0; i--){
    				for (j = 1; j <= i; j++){
      					if (listaX[j-1] > listaX[j]){
        					temp = listaX[j-1];
        					listaX[j-1] = listaX[j];
        					listaX[j] = temp;
      					}
    				}
				}
				
				//PINTA DE X EN X
				if(id == 1) {
					for(j = 0; j < contX; j += 2 ) {
						for(i = listaX[j]+1; i <= listaX[j+1]; i++)
							glVertex2i(i, scanline);
					}
				
				}
				//PINTA TEXTURAS
				else {
					if(!ActPolys[poly].fileFlag)
					for(j = 0; j < contX; j += 2 ) {
						for(i = listaX[j]+1; i < listaX[j+1]+1; i++ ) {
							//Determina el pixel equivalente
							int x, z;
							//x = (i-x_min)%width;  // <===REEMPLAZAR PARA TEXTURAS ESTATICAS 
							//z = (y_max-scanline)%height; // <===REEMPLAZAR PARA TEXTURAS ESTATICAS 
							x = i%width;
							z = height - (scanline%height)-1;
							glColor3f((double)array[z][x][0], (double)array[z][x][1], (double)array[z][x][2]);
							glVertex2i(i, scanline); 
						}
					}
				}
			}
			}//===CIERRA POR CADA SCANLINE
			
		}//===CIERRA POR CADA POLIGONO
		
			glColor3f (0, 0, 0);
			for (vertex = 0; vertex < ActPolys[poly].pixCount; vertex++){
				if(vertex == ActPolys[poly].pixCount-1){				
					Bresenham(ActPixelsArray[ActPolys[poly].pixels[vertex]].x, ActPixelsArray[ActPolys[poly].pixels[vertex]].y, ActPixelsArray[ActPolys[poly].pixels[0]].x, ActPixelsArray[ActPolys[poly].pixels[0]].y);
				}	
				else{
					Bresenham(ActPixelsArray[ActPolys[poly].pixels[vertex]].x, ActPixelsArray[ActPolys[poly].pixels[vertex]].y, ActPixelsArray[ActPolys[poly].pixels[vertex+1]].x, ActPixelsArray[ActPolys[poly].pixels[vertex+1]].y);
				}
			}
	}
	
	//Dibuja los bordes del poligono
	//recorre cada poligono
	kflag = 0;
}

void addNewPoly(int id){
	
	int i;
	//Transfiere valores de lista temporal a un array
	int *newPixels;
	newPixels = malloc(idsElem * sizeof(int));
	
	for(i = 0; i < idsElem; i++){
		newPixels[i] = tempIDs[i];
	} 
	
	//Crea nuevo poligono
	POLY newPoly;
	newPoly.pixels = newPixels;
	
	newPoly.pixCount = idsElem;
	
	if(id){
		ActPolys[actPolyElem] = newPoly;
		actPolyElem++;	
	}
	else{
		OriPolys[oriPolyElem] = newPoly;
		oriPolyElem++;
	}
	free(tempIDs);
	tempIDs = (int*) calloc(0, sizeof(int));
	idsElem = 0;
	idsAlloc = 0;
}

void addNewVertex(int a, int b, int id){
	VERTEX temp;
	
	temp.x = a;
	temp.y = b;
	
	if(id==1){
		//Chequea si debe cambiar el tamano del array
		ChangePixArray(&ActPixelsArray, &ActPixElements, &ActPixAllocated);
		//Agrega pixel a la tabla
		ActPixelsArray[ActPixElements] = temp;
		ActPixElements++;
	}
	else if(id == 0){
		ChangePixArray(&OriPixelsArray, &OriPixElements, &OriPixAllocated);
		OriPixelsArray[OriPixElements] = temp;
		OriPixElements++;
	}
	else{
		ChangePixArray(&TempPixelsArray, &TempPixElements, &TempPixAllocated);
		TempPixelsArray[TempPixElements] = temp;
		TempPixElements++;
	}
}

void addNewNum(int a){
	//Chequea si debe cambiar el tamano del array
	ChangePidArray(&tempIDs, &idsElem, &idsAlloc);
	tempIDs[idsElem] = a;
	idsElem++;
}

//Cambia el tamano del array especificado de ser necesario.
void ChangePidArray (int **array, int *numElements, int *numAllocated){
	if(*numElements == *numAllocated) { 
		if (*numAllocated == 0)
			*numAllocated = 2000; 
		else
			*numAllocated *= 1.5; 
		
		*array = (int*)realloc(*array, (*numAllocated * sizeof(int)));

		if (*array == NULL){
			printf("Error!\n");
			exit(2);
		}
	}
	return;
}

//Cambia el tamano del array especificado de ser necesario.
void ChangePixArray(VERTEX **array, int *numElements, int *numAllocated){
	if(*numElements == *numAllocated) { 
		if (*numAllocated == 0)
			*numAllocated = 2000; 
		else
			*numAllocated = *numAllocated * 1.5; 
		
		*array = (VERTEX*)realloc(*array, (*numAllocated * sizeof(VERTEX)));

		if (*array == NULL){
			printf("Error!\n");
			exit(2);
		}
	}
	return;
}

//Determina en que octante se encuentra la linea de entrada
int analizaLinea(int x0,int y0,int x1,int y1){
	
	int y = y1 - y0;
	int x = x1 - x0;
	
	if(y > 0){
		if(y > x)return 2; 	//Segundo Octante
		else return 1; 		//Primer Octante
	}
	else{
		if(abs(y) > x)return 7;  //Setimo Octante
		else return 8;			 //Octavo Octante :P
	}
}

void Bresenham_aux(int X0, int Y0, int X1, int Y1,int id){

int Delta_A, Delta_B, d, Xp, Yp;
Xp = X0; Yp = Y0;				//Pixel de inicio
glVertex2i(X0, Y0);
glVertex2i(X1, Y1);

/*La variable id determina a que cuadrante pertenece la linea.
Para cada cuadrante hay un valor distinto de "d" y de "Delta", segun la
direccion que deba pintar.*/				 

/*Cuadrante 1*/
	if(id==1){	
    	Delta_A = 2*(Y1 - Y0);					//Delta_E
    	Delta_B = 2*((Y1 - Y0)-(X1 - X0));		//Delta_NE
    	d = 2*(Y1 - Y0)-(X1 - X0); 				//Valor inicial de d
    	
    	/*Se pinta desde X0 hasta X1*/
    	while (Xp < X1){		
        	
        	if (d < 0){
			/*Pintar E*/
				Xp++;
				d = d + Delta_A;
         	}
         	else{ 
         	/*Pintar NE*/
            	Xp++;
            	Yp++;
            	d = d + Delta_B;
         	}
         	glVertex2i(Xp, Yp);
		}
	}
/*Cuadrante 2 */ 
	else if(id==2){
		Delta_A =  -2*(X1 - X0);				//Delta_N
		Delta_B = 2*((Y1 - Y0) - (X1 - X0));	//Delta_NE
		d = (Y1 - Y0) - 2*(X1 - X0);			//Valor inicial de d
		
		/*Se pinta desde Y0 hasta Y1*/
		while (Yp < Y1){
			if (d > 0){
				/*Pintar N*/
				Yp++;
				d = d + Delta_A;
			}
			else{
				/*Pintar NE*/
				Yp++;
				Xp++;
				d = d + Delta_B;
			}
			glVertex2i(Xp, Yp);
		}	
	}
	
/*Cuadrante 7*/
	else if(id==7){
		Delta_A =  2*(X1-X0);				//Delta_S
		Delta_B = 2*((Y1-Y0)+(X1-X0));			//Delta_SE
		d = (Y1 - Y0) + 2*(X1 - X0);		//Valor inicial de d
		
		/*Se pinta desde Y0 hasta Y1*/
		while (Yp > Y1){
			if (d < 0){ 
				/*Pintar S*/
				Yp--;
				d = d + Delta_A;
			}
			else{
				/*Pintar SE*/
				Xp++;
				Yp--;
				d = d + Delta_B;
			}
			glVertex2i(Xp, Yp);
		}
	}
	else{
/*Cuadrante 8*/
		Delta_A =  2*(Y1 - Y0);					//Delta_E
		Delta_B = 2*((Y1-Y0)+(X1-X0));			//Delta_SE
		d = 2*((Y1 - Y0) + (X1 - X0));		//Valor inicial de d
		
		/*Se pinta desde X0 hasta X1*/
		while (Xp < X1){
			if (d > 0){
				/*Pintar E*/
				Xp++;
				d = d + Delta_A;
			}
			else{
				/*Pintar SE*/
				Xp++;
				Yp--;
				d = d + Delta_B;
			}
			glVertex2i(Xp, Yp);
		}
	}
}

//Llamada principal del algoritmo de Bresenham 
void Bresenham(int x0, int y0, int x1, int y1){
	/*Si se encuentra en el cuadrante 2 o 3 se invierten los puntos*/
	if(x0>x1){
		Bresenham_aux(x1,y1,x0,y0,analizaLinea(x1,y1,x0,y0));
	}
	else
	/*Sino se llama al algoritmo con los puntos de entrada*/
		Bresenham_aux(x0,y0,x1,y1,analizaLinea(x0,y0,x1,y1));	
}

int result[2]; //Coordenadas de buffer

int ubicacion(int x, int y,int id,int idValue){	
		
	if(id == 1){				//X-min
		if(x>=idValue)
		return 1;
		else if(x<idValue)
		return 0;
	}
	else if(id == 2){			//X-max
		if(x>idValue)
		return 0;
		else if(x<=idValue)
		return 1;
	}
	else if(id == 3){			//Y-min
		if(y>=idValue)
		return 1;
		else if(y<idValue)
		return 0;
	}else if(id == 4){			//Y-max
		if(y>idValue)
		return 0;
		else if(y<=idValue)
		return 1;
	}
	return 2;
}

 int calculaInterseccion(int x0,int y0,int x1,int y1,int id,int bordeValor){
	long double result;
       
    if(x0 == x1)
    return  x0;
      
    long double m = ((long double)(y0-y1)/(long double)(x0-x1));
    long double b = (long double)y1-(m*(long double)x1);
    
    //Interseccion en el eje Y
    if(id == 3 || id == 4)
        result = (bordeValor-b)/m;
    //Interseccion en el eje X
    else
        result = (m*bordeValor)+b;
                       
    return abs(result); 
}

//Bordes:xmin,xmax,ymin,ymax
void sutherland(int bordes[]){
	int i,j,k,cont;
	cont= 0;
	resetToOriginal();
	rotarRefresh();
	actToTemp();
	idsElem = TempPixAllocated = TempPixElements = 0;
	POLY *polyTemp;
	polyTemp = (POLY*) calloc(8, sizeof(POLY));
	
//Por cada borde
	for(i = 0; i<4;i++){
		TempPixElements = 0;
		//Por cada poligono
		for(j=0;j< actPolyElem ;j++){
			//Por cada vertice
			for(k=0;k<ActPolys[j].pixCount;k++){
				int x0,y0,x1,y1;
				
				if(k == ActPolys[j].pixCount -1){
					x0=ActPixelsArray[ActPolys[j].pixels[k]].x;
					y0=ActPixelsArray[ActPolys[j].pixels[k]].y;
					x1=ActPixelsArray[ActPolys[j].pixels[0]].x;
					y1=ActPixelsArray[ActPolys[j].pixels[0]].y;
				}else{
					
					x0=ActPixelsArray[ActPolys[j].pixels[k]].x;
					y0=ActPixelsArray[ActPolys[j].pixels[k]].y;
					x1=ActPixelsArray[ActPolys[j].pixels[k+1]].x;
					y1=ActPixelsArray[ActPolys[j].pixels[k+1]].y;
				}
				
				//Caso Adentro-Adentro
				if(ubicacion(x0,y0,i+1,bordes[i]) == 1 && ubicacion(x1,y1,i+1,bordes[i]) == 1){
					//Add to lista nodo i(destino)
					addNewVertex(x1,y1,2);
					addNewNum(TempPixElements-1);

				} 
				
				//Caso Adentro-Afuera
				else if(ubicacion(x0,y0,i+1,bordes[i]) == 1 && ubicacion(x1,y1,i+1,bordes[i]) == 0){ 
					//Add to lista interseccion
					if(i+1 == 3 || i+1 == 4){
						addNewVertex(calculaInterseccion(x0,y0,x1,y1,i+1,bordes[i]),bordes[i],2);
						}
						else{
						addNewVertex(bordes[i],calculaInterseccion(x0,y0,x1,y1,i+1,bordes[i]),2);
					}
					addNewNum(TempPixElements-1);
				}
				//Caso Afuera-Adentro
				else if(ubicacion(x0,y0,i+1,bordes[i]) == 0 && ubicacion(x1,y1,i+1,bordes[i]) == 1){
					//Add to lista interseccion y nodo i+1
					if(i+1 == 3 || i+1 == 4){
						addNewVertex(calculaInterseccion(x0,y0,x1,y1,i+1,bordes[i]),bordes[i],2);
						}
						else{
						addNewVertex(bordes[i],calculaInterseccion(x0,y0,x1,y1,i+1,bordes[i]),2);}
					
					addNewNum(TempPixElements-1);
					addNewVertex(x1,y1,2);				
					addNewNum(TempPixElements-1);

					} 
					//Caso Afuera-Afuera
					else if(ubicacion(x0,y0,i+1,bordes[i]) == 0 && ubicacion(x1,y1,i+1,bordes[i]) == 0){
					}else
					{
						printf("---------Caso raro xD------\n");
					}						
			}
			if(idsElem > 0){
				int i;
				//Transfiere valores de lista temporal a un array
				int *newPixels;
				newPixels = malloc(idsElem * sizeof(int));
				
				for(i = 0; i < idsElem; i++){
					newPixels[i] = tempIDs[i];
				} 			
				//Crea nuevo poligono
				POLY newPoly;
				newPoly.pixels = newPixels;
				newPoly.pixCount = idsElem;
				newPoly.color[0] = ActPolys[j].color[0];
				newPoly.color[1] = ActPolys[j].color[1];
				newPoly.color[2] = ActPolys[j].color[2];
				newPoly.img = ActPolys[j].img;
				newPoly.imgId = ActPolys[j].imgId;
				newPoly.imgH = ActPolys[j].imgH;
				newPoly.imgW = ActPolys[j].imgW;
				newPoly.fileFlag = ActPolys[j].fileFlag;
				idsElem = idsAlloc = 0;
				polyTemp[cont] = newPoly;
				cont++;
			}
		}
		int h;
		tempToAct();
		for(h=0;h<cont;h++){
			ActPolys[h] = polyTemp[h];
		}
		
		actPolyElem = cont;
		cont = 0;
	}
	tranformarUniversalaFrameBuffer();
	free(TempPixelsArray);
	free(tempIDs);
	tempIDs = (int*) calloc(1, sizeof(int));
}
		
//Bordes: xmin,xmax,ymin,ymax
void tranformarUniversalaFrameBuffer(){
	int k;
	
	for(k=0;k<ActPixElements;k++){
		double x = hRes*((double)(ActPixelsArray[k].x-bordes[0])/(double)(bordes[1]-bordes[0]));
		double y = vRes*((double)(ActPixelsArray[k].y-bordes[2])/(double)(bordes[3]-bordes[2]));
		ActPixelsArray[k].x = (int)x;
		ActPixelsArray[k].y = (int)y;
	}
}

void zoom(int inORout){
	int percH = (bordes[1] - bordes[0])*10/100;
	int percV = (bordes[3] - bordes[2])*10/100;
	if(inORout){
		if(percH == 0) percH = (bordes[1] - bordes[0])*30/100;
		if(percV == 0) percV = (bordes[3] - bordes[2])*30/100;
	}
	if(!inORout){
		if(percH == 0) percH = 1;
		if(percV == 0) percV = 1;
	}
	if(inORout){
		bordes[0] = bordes[0]+ percH;
		bordes[1] = bordes[1]- percH;
		bordes[2] = bordes[2]+ percV;
		bordes[3] = bordes[3]- percV;
		zoomInOut++;
	}else{
		bordes[0] = bordes[0]-percH;
		bordes[1] = bordes[1]+percH;
		bordes[2] = bordes[2]-percV;
		bordes[3] = bordes[3]+percV;
		zoomInOut--;
	}
	actPolyElem = 0;
	ActPixAllocated = 0;
	ActPixElements = 0;
	sutherland(bordes);
}

void pan(int direccion){
	actPolyElem = 0;
	ActPixAllocated = 0;
	ActPixElements = 0;
	int aum = aumento/(zoomInOut*1.5);
	if(aum < 1) aum = 1;
	if(direccion == 1){
		bordes[2] = bordes[2]+aum; 
		bordes[3] = bordes[3]+aum;
	}
	else if(direccion == 2){			//Abajo
		bordes[2] = bordes[2]-aum; 
		bordes[3] = bordes[3]-aum;
	}	
	else if (direccion == 3){		//Derecha
		bordes[0] = bordes[0]+aum; 
		bordes[1] = bordes[1]+aum;
	}			
	else if(direccion == 4){	//Izquierda
		bordes[0] = bordes[0]-aum; 
		bordes[1] = bordes[1]-aum;
	}
		sutherland(bordes);
}

void rotar(double grados,int xRotate, int yRotate){
	result[0] = (cos(grados)*(long double)xRotate)-(sin(grados)*(long double)yRotate);
	result[1] = (sin(grados)*(long double)xRotate)+(cos(grados)*(long double)yRotate);
}

void translate(int xMat,int yMat,int xIn,int yIn){	
	result[0] = xIn+xMat;
	result[1] = yIn+yMat;
}

void transladarTodos(int xRotate, int yRotate){
	int i;
	
	for(i=0;i< ActPixElements;i++){
		translate(xRotate, yRotate,ActPixelsArray[i].x,ActPixelsArray[i].y);
		ActPixelsArray[i].x = result[0];
		ActPixelsArray[i].y = result[1];
	}
}

void rotarTodos(long double grados){
	int i;
	if(rotarAcum > 1150)
		{
		rotarAcum = 0;
		}
	if(rotarAcum < -1150)
		{
		rotarAcum = 0;
		}
	else	
	for(i=0;i< ActPixElements ;i++){
		rotar(grados,ActPixelsArray[i].x,ActPixelsArray[i].y);
		ActPixelsArray[i].x = result[0];
		ActPixelsArray[i].y = result[1];
	}
}

void rotarRefresh(){
	transladarTodos(-origin[0], -origin[1]);
    rotarTodos(rotarAcum);
    transladarTodos(origin[0], origin[1]);
}
