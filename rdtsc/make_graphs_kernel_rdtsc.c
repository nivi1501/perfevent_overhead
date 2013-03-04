#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FONTSIZE 11

#define NUM_EVENTS 16
#define NUM_RUNS 1024

#define NUM_KERNELS 16

#define EVENT_TO_PLOT 3

char kernel_names[NUM_KERNELS][64]={
   "2.6.32",
   "2.6.33",
   "2.6.34",
   "2.6.35",
   "2.6.36",
   "2.6.37",
   "2.6.38",
   "2.6.39",
   "3.0.0",
   "3.1.0",
   "3.2.0",
   "3.3.0",
   "3.4.0",
   "3.4.0-rdpmc",
};

char colors[NUM_KERNELS][64]={
  "1.0 0.0   0.0",  /* red */
  "1.0 0.625 0.0",  /* orange */
  "1.0 1.0   0.0",  /* yellow */
  "0.0 0.375 0.0",  /* dark green */
  "0.0 1.0   0.0",  /* light green */
  "0.0 1.0   1.0",  /* cyan */
  "0.375 0.0 1.0",  /* indigo */
  "1.0 0.0 1.0",    /* magenta */
  "0.625 0.0 0.5",  /* purple */
  "0.0 0.0 0.0",    /* black */
  "0.5 0.5 0.5",    /* grey */
};

long long times[NUM_KERNELS][NUM_EVENTS][NUM_RUNS];
long long times_start,times_stop,times_read;

struct freq_list {
  long long value;
  long long count;
  struct freq_list *next;
};

#define PLOT_TYPE_START 0
#define PLOT_TYPE_STOP  1
#define PLOT_TYPE_READ  2
#define PLOT_TYPE_TOTAL 3

int main(int argc, char **argv) {

  int events,run,kernel;
  char filename[BUFSIZ];
  FILE *fff;
  char string[BUFSIZ];
  char *machine,*plot_name;
  int plot_type=PLOT_TYPE_START;
  long long maxy,minx,maxx;
   
  struct freq_list *head[NUM_KERNELS],*tmp,*last;
  
  if (argc<3) {
     printf("Must specify machine and start/stop/read/total\n");
     exit(1);
  }
  
  machine=strdup(argv[1]);

  plot_name=strdup(argv[2]);

  if (!strcmp(plot_name,"start")) {
     plot_type=PLOT_TYPE_START;
  } else if (!strcmp(plot_name,"stop")) {
     plot_type=PLOT_TYPE_STOP;
  } else if (!strcmp(plot_name,"read")) {
     plot_type=PLOT_TYPE_READ;
  } else if (!strcmp(plot_name,"total")) {
     plot_type=PLOT_TYPE_TOTAL;
  }
  else {
    printf("Unknown plot type %s\n",plot_name);
    exit(1);
  }
   
  for(kernel=0;kernel<NUM_KERNELS;kernel++) {
     for(events=0;events<NUM_EVENTS;events++) {
        for(run=0;run<NUM_RUNS;run++) {
	   times[kernel][events][run]=0LL;
	}
     }
  }

  fprintf(stderr,"Reading in data for %s...\n",argv[1]);

  for(kernel=0;kernel<NUM_KERNELS;kernel++) {
     fprintf(stderr,"\tReading data for kernel %s: ",kernel_names[kernel]); 
     for(events=0;events<NUM_EVENTS;events++) {
        fprintf(stderr,"%d ",events);
        for(run=0;run<NUM_RUNS;run++) {
           sprintf(filename,"results/rdtsc_null_raw/%s/%s/%d/null_test.%d",
	           machine,kernel_names[kernel],events,run);
           fff=fopen(filename,"r");
           if (fff==NULL) {
	      fprintf(stderr,"Can't open %s\n",filename);
	      break;
           }
           if (fgets(string,BUFSIZ,fff)==NULL) break;
           if (fgets(string,BUFSIZ,fff)==NULL) break;
	   if (fgets(string,BUFSIZ,fff)==NULL) break;
	   if (fgets(string,BUFSIZ,fff)==NULL) break;
           sscanf(string,"%*s %*s %lld",&times_start);
	   if (fgets(string,BUFSIZ,fff)==NULL) break;
	   sscanf(string,"%*s %*s %lld",&times_stop);
	   if (fgets(string,BUFSIZ,fff)==NULL) break;
	   sscanf(string,"%*s %*s %lld",&times_read);

	   if (plot_type==PLOT_TYPE_START) {
	      times[kernel][events][run]=times_start;
	   }
	   if (plot_type==PLOT_TYPE_STOP) {
	      times[kernel][events][run]=times_stop;
	   }
	   if (plot_type==PLOT_TYPE_READ) {
	      times[kernel][events][run]=times_read;
	   }
	   if (plot_type==PLOT_TYPE_TOTAL) {
	      times[kernel][events][run]=times_start+times_stop+times_read;
	   }


           fclose(fff);
	}
     }
     fprintf(stderr,"\n");
  }


  events=EVENT_TO_PLOT;

  /* Put info in the linked lists */
  for(kernel=0;kernel<NUM_KERNELS;kernel++) {

     head[kernel]=calloc(1,sizeof(struct freq_list));

     for(run=0;run<NUM_RUNS;run++) {

        tmp=head[kernel];
	last=head[kernel];

	while(1) {

	   if (tmp->value==times[kernel][events][run]) {
	      tmp->count++;
	      break;
	   }

	   if (times[kernel][events][run] < tmp->value) {
	      last->next=calloc(1,sizeof(struct freq_list));
	      last->next->count=1;
	      last->next->value=times[kernel][events][run];
	      last->next->next=tmp;
	      break;
	   }

	   if (tmp->next==NULL) {
	      tmp->next=calloc(1,sizeof(struct freq_list));
	      tmp->next->count=1;
	      tmp->next->value=times[kernel][events][run];
	      tmp->next->next=NULL;
	      break;
	   }

	   last=tmp;
	   tmp=tmp->next;
	}

     }    
  }

  /* calculate mins and maxes */

  minx=1000000000;
  maxx=0;
  maxy=0;

  for(kernel=0;kernel<NUM_KERNELS;kernel++) {

     for(tmp=head[kernel];tmp!=NULL;tmp=tmp->next) {
       if (tmp->count!=0) {
	 if (tmp->value<minx) minx=tmp->value;
	 if ((tmp->value>maxx) && (tmp->count>1)) maxx=tmp->value;
	 if (tmp->count>maxy) maxy=tmp->count;
       }
     }
  }


  printf("(* Begin Graph *)\n");
  printf("newgraph\n");
  printf("\n");
  printf("X 7\n");
  printf("Y 5\n");
  printf("\n");
  printf("(* Legend *)\n");
  printf("legend custom\n");
  printf("\n");
  printf("(* Y-Axis *)\n");
  printf("yaxis size 4 min 0 max %lld\n",maxy);
  printf("grid_gray 0.9 grid_lines\n");
  printf("label font Helvetica fontsize %d  : Times Observed\n",FONTSIZE);
  printf("hash_labels font Helvetica fontsize %d\n",FONTSIZE);
  printf("\n");
  printf("(* X-Axis *)\n");
  printf("xaxis size 6 min %lld max %lld\n",minx,maxx);
  printf("hash_labels font Helvetica fontsize %d\n",FONTSIZE);
  printf("label font Helvetica fontsize %d  : Overhead (cycles)\n",
	 FONTSIZE);
  printf("\n");
  printf("(* Title *)\n");
  printf("title font Helvetica fontsize %d y %lf : "
	 "%s - ",
	 FONTSIZE+2,
	 (double)maxy+((double)maxy/8.0),machine);

  if (plot_type==PLOT_TYPE_START) printf("Start");
  if (plot_type==PLOT_TYPE_STOP) printf("Stop");
  if (plot_type==PLOT_TYPE_READ) printf("Read");
  if (plot_type==PLOT_TYPE_TOTAL) printf("Start/Stop/Read");

  printf(" Overhead with %d Events\n",events);
  printf("\n");


  for(kernel=0;kernel<NUM_KERNELS;kernel++) {

     int xsize=maxx-minx;

     printf("newcurve marktype none linetype solid color %s\n",colors[kernel]);
     printf("label hjl vjc fontsize 14 font Helvetica x %lld y %.2lf : %s\n",
	    maxx-(xsize/5),
	    (double)maxy-(((double)maxy/20.0)*(double)kernel),
	    kernel_names[kernel]);
     printf("pts\n");

     for(tmp=head[kernel];tmp!=NULL;tmp=tmp->next) {
       if (tmp->count!=0) 
        printf("\t%lld %lld\n",tmp->value,tmp->count);
     }
  }

  return 0;
}

