/* by Vince Weaver, vweaver1@eecs.utk.edu                      */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <time.h>

#include <unistd.h>
#include <asm/unistd.h>

#include <sys/ioctl.h>
#include <errno.h>

#include <stddef.h> /* offsetof() */


#include "perfmon2/pfmlib.h"
#include "perfmon2/perfmon.h"

long long convert_to_ns(struct timespec *before,
			struct timespec *after) {

  long long seconds;
  long long ns;

  seconds=after->tv_sec - before->tv_sec;
  ns = after->tv_nsec - before->tv_nsec;

  ns = (seconds*1000000000ULL)+ns;

  return ns;
}

#define MAX_EVENTS 16


char core2_events[MAX_EVENTS][64]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  "L1I_READS",                    /* PAPI_L1_ICA  */
  "L1I_MISSES",                   /* PAPI_L1_ICM  */
  "L1D_ALL_REF",                  /* PAPI_L1_DCA  */
  "L1D_REPL",                     /* PAPI_L1_DCM  */
  "HW_INT_RCV",                   /* PAPI_HW_INT  */
  "FP_COMP_OPS_EXE",              /* PAPI_FP_OPS  */
  "INST_RETIRED:LOADS",           /* PAPI_LD_INS  */
  "INST_RETIRED:STORES",          /* PAPI_SR_INS  */
  "L2_RQSTS:SELF:ANY:MESI",       /* PAPI_L2_TCA  */
  "L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

char atom_events[MAX_EVENTS][64]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  "DATA_TLB_MISSES:DTLB_MISS",    /* PAPI_TLB_DM  */
  "ICACHE:ACCESSES",              /* PAPI_L1_ICA  */
  "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  "L1D_CACHE:LD",                 /* PAPI_L1_DCA  */
  "L2_RQSTS:SELF",                /* PAPI_L1_DCM  */
  "HW_INT_RCV",                   /* PAPI_HW_INT  */
  "SIMD_INST_RETIRED:ANY",        /* PAPI_FP_OPS  */
  "L2_LD:SELF:ANY:MESI",          /* PAPI_LD_INS  */
  "L2_ST:SELF:MESI",              /* PAPI_SR_INS  */
  "L2_LD:SELF:ANY:MESI",          /* PAPI_L2_TCA  */
  "L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

char amd10h_events[MAX_EVENTS][64]={
  "CPU_CLK_UNHALTED",                         /* PAPI_TOT_CYC */
  "RETIRED_INSTRUCTIONS",                     /* PAPI_TOT_INS */
  "RETIRED_BRANCH_INSTRUCTIONS",              /* PAPI_BR_INS  */
  "RETIRED_MISPREDICTED_BRANCH_INSTRUCTIONS", /* PAPI_BR_MSP  */
  "L1_ITLB_MISS_AND_L2_ITLB_MISS:ALL",        /* PAPI_TLB_IM  */
  "L1_DTLB_AND_L2_DTLB_MISS",                 /* PAPI_TLB_DM  */
  "INSTRUCTION_CACHE_FETCHES",                /* PAPI_L1_ICA  */
  "INSTRUCTION_CACHE_MISSES",                 /* PAPI_L1_ICM  */
  "DATA_CACHE_ACCESSES",                      /* PAPI_L1_DCA  */
  "DATA_CACHE_MISSES",                        /* PAPI_L1_DCM  */
  "INTERRUPTS_TAKEN",                         /* PAPI_HW_INT  */
  "DISPATCHED_FPU:OPS_MULTIPLY:OPS_ADD",      /* PAPI_FP_OPS  */
  "DECODER_EMPTY",                            /* PAPI_LD_INS  */ /* nope */
  "DISPATCH_STALLS",                          /* PAPI_SR_INS  */ /* nope */
  "REQUESTS_TO_L2:ALL",                       /* PAPI_L2_TCA  */
  "L2_CACHE_MISS:INSTRUCTIONS:DATA",          /* PAPI_L2_TCM  */
};

char nehalem_events[MAX_EVENTS][64]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BR_INST_EXEC:ANY",             /* PAPI_BR_INS  */
  "BR_MISP_EXEC:ANY",             /* PAPI_BR_MSP  */
  "ITLB_MISSES:ANY",              /* PAPI_TLB_IM  */
  "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  "L1I:READS",                    /* PAPI_L1_ICA  */
  "L1I:MISSES",                   /* PAPI_L1_ICM  */
  "L1D_ALL_REF:ANY",              /* PAPI_L1_DCA  */
  "L1D:REPL",                     /* PAPI_L1_DCM  */
  "HW_INT:RCV",                   /* PAPI_HW_INT  */
  "FP_COMP_OPS_EXE:SSE_FP",       /* PAPI_FP_OPS  */
  "MEM_INST_RETIRED:LOADS",       /* PAPI_LD_INS  */
  "MEM_INST_RETIRED:STORES",      /* PAPI_SR_INS  */
  "L2_RQSTS:REFERENCES",          /* PAPI_L2_TCA  */
  "L2_RQSTS:PREFETCHES",          /* PAPI_L2_TCM  */
};

char power6_events[MAX_EVENTS][64]={
  "PM_RUN_CYC",         	/* PAPI_TOT_CYC */
  "PM_INST_CMPL",         	/* PAPI_TOT_INS */
  "PM_BRU_FIN",             	/* PAPI_BR_INS  */
  "PM_BR_MPRED",             	/* PAPI_BR_MSP  */
  "PM_DATA_FROM_L2MISS",        /* PAPI_TLB_IM  */  /* nope */
  "PM_DATA_FROM_L2MISS",        /* PAPI_TLB_DM  */  /* nope */
  "PM_ST_REF_L1",               /* PAPI_L1_ICA  */ /* nope */
  "PM_L1_ICACHE_MISS",          /* PAPI_L1_ICM  */
  "PM_LD_REF_L1",               /* PAPI_L1_DCA  */  /* nope */
  "PM_LD_MISS_L1",              /* PAPI_L1_DCM  */  /* nope */
  "PM_EXT_INT",                 /* PAPI_HW_INT  */
  "PM_FPU_FLOP",       		/* PAPI_FP_OPS  */
  "PM_LD_REF_L1",       	/* PAPI_LD_INS  */
  "PM_ST_REF_L1",      		/* PAPI_SR_INS  */
  "PM_LD_REF_L1",          	/* PAPI_L2_TCA  */ /* nope */
  "PM_DATA_FROM_L2MISS",        /* PAPI_L2_TCM  */ /* nope */
};

char cortexA9_events[MAX_EVENTS][64]={
  "CPU_CYCLES",                    /* PAPI_TOT_CYC */
  "INST_OUT_OF_RENAME_STAGE",      /* PAPI_TOT_INS */
  "PC_WRITE",                      /* PAPI_BR_INS  */
  "PC_BRANCH_MIS_PRED",            /* PAPI_BR_MSP  */
  "ITLB_MISS",                     /* PAPI_TLB_IM  */
  "DTLB_REFILL",                   /* PAPI_TLB_DM  */
  "MAIN_UNIT_EXECUTED_INST",       /* PAPI_L1_ICA  */ /* nope */
  "IFETCH_MISS",                   /* PAPI_L1_ICM  */
  "DCACHE_ACCESS",                  /* PAPI_L1_DCA  */
  "DCACHE_REFILL",                 /* PAPI_L1_DCM  */
  "EXT_INTERRUPTS",                /* PAPI_HW_INT  */
  "FP_EXECUTED_INST",              /* PAPI_FP_OPS  */
  "DREAD",                         /* PAPI_LD_INS  */
  "DWRITE",                        /* PAPI_SR_INS  */
  "DCACHE_ACCESS",                 /* PAPI_L2_TCA  */  /* nope */
  "DCACHE_REFILL",                 /* PAPI_L2_TCM  */  /* nope */
};

char events[MAX_EVENTS][64];

#define MATRIX_SIZE 128


int main(int argc, char **argv) {

  int i,ctx_fd,j;
   
   struct timespec before,after;
   struct timespec start_before;
   struct timespec stop_after;

   long long init_ns=0,eventset_ns=0;
   long long ns=0;
   int ret1,ret2,ret3;
   pfm_err_t ret;

   int count;

   long long results[MATRIX_SIZE][MAX_EVENTS];

   int x,y,z;
   double s;


   char *machine_name; 

   if (argc>2) {
     count=atoi(argv[2]);
     machine_name=strdup(argv[1]);
   }
   else {
     count=1;
     machine_name=strdup("core2");
   }

   if (!strncmp(machine_name,"core2",5)) {
      memcpy(events,core2_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"nehalem",7)) {
     memcpy(events,nehalem_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"atom",4)) {
     memcpy(events,atom_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"amd10h",6)) {
     memcpy(events,amd10h_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"amd0fh",6)) {
     memcpy(events,amd10h_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"power6",6)) {
     memcpy(events,power6_events,MAX_EVENTS*64);
   }
   else if (!strncmp(machine_name,"cortexA9",8)) {
     memcpy(events,cortexA9_events,MAX_EVENTS*64);
   }
   else {
      fprintf(stderr,"Unknown machine name %s\n",machine_name);
      exit(0);
   }

   
   /* measure init latency */

   clock_gettime(CLOCK_REALTIME,&before);

   /* init */

   clock_gettime(CLOCK_REALTIME,&after);

   init_ns=convert_to_ns(&before,&after);

   printf("init_latency: %lld ns\n",init_ns);

   /*****************************/
   /* measure eventset creation */
   /*****************************/

   clock_gettime(CLOCK_REALTIME,&before);

   pfmlib_options_t pfmlib_options;

   memset(&pfmlib_options,0,sizeof(pfmlib_options));
   pfm_set_options(&pfmlib_options);

   ret=pfm_initialize();
   if (ret != PFMLIB_SUCCESS) {
     fprintf(stderr,"Error starting pfmlib\n");
     exit(1);
   }

   unsigned int num_counters;
   pfmlib_input_param_t inp;
   pfmlib_output_param_t outp;

#define NUM_PMCS PFMLIB_MAX_PMCS
#define NUM_PMDS PFMLIB_MAX_PMDS
   pfarg_pmd_t pd[NUM_PMDS];
   pfarg_pmc_t pc[NUM_PMCS];
   pfarg_ctx_t ctx;
   pfarg_load_t load_args;
   

   pfm_get_num_counters(&num_counters);

   memset(pd,0,sizeof(pd));
   memset(pc,0,sizeof(pc));
   memset(&ctx,0,sizeof(ctx));
   memset(&load_args,0,sizeof(load_args));
   
   memset(&inp,0,sizeof(inp));
   memset(&outp,0,sizeof(outp));

   inp.pfp_dfl_plm = PFM_PLM3; /* user only */

   inp.pfp_event_count=count;

   for(i=0;i<count;i++) {
      if (i<num_counters) {  
	 pfm_find_full_event(events[i],&inp.pfp_events[i]);
      }
      
   }
   
   
   ctx_fd=pfm_create_context(&ctx,NULL,NULL,0);
   if (ctx_fd==-1) {
     fprintf(stderr,"Error in pfm_create()\n");
     exit(1);
   }
   
   //detect_unavail_pmcs(ctx_fd, &inp.pfp_unavail_pmcs);
   
   ret=pfm_dispatch_events(&inp,NULL,&outp,NULL);
   if (ret!=PFMLIB_SUCCESS) {
      printf("Eventset_creation_%d: %lld ns\n",i,0LL);
      printf("start/stop/read latency: %lld ns\n",0LL);
      printf("dispatch_events failed\n");
      exit(1);
   }
   
   for(i=0;i<outp.pfp_pmc_count;i++) {
      pc[i].reg_num=outp.pfp_pmcs[i].reg_num;
      pc[i].reg_value=outp.pfp_pmcs[i].reg_value;


   }
   for(i=0;i<outp.pfp_pmd_count;i++) {
      pd[i].reg_num=outp.pfp_pmds[i].reg_num;
   }
   
   ret=pfm_write_pmcs(ctx_fd,pc,outp.pfp_pmc_count);
   if (ret!=PFMLIB_SUCCESS) {
      fprintf(stderr,"pfm_write_pmcs() failed\n");
      exit(1);
   }
   
   ret=pfm_write_pmds(ctx_fd,pd,outp.pfp_pmd_count);
   if (ret!=PFMLIB_SUCCESS) {
      fprintf(stderr,"pfm_write_pmds() failed\n");
      exit(1);
   }
   
   load_args.load_pid=getpid();
   ret=pfm_load_context(ctx_fd, &load_args);
   if (ret!=PFMLIB_SUCCESS) {
      fprintf(stderr,"pfm_load_context() failed\n");
      exit(1);
   }
   
   clock_gettime(CLOCK_REALTIME,&after);

   eventset_ns=convert_to_ns(&before,&after);

   printf("Eventset_creation_%d: %lld ns\n",i,eventset_ns);

   float a[MATRIX_SIZE][MATRIX_SIZE];
   float b[MATRIX_SIZE][MATRIX_SIZE];
   float c[MATRIX_SIZE][MATRIX_SIZE];

   for(x=0;x<MATRIX_SIZE;x++) {
     for(y=0;y<MATRIX_SIZE;y++) {
       a[x][y]=(float)x*(float)y;
       b[x][y]=(float)x/(float)(y+5);
     }
   }


   /*********/
   /* start */
   /*********/

   clock_gettime(CLOCK_REALTIME,&start_before);


   ret1=pfm_start(ctx_fd,NULL);
   /* syscall(PFM_pfm_start,fd,NULL) */


   /********/
   /* read */
   /********/

   for(x=0;x<MATRIX_SIZE;x++) {

      ret3=pfm_read_pmds(ctx_fd,pd,inp.pfp_event_count);

      for(i=0;i<count;i++) {
         results[x][i]=(long long)pd[i].reg_value;
      }

      /* syscall(PFM_pfm_read_pmds, fd, pmds, count); */
      for(y=0;y<MATRIX_SIZE;y++) {

	s=0;
	for(z=0;z<MATRIX_SIZE;z++) {
	  s+=a[y][z]*b[z][x];
	}
	c[y][x] = s;
      }
   }



   /********/
   /* stop */
   /********/
   
   ret2=pfm_stop(ctx_fd);
   /* syscall(PFM_pfm_stop,fd); */




   clock_gettime(CLOCK_REALTIME,&stop_after);

   if (ret3<0) {
      printf("start/stop/read latency: %lld ns\n",0LL);
      printf("Unexpected read result %d %s\n",ret3,strerror(errno));
      exit(0);
   }


   if (ret1<0) {
      printf("start/stop/read latency: %lld ns\n",0LL);
      printf("Error starting!\n");
      exit(1);
   }
   
   ns=convert_to_ns(&start_before,&stop_after);
   
   printf("start/stop/read latency: %lld ns\n",ns);

   if (ret2<0) {
      printf("Error stopping!\n");
      exit(1);
   }


   s=0.0;
   for(i=0;i<MATRIX_SIZE;i++) {
     for(j=0;j<MATRIX_SIZE;j++) {
       s+=c[i][j];
     }
   }

   printf("Validation: s=%f\n",s);


   for(i=0;i<count;i++) {
     printf("%llx: ",outp.pfp_pmcs[i].reg_value);
     for(j=0;j<MATRIX_SIZE;j++) {
       printf("%lld ",results[j][i]);
     }
     printf("\n");
   }

   /* close */

   return 0;
}
