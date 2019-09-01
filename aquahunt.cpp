//
//Universidade de Brasilia
//Principios de computacao grafica
//
//Author: Jonathan Alis Salgado Lima
//matricula: 06/87391
//
//

// documentacao nos comentarios.
//compilar: gcc aquahunt.cpp -L/usr/lib -L/usr/X11R6/lib -lX11 -lGL -lGLU -lglut -ljpeg -lm -lstdc++ -o aquahunt


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> 
#include<GL/gl.h>
#include<GL/glut.h>
#include<GL/glu.h>
#include <GL/freeglut.h>
#include <jpeglib.h>
#include <jerror.h>

//defines, pode mudar eles, compilar denovo e ver o q acontece.
#define NP1 1 //so controla o primeiro.
#define NP2 400
#define NP3 150
#define NP4 5
#define NP5 1
#define NP6 2//deixe igual a 2.
#define NIVEL_MAR 200
#define PROF_MAX 1000
#define GRAVIDADE 100
#define PI 3.1415926//nao mude esse tbm...
#define		RESOLUTION 64
//estrutura de um vetor.
typedef struct vetor
{
double x;
double y;
double z;
double ang;
struct vetor *prox;
struct vetor *ant;
}vetor;
//estrutura de um objeto(no caso um peixe)
typedef struct objeto
{
	vetor *posicao;
	vetor *velocidade;//igual direcao pra frente.
	vetor *aceleracao;
	vetor *rotacao;
	vetor *direcao_lado;
	vetor *direcao_cima;
	double massa;
	double tam;
	double vel_max;
	int existe;
	int paralizado;
	objeto *proximo;
}objeto;


typedef float M3DMatrix33f[9];
typedef float	M3DVector3f[3];

//definicoes de funcoes

//funcoes de calculo vetorial
void printvetor(vetor *vet);
vetor* new_vetor(double x, double y, double z);
void zerar(vetor* vet);
vetor *rotacao(vetor *inicial, vetor*final);
vetor *derivada(vetor *ant, vetor *prox, double delta_t);
vetor *integral(vetor* v, vetor*v0, double );
vetor* soma(vetor* a, vetor* b);
vetor* mult_escalar(vetor *vet, double escalar);
double norma(vetor* a);
vetor* normalizar(double tamanho, vetor *vet);
double angulo_entre(vetor *um, vetor* outro);
double prod_escalar(vetor* a, vetor* b);
vetor* prod_vetorial(vetor* a, vetor* b);
int estanamira(vetor *v1, vetor *p1, vetor *p2);
//funcoes pra objetos
vetor *seguir(objeto *eu, objeto* ele, double intensidade);
vetor *seguir(objeto *eu, vetor* ele, double intensidade);
vetor *fugir(objeto *eu, objeto* ele, double intensidade);
double distancia(objeto *esse, objeto* outro);
objeto* mais_perto(objeto *esse, objeto *outros[], int quantos);
objeto* mais_perto_que(double distancia, objeto *esse, objeto *outros[], int quantos);
objeto* atualiza(objeto *nave, vetor* aceleracao, int boom);
objeto* iniciar(double vx, double vy, double vz, double px, double py, double pz, double massa, double tam, double vel_max);
//peguei essas de math3d.h
void m3dFindNormal(M3DVector3f result, const M3DVector3f point1, const M3DVector3f point2, const M3DVector3f point3);
void m3dRotationMatrix33(M3DMatrix33f m, float angle, float x, float y, float z);
inline void m3dCrossProduct(M3DVector3f result, const M3DVector3f u, const M3DVector3f v);
void m3dLoadIdentity33(M3DMatrix33f m);
__inline void m3dRotateVector(M3DVector3f vOut, const M3DVector3f p, const M3DMatrix33f m);
//funcoes q desenham
void desenha(int, int, int, double);
void desenha_peixe(int corR, int corG, int corB, double tam, int qual, double nivel);
void desenha_tubarao(double tam_max, double nivel);
void desenha_baleia(double tam_max, double nivel);
void desenha_mar(int corR, int corG, int corB, float t, double q, double tamanho);
void desenha_mar(double t);
void desenha_barco(int corR, int corG, int corB, double escala,double nivel);
void desenha_chao(double);
void desenha_baiacu(double escala, double tamanho, double nivel, int n);
void desenha_cone(double h, double raio, int n);
void preenche(double xeye, double yeye, double zeye, double x, double y, double z, double cimax, double cimay, double cimaz);
void FGAPIENTRY calota(GLdouble radius,GLdouble angulo ,GLint slices, GLint stacks);
static void fghCircleTable(double **sint,double **cost,const int n);

//funcao q gera um numero segundo a funcao de densidade de probabilidade normal.
double gaussiano();
//carrega jpeg
int	load_texture (const char * filename,unsigned char * dest,const int format,const unsigned int size);
void		InitNoise (void);
float		Noise (const float, const float, const float, const float);
void instrucoes();
//declaracao de variaveis globais.
static int xRot = 0;
static int yRot = 0;
static int longe =300;
static int cam_no_meio =0;
static int luz1=1, luz2=1;
static int neblina=1;
static int ruido=1;
static double tempo_total = 0.0f, tmp_s_apertar=2.0f, tmp_s_ar=0.0f, delta_t=0.1;
static int modo_de_vizualizacao=0;
static int np1, np2, np3, np4, np5, np6, np1c, np2c, np3c, np4c, np5c, np6c;
static double ang=asin(NIVEL_MAR*1.0/PROF_MAX)*180/PI;

static GLuint	texture;
static float	surface[6 * RESOLUTION * (RESOLUTION + 1)];
static float	normal[6 * RESOLUTION * (RESOLUTION + 1)];


objeto *peixes1[NP1];
objeto *peixes2[NP2];
objeto *peixes3[NP3];
objeto *peixes4[NP4];
objeto *peixes5[NP5];
objeto *peixes6[NP6];
vetor *meio;
GLfloat lookAtParam[9];
GLfloat sol[4]   = { 0, PROF_MAX, 0, 1.0f };  // posicao do sol
GLfloat peixe_lanterna1[4]   = { 0, 0, 0, 0 };
GLfloat peixe_lanterna2[4]   = { 0, 0, 0, 0 };

// Called to draw scene
void RenderScene(void)
	{
	// limpa a tela
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //aplica luz do sol
        glLightfv(GL_LIGHT0, GL_POSITION, sol);
        
        //aplica a liz dos peixes lanterna, botao end acende ou apaga.
        peixe_lanterna1[0]=peixes6[0]->posicao->x;
        peixe_lanterna1[1]=peixes6[0]->posicao->y;
        peixe_lanterna1[2]=peixes6[0]->posicao->z;
        peixe_lanterna2[0]=peixes6[1]->posicao->x;
        peixe_lanterna2[1]=peixes6[1]->posicao->y;
        peixe_lanterna2[2]=peixes6[1]->posicao->z;
        if(luz1)	glEnable(GL_LIGHT1);
        else    glDisable(GL_LIGHT1);
		if(luz2)	glEnable(GL_LIGHT2);
        else    glDisable(GL_LIGHT2);
        glLightfv(GL_LIGHT1, GL_POSITION, peixe_lanterna1);
        glLightfv(GL_LIGHT2, GL_POSITION, peixe_lanterna2);	            


	int i;
	int j=0;
	
	//loop pra definir aceleracao dos peixes verdes(e a engine da a posicao, velocidade e rotacao de cada um) e desenhalos
for(i=0;i<NP2;i++)
{
	if(peixes2[i]->existe){
		
			peixes2[i]->aceleracao=seguir(peixes2[i],meio,1);//meio tem os fitoplanctons
			if(np3) peixes2[i]->aceleracao=soma(peixes2[i]->aceleracao,fugir(peixes2[i],mais_perto(peixes2[i], peixes3,NP3),2));
			if(np1) peixes2[i]->aceleracao=soma(peixes2[i]->aceleracao,fugir(peixes2[i],mais_perto(peixes2[i], peixes1,NP1),1));
			
		atualiza(peixes2[i], peixes2[i]->aceleracao,0);
	
		 glPushMatrix();
			glTranslatef(peixes2[i]->posicao->x, peixes2[i]->posicao->y, peixes2[i]->posicao->z);
			glRotatef(peixes2[i]->rotacao->ang*180/PI, peixes2[i]->rotacao->x,peixes2[i]->rotacao->y, peixes2[i]->rotacao->z);
			desenha_peixe(50, 150, 50, peixes2[i]->tam,0, 10*tempo_total*norma(peixes2[i]->velocidade)/peixes2[i]->vel_max);
			j++;
		 glPopMatrix();
	}	

}
//mesmo pros peixes azuis
for(i=0;i<NP3;i++)
	{
	if(peixes3[i]->existe)
	{
		//define a aceleracao
	if(np2)
	peixes3[i]->aceleracao=soma(soma(seguir(peixes3[i], mais_perto(peixes3[i], peixes2, NP2), 5), fugir(peixes3[i], mais_perto(peixes3[i], peixes4, NP4), 3)),fugir(peixes3[i], mais_perto(peixes3[i], peixes1, NP1), 1));
		else peixes3[i]->aceleracao=soma(fugir(peixes3[i], mais_perto(peixes3[i], peixes4, NP4), 6),fugir(peixes3[i], mais_perto(peixes3[i], peixes1, NP1), 2));

//colisao com peixes verdes, come os peixes verdes q estiverem perto(eles deixam de existir).
	objeto *pertos2=mais_perto_que(10,peixes3[i],peixes2,NP2 );
	while(pertos2)
	{
	pertos2->existe=0;
	np2--;
	np3c++;
	pertos2=pertos2->proximo;
	//if(pertos2==NULL)printf("peixes azuis comeu %d peixes\nnumero de peixes verdes: %d\n", np2c, np2);
	}	

	while(pertos2)
	{objeto *aux=pertos2;
	pertos2=pertos2->proximo;
	free(aux);
	}

	atualiza(peixes3[i], peixes3[i]->aceleracao,0);
	
	 glPushMatrix();
		glTranslatef(peixes3[i]->posicao->x, peixes3[i]->posicao->y, peixes3[i]->posicao->z);
		glRotatef(peixes3[i]->rotacao->ang*180/PI, peixes3[i]->rotacao->x,peixes3[i]->rotacao->y, peixes3[i]->rotacao->z);
		
		desenha_peixe(0, 50, 200, peixes3[i]->tam,3, 10*tempo_total*norma(peixes3[i]->velocidade)/peixes3[i]->vel_max);

	 glPopMatrix();
   }
}
//baleias

for(i=0;i<NP4;i++)
{	
if(peixes4[i]->existe)
{//como eh um cetaceo, ela tem q respirar a cada 50 ut, subindo ate a superficie
if(peixes4[i]->posicao->y>NIVEL_MAR) {tmp_s_ar=0;}
	else if(tmp_s_ar>50)	{peixes4[i]->aceleracao->x=0; peixes4[i]->aceleracao->y=GRAVIDADE;peixes4[i]->aceleracao->z=0;}
	else
		{zerar(peixes4[i]->aceleracao);
		//segue os azuis
		if(np3) peixes4[i]->aceleracao=seguir(peixes4[i],mais_perto(peixes4[i],peixes3,NP3),5);
	}
	//come verdes e azuis
objeto *pertos2=mais_perto_que(30,peixes4[i],peixes2,NP2 );
	while(pertos2)
	{
	pertos2->existe=0;
	np2--;
	np4c++;
	pertos2=pertos2->proximo;
//	if(pertos2==NULL)printf("baleias comeram %d peixes\nnumero de peixes verdes: %d\n", np4c, np2);
	}	
	while(pertos2)
	{objeto *aux=pertos2;
	pertos2=pertos2->proximo;
	free(aux);
	}

	objeto *pertos3=mais_perto_que(30,peixes4[i],peixes3,NP3 );
	while(pertos3)
	{
	pertos3->existe=0;
	np3--;
	np4c++;
	pertos3=pertos3->proximo;
	//if(pertos3==NULL)printf("baleias comeram %d peixes\nnumero de peixes azuis: %d\n", np4c, np3);
	}	

	while(pertos3)
	{objeto *aux=pertos3;
	pertos3=pertos3->proximo;
	free(aux);
	}


	atualiza(peixes4[i], peixes4[i]->aceleracao,0);
	 glPushMatrix();
		glTranslatef(peixes4[i]->posicao->x, peixes4[i]->posicao->y, peixes4[i]->posicao->z);
		glRotatef(peixes4[i]->rotacao->ang*180/PI, peixes4[i]->rotacao->x,peixes4[i]->rotacao->y, peixes4[i]->rotacao->z);
		desenha_baleia(peixes4[i]->tam, 10*tempo_total*norma(peixes4[i]->velocidade)/peixes4[i]->vel_max);

	 glPopMatrix();
   }
}
//printf("%f, %f e %f\n", prod_escalar(peixes1[0]->direcao_cima, peixes1[0]->velocidade), prod_escalar(peixes1[0]->direcao_cima, peixes1[0]->rotacao), prod_escalar(peixes1[0]->rotacao, peixes1[0]->velocidade));


//baiacus
srand(time(0));
for(i=0;i<NP5;i++)
{	
if(peixes5[i]->existe)
{
	//aceleracao aleatoria e tende a ficar no meio tbm, come fitoplancton tbm.
	
	peixes5[i]->aceleracao->x=30+rand()%100;
	peixes5[i]->aceleracao->y=30+rand()%100;
	peixes5[i]->aceleracao->z=30+rand()%100;
	peixes5[i]->aceleracao=soma(peixes5[i]->aceleracao,seguir(peixes5[i],meio,0.5));
	//cresce com o tempo
	double tamanho=((int(tempo_total))%45)/45.0;
	
	//colisao, se algum peixe encostar, ele fica paralizado(doidao), peixes luz sao imunes.
objeto *pertos2=mais_perto_que(60*tamanho,peixes5[i],peixes2,NP2 );
	while(pertos2)
	{
	pertos2->paralizado=1;
	pertos2=pertos2->proximo;
	//if(pertos2==NULL)printf("baiacu paralizou peixes verdes");
	}	
	while(pertos2)
	{objeto *aux=pertos2;
	pertos2=pertos2->proximo;
	free(aux);
	}

	objeto *pertos3=mais_perto_que(70*tamanho,peixes5[i],peixes3,NP3 );
	while(pertos3)
	{
	pertos3->paralizado=1;
	pertos3=pertos3->proximo;
	//if(pertos3==NULL)printf("baiacu paralizou peixes azuis");
	}	

	while(pertos3)
	{objeto *aux=pertos3;
	pertos3=pertos3->proximo;
	free(aux);
	}

	objeto *pertos4=mais_perto_que(90*tamanho,peixes5[i],peixes4,NP4 );
	while(pertos4)
	{
	pertos4->paralizado=1;
	pertos4=pertos4->proximo;
	//if(pertos4==NULL)printf("baiacu paralizou peixes vermelhos");
	}	

	while(pertos4)
	{objeto *aux=pertos4;
	pertos4=pertos4->proximo;
	free(aux);
	}


	objeto *pertos1=mais_perto_que(80*tamanho,peixes5[i],peixes1,NP1 );
	while(pertos1)
	{
	pertos1->paralizado=1;
	pertos1=pertos1->proximo;
	//if(pertos1==NULL)printf("baiacu paralizou peixes amarelos");
	}	

	while(pertos1)
	{objeto *aux=pertos1;
	pertos1=pertos1->proximo;
	free(aux);
	}

	atualiza(peixes5[i], peixes5[i]->aceleracao,0);
	 glPushMatrix();
		glTranslatef(peixes5[i]->posicao->x, peixes5[i]->posicao->y, peixes5[i]->posicao->z);
		glRotatef(peixes5[i]->rotacao->ang*180/PI, peixes5[i]->rotacao->x,peixes5[i]->rotacao->y, peixes5[i]->rotacao->z);
		desenha_baiacu(0.5, tamanho,10*tempo_total*norma(peixes6[i]->velocidade)/peixes6[i]->vel_max, 20);

	 glPopMatrix();
   }
}
//peixes luz
for(i=0;i<NP6;i++)
{	
if(peixes6[i]->existe)
{
	if(tmp_s_apertar<2 && modo_de_vizualizacao==6){
		zerar(peixes6[i]->aceleracao);
		if(yRot!=0)peixes6[i]->aceleracao=mult_escalar(peixes6[i]->direcao_lado,-yRot*2000);
		if(xRot!=0)peixes6[i]->aceleracao=mult_escalar(peixes6[i]->direcao_cima,xRot*2000);
				
		}else{
	//acel aleatoria, comem fitoplancton
	
	peixes6[i]->aceleracao->x=30+rand()%100;
	peixes6[i]->aceleracao->y=30+rand()%100;
	peixes6[i]->aceleracao->z=30+rand()%100;
	peixes6[i]->aceleracao=soma(peixes6[i]->aceleracao,seguir(peixes6[i],meio,0.3));
	//mas tendem a ficar no fundo do mar, abaixo do meio
	if (peixes6[i]->posicao->y>meio->y) peixes6[i]->aceleracao->y=-GRAVIDADE;
	atualiza(peixes6[i], peixes6[i]->aceleracao,0);
	 }
	 
	 glPushMatrix();
		glTranslatef(peixes6[i]->posicao->x, peixes6[i]->posicao->y, peixes6[i]->posicao->z);
		glRotatef(peixes6[i]->rotacao->ang*180/PI, peixes6[i]->rotacao->x,peixes6[i]->rotacao->y, peixes6[i]->rotacao->z);
		desenha_peixe(255,255,150, peixes6[i]->tam,i+1, 10*tempo_total*norma(peixes6[i]->velocidade)/peixes6[i]->vel_max);

	 glPopMatrix();
   }
}
//tubaroes
for(i=0;i<NP1;i++)
{	//controle manual, desabilitado se ficar 2 segundos sem apertar alguma seta.
	if(tmp_s_apertar<2 && modo_de_vizualizacao==1){
		zerar(peixes1[i]->aceleracao);
		if(yRot!=0)peixes1[i]->aceleracao=mult_escalar(peixes1[i]->direcao_lado,-yRot*2000);
		if(xRot!=0)peixes1[i]->aceleracao=mult_escalar(peixes1[i]->direcao_cima,xRot*2000);
		//controle automatico, segue azuis ou verdes, o meis perto entre eles.		
		}else{zerar(peixes1[i]->aceleracao);
		if(distancia(peixes1[i], mais_perto(peixes1[i], peixes3, NP3))<distancia(peixes1[i], mais_perto(peixes1[i], peixes2, NP2)))
		{if(np3) peixes1[i]->aceleracao=seguir(peixes1[i],mais_perto(peixes1[i], peixes3, NP3),50);}
		else { if(np2) peixes1[i]->aceleracao=seguir(peixes1[i],mais_perto(peixes1[i], peixes2, NP2),50);}
	}

//colisao pra comer os verdes e azuis q estao perto.
	objeto *pertos2=mais_perto_que(20,peixes1[i],peixes2,NP2 );
	while(pertos2)
	{
	pertos2->existe=0;
	np2--;
	np1c++;
	pertos2=pertos2->proximo;
	//if(pertos2==NULL)printf("peixe amerelo comeu %d peixes\nnumero de peixes verdes: %d\n", np1c, np2);
	}	
	while(pertos2)
	{objeto *aux=pertos2;
	pertos2=pertos2->proximo;
	free(aux);
	}
	objeto *pertos3=mais_perto_que(30,peixes1[i],peixes3,NP3 );
	while(pertos3)
	{
	pertos3->existe=0;
	np3--;
	np1c++;
	pertos3=pertos3->proximo;
	//if(pertos3==NULL)printf("peixe amerelo comeu %d peixes\nnumero de peixes azuis: %d\n", np1c, np3);
	}	
	while(pertos3)
	{objeto *aux=pertos3;
	pertos3=pertos3->proximo;
	free(aux);
	}
	atualiza(peixes1[i], peixes1[i]->aceleracao,0);

	glPushMatrix();
		glTranslatef(peixes1[i]->posicao->x, peixes1[i]->posicao->y, peixes1[i]->posicao->z);
		glRotatef(peixes1[i]->rotacao->ang*180/PI, peixes1[i]->rotacao->x,peixes1[i]->rotacao->y, peixes1[i]->rotacao->z);
		
		desenha_tubarao(peixes1[i]->tam,10*tempo_total*norma(peixes1[i]->velocidade)/peixes1[i]->vel_max);
	
	 glPopMatrix();
}
		// incrementa os tempos.
		tempo_total += 0.1f;
		tmp_s_apertar+=0.1;
		tmp_s_ar+=0.1;
		if(tempo_total > 360.0f)
			tempo_total = 0.0f;
			//condicao pra desparalizar.
		if(((int(tempo_total))%45)>40){
			for(i=0;i<NP1;i++)
			peixes1[i]->paralizado=0;
			for(i=0;i<NP2;i++)
			peixes2[i]->paralizado=0;
			for(i=0;i<NP3;i++)
			peixes3[i]->paralizado=0;
			for(i=0;i<NP4;i++)
			peixes4[i]->paralizado=0;
			for(i=0;i<NP5;i++)
			peixes5[i]->paralizado=0;
			for(i=0;i<NP6;i++)
			peixes6[i]->paralizado=0;
		}
			
			glPushMatrix();
			
			//desenha as outras coisas.
desenha_mar(142,107,35,tempo_total, 10, 2*PROF_MAX);
glTranslatef(0,NIVEL_MAR,0);
		
		glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if(ruido)   glEnable (GL_TEXTURE_2D);
        else glDisable (GL_TEXTURE_2D);
        desenha_mar(tempo_total);
        desenha_chao(30);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
		desenha_barco(50,50,0,1,0);
		glPopMatrix();

//escolhe o ponto de vizualizacao(os parametros do gluLookAt())

		switch(modo_de_vizualizacao){
		case 1:
			if(peixes1[0]->posicao->y>NIVEL_MAR)
			{preenche(peixes1[0]->posicao->x+200, peixes1[0]->posicao->y+300, peixes1[0]->posicao->z+200, peixes1[0]->posicao->x, peixes1[0]->posicao->y, peixes1[0]->posicao->z,0, 1,0);}
			else preenche(peixes1[0]->posicao->x-normalizar(longe,peixes1[0]->velocidade)->x, peixes1[0]->posicao->y-normalizar(longe,peixes1[0]->velocidade)->y, peixes1[0]->posicao->z-normalizar(longe,peixes1[0]->velocidade)->z, peixes1[0]->velocidade->x+peixes1[0]->posicao->x, peixes1[0]->velocidade->y+peixes1[0]->posicao->y, peixes1[0]->velocidade->z+peixes1[0]->posicao->z,peixes1[0]->direcao_cima->x, peixes1[0]->direcao_cima->y, peixes1[0]->direcao_cima->z);
			//else preenche(peixes1[0]->posicao->x-normalizar(longe,peixes1[0]->velocidade)->x, peixes1[0]->posicao->y-normalizar(longe,peixes1[0]->velocidade)->y, peixes1[0]->posicao->z-normalizar(longe,peixes1[0]->velocidade)->z, peixes1[0]->velocidade->x+peixes1[0]->posicao->x, peixes1[0]->velocidade->y+peixes1[0]->posicao->y, peixes1[0]->velocidade->z+peixes1[0]->posicao->z,0,1,0);
		break;
		case 2:
			for(i=0;i<NP2;i++)
				if(peixes2[i]->existe) break;
			if(i==NP2) {
				modo_de_vizualizacao++;
				break;}
			if(peixes2[i]->posicao->y>NIVEL_MAR) {preenche(peixes2[i]->posicao->x, peixes2[i]->posicao->y+300, peixes2[i]->posicao->z, peixes2[i]->posicao->x, peixes2[i]->posicao->y, peixes2[i]->posicao->z,1, 0 ,0);}
			else preenche(peixes2[i]->posicao->x-normalizar(longe,peixes2[i]->velocidade)->x, peixes2[i]->posicao->y-normalizar(longe,peixes2[i]->velocidade)->y, peixes2[i]->posicao->z-normalizar(longe,peixes2[i]->velocidade)->z, peixes2[i]->velocidade->x+peixes2[i]->posicao->x, peixes2[i]->velocidade->y+peixes2[i]->posicao->y, peixes2[i]->velocidade->z+peixes2[i]->posicao->z,peixes2[i]->direcao_cima->x, peixes2[i]->direcao_cima->y, peixes2[i]->direcao_cima->z);
		break;
		case 3:
			for(i=0;i<NP3;i++)
				if(peixes3[i]->existe) break;
			if(i==NP3) {
				modo_de_vizualizacao++;
				break;}
			if(peixes3[i]->posicao->y>NIVEL_MAR) preenche(peixes3[i]->posicao->x, peixes3[i]->posicao->y+300, peixes3[i]->posicao->z, peixes3[i]->posicao->x, peixes3[i]->posicao->y, peixes3[i]->posicao->z,1, 0 ,0);
			else preenche(peixes3[i]->posicao->x-normalizar(longe,peixes3[i]->velocidade)->x, peixes3[i]->posicao->y-normalizar(longe,peixes3[i]->velocidade)->y, peixes3[i]->posicao->z-normalizar(longe,peixes3[i]->velocidade)->z, peixes3[i]->velocidade->x+peixes3[i]->posicao->x, peixes3[i]->velocidade->y+peixes3[i]->posicao->y, peixes3[i]->velocidade->z+peixes3[i]->posicao->z,peixes3[i]->direcao_cima->x, peixes3[i]->direcao_cima->y, peixes3[i]->direcao_cima->z);
		break;
		case 4:
			for(i=0;i<NP4;i++)
				if(peixes4[i]->existe) break;
			if(i==NP4) {
				modo_de_vizualizacao++;
				break;}
			if(peixes4[i]->posicao->y>NIVEL_MAR) preenche(peixes4[i]->posicao->x, peixes4[i]->posicao->y+300, peixes4[i]->posicao->z, peixes4[i]->posicao->x, peixes4[i]->posicao->y, peixes4[i]->posicao->z,1, 0 ,0);
			else preenche(peixes4[i]->posicao->x-normalizar(longe,peixes4[i]->velocidade)->x, peixes4[i]->posicao->y-normalizar(longe,peixes4[i]->velocidade)->y, peixes4[i]->posicao->z-normalizar(longe,peixes4[i]->velocidade)->z, peixes4[i]->velocidade->x+peixes4[i]->posicao->x, peixes4[i]->velocidade->y+peixes4[i]->posicao->y, peixes4[i]->velocidade->z+peixes4[i]->posicao->z,peixes4[i]->direcao_cima->x, peixes4[i]->direcao_cima->y, peixes4[i]->direcao_cima->z);
		break;
				case 5:
			for(i=0;i<NP5;i++)
				if(peixes5[i]->existe) break;
			if(i==NP5) {
				modo_de_vizualizacao++;
				break;}
			if(peixes5[i]->posicao->y>NIVEL_MAR) preenche(peixes5[i]->posicao->x, peixes5[i]->posicao->y+300, peixes5[i]->posicao->z, peixes5[i]->posicao->x, peixes5[i]->posicao->y, peixes5[i]->posicao->z,1, 0 ,0);
			else preenche(peixes5[i]->posicao->x-normalizar(longe,peixes5[i]->velocidade)->x, peixes5[i]->posicao->y-normalizar(longe,peixes5[i]->velocidade)->y, peixes5[i]->posicao->z-normalizar(longe,peixes5[i]->velocidade)->z, peixes5[i]->velocidade->x+peixes5[i]->posicao->x, peixes5[i]->velocidade->y+peixes5[i]->posicao->y, peixes5[i]->velocidade->z+peixes5[i]->posicao->z,peixes5[i]->direcao_cima->x, peixes5[i]->direcao_cima->y, peixes5[i]->direcao_cima->z);
		break;
				case 6:
			for(i=0;i<NP6;i++)
				if(peixes6[i]->existe) break;
			if(i==NP6) {
				modo_de_vizualizacao++;
				break;}
			if(peixes6[i]->posicao->y>NIVEL_MAR) preenche(peixes6[i]->posicao->x, peixes6[i]->posicao->y+300, peixes6[i]->posicao->z, peixes6[i]->posicao->x, peixes6[i]->posicao->y, peixes6[i]->posicao->z,1, 0 ,0);
			else preenche(peixes6[i]->posicao->x-normalizar(longe,peixes6[i]->velocidade)->x, peixes6[i]->posicao->y-normalizar(longe,peixes6[i]->velocidade)->y, peixes6[i]->posicao->z-normalizar(longe,peixes6[i]->velocidade)->z, peixes6[i]->velocidade->x+peixes6[i]->posicao->x, peixes6[i]->velocidade->y+peixes6[i]->posicao->y, peixes6[i]->velocidade->z+peixes6[i]->posicao->z,peixes6[i]->direcao_cima->x, peixes6[i]->direcao_cima->y, peixes6[i]->direcao_cima->z);
		break;
	}
	double dist_cam=sqrt(lookAtParam[0]*lookAtParam[0]+lookAtParam[1]*lookAtParam[1]+lookAtParam[2]*lookAtParam[2]);
	//cuida de caso o olho da camera fique abaixo do chao.
	if(dist_cam>PROF_MAX-10)
	{
		lookAtParam[0]*=(PROF_MAX-10)/dist_cam;
		lookAtParam[1]*=(PROF_MAX-10)/dist_cam;
		lookAtParam[2]*=(PROF_MAX-10)/dist_cam;
		if(lookAtParam[0]==lookAtParam[3]&&lookAtParam[1]==lookAtParam[4]&&lookAtParam[2]==lookAtParam[5])
		lookAtParam[1]+=30;
	}
	if(cam_no_meio){
		lookAtParam[0]=meio->x;
		lookAtParam[1]=meio->y;
		lookAtParam[2]=meio->z;
		}
	//fumaca
glPushMatrix();

	GLfloat fogColor[] = {0, 0.05f, 0.2, 1};

	glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    if(neblina)glEnable(GL_FOG);
    else glDisable(GL_FOG);
    glFogf(GL_FOG_START, longe);
    glFogf(GL_FOG_END, PROF_MAX);
    glFogf (GL_FOG_DENSITY,(NIVEL_MAR-lookAtParam[1])/(PROF_MAX+NIVEL_MAR));//quanto mais profundo, maior a densidade
	glHint (GL_FOG_HINT, GL_NICEST); 
	//se estiver acima do nivel do mar, afasta o inicio da fumaca
    if(lookAtParam[1]>NIVEL_MAR)glFogf(GL_FOG_START, lookAtParam[1]-NIVEL_MAR);
    
glPopMatrix();	
//o gluLookAt
glLoadIdentity();
gluLookAt(lookAtParam[0],lookAtParam[1],lookAtParam[2],lookAtParam[3],lookAtParam[4],lookAtParam[5],lookAtParam[6],lookAtParam[7],lookAtParam[8]);

//printf("cam=(%.2f,%.2f,%.2f), peixe=(%.2f,%.2f, %.2f), cima=(%.2f, %.2f, %.2f)\n",lookAtParam[0],lookAtParam[1],lookAtParam[2],lookAtParam[3],lookAtParam[4],lookAtParam[5],lookAtParam[6],lookAtParam[7],lookAtParam[8]);

// imprime os dados e mostra a imagem
printf("\r%d/%d verdes, %d/%d azuis, %d/%d baleias, %d/%d tubaroes, %d/%d baiacus, %d/%d peixes luz\r", np2, NP2, np3, NP3, np4, NP4, np1, NP1, np5, NP5,np6, NP6);
		glutSwapBuffers();
		xRot=0.0;
		yRot=0.0;
		}


//setup
void SetupRC()
	{ // valores de luz
    GLfloat  luzBaixa[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat  luzTotal[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glEnable(GL_DEPTH_TEST);	// Hidden surface removal
    glFrontFace(GL_CCW);		// Counter clock-wise polygons face out
    glEnable(GL_CULL_FACE);		

    // habilita luz
    glEnable(GL_LIGHTING);

    // luz do sol
    glLightfv(GL_LIGHT0,GL_AMBIENT,luzBaixa);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,luzTotal);
    glEnable(GL_LIGHT0);
	//luzes dos peixes lanterna
	glLightfv(GL_LIGHT1,GL_DIFFUSE,luzTotal);
    glEnable(GL_LIGHT1);

	glLightfv(GL_LIGHT2,GL_DIFFUSE,luzTotal);
    glEnable(GL_LIGHT2);
    //habilita fumaca(pra simular q quanto mais fundo menos se ve)
	glEnable(GL_FOG);
    
    glEnable(GL_COLOR_MATERIAL);
	
    // material vai pegar a cor do ambiente e luz difusa
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  
    glMateriali(GL_FRONT, GL_SHININESS, 128);
   
      unsigned char total_texture[4 * 256 * 256];
  unsigned char alpha_texture[256 * 256];
  unsigned char caustic_texture[3 * 256 * 256];
  unsigned int i;

  InitNoise ();


    /* Texture loading  */
  glGenTextures (1, &texture);
  if (load_texture ("alpha.jpg", alpha_texture, GL_ALPHA, 256) != 0 ||
      load_texture ("reflection.jpg", caustic_texture, GL_RGB, 256) != 0)
    return ;
  for (i = 0; i < 256 * 256; i++)
    {
      total_texture[4 * i] = caustic_texture[3 * i];
      total_texture[4 * i + 1] = caustic_texture[3 * i + 1];
      total_texture[4 * i + 2] = caustic_texture[3 * i + 2];
      total_texture[4 * i + 3] = alpha_texture[i];
    }
  glBindTexture (GL_TEXTURE_2D, texture);
  gluBuild2DMipmaps (GL_TEXTURE_2D, GL_RGBA, 256, 256, GL_RGBA,
		     GL_UNSIGNED_BYTE,  total_texture);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glEnable (GL_TEXTURE_GEN_S);
  glEnable (GL_TEXTURE_GEN_T);
  glTexGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
  glTexGeni (GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
    // azul escuro
    glClearColor(0.4f, 0.4f, 1, 1.0f );
    
    glEnable(GL_NORMALIZE);	
    }
    
    
void keyboard(int key, int x, int y)
	{
		
		switch(key){
			case 27:
				exit(0);
				
			break;
			
			case GLUT_KEY_UP:
				xRot=-1;
				tmp_s_apertar=0.0;
			break;

			case 103:
				xRot= 1;
				tmp_s_apertar=0.0;
			break;
			
			case GLUT_KEY_LEFT:
				yRot= -1;
				tmp_s_apertar=0.0;
			break;
			
			case GLUT_KEY_RIGHT:
				yRot= 1;
				tmp_s_apertar=0.0;
			break;
			
			
			case GLUT_KEY_PAGE_DOWN:
				if(modo_de_vizualizacao)
				longe+=10;
			break;
			
			case GLUT_KEY_PAGE_UP:
				if(modo_de_vizualizacao)
				longe-=10;
			break;
			
            case GLUT_KEY_HOME:
				modo_de_vizualizacao=(modo_de_vizualizacao+1)%7;
			break;
			
			case GLUT_KEY_F1:
				neblina=(neblina+1)%2;
			break;
			
			case GLUT_KEY_F2:
				ruido=(ruido+1)%2;
			break;
			
			case GLUT_KEY_F3:
				luz1=(luz1+1)%2;
			break;

			case GLUT_KEY_F4:
				luz2=(luz2+1)%2;
			break;

			case GLUT_KEY_END:
				cam_no_meio=(cam_no_meio+1)%2;
			break;

			case GLUT_KEY_INSERT:
				printf("\npeixes verdes comeram %d peixes;\npeixes azuis comeram %d peixes;\nbaleias comeram %d peixes;\ntubaroes comeram %d peixes;\nbaiacus comeram %d peixes;\npeixes luz comeram %d peixes;\n",np2c,np3c,np4c,np1c,np5c,np6c);
			break;
			case GLUT_KEY_F12:
				instrucoes();
			break;
		}
	
	
	}

void TimerFunc(int value)
    {
    glutPostRedisplay();
    glutTimerFunc(50, TimerFunc, 50);
    }

void ChangeSize(int w, int h)
    {
    GLdouble fAspect;

    // Prevent a divide by zero
    if(h == 0)
        h = 1;

    // Set Viewport to window dimensions
    glViewport(0, 0, w, h);

    // Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    fAspect = (double)w/(double)h;
    gluPerspective(45.0, fAspect, 1.0, 10*PROF_MAX);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -300.0f);
    }

int main(int argc, char* argv[])
	{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(800, 600);
	glutCreateWindow("Aquahunt");
	glutReshapeFunc(ChangeSize);
	glutSpecialFunc(keyboard);
	
	//comeca na visao do tubarao

	modo_de_vizualizacao=1;
	np1=NP1;
	np2=NP2;
	np3=NP3;
	np4=NP4;
	np5=NP5;
	np6=NP6;
	np1c=0;
	np2c=0;
	np3c=0;
	np4c=0;
	np5c=0;
	np6c=0;
	srand(time(0));
	int i;
	//inicia os objetos dos peixes
	for(i=0;i<NP1;i++)
	peixes1[i]=iniciar(8, 40, 0,0, -500+i*1000, i*500, 20, 60, 300);
	for(i=0;i<NP2;i++)
	//massa, tamanho, velocidade maxima, seguem a funcao densidade de probabilidade normal, eh o q geralmente ocorre na natureza.
	peixes2[i]=iniciar(30, 0, -100, 400+37*i%100, -50+50*i%70, i%51, fabs(gaussiano()+4), fabs(gaussiano()*2+9), fabs(gaussiano()*10+50));
	for(i=0;i<NP3;i++)
	//pros azuis, a posicao tbm segue a normal.
	peixes3[i]=iniciar(0, -20, 60, -500+200*gaussiano(), -PROF_MAX+100*gaussiano(), 100+50*gaussiano(), fabs(10+4*gaussiano()),fabs(gaussiano()*4+18), fabs(65+gaussiano()*15));
	for(i=0;i<NP4;i++)
	peixes4[i]=iniciar(0, 0, 6, i*500, (6+290*i)%500, -300+200*i, fabs(30+15*gaussiano()),fabs(gaussiano()*10+108), fabs(400+100*gaussiano()));
	for(i=0;i<NP5;i++)
	peixes5[i]=iniciar(0,0,1,0,-200*i,0,300, 100,100);
	for(i=0;i<NP6;i++)
	peixes6[i]=iniciar(0, -40*i, 40*i,0, -PROF_MAX+30, i*500, 30, 30, 150);
	//os parametros da funcao iniciar sao a velocidade(x,y,z), a posicao(x,y,z), massa, tamanho e velocidade maxima.
	
	meio=new_vetor(0,NIVEL_MAR-(NIVEL_MAR+PROF_MAX)/2,0);
	instrucoes();
	printf("Aperte alguma coisa pra começar.\n");
	getchar();
	glutDisplayFunc(RenderScene);
        glutTimerFunc(50, TimerFunc, 50);
	SetupRC();
	glutMainLoop();

	return 0;
	}
	
	//imprime um vetor
void printvetor(vetor *vet)
{
	printf("(%f,%f,%f)\n",vet->x,vet->y,vet->z);
}
//aloca espaco prum vetor e o retorna
vetor* new_vetor(double x, double y, double z){
	vetor* vet=(vetor*)malloc(sizeof(vetor));	
	vet->x=x;
	vet->y=y;
	vet->z=z;
	return vet;
	}
	//derivada discreta eq a a=v/t
vetor *derivada(vetor *prox, vetor *ant, double delta_t)
{
	vetor* mult1=mult_escalar(ant, -1);
	vetor* soma1=soma(prox,mult1);
	vetor* res=mult_escalar(soma1,1/delta_t);
	free(mult1);
	free(soma1);
	return res;
}

//integral discreta, eq a v=v0+at
vetor *integral(vetor *v, vetor *v0, double delta_t)
{
	vetor* vet=(vetor*)malloc(sizeof(vetor));
	vet->x=v0->x+v->x*delta_t;
	vet->y=v0->y+v->y*delta_t;
	vet->z=v0->z+v->z*delta_t;

	return vet;
}

//referencia: http://pt.wikibooks.org/wiki/C%C3%A1lculo_(Volume_2)/Aplica%C3%A7%C3%B5es_de_fun%C3%A7%C3%B5es_vetoriais#Acelera.C3.A7.C3.B5es_tangentes_e_normais


vetor* prod_vetorial(vetor* a, vetor* b)
	{
	vetor* c=(vetor*)malloc(sizeof(vetor));
	c->x=(a->y*b->z)-(a->z*b->y);
	c->y=(a->z*b->x)-(a->x*b->z);
	c->z=(a->x*b->y)-(a->y*b->x);
	return c;
	}

double prod_escalar(vetor* a, vetor* b)
	{return a->x*b->x+a->y*b->y+a->z*b->z;}

double norma(vetor* a)
	{return sqrt((a->x*a->x)+(a->y*a->y)+(a->z*a->z));}

vetor* mult_escalar(vetor *vet, double escalar)
	{
	vetor* ret=(vetor*)malloc(sizeof(vetor));
	ret->x=vet->x*escalar;
	ret->y=vet->y*escalar;
	ret->z=vet->z*escalar;

	return ret;
	}
vetor* escala(vetor *vet, double valor)
	{
	vet->x*=valor;	
	vet->y*=valor;
	vet->z*=valor;
	return vet;	
	}
vetor* soma(vetor* a, vetor* b)
	{
	vetor* c=(vetor*)malloc(sizeof(vetor));
	c->x=a->x+b->x;
	c->y=a->y+b->y;
	c->z=a->z+b->z;
	return c;
	}
	
vetor* subtracao(vetor* a, vetor* b)
	{			vetor* c=(vetor*)malloc(sizeof(vetor));
	c->x=a->x-b->x;
	c->y=a->y-b->y;
	c->z=a->z-b->z;
	return c;
	}
//desixa o vetor ocm tamanho 1
vetor *normalizar(double tamanho, vetor *vet){
	return mult_escalar(vet,tamanho/norma(vet));
	
	}
//retorna um vetor perpenticular aos 2 passados como parametro.
//com angulo pra q o inicial atinja o final, em radianos
//o valor de retorno sao os parametros do glRotate
vetor *rotacao(vetor *inicial, vetor*final)
{	

	vetor* res;
	
	res=prod_vetorial(inicial,final);
	double nor=norma(res);
	if(nor!=0)
		{
		res->x=res->x*1/nor;
		res->y=res->y*1/nor;
		res->z=res->z*1/nor;
		res->ang=angulo_entre(inicial, final);
	}else 
		res->ang=0;
	return res;

}

double angulo_entre(vetor *um, vetor* outro)
{	
	return acos(prod_escalar(um,outro)/(norma(um)*norma(outro)));
	
	}
	
//feito inicialmente pra naves, pois elas atiram...
int estanamira(vetor *v1, vetor *p1, vetor *p2)
{
	vetor* d=(vetor*)malloc(sizeof(vetor));
	vetor* v=(vetor*)malloc(sizeof(vetor));
	d=soma(p2,mult_escalar(p1,-1));
	d=mult_escalar(d,1/norma(d));
	v=mult_escalar(v1,1/norma(v1));
	//
	if(prod_escalar(d,v)>0.9)
		{free(d);
		free(v);
		return 1;}

	free(d);
	free(v);
	return 0;
}
//pra objetos.

//retorna a aceleracao na direcao do objeto a ser seguido, quanto mais longe, maior ela eh
vetor *seguir(objeto *eu, objeto* ele, double intensidade)
{
	vetor* sub1=subtracao(ele->posicao, eu->posicao);
	vetor* res=mult_escalar(sub1,intensidade);
	free(sub1);
	return res;
	
	}
vetor *seguir(objeto *eu, vetor* ele, double intensidade)
{
	vetor* sub1=subtracao(ele, eu->posicao);
	vetor* res=mult_escalar(sub1,intensidade);
	free(sub1);
	return res;
	
	}
//retorna a aceleracao na direcao contraria do objeto ao qual esta fugindo, quanto mais perto, maior ela eh
vetor *fugir(objeto *eu, objeto* ele, double intensidade)
{	vetor* res=seguir(eu,ele,intensidade);
	vetor* res1=mult_escalar(res,100/norma(res));
	free(res);
	return escala(res1, -1);
	}

//varre todos os outros e retorna o mais perto
objeto* mais_perto(objeto *esse, objeto *outros[], int quantos)
{int i, maior=0;
double distance=PROF_MAX, aux;
	
for(i=0;i<quantos;i++)
	if (outros[i]->existe)
	{
	aux=distancia(esse,outros[i]);
		if (aux<distance){
			distance=aux;
			maior=i;
		}
	}
return outros[maior];

}
double distancia(objeto *esse, objeto* outro)
{	double dis;
	vetor* soma1;
		vetor* mult1;
		mult1=mult_escalar(outro->posicao, -1);
		soma1=soma(esse->posicao,mult1);
	dis=norma(soma1);
	free(soma1);
	free(mult1);
	
return dis;
}
//cria uma lista com os objetos mais perto que uma certa distancia, retorna o ponteiro pro primeiro.
objeto* mais_perto_que(double dist, objeto *esse, objeto *outros[], int quantos)
{int i;
objeto *peixes, *ultimo;
peixes=NULL;
ultimo=NULL;
for(i=0;i<quantos;i++)
	if (outros[i]->existe)
	{
	if(distancia(esse, outros[i])<dist)
		{	
			if(peixes)//se nao eh null
			{ultimo->proximo=outros[i];
			ultimo=ultimo->proximo;
			
			}
			else//se eh null
			{peixes=outros[i];
			ultimo=peixes;
			
			}
			ultimo->proximo=NULL;
		}
	}
	
return peixes;

}

void zerar(vetor* vet){
	vet->x=0;
	vet->y=0;
	vet->z=0;
}


//a engine esta aqui.
objeto* atualiza(objeto *nave, vetor* forca, int boom)
{
	vetor *ac=mult_escalar(forca,nave->paralizado == 0 ? 1/nave->massa : 0);
	
	if(nave->paralizado){
		ac->x=0; ac->y=0; ac->z=0;
	}
	if(nave->posicao->y>NIVEL_MAR) {
		ac->x=0; ac->y=-GRAVIDADE; ac->z=0;
	}
	nave->aceleracao=ac;
	
	free(forca);
	if(norma(nave->posicao)>PROF_MAX)
		nave->velocidade=mult_escalar(nave->posicao,-0.5*norma(nave->velocidade)/PROF_MAX);
	else nave->velocidade=integral(ac, nave->velocidade,delta_t);

	if(norma(nave->velocidade)>nave->vel_max && nave->posicao->y<NIVEL_MAR)
		nave->velocidade=normalizar(nave->vel_max,nave->velocidade);
	nave->posicao=integral(nave->velocidade, nave->posicao,delta_t);
	
	vetor* oriIni=new_vetor(0,0,1);
	
	nave->rotacao=rotacao(oriIni,nave->velocidade);
	
	M3DMatrix33f m;
	m3dLoadIdentity33(m);
	m3dRotationMatrix33(m, nave->rotacao->ang, nave->rotacao->x, nave->rotacao->y, nave->rotacao->z);
	M3DVector3f cima;
	M3DVector3f cima_inicial={0,1,0};
	
	m3dRotateVector(cima, cima_inicial, m);
	
	nave->direcao_cima->x=cima[0];
	nave->direcao_cima->y=cima[1];
	nave->direcao_cima->z=cima[2];
		
	nave->direcao_lado=rotacao(nave->direcao_cima, nave->velocidade);
	nave->direcao_lado=normalizar(1,nave->direcao_lado);
	if(nave->paralizado)
	nave->rotacao->ang+=tempo_total*10;
	
	free(oriIni);
return nave;
}

objeto* iniciar(double vx, double vy, double vz,double px, double py, double pz, double massa, double tam, double vel_max)
    
{
	
	vetor *a=new_vetor(0,0,0);
	vetor *v0=new_vetor(vx,vy,vz);
	vetor *p0=new_vetor(px, py, pz);
	if(norma(p0)>PROF_MAX-10)
	escala(p0,(PROF_MAX-10)/norma(p0));
	vetor* oriIni=new_vetor(0,0,1);
	vetor* rot=(vetor*)malloc(sizeof(vetor));
	rot=rotacao(oriIni, v0);
	M3DMatrix33f m;
	m3dLoadIdentity33(m);
	m3dRotationMatrix33(m, rot->ang, rot->x, rot->y, rot->z);
	M3DVector3f cima;
	M3DVector3f cima_inicial={0,1,0};
	
	m3dRotateVector(cima, cima_inicial, m);
	vetor *vcima=new_vetor(cima[0],cima[1],cima[2]);
	vetor *lado;
	lado=rotacao(vcima, v0);
	
	objeto *esse =(objeto*)malloc(sizeof(objeto));

	esse->posicao=p0;
	esse->velocidade=v0;
	esse->aceleracao=a;
	esse->rotacao=rot;
	esse->direcao_cima=vcima;
	esse->direcao_lado=lado;
	esse->massa=massa;
	esse->tam=tam;
	esse->vel_max=vel_max;
	esse->existe=1;
	esse->paralizado=0;
	
	return esse;
}

void desenha_mar(int corR, int corG, int corB, float t, double q, double tamanho)
{
	glPushMatrix();
glColor3ub(100, 100, 0);
	 glFrontFace(GL_CW);
	 glRotatef(90, 1, 0,0);
	calota(PROF_MAX,92+ang,  50, 50);
	 glFrontFace(GL_CCW);
	 glPopMatrix();
        glEnd();
}


/*
** Function called to update rendering
*/
static float	z (const float x, const float y, const float t)
{
  const float x2 = x - 3;
  const float y2 = y + 1;
  const float xx = x2 * x2;
  const float yy = y2 * y2;
  return ((2 * sinf (20 * sqrtf (xx + yy) - 4 * t) +
	   Noise (10 * x, 10 * y, t, 0)) / 200);
}
void		desenha_mar (double t)
{
	//glPushMatrix();
 
  const float delta = 2. / RESOLUTION;
  const unsigned int length = 2 * (RESOLUTION + 1);
  const float xn = (RESOLUTION + 1) * delta + 1;
  unsigned int i;
  unsigned int j;
  float x;
  float y;
  unsigned int indice;
  unsigned int preindice;

  /* Yes, I know, this is quite ugly... */
  float v1x;
  float v1y;
  float v1z;

  float v2x;
  float v2y;
  float v2z;

  float v3x;
  float v3y;
  float v3z;

  float vax;
  float vay;
  float vaz;

  float vbx;
  float vby;
  float vbz;

  float nx;
  float ny;
  float nz;

  float l;


 


  /* Vertices */
  for (j = 0; j < RESOLUTION; j++)
    {
      y = (j + 1) * delta - 1;
      for (i = 0; i <= RESOLUTION; i++)
	{
	  indice = 6 * (i + j * (RESOLUTION + 1));

	  x = i * delta - 1;
	  surface[indice + 3] = x;
	  surface[indice + 4] = z (x, y, t);
	  surface[indice + 5] = y;
	  if (j != 0)
	    {
	      /* Values were computed during the previous loop */
	      preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
	      surface[indice] = surface[preindice + 3];
	      surface[indice + 1] = surface[preindice + 4];
	      surface[indice + 2] = surface[preindice + 5];
	    }
	  else
	    {
	      surface[indice] = x;
	      surface[indice + 1] = z (x, -1, t);
	      surface[indice + 2] = -1;
	    }
	}
    }

  /* Normals */
  for (j = 0; j < RESOLUTION; j++)
    for (i = 0; i <= RESOLUTION; i++)
      {
	indice = 6 * (i + j * (RESOLUTION + 1));

	v1x = surface[indice + 3];
	v1y = surface[indice + 4];
	v1z = surface[indice + 5];

	v2x = v1x;
	v2y = surface[indice + 1];
	v2z = surface[indice + 2];

	if (i < RESOLUTION)
	  {
	    v3x = surface[indice + 9];
	    v3y = surface[indice + 10];
	    v3z = v1z;
	  }
	else
	  {
	    v3x = xn;
	    v3y = z (xn, v1z, t);
	    v3z = v1z;
	  }

	vax =  v2x - v1x;
	vay =  v2y - v1y;
	vaz =  v2z - v1z;

	vbx = v3x - v1x;
	vby = v3y - v1y;
	vbz = v3z - v1z;

	nx = (vby * vaz) - (vbz * vay);
	ny = (vbz * vax) - (vbx * vaz);
	nz = (vbx * vay) - (vby * vax);

	l = sqrtf (nx * nx + ny * ny + nz * nz);
	if (l != 0)
	  {
	    l = 1 / l;
	    normal[indice + 3] = nx * l;
	    normal[indice + 4] = ny * l;
	    normal[indice + 5] = nz * l;
	  }
	else
	  {
	    normal[indice + 3] = 0;
	    normal[indice + 4] = 1;
	    normal[indice + 5] = 0;
	  }


	if (j != 0)
	  {
	    /* Values were computed during the previous loop */
	    preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
	    normal[indice] = normal[preindice + 3];
	    normal[indice + 1] = normal[preindice + 4];
	    normal[indice + 2] = normal[preindice + 5];
	  }
	else
	  {
/* 	    v1x = v1x; */
	    v1y = z (v1x, (j - 1) * delta - 1, t);
	    v1z = (j - 1) * delta - 1;

/* 	    v3x = v3x; */
	    v3y = z (v3x, v2z, t);
	    v3z = v2z;

	    vax = v1x - v2x;
	    vay = v1y - v2y;
	    vaz = v1z - v2z;

	    vbx = v3x - v2x;
	    vby = v3y - v2y;
	    vbz = v3z - v2z;

	    nx = (vby * vaz) - (vbz * vay);
	    ny = (vbz * vax) - (vbx * vaz);
	    nz = (vbx * vay) - (vby * vax);

	    l = sqrtf (nx * nx + ny * ny + nz * nz);
	    if (l != 0)
	      {
		l = 1 / l;
		normal[indice] = nx * l;
		normal[indice + 1] = ny * l;
		normal[indice + 2] = nz * l;
	      }
	    else
	      {
		normal[indice] = 0;
		normal[indice + 1] = 1;
		normal[indice + 2] = 0;
	      }
	  }
      }



  /* The water */

  glColor3f (1, 1, 1);
  glEnableClientState (GL_NORMAL_ARRAY);
  glEnableClientState (GL_VERTEX_ARRAY);
  glNormalPointer (GL_FLOAT, 0, normal);
  glVertexPointer (3, GL_FLOAT, 0, surface);
  for (i = 0; i < RESOLUTION; i++)
    glDrawArrays (GL_TRIANGLE_STRIP, i * length, length);
//glPopMatrix();
}


void desenha_baiacu(double escala, double tamanho, double nivel, int n){
	glScalef(escala, escala, escala);
	glPushMatrix();
		glScalef(tamanho, tamanho, 1);
		glColor3ub(200,200,0);
		glutSolidSphere(100, 20, 20);
		glColor3ub(200,0,0);
		float i, j;
		for(i=0;i<n;i++){
			for(j=0;j<n;j++){
				if(i>1||j>1){
					glPushMatrix();
					glRotatef(90-180*i/n, 1,0, 0);
					glRotatef(360*j/n, 0,0, 1);
					//glTranslatef();
					//glRotatef(-90, 1,0, 0);
					//glTranslatef(0,-60,0);
					desenha_cone(110,10,30);	
					glPopMatrix();
				}
			}
		
		}
		M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);
	 {
        M3DVector3f vPoints[3] = {{ 0, 0, -100.0f  },
                                  { -50, 50*sin(nivel+PI/4), -140.0f },
                                  { 50, 50*sin(nivel+PI/4), -140.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 0, 0, -100.0f  },
                                  { 50, 50*sin(nivel+PI/4), -140.0f },
                                  { -50, 50*sin(nivel+PI/4), -140.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        glEnd();
		glTranslatef(0,0,100);
		glScalef(3, 1, 0.1);
		glutSolidTorus(3,6, 20, 20);
	glPopMatrix();	
}
void desenha_peixe(int corR, int corG, int corB, double tam_max, int qual, double nivel)//nivel vai de 0 a 360
{

glColor3ub(255,255,255);
glScalef(tam_max/60,tam_max/60,tam_max/60);
  glPushMatrix();
        glTranslatef(3,3, 45);
	glutSolidSphere(3, 10,10);	
	glPopMatrix();
	
	  glPushMatrix();
        glTranslatef(-3,3, 45);
	glutSolidSphere(3, 10,10);	
	glPopMatrix();
	glPushMatrix();
        glTranslatef(0,3, 47);
        glColor3ub(0,0,0);

	glScalef(1, 0.2, 0.2);
	glutSolidSphere(6, 10,10);	
	glPopMatrix();
if(qual==1)
{
glPushMatrix();
	    if(luz1)glColor3ub(255,255,200);
	    else glColor3ub(0,0,0);
        glTranslatef(0,20, 100);
		glutSolidSphere(6, 10,10);
glPopMatrix();
}
if(qual==2)
{
glPushMatrix();
	    if(luz2)glColor3ub(255,255,200);
	    else glColor3ub(0,0,0);
        glTranslatef(0,20, 100);
		glutSolidSphere(6, 10,10);
glPopMatrix();
}

    glColor3ub(corR, corG, corB);
    
	M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);
        
                
	
        // Verticies for this panel
        {
        M3DVector3f vPoints[3] = {{ 15.0f, 0.0f,  30.0f},
                                        { 0.0f,  15.0f, 30.0f},
                                        { 0.0f,  0.0f,  60.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	


        {
        M3DVector3f vPoints[3] = {{ 0.0f, 0.0f, 60.0f },
                                  { 0.0f, 15.0f, 30.0f },
                                  { -15.0f, 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 0.0f, -15.0f,  30.0f},
                                        { 15.0f,  0.0f, 30.0f},
                                        { 0.0f,  0.0f,  60.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	


        {
        M3DVector3f vPoints[3] = {{ 0.0f, 0.0f, 60.0f },
                                  { -15.0f, 0.0f, 30.0f },
                                  { 0.0f, -15.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
 {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 0.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 0.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        // Body of the Fish////////////////////////
        {
        M3DVector3f vPoints[3] = {{ -15.0f, 0.0f, 30.0f },
                                  { 0.0f, 15.0f, 30.0f },
                                  { 10*cos(nivel), 0.0f, -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 0.0f, 15.0f, 30.0f },
                                  { 15.0f,0.0f,30.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
//glColor3ub(255,255,255);
    {
        M3DVector3f vPoints[3] = {{ 0.0f, -15.0f, 30.0f },
                                  { -15.0f, 0.0f, 30.0f },
                                  { 10*cos(nivel), 0.0f, -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15.0f, 0.0f, 30.0f },
                                  { 0.0f,-15.0f,30.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
      
    
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), 15.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), -15.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), -15.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), 15.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                
        glEnd();
}

void desenha_tubarao(double tam_max, double nivel)//nivel vai de 0 a 360
{


glScalef(tam_max/60,tam_max/60,tam_max/60);
  
  glPushMatrix();
        glTranslatef(0,8, 52);
        glColor3ub(0,0,0);

	glScalef(1, 0.1, 0.1);
	glutSolidSphere(11, 20,20);	
	glPopMatrix();
	    
    
	M3DVector3f vNormal;                      
	glColor3ub(80,80,120);
	glScalef(1, 1.3, 1);
	glBegin(GL_TRIANGLES);
        
                
	
        // cabeca cima
        {
        M3DVector3f vPoints[3] = {{ 15.0f, 0.0f,  30.0f},
                                        { 0.0f,  12.0f, 30.0f},
                                        { 8.0f,  8.0f,  60.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	
 
        {
        M3DVector3f vPoints[3] = {{ 8.0f, 8.0f,  60.0f},
                                        { 0.0f,  12.0f, 30.0f},
                                        { -8.0f,  8.0f,  60.0f}};

 
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	

        {
        M3DVector3f vPoints[3] = {{ -8.0f, 8.0f, 60.0f },
                                  { 0.0f, 12.0f, 30.0f },
                                  { -15.0f, 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        //cabeca baixo
        glColor3ub(255,255,255);
         {
        M3DVector3f vPoints[3] = {{ -8.0f, 8.0f, 60.0f },
                                  { -15.0f, 0.0f, 30.0f },
                                  { 0.0f, 0.0f, 40.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
 {
        M3DVector3f vPoints[3] = {{ 8.0f, 8.0f, 60.0f },
                                  { 0.0f, 0.0f, 40.0f },
                                  { 15.0f, 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
 
        {
        M3DVector3f vPoints[3] = {{ -8.0f, 8.0f,  60.0f},
                                        { 0.0f,  0.0f, 40.0f},
                                        { 8.0f,  8.0f,  60.0f}};

 
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	        
        //marca
        glColor3ub(120,20,20);
{
        M3DVector3f vPoints[3] = {{ 0.0f, -5.0f,  30.0f},
                                        { 15.0f,  0.0f, 30.0f},
                                        { 0.0f,  0.0f,  40.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	


        {
        M3DVector3f vPoints[3] = {{ 0.0f, 0.0f, 40.0f },
                                  { -15.0f, 0.0f, 30.0f },
                                  { 0.0f, -5.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
glColor3ub(255,255,255);
        // Body of the Plane ////////////////////////
    {
        M3DVector3f vPoints[3] = {{ 0.0f, -5.0f, 30.0f },
                                  { -15.0f, 0.0f, 30.0f },
                                  { 10*cos(nivel), 0.0f, -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15.0f, 0.0f, 30.0f },
                                  { 0.0f,-5.0f,30.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
      //corpo cima
glColor3ub(80,80,120);    
		//barbatana de cima
		{
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 30.0f },
                                  { 1*cos(nivel), 20.0f, 0.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { 1*cos(nivel), 20.0f, 0.0f },
                                  { 1*cos(nivel), 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 30.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 30.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 30.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ -15.0f, 0.0f, 30.0f },
                                  { 0.0f, 12.0f, 30.0f },
                                  { 10*cos(nivel), 0.0f, -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 0.0f, 12.0f, 30.0f },
                                  { 15.0f,0.0f,30.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }

    
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), 12.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), 0.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), 0.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), 12.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
glColor3ub(255,255,255);        
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), 0.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), -5.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 10*cos(nivel), 0.0f, -56.0f },
                                  { 15*sin(nivel+PI/4), -5.0f, -70.0f },
                                  { 15*sin(nivel+PI/4), 0.0f, -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }        
        glEnd();
}


void desenha_baleia(double tam_max, double nivel)//nivel vai de 0 a 360
{


glScalef(tam_max/60,tam_max/60,tam_max/60);
  
  glPushMatrix();
        glTranslatef(0,4, 45);
        glColor3ub(0,0,0);

	glScalef(1, 0.1, 0.1);
	glutSolidSphere(8, 20,20);	
	glPopMatrix();
	    
    
	M3DVector3f vNormal;                      
	glColor3ub(20,30,20);
	glScalef(1, 1.5, 1);
	glBegin(GL_TRIANGLES);
        
                
	
        // cabeca cima
        {
        M3DVector3f vPoints[3] = {{ 15.0f, 0.0f,  0.0f},
                                        { 0.0f,  10.0f, 0.0f},
                                        { 6.0f,  2.0f,  60.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	
 
        {
        M3DVector3f vPoints[3] = {{ 6.0f, 2.0f,  60.0f},
                                        { 0.0f,  10.0f, 0.0f},
                                        { -6.0f,  2.0f,  60.0f}};

 
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	

        {
        M3DVector3f vPoints[3] = {{ -6.0f, 2.0f, 60.0f },
                                  { 0.0f, 10.0f, 0.0f },
                                  { -15.0f, 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        //cabeca baixo
       glColor3ub(255,255,255);
        {
        M3DVector3f vPoints[3] = {{ -6.0f, 2.0f,  60.0f},
                                        { 0.0f,  0.0f, 50.0f},
                                        { 6.0f,  2.0f,  60.0f}};

 
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	
        
         {
        M3DVector3f vPoints[3] = {{ -6.0f, 2.0f, 60.0f },
                                  { -15.0f, 0.0f, 0.0f },
                                  { 0.0f, 0.0f, 50.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
 {
        M3DVector3f vPoints[3] = {{ 6.0f, 2.0f, 60.0f },
                                  { 0.0f, 0.0f, 50.0f },
                                  { 15.0f, 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
 
                
        //marca
      
{
        M3DVector3f vPoints[3] = {{ 0.0f, -3.0f,  0.0f},
                                        { 15.0f,  0.0f, 0.0f},
                                        { 0.0f,  0.0f,  50.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	


        {
        M3DVector3f vPoints[3] = {{ 0.0f, 0.0f, 50.0f },
                                  { -15.0f, 0.0f, 0.0f },
                                  { 0.0f, -3.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }

        // corpo baixo
    {
        M3DVector3f vPoints[3] = {{ 0.0f, -3.0f, 0.0f },
                                  { -15.0f, 0.0f, 0.0f },
                                  { 0, 3+5*cos(nivel), -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 0, 3+5*cos(nivel), -56.0f  },
                                  { 15.0f, 0.0f, 0.0f },
                                  { 0.0f,-3.0f,0.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
      //corpo cima
      glColor3ub(20,30,20);
        {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 0.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { 20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 1*cos(nivel), 0.0f, 0.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 5*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
{
        M3DVector3f vPoints[3] = {{ 5*cos(nivel), 0.0f, 0.0f },
                                  { -20, -3+5*cos(nivel), -10.0f },
                                  { 1*cos(nivel), 0.0f, 0.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ -15.0f, 0.0f, 0.0f },
                                  { 0.0f, 10.0f, 0.0f },
                                  { 0, 3+5*cos(nivel), -56.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
                	
        {
        M3DVector3f vPoints[3] = {{ 0, 3+5*cos(nivel), -56.0f  },
                                  { 0.0f, 10.0f, 0.0f },
                                  { 15.0f,0.0f,0.0f }};
	
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }

    
        {
        M3DVector3f vPoints[3] = {{ 0, 3+5*cos(nivel), -56.0f  },
                                  { -15, 10*sin(nivel+PI/4), -70.0f },
                                  { 15, 10*sin(nivel+PI/4), -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
        
        {
        M3DVector3f vPoints[3] = {{ 0, 3+5*cos(nivel), -56.0f },
                                  { 15, 10*sin(nivel+PI/4), -70.0f  },
                                  { -15, 10*sin(nivel+PI/4), -70.0f }};

        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
        glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }
      
         
        glEnd();
}



void desenha_barco(int corR, int corG, int corB, double escala,double nivel)//nivel vai de 0 a 360
{

glColor3ub(corR, corG, corB);
glScalef(escala, escala, escala);

glutSolidSphere(15, 20, 20);


glColor3ub(255,255,255);
	M3DVector3f vNormal;
	glBegin(GL_TRIANGLES);
                        
	
        // Verticies for this panel
        {
        M3DVector3f vPoints[3] = {{ 0.0f, 75.0f,  0.0f},
                                        { 0.0f,  15.0f, 30.0f},
                                        { 0.0f,  15.0f,  0.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	
 // Verticies for this panel
        {
        M3DVector3f vPoints[3] = {{ 0.0f, 75.0f,  0.0f},
                                        { 0.0f,  15.0f, 0.0f},
                                        { 0.0f,  15.0f,  30.0f}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, vPoints[0], vPoints[1], vPoints[2]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[0]);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[2]);
        }	
     
        glEnd();
}
void desenha_chao(double level)
    {
    GLfloat fExtent = PROF_MAX*1.5;
    GLfloat fStep = 50.0f;
    GLfloat y = 0.0f;
    GLfloat fColor;
    GLfloat iStrip, iRun;
    GLint iBounce = 0;
    
    glShadeModel(GL_FLAT);
    for(iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
        {
        glBegin(GL_TRIANGLE_STRIP);
            for(iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
                {
                if((iBounce %2) == 0)
                    fColor = 0.85f;
                else
                    fColor = 0.8f;
                   // if()
                glColor4f(0, 0, fColor, 0.5f);
                glVertex3f(iStrip, y, iRun);
                glVertex3f(iStrip + fStep, y, iRun);
                
                iBounce++;
                }
        glEnd();
        }
        
        for(iStrip = -fExtent; iStrip <= fExtent; iStrip += fStep)
        {
        glBegin(GL_TRIANGLE_STRIP);
            for(iRun = fExtent; iRun >= -fExtent; iRun -= fStep)
                {
                
                if((iBounce %2) == 0)
                    fColor = 0.85f;
                else
                    fColor = 0.8f;
                        
                glColor4f(0,0, fColor, 0.5f);
                glVertex3f(iStrip+fStep, y, iRun);
                glVertex3f(iStrip , y, iRun);
                
                iBounce++;
                }
        glEnd();
        }
    glShadeModel(GL_SMOOTH);
    }

void desenha_cone(double h, double raio, int n){
	
	M3DVector3f vNormal;
	M3DVector3f topo={0,h,0};
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(topo);
		float i;
		for (i=0;i<n;i++)
	   {
        M3DVector3f vPoints[2] = {{raio*cos(2*PI*i/n),  0.0f, raio*sin(2*PI*i/n)},
                                        { raio*cos(2*PI*(i+1)/n),  0.0f, raio*sin(2*PI*(1+i)/n)}};

        // Calculate the normal for the plane
        m3dFindNormal(vNormal, topo, vPoints[0], vPoints[1]);
		glNormal3fv(vNormal);
		glVertex3fv(vPoints[1]);
		glVertex3fv(vPoints[0]);
		
        }	
        glEnd();
	
	}
void preenche(double xeye, double yeye, double zeye, double x, double y, double z, double cimax, double cimay, double cimaz){
	
	(lookAtParam[0])=xeye;
	(lookAtParam[1])=yeye;
	(lookAtParam[2])=zeye;
	(lookAtParam[3])=x;
	(lookAtParam[4])=y;
	(lookAtParam[5])=z;
	(lookAtParam[6])=cimax;
	(lookAtParam[7])=cimay;
	(lookAtParam[8])=cimaz;	
	
	}



static void fghCircleTable(double **sint,double **cost,const int n)
{
    int i;

    /* Table size, the sign of n flips the circle direction */

    const int size = abs(n);

    /* Determine the angle between samples */

    const double angle = 2*M_PI/(double)( ( n == 0 ) ? 1 : n );

    /* Allocate memory for n samples, plus duplicate of first entry at the end */

    *sint = (double *) calloc(sizeof(double), size+1);
    *cost = (double *) calloc(sizeof(double), size+1);

    /* Bail out if memory allocation fails, fgError never returns */

    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        printf("Failed to allocate memory in fghCircleTable");
    }

    /* Compute cos and sin around the circle */

    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = sin(angle*i);
        (*cost)[i] = cos(angle*i);
    }

    /* Last sample is duplicate of the first */

    (*sint)[size] = (*sint)[0];
    (*cost)[size] = (*cost)[0];
}


void FGAPIENTRY calota(GLdouble radius,GLdouble angulo ,GLint slices, GLint stacks)
{
	if (angulo>180) angulo=180;
	if (angulo<0) angulo=0;
    int i,j;

    /* Adjust z and radius as stacks are drawn. */

    double z0,z1;
    double r0,r1;

    /* Pre-computed circle */

    double *sint1,*cost1;
    double *sint2,*cost2;

    fghCircleTable(&sint1,&cost1,-slices);
    fghCircleTable(&sint2,&cost2,stacks*2);

    /* The top stack is covered with a triangle fan */

    z0 = 1.0;
    z1 = cost2[(stacks>0)?1:0];
    r0 = 0.0;
    r1 = sint2[(stacks>0)?1:0];
	if(angulo>0)
	{
		glBegin(GL_TRIANGLE_FAN);

			glNormal3d(0,0,1);
			glVertex3d(0,0,radius);

			for (j=slices; j>=0; j--)
			{
				glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
				glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
			}

		glEnd();
	}
    /* Cover each stack with a quad strip, except the top and bottom stacks */

    for( i=1; i<stacks*angulo/180; i++ )
    {
        z0 = z1; z1 = cost2[i+1];
        r0 = r1; r1 = sint2[i+1];

        glBegin(GL_QUAD_STRIP);

            for(j=0; j<=slices; j++)
            {
                glNormal3d(cost1[j]*r1,        sint1[j]*r1,        z1       );
                glVertex3d(cost1[j]*r1*radius, sint1[j]*r1*radius, z1*radius);
                glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
                glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
            }

        glEnd();
    }
    z0 = z1;
    r0 = r1;
	if(angulo>=180){
		glBegin(GL_TRIANGLE_FAN);

			glNormal3d(0,0,-1);
			glVertex3d(0,0,-radius);

			for (j=0; j<=slices; j++)
			{
				glNormal3d(cost1[j]*r0,        sint1[j]*r0,        z0       );
				glVertex3d(cost1[j]*r0*radius, sint1[j]*r0*radius, z0*radius);
			}

		glEnd();
	}
    /* Release sin and cos tables */

    free(sint1);
    free(cost1);
    free(sint2);
    free(cost2);
}
double gaussiano(){
  
  
double GaussNum = 0.0;
int NumInSum = 10;
for(int i = 0; i < NumInSum; i++)
{
GaussNum += ((double)rand()/(double)RAND_MAX - 0.5);
}
GaussNum = GaussNum*sqrt((double)12/(double)NumInSum);
return GaussNum;}


/*
** Function to load a Jpeg file.
*/
int		load_texture (const char * filename,
			      unsigned char * dest,
			      const int format,
			      const unsigned int size)
{
  FILE *fd;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  unsigned char * line;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);

  if (0 == (fd = fopen(filename, "rb")))
    return 1;

  jpeg_stdio_src (&cinfo, fd);
  jpeg_read_header (&cinfo, TRUE);
  if ((cinfo.image_width != size) || (cinfo.image_height != size))
    return 1;

  if (GL_RGB == format)
    {
      if (cinfo.out_color_space == JCS_GRAYSCALE)
	return 1;
    }
  else
    if (cinfo.out_color_space != JCS_GRAYSCALE)
      return 1;

  jpeg_start_decompress (&cinfo);

  while (cinfo.output_scanline < cinfo.output_height)
    {
      line = dest +
	(GL_RGB == format ? 3 * size : size) * cinfo.output_scanline;
      jpeg_read_scanlines (&cinfo, &line, 1);
    }
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  return 0;
}

#include <stdlib.h>

#define		MOD	0xff

static int		permut[256];
static const char	gradient[32][4] =
  {
    { 1,  1,  1,  0}, { 1,  1,  0,  1}, { 1,  0,  1,  1}, { 0,  1,  1,  1},
    { 1,  1, -1,  0}, { 1,  1,  0, -1}, { 1,  0,  1, -1}, { 0,  1,  1, -1},
    { 1, -1,  1,  0}, { 1, -1,  0,  1}, { 1,  0, -1,  1}, { 0,  1, -1,  1},
    { 1, -1, -1,  0}, { 1, -1,  0, -1}, { 1,  0, -1, -1}, { 0,  1, -1, -1},
    {-1,  1,  1,  0}, {-1,  1,  0,  1}, {-1,  0,  1,  1}, { 0, -1,  1,  1},
    {-1,  1, -1,  0}, {-1,  1,  0, -1}, {-1,  0,  1, -1}, { 0, -1,  1, -1},
    {-1, -1,  1,  0}, {-1, -1,  0,  1}, {-1,  0, -1,  1}, { 0, -1, -1,  1},
    {-1, -1, -1,  0}, {-1, -1,  0, -1}, {-1,  0, -1, -1}, { 0, -1, -1, -1},
};


void		InitNoise (void)
{
  unsigned int i = 0;
  while (i < 256)
    permut[i++] = rand () & MOD;
}

/*
** Function finding out the gradient corresponding to the coordinates
*/
static int	Indice (const int i,
			const int j,
			const int k,
			const int l)
{
  return (permut[(l + permut[(k + permut[(j + permut[i & MOD])
					 & MOD])
			     & MOD])
		 & MOD]
	  & 0x1f);
}

/*
** Functions computing the dot product of the vector and the gradient
*/
static float	Prod (const float a, const char b)
{
  if (b > 0)
    return a;
  if (b < 0)
    return -a;
  return 0;
}

static float	Dot_prod (const float x1, const char x2,
			  const float y1, const char y2,
			  const float z1, const char z2,
			  const float t1, const char t2)
{
  return (Prod (x1, x2) + Prod (y1, y2) + Prod (z1, z2) + Prod (t1, t2));
}

/*
** Functions computing interpolations
*/
static float	Spline5 (const float state)
{
  /*
  ** Enhanced spline :
  ** (3x^2 + 2x^3) is not as good as (6x^5 - 15x^4 + 10x^3)
  */
  const float sqr = state * state;
  return state * sqr * (6 * sqr - 15 * state + 10);
}

static float	Linear (const float start,
			const float end,
			const float state)
{
  return start + (end - start) * state;
}

/*
** Noise function, returning the Perlin Noise at a given point
*/
float		Noise (const float x,
		       const float y,
		       const float z,
		       const float t)
{
  /* The unit hypercube containing the point */
  const int x1 = (int) (x > 0 ? x : x - 1);
  const int y1 = (int) (y > 0 ? y : y - 1);
  const int z1 = (int) (z > 0 ? z : z - 1);
  const int t1 = (int) (t > 0 ? t : t - 1);
  const int x2 = x1 + 1;
  const int y2 = y1 + 1;
  const int z2 = z1 + 1;
  const int t2 = t1 + 1;

  /* The 16 corresponding gradients */
  const char * g0000 = gradient[Indice (x1, y1, z1, t1)];
  const char * g0001 = gradient[Indice (x1, y1, z1, t2)];
  const char * g0010 = gradient[Indice (x1, y1, z2, t1)];
  const char * g0011 = gradient[Indice (x1, y1, z2, t2)];
  const char * g0100 = gradient[Indice (x1, y2, z1, t1)];
  const char * g0101 = gradient[Indice (x1, y2, z1, t2)];
  const char * g0110 = gradient[Indice (x1, y2, z2, t1)];
  const char * g0111 = gradient[Indice (x1, y2, z2, t2)];
  const char * g1000 = gradient[Indice (x2, y1, z1, t1)];
  const char * g1001 = gradient[Indice (x2, y1, z1, t2)];
  const char * g1010 = gradient[Indice (x2, y1, z2, t1)];
  const char * g1011 = gradient[Indice (x2, y1, z2, t2)];
  const char * g1100 = gradient[Indice (x2, y2, z1, t1)];
  const char * g1101 = gradient[Indice (x2, y2, z1, t2)];
  const char * g1110 = gradient[Indice (x2, y2, z2, t1)];
  const char * g1111 = gradient[Indice (x2, y2, z2, t2)];

  /* The 16 vectors */
  const float dx1 = x - x1;
  const float dx2 = x - x2;
  const float dy1 = y - y1;
  const float dy2 = y - y2;
  const float dz1 = z - z1;
  const float dz2 = z - z2;
  const float dt1 = t - t1;
  const float dt2 = t - t2;

  /* The 16 dot products */
  const float b0000 = Dot_prod(dx1, g0000[0], dy1, g0000[1],
			       dz1, g0000[2], dt1, g0000[3]);
  const float b0001 = Dot_prod(dx1, g0001[0], dy1, g0001[1],
			       dz1, g0001[2], dt2, g0001[3]);
  const float b0010 = Dot_prod(dx1, g0010[0], dy1, g0010[1],
			       dz2, g0010[2], dt1, g0010[3]);
  const float b0011 = Dot_prod(dx1, g0011[0], dy1, g0011[1],
			       dz2, g0011[2], dt2, g0011[3]);
  const float b0100 = Dot_prod(dx1, g0100[0], dy2, g0100[1],
			       dz1, g0100[2], dt1, g0100[3]);
  const float b0101 = Dot_prod(dx1, g0101[0], dy2, g0101[1],
			       dz1, g0101[2], dt2, g0101[3]);
  const float b0110 = Dot_prod(dx1, g0110[0], dy2, g0110[1],
			       dz2, g0110[2], dt1, g0110[3]);
  const float b0111 = Dot_prod(dx1, g0111[0], dy2, g0111[1],
			       dz2, g0111[2], dt2, g0111[3]);
  const float b1000 = Dot_prod(dx2, g1000[0], dy1, g1000[1],
			       dz1, g1000[2], dt1, g1000[3]);
  const float b1001 = Dot_prod(dx2, g1001[0], dy1, g1001[1],
			       dz1, g1001[2], dt2, g1001[3]);
  const float b1010 = Dot_prod(dx2, g1010[0], dy1, g1010[1],
			       dz2, g1010[2], dt1, g1010[3]);
  const float b1011 = Dot_prod(dx2, g1011[0], dy1, g1011[1],
			       dz2, g1011[2], dt2, g1011[3]);
  const float b1100 = Dot_prod(dx2, g1100[0], dy2, g1100[1],
			       dz1, g1100[2], dt1, g1100[3]);
  const float b1101 = Dot_prod(dx2, g1101[0], dy2, g1101[1],
			       dz1, g1101[2], dt2, g1101[3]);
  const float b1110 = Dot_prod(dx2, g1110[0], dy2, g1110[1],
			       dz2, g1110[2], dt1, g1110[3]);
  const float b1111 = Dot_prod(dx2, g1111[0], dy2, g1111[1],
			       dz2, g1111[2], dt2, g1111[3]);

  /* Then the interpolations, down to the result */
  const float idx1 = Spline5 (dx1);
  const float idy1 = Spline5 (dy1);
  const float idz1 = Spline5 (dz1);
  const float idt1 = Spline5 (dt1);

  const float b111 = Linear (b1110, b1111, idt1);
  const float b110 = Linear (b1100, b1101, idt1);
  const float b101 = Linear (b1010, b1011, idt1);
  const float b100 = Linear (b1000, b1001, idt1);
  const float b011 = Linear (b0110, b0111, idt1);
  const float b010 = Linear (b0100, b0101, idt1);
  const float b001 = Linear (b0010, b0011, idt1);
  const float b000 = Linear (b0000, b0001, idt1);

  const float b11 = Linear (b110, b111, idz1);
  const float b10 = Linear (b100, b101, idz1);
  const float b01 = Linear (b010, b011, idz1);
  const float b00 = Linear (b000, b001, idz1);

  const float b1 = Linear (b10, b11, idy1);
  const float b0 = Linear (b00, b01, idy1);

  return Linear (b0, b1, idx1);
}
void instrucoes(){
printf("\n\t\t --- INSTRUCOES --- \n\nControle:\nComeça com camera no tubarão, a distancia de 300, só ele pode ser controlado.\nBotões:\nSetas: controla o primeiro tubarão, ao soltar por certo tempo, o controle dele fica automático.\nHome: muda de peixe\nPage Up/Page Down: aproxima ou afasta a camera\nEnd: poe a camera na posição do meio do mar\nIns: Imprime a os resultados parciais\nF12: imprime essas instruções\n\nParametros de visualização:\nF1: liga/desliga a neblina\nF2: liga/desliga efeito da água\nF3: liga/desliga luz do primeiro peixe luz\nF4: liga/desliga luz do segundo leixe luz\n\nF13: EXPLODE TUDO, hauhuahuauha\n\n");

}
void m3dFindNormal(M3DVector3f result, const M3DVector3f point1, const M3DVector3f point2, 
							const M3DVector3f point3)
	{
	M3DVector3f v1,v2;		// Temporary vectors

	// Calculate two vectors from the three points. Assumes counter clockwise
	// winding!
	v1[0] = point1[0] - point2[0];
	v1[1] = point1[1] - point2[1];
	v1[2] = point1[2] - point2[2];

	v2[0] = point2[0] - point3[0];
	v2[1] = point2[1] - point3[1];
	v2[2] = point2[2] - point3[2];

	// Take the cross product of the two vectors to get
	// the normal vector.
	m3dCrossProduct(result, v1, v2);
	}

void m3dLoadIdentity33(M3DMatrix33f m)
	{
	// Don't be fooled, this is still column major
	static M3DMatrix33f	identity = { 1.0f, 0.0f, 0.0f ,
									 0.0f, 1.0f, 0.0f,
									 0.0f, 0.0f, 1.0f };

	memcpy(m, identity, sizeof(M3DMatrix33f));
	}

__inline void m3dRotateVector(M3DVector3f vOut, const M3DVector3f p, const M3DMatrix33f m)
	{
    vOut[0] = m[0] * p[0] + m[3] * p[1] + m[6] * p[2];	
    vOut[1] = m[1] * p[0] + m[4] * p[1] + m[7] * p[2];	
    vOut[2] = m[2] * p[0] + m[5] * p[1] + m[8] * p[2];	
	}


#define M33(row,col)  m[col*3+row]

///////////////////////////////////////////////////////////////////////////////
// Creates a 3x3 rotation matrix, takes radians NOT degrees
void m3dRotationMatrix33(M3DMatrix33f m, float angle, float x, float y, float z)
	{
	
	float mag, s, c;
	float xx, yy, zz, xy, yz, zx, xs, ys, zs, one_c;

	s = float(sin(angle));
	c = float(cos(angle));

	mag = float(sqrt( x*x + y*y + z*z ));

	// Identity matrix
	if (mag == 0.0f) {
		m3dLoadIdentity33(m);
		return;
	}

	// Rotation matrix is normalized
	x /= mag;
	y /= mag;
	z /= mag;



	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * s;
	ys = y * s;
	zs = z * s;
	one_c = 1.0f - c;

	M33(0,0) = (one_c * xx) + c;
	M33(0,1) = (one_c * xy) - zs;
	M33(0,2) = (one_c * zx) + ys;

	M33(1,0) = (one_c * xy) + zs;
	M33(1,1) = (one_c * yy) + c;
	M33(1,2) = (one_c * yz) - xs;

	M33(2,0) = (one_c * zx) - ys;
	M33(2,1) = (one_c * yz) + xs;
	M33(2,2) = (one_c * zz) + c;
	}

#undef M33

inline void m3dCrossProduct(M3DVector3f result, const M3DVector3f u, const M3DVector3f v)
	{
	result[0] = u[1]*v[2] - v[1]*u[2];
	result[1] = -u[0]*v[2] + v[0]*u[2];
	result[2] = u[0]*v[1] - v[0]*u[1];
	}
