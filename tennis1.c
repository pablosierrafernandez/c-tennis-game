/*****************************************************************************/
/*									     */
/*				     Tennis1.c				     */
/*									     */
/*  Programa modificat d'exemple per a les practiques 2 i 3 de ISO.	     */
/*     Es tracta del joc del tennis: es dibuixa un camp de joc rectangular    */
/*     amb una porteria a cada costat, una paleta per l'usuari, una paleta   */
/*     per l'ordinador i una pilota que va rebotant per tot arreu; l'usuari  */
/*     disposa de dues tecles per controlar la seva paleta, mentre que l'or- */
/*     dinador mou la seva automaticament (amunt i avall). Evidentment, es   */
/*     tracta d'intentar col.locar la pilota a la porteria de l'ordinador    */
/*     (porteria de la dreta), abans que l'ordinador aconseguixi col.locar   */
/*     la pilota dins la porteria de l'usuari (porteria de l'esquerra).      */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil n_col m_por l_pal					     */
/*		pil_pf pil_pc pil_vf pil_vc				     */
/*		ipo_pf1 ipo_pc1 po_vf1					     */
/*		ipo_pf2 ipo_pc2 po_vf2					     */
/*		...							     */
/*		ipo_pfn ipo_pcn po_vfn					     */
/*									     */
/*     on 'n_fil', 'n_col' son les dimensions del taulell de joc, 'm_por'    */
/*     es la mida de les dues porteries, 'l_pal' es la longitud de les dues  */
/*     paletas; 'pil_pf', 'pil_pc' es la posicio inicial (fila,columna) de   */
/*     la pilota, mentre que 'pil_vf', 'pil_vc' es la velocitat inicial;     */
/*     finalment, 'ipo_pfn', 'ipo_pcn' indicara la posicio del primer 	     */
/*     caracter de la paleta de l'ordinador, mentre que la seva velocitat    */
/*     vertical ve determinada pel parametre 'po_fvn'.			     */
/*									     */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment de la pilota i la paleta de l'ordinador (en ms);   */
/*     el valor d'aquest parametre per defecte es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides en 'winsuport.o', les       */
/*     quals proporcionen una interficie senzilla per a crear una finestra   */
/*     de text on es poden imprimir caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses', tal i com ens permet Makefile     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  fitxer no accessible					     */
/*	3  ==>  dimensions del taulell incorrectes			     */
/*	4  ==>  parametres de la pilota incorrectes			     */
/*	5  ==>  parametres d'alguna de les paletas incorrectes		     */
/*	6  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*****************************************************************************/



#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include "winsuport.h"		/* incloure definicions de funcions propies */
#include <pthread.h>		/* incloure threads */
#include <stdint.h>
#include <time.h>

#define MIN_FIL 7		/* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
#define MAX_PALETAS 9
#define MAX_THREADS MAX_PALETAS+2

/* definir struct de una paleta */

typedef struct{
 int ipo_pf, ipo_pc;      	/* posicio del la paleta de l'ordinador */
 float po_pf;			/* pos. vertical de la paleta de l'ordinador, en valor real */
 float v_pal;			/* velocitat de la paleta del programa */
} Paleta_Or;

/* variables globals */
int n_fil, n_col, m_por;	/* dimensions del taulell i porteries */
int l_pal;			/* longitud de les paletas */

int ipu_pf, ipu_pc;      	/* posicio del la paleta d'usuari */


int ipil_pf, ipil_pc;		/* posicio de la pilota, en valor enter */
float pil_pf, pil_pc;		/* posicio de la pilota, en valor real */
float pil_vf, pil_vc;		/* velocitat de la pilota, en valor real*/

int retard;			/* valor del retard de moviment, en mil.lisegons */

Paleta_Or paletas_or [MAX_PALETAS];
pthread_t tid[MAX_THREADS];
int tec=0;
int cont=-1;
int numPaletas=0;



/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins un fitxer de text, el nom del qual es passa per referencia en   */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris del principi del programa).		*/
void carrega_parametres(const char *nom_fit){
  FILE *fit;

  fit = fopen(nom_fit,"rt");		/* intenta obrir fitxer */
  if (fit == NULL){
    fprintf(stderr,"No s'ha pogut obrir el fitxer \'%s\'\n",nom_fit);
  	exit(2);
  }

  if (!feof(fit))
    fscanf(fit,"%d %d %d %d\n",&n_fil,&n_col,&m_por,&l_pal);
   if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) || (n_col < MIN_COL) || (n_col > MAX_COL) || (m_por < 0) || (m_por > n_fil-3) || (l_pal < MIN_PAL) || (l_pal > n_fil-3)){
  	fprintf(stderr,"Error: dimensions del camp de joc incorrectes:\n");
  	fprintf(stderr,"\t%d =< n_fil (%d) =< %d\n",MIN_FIL,n_fil,MAX_FIL);
  	fprintf(stderr,"\t%d =< n_col (%d) =< %d\n",MIN_COL,n_col,MAX_COL);
  	fprintf(stderr,"\t0 =< m_por (%d) =< n_fil-3 (%d)\n",m_por,(n_fil-3));
  	fprintf(stderr,"\t%d =< l_pal (%d) =< n_fil-3 (%d)\n",MIN_PAL,l_pal,(n_fil-3));
  	fclose(fit);
  	exit(3);
  }

  if (!feof(fit))
    fscanf(fit,"%d %d %f %f\n",&ipil_pf,&ipil_pc,&pil_vf,&pil_vc);
  if ((ipil_pf < 1) || (ipil_pf > n_fil-3) || (ipil_pc < 1) || (ipil_pc > n_col-2) || (pil_vf < MIN_VEL) || (pil_vf > MAX_VEL) || (pil_vc < MIN_VEL) || (pil_vc > MAX_VEL)){
  	fprintf(stderr,"Error: parametre pilota incorrectes:\n");
  	fprintf(stderr,"\t1 =< ipil_pf (%d) =< n_fil-3 (%d)\n",ipil_pf,(n_fil-3));
  	fprintf(stderr,"\t1 =< ipil_pc (%d) =< n_col-2 (%d)\n",ipil_pc,(n_col-2));
  	fprintf(stderr,"\t%.1f =< pil_vf (%.1f) =< %.1f\n",MIN_VEL,pil_vf,MAX_VEL);
  	fprintf(stderr,"\t%.1f =< pil_vc (%.1f) =< %.1f\n",MIN_VEL,pil_vc,MAX_VEL);
  	fclose(fit);
  	exit(4);
  }

  while(!feof(fit) && (numPaletas<MAX_PALETAS)){
    fscanf(fit,"%d %d %f\n", &paletas_or[numPaletas].ipo_pf, &paletas_or[numPaletas].ipo_pc, &paletas_or[numPaletas].v_pal);

    if ((paletas_or[numPaletas].ipo_pf < 1) || (paletas_or[numPaletas].ipo_pf+l_pal > n_fil-2) || (paletas_or[numPaletas].ipo_pc < 5) || (paletas_or[numPaletas].ipo_pc > n_col-2) || (paletas_or[numPaletas].v_pal < MIN_VEL) || (paletas_or[numPaletas].v_pal > MAX_VEL)){
    	fprintf(stderr,"Error: parametres paleta ordinador incorrectes:\n");
    	fprintf(stderr,"\t1 =< ipo_pf (%d) =< n_fil-l_pal-3 (%d)\n",paletas_or[numPaletas].ipo_pf,(n_fil-l_pal-3));
    	fprintf(stderr,"\t5 =< ipo_pc (%d) =< n_col-2 (%d)\n",paletas_or[numPaletas].ipo_pc,(n_col-2));
    	fprintf(stderr,"\t%.1f =< v_pal (%.1f) =< %.1f\n",MIN_VEL,paletas_or[numPaletas].v_pal,MAX_VEL);

      fclose(fit);
      exit(5);
    }else
	numPaletas++;
  }

  fclose(fit);			/* fitxer carregat: tot OK! */
}




/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
int inicialitza_joc(void)
{
  int i, j, i_port, f_port, retwin;
  char strin[51];

  retwin = win_ini(&n_fil,&n_col,'+',INVERS);   /* intenta crear taulell */

  if (retwin < 0)       /* si no pot crear l'entorn de joc amb les curses */
  { fprintf(stderr,"Error en la creacio del taulell de joc:\t");
    switch (retwin)
    {   case -1: fprintf(stderr,"camp de joc ja creat!\n");
                 break;
        case -2: fprintf(stderr,"no s'ha pogut inicialitzar l'entorn de curses!\n");
 		 break;
        case -3: fprintf(stderr,"les mides del camp demanades son massa grans!\n");
                 break;
        case -4: fprintf(stderr,"no s'ha pogut crear la finestra!\n");
                 break;
     }
     return(retwin);
  }

  i_port = n_fil/2 - m_por/2;	    /* crea els forats de la porteria */
  if (n_fil%2 == 0) i_port--;
  if (i_port == 0) i_port=1;
  f_port = i_port + m_por -1;
  for (i = i_port; i <= f_port; i++)
  {	win_escricar(i,0,' ',NO_INV);
	win_escricar(i,n_col-1,' ',NO_INV);
  }


  ipu_pf = n_fil/2; ipu_pc = 3;		/* inicialitzar pos. paletas */
  if (ipu_pf+l_pal >= n_fil-3) ipu_pf = 1;
  for (i=0; i< l_pal; i++){	    /* dibuixar paleta inicialment */
 	win_escricar(ipu_pf +i, ipu_pc, '0',INVERS);
  }


  for (j=0; j<numPaletas; j++) {
    for (i=0; i< l_pal; i++){
  	  win_escricar(paletas_or[j].ipo_pf +i, paletas_or[j].ipo_pc, (j+1+'0'),INVERS);
    }
    paletas_or[j].po_pf = paletas_or[j].ipo_pf;		/* fixar valor real paleta ordinador */
  }

  pil_pf = ipil_pf; pil_pc = ipil_pc;	/* fixar valor real posicio pilota */
  win_escricar(ipil_pf, ipil_pc, '.',INVERS);	/* dibuix inicial pilota */

  sprintf(strin,"Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir.",
		TEC_AMUNT, TEC_AVALL);
  win_escristr(strin);
  return(0);
}



/* funcio per moure la pilota; retorna un valor amb alguna d'aquestes	*/
/* possibilitats:							*/
/*	-1 ==> la pilota no ha sortit del taulell			*/
/*	 0 ==> la pilota ha sortit per la porteria esquerra		*/
/*	>0 ==> la pilota ha sortit per la porteria dreta		*/
void * moure_pilota(void * cap){
  int f_h, c_h;
  char rh,rv,rd;

  do{
    win_retard(retard);
    f_h = pil_pf + pil_vf;				/* posicio hipotetica de la pilota */
    c_h = pil_pc + pil_vc;
    rh = rv = rd = ' ';
    if ((f_h != ipil_pf) || (c_h != ipil_pc))
    {							/* si posicio hipotetica no coincideix amb la pos. actual */
      if (f_h != ipil_pf)				/* provar rebot vertical */
      {	rv = win_quincar(f_h,ipil_pc);			/* veure si hi ha algun obstacle */
  	if (rv != ' ')					/* si no hi ha res */
  	{   pil_vf = -pil_vf;				/* canvia velocitat vertical */
  	    f_h = pil_pf+pil_vf;			/* actualitza posicio hipotetica */
  	}
      }
      if (c_h != ipil_pc)				/* provar rebot horitzontal */
      {	rh = win_quincar(ipil_pf,c_h);			/* veure si hi ha algun obstacle */
  	if (rh != ' ')					/* si no hi ha res */
  	{    pil_vc = -pil_vc;				/* canvia velocitat horitzontal */
  	     c_h = pil_pc+pil_vc;			/* actualitza posicio hipotetica */
  	}
      }
      if ((f_h != ipil_pf) && (c_h != ipil_pc))		/* provar rebot diagonal */
      {	rd = win_quincar(f_h,c_h);
  	if (rd != ' ')					/* si no hi ha obstacle */
  	{    pil_vf = -pil_vf; pil_vc = -pil_vc;	/* canvia velocitats */
  	     f_h = pil_pf+pil_vf;
  	     c_h = pil_pc+pil_vc;			/* actualitza posicio entera */
  	}
      }
      if (win_quincar(f_h,c_h) == ' ')			/* verificar posicio definitiva */
      {							/* si no hi ha obstacle */
  	win_escricar(ipil_pf,ipil_pc,' ',NO_INV);	/* esborra pilota */
  	pil_pf += pil_vf; pil_pc += pil_vc;
  	ipil_pf = f_h; ipil_pc = c_h;			/* actualitza posicio actual */
  	if ((ipil_pc > 0) && (ipil_pc <= n_col))	/* si no surt */
  		win_escricar(ipil_pf,ipil_pc,'.',INVERS); /* imprimeix pilota */
  	else
  		cont = ipil_pc;				/* codi de finalitzacio de partida */
      }
    }
    else { pil_pf += pil_vf; pil_pc += pil_vc; }
  } while ((tec != TEC_RETURN) && (cont==-1));
  pthread_exit(0);
}



/* funcio per moure la paleta de l'usuari en funcio de la tec premuda */
void * mou_paleta_usuari(void * cap)
{
  do{
   win_retard(retard);
   tec=win_gettec();
   if ((tec == TEC_AVALL) && (win_quincar(ipu_pf+l_pal,ipu_pc) == ' '))
   {
     win_escricar(ipu_pf,ipu_pc,' ',NO_INV);	   		/* esborra primer bloc */
     ipu_pf++;					   		/* actualitza posicio */
     win_escricar(ipu_pf+l_pal-1,ipu_pc,'0',INVERS); 		/* impri. ultim bloc */
   }
   if ((tec == TEC_AMUNT) && (win_quincar(ipu_pf-1,ipu_pc) == ' '))
   {
     win_escricar(ipu_pf+l_pal-1,ipu_pc,' ',NO_INV); 		/* esborra ultim bloc */
     ipu_pf--;					    		/* actualitza posicio */
     win_escricar(ipu_pf,ipu_pc,'0',INVERS);	    		/* imprimeix primer bloc */
   }
  }while ((tec !=TEC_RETURN) && (cont==-1));
  pthread_exit(0);
}




/* funcio per moure la paleta de l'ordinador autonomament, en funcio de la */
/* velocitat de la paleta (variable global v_pal) */
void * mou_paleta_ordinador(void *index){
  int f_h;
  int ind = (intptr_t) index;
 /* char rh,rv,rd; */
  do{
    win_retard(retard);
    f_h = paletas_or[ind].po_pf + paletas_or[ind].v_pal;					/* posicio hipotetica de la paleta */
    if (f_h != paletas_or[ind].ipo_pf){								/* si pos. hipotetica no coincideix amb pos. actual */
      if (paletas_or[ind].v_pal > 0.0){								/* verificar moviment cap avall */
  	    if (win_quincar(f_h+l_pal-1,paletas_or[ind].ipo_pc) == ' '){   			/* si no hi ha obstacle */
  	      win_escricar(paletas_or[ind].ipo_pf,paletas_or[ind].ipo_pc,' ',NO_INV);      	/* esborra primer bloc */
  	      paletas_or[ind].po_pf += paletas_or[ind].v_pal; paletas_or[ind].ipo_pf = paletas_or[ind].po_pf;		/* actualitza posicio */
  	      win_escricar(paletas_or[ind].ipo_pf+l_pal-1,paletas_or[ind].ipo_pc,(ind+1+'0'),INVERS); /* impr. ultim bloc */
  	    }else										/* si hi ha obstacle, canvia el sentit del moviment */
  	      paletas_or[ind].v_pal = -paletas_or[ind].v_pal;
      }else{											/* verificar moviment cap amunt */
        if (win_quincar(f_h,paletas_or[ind].ipo_pc) == ' '){        				/* si no hi ha obstacle */
      	  win_escricar(paletas_or[ind].ipo_pf+l_pal-1,paletas_or[ind].ipo_pc,' ',NO_INV); 	/* esbo. ultim bloc */
      	  paletas_or[ind].po_pf += paletas_or[ind].v_pal; paletas_or[ind].ipo_pf = paletas_or[ind].po_pf;		/* actualitza posicio */
      	  win_escricar(paletas_or[ind].ipo_pf,paletas_or[ind].ipo_pc,(ind+1+'0'),INVERS);	/* impr. primer bloc */
      	}else											/* si hi ha obstacle, canvia el sentit del moviment */
      	   paletas_or[ind].v_pal = -paletas_or[ind].v_pal;
      }
    }else paletas_or[ind].po_pf += paletas_or[ind].v_pal; 					/* actualitza posicio vertical real de la paleta */

  } while ((tec != TEC_RETURN) && (cont==-1));
  pthread_exit(0);
}




/* programa principal				    */
int main(int n_args, const char *ll_args[]){
  int i, min, seg;
  time_t inicio, fin;
  int marc_u=0, marc_o=0;
  char buffer[20];



  if ((n_args != 2) && (n_args !=3)){
    fprintf(stderr,"Comanda: tennis1 fit_param [retard]\n");
  	exit(1);
  }
  carrega_parametres(ll_args[1]);

  if (n_args == 3) retard = atoi(ll_args[2]);
  else retard = 100;

  if (inicialitza_joc() !=0)    				/* intenta crear el taulell de joc */
     exit(4);   						/* aborta si hi ha algun problema amb taulell */

  pthread_create(&tid[0],NULL,mou_paleta_usuari, NULL);		/* creacio thread paleta usuari */
  pthread_create(&tid[1],NULL,moure_pilota,NULL);		/* creacio thread pilota usuari */
  for(i=2; i<numPaletas; i++){					/* creacio threads paletes ordinador */
    pthread_create(&tid[i],NULL,mou_paleta_ordinador, (void *) (intptr_t) i-2);
  }

  inicio=time(NULL);						/* Tasques independents (temps i marcador) */
  while(cont==-1 && tec != TEC_RETURN){
    fin=time(NULL);
    seg=difftime(fin,inicio);
    min=seg/60;
    seg=seg%60;
    if (cont==0) marc_o++;
    if (cont>0) marc_u++;
    sprintf(buffer,"Us %d - %d Or T:%2d:%2d",marc_u, marc_o, min, seg);
    win_escristr(buffer);
  }

  for(i=0; i<(numPaletas+2); i++){
    pthread_join(tid[i], NULL);
  }
  win_fi();							/* finalitzacio tots els threads */

  if (tec == TEC_RETURN) printf("S'ha aturat el joc amb la tecla RETURN! S'ha jugat durant: %2d:%2d \n", min, seg);
  else { if (cont == 0) printf("Ha guanyat l'ordinador! Temps de joc %2d:%2d \n", min, seg);
         else printf("Ha guanyat l'usuari! Temps de joc: %2d:%2d \n", min, seg);}
  return(0);
}


