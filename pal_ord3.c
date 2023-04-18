
#include <stdio.h>		/* incloure definicions de funcions estandard */
#include <stdlib.h>
#include "winsuport2.h"		/* incloure definicions de funcions propies */
#include <pthread.h>		/* incloure threads */
#include <stdint.h>
#include <time.h>
#include "memoria.h"

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

int n_fil, n_col, m_por;	/* dimensions del taulell i porteries */
int l_pal,ipc_paletas, ipc_mem, tec, numPelotas;			/* longitud de les paletes */
int *p_mem;

Paleta_Or paletas_or;

int main(int n_args, char *ll_args[]){
 int *p_paletas;
	if(n_args !=10){
		fprintf(stderr,"error proceso\n");
		exit(0);
	}
	paletas_or.ipo_pf = atoi(ll_args[1]); 
	paletas_or.ipo_pc = atoi(ll_args[2]);
 	paletas_or.po_pf = atof(ll_args[3]);
 	paletas_or.v_pal = atof(ll_args[4]);
	n_fil = atoi(ll_args[5]); 
	n_col = atoi(ll_args[6]);
 	l_pal = atoi(ll_args[7]);
 	ipc_mem = atoi(ll_args[8]);
	ipc_paletas = atoi(ll_args[9]);
	tec = atoi(ll_args[10]);
	numPelotas = atoi(ll_args[11]);

	
	win_set(p_mem, n_fil, n_col);
	p_paletas=map_mem(ipc_paletas);		
/*	if (p_paletas ==(int*) -1){
		fprintf(stderr, "proceso (%d):error en identificador de memoria\n", getpid());
		exit(0);
	}*/
	setbuf(stdout, NULL); 		/********************************/

  int f_h;

 /* char rh,rv,rd; */
  do{

    f_h = paletas_or.po_pf + paletas_or.v_pal;					/* posicio hipotetica de la paleta */
    if (f_h != paletas_or.ipo_pf){								/* si pos. hipotetica no coincideix amb pos. actual */
      if (paletas_or.v_pal > 0.0){								/* verificar moviment cap avall */
   	    
  	    if (win_quincar(f_h+l_pal-1,paletas_or.ipo_pc) == ' '){   			/* si no hi ha obstacle */
  	      win_escricar(paletas_or.ipo_pf,paletas_or.ipo_pc,' ',NO_INV);      	/* esborra primer bloc */
  	      paletas_or.po_pf += paletas_or.v_pal; paletas_or.ipo_pf = paletas_or.po_pf;		/* actualitza posicio */
  	      win_escricar(paletas_or.ipo_pf+l_pal-1,paletas_or.ipo_pc,ll_args[12][0],INVERS); /* impr. ultim bloc ************************************************ */
   	      
  	    }else{										/* si hi ha obstacle, canvia el sentit del moviment */
   	     
  	      paletas_or.v_pal = -paletas_or.v_pal;
	    }
      }else{											/* verificar moviment cap amunt */
   	
        if (win_quincar(f_h,paletas_or.ipo_pc) == ' '){        				/* si no hi ha obstacle */
      	  win_escricar(paletas_or.ipo_pf+l_pal-1,paletas_or.ipo_pc,' ',NO_INV); 	/* esbo. ultim bloc */
      	  paletas_or.po_pf += paletas_or.v_pal; paletas_or.ipo_pf = paletas_or.po_pf;		/* actualitza posicio */
     	  win_escricar(paletas_or.ipo_pf,paletas_or.ipo_pc, ll_args[12][0],INVERS);	/* impr. primer bloc***************************** */
  	  
      	}else{											/* si hi ha obstacle, canvia el sentit del moviment */
      	   
	  paletas_or.v_pal = -paletas_or.v_pal;
	}
      }
    }else paletas_or.po_pf += paletas_or.v_pal; 					/* actualitza posicio vertical real de la paleta */

  } while ((tec != TEC_RETURN) && (numPelotas>0));
  pthread_exit(0);
}
	
		 


	
