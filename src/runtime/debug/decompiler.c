#include "../inter.h"


#ifndef __EMSCRIPTEN__

// Helper to safely sprintf into a buffer using a temp to avoid overlap warnings
#define SAFE_SPRINTF(dst, fmt, ...) do { char _tmp_buf[512]; snprintf(_tmp_buf,sizeof(_tmp_buf),fmt,__VA_ARGS__); strcpy(dst,_tmp_buf); } while(0)
// Copy between different elements of the same array (compiler can't prove non-overlap)
static inline void array_strcpy(char *dst, const char *src) { char tmp[512]; strncpy(tmp,src,511); tmp[511]=0; strcpy(dst,tmp); }

#define STACK_SIZE 128

extern int stack[STACK_SIZE];
byte stp[STACK_SIZE];
char cstack[STACK_SIZE][512];

#define getvarref(X) cstack[ X ]

FILE *prg=NULL;

byte locvar[STACK_SIZE];
byte localvar;

void getvarname(int i, char *name) {

	char mousestruct[12][10]={
		"x","y","z","file",
		"graph","angle","size",
		"flags","region","left",
		"middle","right"
	};
		
	char m7struct[8][10]={
		"z","camera","height",
		"distance","horizon","focus",
		"color"
	};
	switch(i) {
			case 16:
				strcpy(name,"father");
				break;
			case 21:
				strcpy(name,"ctype");
				break;
			case 22:
				strcpy(name,"x");
				break;
			case 23:
				strcpy(name,"y");
				break;
			case 24:
				strcpy(name,"z");
				break;
			case 25:
				strcpy(name,"graph");
				break;
			case 26:
				strcpy(name,"flags");
				break;
			case 27:
				strcpy(name,"size");
				break;
			case 28:
				strcpy(name,"angle");
				break;
			case 29:
				strcpy(name,"region");
				break;
			case 30:
				strcpy(name,"file");
				break;
			case 34: 
				strcpy(name,"resolution");
				break;
			default:
				sprintf(name,"var%d",i);
				break;
		}	
		if(i>=mem[6] && i<255) {
if(localvar==0)
	i+=mem[5];
			sprintf(name,"var%d",i);

			if(i>=iloc_len && i<=iloc_len+12) {
				
				sprintf(name,"mouse.%s",mousestruct[i-iloc_len]);
				
			
			}
			if(i>=iloc_len+112 && i<=iloc_len+120) {
				
				sprintf(name,"m7.%s",m7struct[i-iloc_len-112]);
				
			
			}
			
		}
}


void printglobals(void) {
	int i=255;//mem[1];
	fprintf(prg,"GLOBAL // %u %u\n",i,mem[1]-1);	
	while(i<=mem[1]-1) {
		fprintf(prg,"var%u=%d;\n",i,mem[i]);
		i++;
	}
}
void printlocal(void) {

	int i=mem[2]+35;

		fprintf(prg,"LOCAL\n");

	while(i<mem[7]) {
		fprintf(prg,"var%u=%d;\n",i,mem[i]);
		i++;
	}

}


void dump(int size) {
	int i=mem[1]-1,itmp=0;
	int sp=0,spp=0;
	
	int *jmp;
	int *jpf;
	int *jpb;
	int *end;
	int *els;
	
	int j=0;
	int args=0;
	int ifloop=0;
	char condstack[255][100];
	int con=0;
	int f=0;
	//					0    1     2    3    4    5    6    7    8    9  
	char keys[128][15]={"","_esc","_1","_2","_3","_4","_5","_6","_7","_8",
	//				   10    11     12       13         14       15 
					   "_9","_0","_minus","_plus","_backspace","_tab",
					   
	//				   16    17   18   19   20   21   22   23   24   25   26           27          28
					   "_q","_w","_e","_r","_t","_y","_u","_i","_o","_p","_l_bracket","_r_bracket","_enter",
	//				   29          30   31   32   33   34   35   36   37   38    39          40            41
					   "_control","_a","_s","_d","_f","_g","_h","_j","_k","_l","_semicolon","_apostrophe","_wave"
	//				   42          43           44   45   46   47   48   49   50    51           52
					   "_l_shift","_backslash","_z","_x","_c","_v","_b","_n","_m","_comma","","_c_backslash",
	//					53
						"_c_backslash","_r_shift","_c_asterisk","_alt","_space","_caps_lock",
	//					59		60    61    62   63    64     65    66    67    68
						"_f1","_f2","_f3","_f4","_f5","_f6","_f7","_f8","_f9","_f10",
						
	//					
						"_num_lock","_scroll_lock","_home","_up","_pgup","_c_minus","_left","_c_center",
						
						"_right","_plus","_end","_down","_c_pgdn","_c_ins","_c_del"
						//						"_f11","_f12"
					   };
jmp = (int *)malloc(size*5);
jpf = (int *)malloc(size*5);
jpb = (int *)malloc(size*5);
end = (int *)malloc(size*5);
els = (int *)malloc(size*5);

	memset(jmp,0,size*4);
	memset(jpf,0,size*4);
	memset(jpb,0,size*4);
	memset(end,0,size*4);
	memset(els,0,size*4);
	
	char name[255];
	
	char cmd[512];
	memset(cmd,0,255);
	printf("pos is %d %d\n",i,size);
	
	FILE *sta = fopen("dump.txt","w+");
	prg = fopen("prg.prg","w+");


// find the jmps

while(i++<mem[7]-36) {
	switch ((byte)mem[i]) {
	    case lnop: fprintf(sta,"%5u\tnop",i); break;
    case lcar: fprintf(sta,"%5u\tcar %u",i,mem[i+1]); i++; break;
    case lasi: fprintf(sta,"%5u\tasi",i); break;
    case lori: fprintf(sta,"%5u\tori",i); break;
    case lxor: fprintf(sta,"%5u\txor",i); break;
    case land: fprintf(sta,"%5u\tand",i); break;
    case ligu: fprintf(sta,"%5u\tigu",i); break;
    case ldis: fprintf(sta,"%5u\tdis",i); break;
    case lmay: fprintf(sta,"%5u\tmay",i); break;
    case lmen: fprintf(sta,"%5u\tmen",i); break;
    case lmei: fprintf(sta,"%5u\tmei",i); break;
    case lmai: fprintf(sta,"%5u\tmai",i); break;
    case ladd: fprintf(sta,"%5u\tadd",i); break;
    case lsub: fprintf(sta,"%5u\tsub",i); break;
    case lmul: fprintf(sta,"%5u\tmul",i); break;
    case ldiv: fprintf(sta,"%5u\tdiv",i); break;
    case lmod: fprintf(sta,"%5u\tmod",i); break;
    case lneg: fprintf(sta,"%5u\tneg",i); break;
    case lptr: fprintf(sta,"%5u\tptr",i); break;
    case lnot: fprintf(sta,"%5u\tnot",i); break;
    case laid: fprintf(sta,"%5u\taid",i); break;
    case lcid: fprintf(sta,"%5u\tcid",i); break;
    case lrng: fprintf(sta,"%5u\trng %u",i,mem[i+1]); i++; break;
    case ljmp: 
		fprintf(sta,"%5u\tjmp %u",i,mem[i+1]); 
			jmp[mem[i+1]]++;
		i++; 
		break;
    case ljpf: 
		fprintf(sta,"%5u\tjpf %u",i,mem[i+1]); 
		if(mem[i+1]<i) {
				printf("jump backward %d %d\n",i,mem[i+1]);
			jpb[mem[i+1]]++;
		} else {
			printf("jump forward %d %d\n",i,mem[i+1]);
			jpf[mem[i+1]]++;
		}
		// check if there is a back jump before
		if(mem[mem[i+1]-2]==ljmp) {
			printf("jpf followed by jump %d %d %d\n",i,mem[i+1],mem[mem[i+1]-1]);
			jmp[mem[mem[i+1]-1]]--;
			printf("jmp[%d]=%d\n",mem[mem[i+1]-1],jmp[mem[mem[i+1]-1]]);

		}
		i++; 
		break;
    case lfun: 
		fprintf(sta,"%5u\tfun %u stack: %d",i,mem[i+1],sp); 
		switch(mem[i+1]) {
			case 2: // load_pal
			case 3: // load_fpg
			case 15: // load_fnt
			case 16: // write
				// check if ref is ptr or static

					// ptr
			case 34: // save
			case 35: // load
			case 37: // load_pcm
			case 47: // system
			case 67: // exit 
				printf("text used at %u\n",i);

			if(mem[i+1]==16) { // write 
				if(mem[i-2]==lcar) // static
					printf("static text val\n");
				else
					printf("by ref\n");
			}

			break;
		}
		i++; 		
		break;
    case lcal: fprintf(sta,"%5u\tcal %u",i,mem[i+1]); i++; break;
    case lret: fprintf(sta,"%5u\tret",i); break;
    case lasp: 
		fprintf(sta,"%5u\tasp",i); 
		break;
    case lfrm: fprintf(sta,"%5u\tfrm",i); break;
    case lcbp: fprintf(sta,"%5u\tcbp %u",i,mem[i+1]); i++; break;
    case lcpa: fprintf(sta,"%5u\tcpa",i); break;
    case ltyp: fprintf(sta,"\n%5u\ttyp %u",i,mem[i+1]); i++; break;
    case lpri: fprintf(sta,"%5u\tpri %u",i,mem[i+1]); i++; break;
    case lcse: fprintf(sta,"%5u\tcse %u",i,mem[i+1]); i++; break;
    case lcsr: fprintf(sta,"%5u\tcsr %u",i,mem[i+1]); i++; break;
    case lshr: fprintf(sta,"%5u\tshr",i); break;
    case lshl: fprintf(sta,"%5u\tshl",i); break;
    case lipt: fprintf(sta,"%5u\tipt",i); break;
    case lpti: fprintf(sta,"%5u\tpti",i); break;
    case ldpt: fprintf(sta,"%5u\tdpt",i); break;
    case lptd: fprintf(sta,"%5u\tptd",i); break;
    case lada: fprintf(sta,"%5u\tada",i); break;
    case lsua: fprintf(sta,"%5u\tsua",i); break;
    case lmua: fprintf(sta,"%5u\tmua",i); break;
    case ldia: fprintf(sta,"%5u\tdia",i); break;
    case lmoa: fprintf(sta,"%5u\tmoa",i); break;
    case lana: fprintf(sta,"%5u\tana",i); break;
    case lora: fprintf(sta,"%5u\tora",i); break;
    case lxoa: fprintf(sta,"%5u\txoa",i); break;
    case lsra: fprintf(sta,"%5u\tsra",i); break;
    case lsla: fprintf(sta,"%5u\tsla",i); break;
    case lpar: fprintf(sta,"%5u\tpar %u",i,mem[i+1]); i++; break;
    case lrtf: fprintf(sta,"%5u\trtf",i); break;
    case lclo: fprintf(sta,"%5u\tclo %u",i,mem[i+1]); i++; break;
    case lfrf: fprintf(sta,"%5u\tfrf",i); break;
    case limp: fprintf(sta,"%5u\timp %u",i,mem[i+1]); i++; break;
    case lext: fprintf(sta,"%5u\text %u",i,mem[i+1]); i++; break;
    case lchk: fprintf(sta,"%5u\tchk",i); break;
    case ldbg: fprintf(sta,"%5u\tdbg",i); break;
}
}


rewind(sta);
j=0;
f=0;

// GLOBAL vars;

	

i=mem[1]-1;
	while (i++<mem[2]-1) { 
		fprintf(sta,"\n");

		if(end[i]>0) {
			while(end[i]>0) {
				fprintf(prg,"END // endstop %d %d\n\n",i,end[i]);
				end[i]--;
			}
		}
		if(els[i]>0) {
			while(els[i]>0) {
				fprintf(prg,"ELSE // else %d %d\n\n",i,els[i]);
				els[i]--;
			}
		}

		if(jmp[i] || jpb[i] || jpf[i]) {
		
			if(jmp[i]>0) {
				fprintf(prg,"\nLOOP // %d %d\n\n",i,jmp[i]);
			}
			
			if(jpb[i]>0) {
				fprintf(prg,"\nREPEAT // %d %d\n",i,jpb[i]);
			} else {
		}
	}
	
		switch ((byte)mem[i]) {
			// no op
    case lnop:
		fprintf(sta,"%5u\tnop",i); 
		fprintf(prg,"// NOP\n");
		break;
    
			// load val to stack
    case lcar: 
		fprintf(sta,"%5u\tcar %d",i,mem[i+1]); 
		stack[sp]=mem[i+1];
		sprintf(cstack[sp],"%d%c",mem[i+1],0);
		strcpy(cmd,cstack[sp]);
		stp[sp]=0; // not a pointer
		locvar[sp]=0; // not a local
		sp++;
		i++; 
		break;
    case lasi: 
		fprintf(sta,"%5u\tasi",i); 
		if(strlen(cmd)>0 && sp>1) {
		fprintf(prg,"// asi stack %s %s %d %d\n",cstack[sp-2],cstack[sp-1],sp,i);
			fprintf(prg,"%s=%s",cstack[sp-2],cmd);
		}
		memset(cmd,0,255);
		con=1;
		sp--;
		break;
    case lori: 
		fprintf(sta,"%5u\tori",i);
		fprintf(prg,"// stack: || %d %d\n",sp,i);
		SAFE_SPRINTF(condstack[con],"%s OR %s",condstack[con-2],condstack[con-1]);
		array_strcpy(condstack[con-2],condstack[con]);
		strcpy(cstack[sp-2],condstack[con]);

		con--;
		strcpy(cmd,condstack[con-1]);

		break;
    case lxor: 
		fprintf(sta,"%5u\txor",i); 
		fprintf(prg,"// UNIMP! XOR\n");
		fprintf(prg,"// stack: %d\n",sp);
		break;
    case land: 
		fprintf(sta,"%5u\tand",i); 
		fprintf(prg,"// stack: && %d\n",sp);
		SAFE_SPRINTF(condstack[con],"(%s && %s)",condstack[con-2],condstack[con-1]);
		array_strcpy(condstack[con-2],condstack[con]);
		con--;
		strcpy(cmd,condstack[con-1]);
		break;
    case ligu: 
		fprintf(sta,"%5u\tigu",i); 
		sprintf(condstack[con],"%s == %s",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;
		break;
    case ldis: 
		fprintf(sta,"%5u\tdis",i); 
		sprintf(condstack[con],"%s != %s",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;

		break;
    case lmay: 
		fprintf(sta,"%5u\tmay",i); 
		sprintf(condstack[con],"%s > %s",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;
		break;
    case lmen: 
		fprintf(sta,"%5u\tmen",i); 
		sprintf(condstack[con],"%s < %s",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;

		break;
    case lmei: 
		fprintf(sta,"%5u\tmei",i); 
		sprintf(condstack[con],"(%s <= %s)",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;

		break;
    case lmai: 
		fprintf(sta,"%5u\tmai",i); 
		sprintf(condstack[con],"(%s >= %s)",cstack[sp-2],cstack[sp-1]);
		strcpy(cmd,condstack[con]);
		con++;
		break;
    case ladd: 
		fprintf(sta,"%5u\tadd",i);
		fprintf(prg,"// ladd %d[%d]+%d[%d] %d %d\n",stack[sp-2],stp[sp-2],stack[sp-1],stp[sp-1],sp,i);
		SAFE_SPRINTF(cstack[sp],"(%s+%s)",cstack[sp-2],cstack[sp-1]);
		stack[sp-2]+=stack[sp-1];
		strcpy(cmd,cstack[sp]);
		array_strcpy(cstack[sp-2],cstack[sp]);
		sp--;
		break;
    case lsub:
		fprintf(sta,"%5u\tsub",i);
		//stack[sp-2]-=stack[sp-1];
		SAFE_SPRINTF(cstack[sp],"(%s-%s)",cstack[sp-2],cstack[sp-1]);
		stack[sp-2]-=stack[sp-1];
		array_strcpy(cstack[sp-2],cstack[sp]);
		strcpy(cmd,cstack[sp-2]);
				
		sp--;
		break;
    case lmul:
		fprintf(sta,"%5u\tmul",i); 
		SAFE_SPRINTF(cstack[sp],"(%s*%s)",cstack[sp-2],cstack[sp-1]);
		array_strcpy(cstack[sp-2],cstack[sp]);
		sp-=1;

		break;
    case ldiv:
		fprintf(sta,"%5u\tdiv",i);
		SAFE_SPRINTF(cstack[sp],"(%s/%s)",cstack[sp-2],cstack[sp-1]);
		array_strcpy(cstack[sp-2],cstack[sp]);
		sp-=1;

		break;
    case lmod:
		fprintf(sta,"%5u\tmod",i);
		SAFE_SPRINTF(cstack[sp],"(%s MOD %s)",cstack[sp-2],cstack[sp-1]);
		array_strcpy(cstack[sp-2],cstack[sp]);
		sp-=1;			
		break;
		
    case lneg: 
		fprintf(sta,"%5u\tneg",i); 
		SAFE_SPRINTF(cstack[sp],"-%s",cstack[sp-1]);
		stack[sp-1]=-stack[sp-1];
		strcpy(cmd,cstack[sp]);
		array_strcpy(cstack[sp-1],cstack[sp]);
		
		break;
    
    case lptr: 
		fprintf(sta,"%5u\tptr",i); 
		//
		fprintf(prg,"// lptr %s %d %d\n",cstack[sp-1],stack[sp-1],i);
		// get var names
		localvar=locvar[sp-1];
		getvarname(stack[sp-1],name);
		sprintf(cstack[sp-1],"%s",name);
		strcpy(cmd,cstack[sp-1]);
		stp[sp-1]=1;
		break;
    case lnot: fprintf(sta,"%5u\tnot",i); break;
    case laid: 
		fprintf(sta,"%5u\taid",i); 
		localvar=locvar[sp-1];
		getvarname(stack[sp-1],cstack[sp-1]);
		fprintf(prg,"// %s %d %d %d\n",cstack[sp-1],stack[sp-1],i,localvar);		
		
		break;
    case lcid: 
		fprintf(sta,"%5u\tcid",i); 
		stack[sp]=0;
		strcpy(cstack[sp],"id");
		strcpy(cmd,cstack[sp]);
		stp[sp]=1;
		sp++;
		break;
    case lrng: 
		fprintf(sta,"%5u\trng %u",i,mem[i+1]); 
		fprintf(prg,"// UNIMP range %d %d\n",i,mem[i+1]);
		i++; 
		break;
    case ljmp: 
		fprintf(sta,"%5u\tjmp %u",i,mem[i+1]); 
		if(mem[i+1]<i) {
			fprintf(prg,"// jmp[%d]=%d JMP BACK TO %d %d\nEND\n\n",i,jmp[i],i,mem[i+1]);
		}
		i++; 
		break;
    case ljpf:
		fprintf(sta,"%5u\tjpf %u",i,mem[i+1]); 
		
		if(mem[i+1]<i) 
			fprintf(prg,"// %d\nUNTIL",i);
		else {
			if(strlen(cmd)==0) 
				strcpy(cmd,cstack[sp-1]);
			// check if the jpf goes past a jmp

			if(mem[mem[i+1]-2]==ljmp) { 
				if(mem[mem[i+1]-1]<mem[i+1]-2)
					fprintf(prg,"WHILE");
				else {
					fprintf(prg,"// else at %d %d\n",mem[mem[i+1]],mem[i+1]);
					fprintf(prg,"IF");
					els[mem[i+1]]++;
					// end after jpf/jmp
					end[mem[mem[i+1]-1]]++;
				}
			}
			else {
				fprintf(prg,"IF");
				// ifs need an end, set an endstop 
			end[mem[i+1]]++;

			}
		}

		fprintf(prg,"(%s) // if val %d %d %s\n",cmd,i,sp,cstack[sp-1]);

		memset(cmd,0,255);
		i++;
		con=0;
		sp=0;
		break;
    case lfun: 
		fprintf(sta,"%5u\tfun %u",i,mem[i+1]); 
		// calling function


		switch(mem[i+1]) {
			
			case 0: // signal

				sprintf(cmd,"signal(%s,",cstack[sp-2]);

				switch(stack[sp-1]) {
					case 0:
						strcat(cmd,"s_kill");
						break;
					case 1:
						strcat(cmd,"s_wakeup");
						break;
					case 2:
						strcat(cmd,"s_sleep");
						break;
					case 3:
						strcat(cmd,"s_freeze");
						break;
					case 100:
						strcat(cmd,"s_kill_tree");
						break;
					case 101:
						strcat(cmd,"s_wakeup_tree");
						break;
					case 102:
						strcat(cmd,"s_sleet_tree");
						break;
					case 103:
						strcat(cmd,"s_freeze_tree");
						break;
					default:
						strcat(cmd,"0");
						break;
				}
				strcat(cmd,")");
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;

			case 1:
				sprintf(cmd, "key(%s)",keys[stack[sp-1]]);
				strcpy(cstack[sp-1],cmd);
				break;

			case 2: // load_pal
				sprintf(cmd,"load_pal(\"%s\")",(byte*)&mem[mem[7]+stack[sp-1]]);
				strcpy(cstack[sp-1],cmd);

				break;
				
			case 3:
				if(stp[sp-1]) {
					sprintf(cmd,"load_fpg(%s)",cstack[sp-1]);
					strcpy(cstack[sp-1],cmd);
				}
				else {
					sprintf(cmd,"load_fpg(\"%s\")",(byte*)&mem[mem[7]+stack[sp-1]]);
					strcpy(cstack[sp-1],cmd);
				}
				break;
			
			case 4: // start_scroll (6)
				sprintf(cmd,"start_scroll(%s, %s, %s, %s, %s, %s)",cstack[sp-6],cstack[sp-5],cstack[sp-4],cstack[sp-3],cstack[sp-2],cstack[sp-1]);
				sp-=5;
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 5: // stop_scroll
				sprintf(cmd,"stop_scroll(%s)",cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 6: // out_region (2)	
				sprintf(cmd,"out_region(%s,%s)",cstack[sp-2],cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				sp--;
				break;
			case 8:
				sprintf(cmd,"collision(type proc%s)",cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 9: // get_id (1)
				fprintf(prg,"// GET_ID\n");
				sprintf(cmd,"get_id(type proc%s)",cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 12:
				sprintf(cmd,"get_angle(%s)",cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;

			case 15:
				sprintf(cmd,"load_fnt(\"%s\")",(byte*)&mem[mem[7]+stack[sp-1]]);
				strcpy(cstack[sp-1],cmd);
				break;	
			
			case 16: // write (5)
				sprintf(cmd,"write(%d,%d,%d,%d,\"%s\")",stack[sp-5],stack[sp-4],stack[sp-3],stack[sp-2],(byte*)&mem[mem[7]+stack[sp-1]]);
				sp-=4;
				strcpy(cstack[sp-1],cmd);
				break;

			case 17:
				sprintf(cmd,"write_int(%d,%d,%d,%d,&var%d)",stack[sp-5],stack[sp-4],stack[sp-3],stack[sp-2],stack[sp-1]);
				sp-=4;
				strcpy(cstack[sp-1],cmd);
				break;

			case 18:
				if(stack[sp-1]==0)
					sprintf(cmd,"delete_text(all_text)");
				else
					sprintf(cmd,"delete_text(%d)",stack[sp-1]);

				strcpy(cstack[sp-1],cmd);

				break;
			case 20: // unload_fpg (1)
				sprintf(cmd,"unload_fpg(%s)",cstack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;

			case 21: // rand (2)
				sprintf(cmd,"rand(%d,%d)",stack[sp-2],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;

			case 22: // define region (5)
				sprintf(cmd,"define_region(%d,%d,%d,%d,%d)",stack[sp-5],stack[sp-4],stack[sp-3],stack[sp-2],stack[sp-1]);
				sp-=4;
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 24: // put (4)
				sprintf(cmd,"put(%d,%d,%d,%d)",stack[sp-4],stack[sp-3],stack[sp-2],stack[sp-1]);
				sp-=3;
				strcpy(cstack[sp-1],cmd);
				break;

			
			case 25:
				sprintf(cmd,"put_screen(%d,%d)",stack[sp-2],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 27: // map_put(); 5 
				sprintf(cmd,"map_put(%s,%s,%s,%s,%s)",cstack[sp-5],cstack[sp-4],cstack[sp-3],cstack[sp-2],cstack[sp-1]);
				sp-=4;
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 29:
				sprintf(cmd,"get_pixel(%s,%s)",cstack[sp-2],cstack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;

			case 33: // clear_screen
				sprintf(cmd,"clear_screen()");
				sp++;
				strcpy(cstack[sp],cmd);
				break;
			
			case 35: //load				
				sprintf(cmd,"load(\"%s\",offset var%d)",(byte*)&mem[mem[7]+stack[sp-2]],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;

			case 36:
				sprintf(cmd,"set_mode(%d)",stack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 37: // load_pcm (2)
				sprintf(cmd,"load_pcm(\"%s\",%d)",(byte*)&mem[mem[7]+stack[sp-2]],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 39: // sound (3)
				sprintf(cmd,"sound(%s,%s,%s)",getvarref(sp-3),getvarref(sp-2),getvarref(sp-1));
				sp-=2;
				strcpy(cstack[sp-1],cmd);

				break;

			case 40: // stop_sound (1)
				sprintf(cmd,"stop_sound(%d)",stack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 42: // set_fps (2)
				sprintf(cmd,"set_fps(%d,%d)",stack[sp-2],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 54: // start_mode7 (6)
				sprintf(cmd,"start_mode7(%s,%s,%s,%s,%s,%s)",cstack[sp-6],cstack[sp-5],cstack[sp-4],cstack[sp-3],cstack[sp-2],cstack[sp-1]);
				sp-=5;
				strcpy(cstack[sp-1],cmd);
				break;
				
			case 56: // advance (1)
				sprintf(cmd,"advance(%d)",stack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				//sp;
				break;
			
			case 57: // abs(); (1)
				sprintf(cmd,"abs(%d)",stack[sp-1]);
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 58: // fade_on (0)
				sprintf(cmd,"fade_on()");
				sp++;
				strcpy(cstack[sp-1],cmd);
				break;
			
			case 66: // let_me_alone (0)
				sprintf(cmd,"let_me_alone()");
				sp++;
				strcpy(cstack[sp-1],cmd);
				break;

			case 67: // exit (2)
				sprintf(cmd,"exit(\"%s\",%d)",(byte*)&mem[mem[7]+stack[sp-2]],stack[sp-1]);
				sp--;
				strcpy(cstack[sp-1],cmd);
				break;

				
			default: 
				sprintf(cmd,"func(%d)",mem[i+1]);
				strcpy(cstack[sp-1],cmd);
				break;
		}
		strcpy(condstack[con],cmd);
		con++;
		i++; break;
    case lcal: 
		fprintf(sta,"%5u\tcal %u",i,mem[i+1]); 
		sprintf(cmd,"proc%u(",mem[mem[i+1]+1]);
		
		fprintf(prg,"// assigned: %s\n",mem[i+2]==lasi?"yes":"no");
		
		spp=sp;
		while(sp>(mem[i+2]==lasi?1:0)) {
			strcat(cmd,cstack[spp-sp]);
			sp--;
			if(sp>(mem[i+2]==lasi?1:0))
			strcat(cmd,",");
		}
		strcat(cmd,")");
		strcpy(cstack[sp],cmd);
		sp++;
		i++; 
		break;
    
    case lret: 
		fprintf(sta,"%5u\tret",i); 
		fprintf(prg,"\nEND // lret %d\n\n",i);
		if(i&&1)
			i++;
		break;
    case lasp: 
		fprintf(sta,"%5u\tasp",i); 
		fprintf(prg,"%s;\n",cmd);
		memset(cmd,0,255);
		con=0;
		sp--;

		break;
    case lfrm: 
		fprintf(sta,"%5u\tfrm",i); 
		fprintf(prg,"FRAME;\n"); 
		break;
		
    case lcbp: 
		fprintf(sta,"%5u\tcbp %u",i,mem[i+1]); 
	//	fprintf(prg,"// %d args\n",mem[i+1]);
		args=mem[i+1];
		
		if(args==0)
			fprintf(prg,");\n");

		i++; break;
    case lcpa: 
		fprintf(sta,"%5u\tcpa",i); 
		if(args>0) {
			args--;
			localvar=locvar[0];
			getvarname(stack[0],name);
			fprintf(prg,"%s ",name);
			if(args==0)
				fprintf(prg,");\n\n");
				//BEGIN /// lcpa\n\n");
			else 
				fprintf(prg,",");
		}
		sp=0;
		break;
    case ltyp: 
		fprintf(sta,"\n%5u\ttyp %u",i,mem[i+1]); 
		if(mem[i+2]==lfrm) {
			fprintf(prg,"PROGRAM myprg;\n");
			i+=2;
			printglobals();
			printlocal();
		} else {
			fprintf(prg,"\n\nPROCESS proc%u(",mem[i+1]);
		}
		
		i++; break;
    case lpri: 
		fprintf(sta,"%5u\tpri %u",i,mem[i+1]); 
		fprintf(prg,"PRIVATE\n");
		itmp=i;
		i++;
		while(i<(mem[itmp+1]-1)) {
			fprintf(prg,"var%u=%u; // %d\n",mem[6]-1+(i-itmp),mem[i+1],i);
			i++;
		}
		fprintf(prg,"\nBEGIN // lpri\n");	
		sp=0;
		break;
    case lcse: fprintf(sta,"%5u\tcse %u",i,mem[i+1]); i++; break;
    case lcsr: fprintf(sta,"%5u\tcsr %u",i,mem[i+1]); i++; break;
    case lshr: fprintf(sta,"%5u\tshr",i); break;
    case lshl: fprintf(sta,"%5u\tshl",i); break;
    case lipt: fprintf(sta,"%5u\tipt",i); break;
    case lpti: 
		fprintf(sta,"%5u\tpti",i); 
		localvar=locvar[0];
		getvarname(stack[0],cmd);
		strcat(cmd,"++");
		break;
    case ldpt: fprintf(sta,"%5u\tdpt",i); break;

    case lptd: 
		fprintf(sta,"%5u\tptd",i);
		localvar=locvar[0];
    	getvarname(stack[0],cmd);
		strcat(cmd,"++");
		break;		

    case lada: 
		fprintf(sta,"%5u\tada",i); 
		localvar=locvar[sp-2];
		getvarname(stack[sp-2],cstack[sp-2]);
		sprintf(cmd,"%s+=%s",cstack[sp-2],cstack[sp-1]);
		fprintf(prg,"// %s %i\n",cmd,i);
		sp--;
		break;
    case lsua: 
		fprintf(sta,"%5u\tsua",i); 
		localvar=locvar[sp-2];
		getvarname(stack[sp-2],name);
		sprintf(cmd,"%s-=%s",name,cstack[sp-1]);
		break;
    case lmua: fprintf(sta,"%5u\tmua",i); break;
    case ldia: fprintf(sta,"%5u\tdia",i); break;
    case lmoa: fprintf(sta,"%5u\tmoa",i); break;
    case lana: fprintf(sta,"%5u\tana",i); break;
    case lora: fprintf(sta,"%5u\tora",i); break;
    case lxoa: fprintf(sta,"%5u\txoa",i); break;
    case lsra: fprintf(sta,"%5u\tsra",i); break;
    case lsla: fprintf(sta,"%5u\tsla",i); break;
    case lpar: 
		fprintf(sta,"%5u\tpar %u",i,mem[i+1]); 
		fprintf(prg,"// lpar: %d\n",mem[i+1]);
		i++; 
		break;
    case lrtf: fprintf(sta,"%5u\trtf",i); break;
    case lclo: fprintf(sta,"%5u\tclo %u",i,mem[i+1]); i++; break;
    case lfrf: fprintf(sta,"%5u\tfrf",i); break;
    case limp: fprintf(sta,"%5u\timp %u",i,mem[i+1]); i++; break;
    case lext: fprintf(sta,"%5u\text %u",i,mem[i+1]); i++; break;
    case lchk: 
		fprintf(sta,"%5u\tchk",i); 
		fprintf(prg,"// offset %d %s %d %s %s\n",mem[i+2],cstack[sp-1],stack[sp-1],condstack[con-1],cmd);
		getvarname(mem[i+2],name);
		strcat(cstack[sp-1],".");
		strcat(cstack[sp-1],name);
		i+=2;
		fprintf(prg,"// lchk next %d %d\n",i,mem[i+1]);
		if(mem[i+1]==lptr || mem[i+1]==lchk)
			i++;
			
		if(mem[i+1]==ladd)
			i++;

		if(mem[i+1]==lptr || mem[i+1]==lchk)
			i++;
			
		strcpy(cmd,cstack[sp-1]);			
		fprintf(prg,"// cmd is %s\n",cmd);
		fprintf(prg,"// lchk next %d %d\n",i,mem[i+1]);

		break;
		
    case ldbg: fprintf(sta,"%5u\tdbg",i); break;
    default: fprintf(sta,"***"); break;
  } 
}
fflush(sta);
fflush(prg);
fclose(sta);
fclose(prg);
printf("stack: %d\n",sp);
}


#endif
