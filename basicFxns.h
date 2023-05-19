/*
Description:
Dump of all the basic function I use for parsing, calculations, dimension conversion, file opening, etc. It's a mess
*/

/*
note from Author:
All read-functions that come after compMode 5 (basically rsq_comp.c)
and other succeeding programs, has a i-1 in the index. All compModes <=5
have the same bug. Since they have the same bug, it all works out fine. I think. It should be noted that any functions that newer functions to be written that will use the first set of read functions must (MUST!) update the old functions to have i-1 index and test for any changes in output.

*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<malloc.h>
#include<string.h>
#include <sys/types.h>
#include <dirent.h>

//float pi = 3.14159265359;

int getNumFile(const char *path)
{
	int num=-2;//start with -2 to account for the "." and ".." files within the dir
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir)	return -1; 

    while ((dp = readdir(dir)) != NULL)
    {
		num++;
    }

    // Close directory stream
    closedir(dir);
	return num;
}

//list files in current directory
char** listFiles(const char *path,char** fileList,int num)
{
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir)	return NULL;
	
	int i=-2;
    while ((dp = readdir(dir)) != NULL)
    {
		if(i<0)	dp->d_name;//skip first two lines
		else{
			fileList[i] = (char*) malloc(26*sizeof(char));
			sprintf(fileList[i],dp->d_name);
			//printf("%s\n",fileList[i]);
		}
		i++;
    }

    // Close directory stream
    closedir(dir);
	return fileList;
}

//list files in current directory
char** listFiles2(const char *path,char** fileList,int num)
{
    struct dirent *dp;
    DIR *dir = opendir(path);

    // Unable to open directory stream
    if (!dir)	{printf("not a dir!!!!\n");return NULL; }
	
	int i=-2;
    while ((dp = readdir(dir)) != NULL)
    {
		if(i<0)	dp->d_name;//skip first two lines
		else{
			fileList[i] = (char*) malloc(33*sizeof(char));
			sprintf(fileList[i],dp->d_name);			
			//printf("%s\n",fileList[i]);
		}
		i++;
    }

    // Close directory stream
    closedir(dir);
	return fileList;
}

int getWindDir(char* windDir){
	
	if(strcmp(windDir,"E") == 0) return 0;
	else if(strcmp(windDir,"ENE") == 0) return 23;
	else if(strcmp(windDir,"NE") == 0) return 45;
	else if(strcmp(windDir,"NNE") == 0) return 68;
	else if(strcmp(windDir,"N") == 0) return 90;
	else if(strcmp(windDir,"NNW") == 0) return 113;
	else if(strcmp(windDir,"NW") == 0) return 135;
	else if(strcmp(windDir,"WNW") == 0) return 158;
	else if(strcmp(windDir,"W") == 0) return 180;
	else if(strcmp(windDir,"WSW") == 0) return 203;
	else if(strcmp(windDir,"SW") == 0) return 225;	
	else if(strcmp(windDir,"SSW") == 0) return 258;
	else if(strcmp(windDir,"S") == 0) return 270;
	else if(strcmp(windDir,"SSE") == 0) return 293;
	else if(strcmp(windDir,"SE") == 0) return 315;
	else if(strcmp(windDir,"ESE") == 0) return 338;
	
	else if(atof(windDir) == 0) return -1;
}//end of fxn



double** read_bathyFilePTs(char* fileName, int fileLen){
	double** bathy;
	int col = 3;
	int i = 0;
	FILE *file;
	file = fopen(fileName, "r");
	
	bathy = (double**) malloc(fileLen*sizeof(double*));
	
    char line[4098];	
	while (fgets(line, 4098, file) && (i < fileLen))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		bathy[i] = (double*) malloc(3*sizeof(double));
	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<3; j++, tok = strtok(NULL, ","))
	    {
	       bathy[i][j] = atof(tok);
	    }

        i++;
    }
	
	return bathy;
}


double** read_LLshpFilePts(char* fileName, int fileLen){
	double** LL;
	int col = 2;
	int i = 0;
	FILE *file;
	file = fopen(fileName, "r");
	
	LL = (double**) malloc(fileLen*sizeof(double*));
	
	
    char line[4098];	
	while (fgets(line, 4098, file) && (i < fileLen))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		if(i!=0)LL[i] = (double*) malloc(2*sizeof(double));
	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<2; j++, tok = strtok(NULL, ","))
	    {
	       if(i!=0)LL[i][j] = atof(tok);
	    }

        i++;
    }
	fclose(file);
	return LL;
}//end of fxn


double** read_TSSpts(char* fileName, int fileLen){
	double** LL;
	int col = 2;
	int i = 0;
	FILE *file;
	file = fopen(fileName, "r");
	printf("FILELEN: %d\n",fileLen);
	LL = (double**) malloc(fileLen*sizeof(double*));
    char line[4098];
	
	while (fgets(line, 4098, file) && (i < fileLen))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		if(i!=0)LL[i-1] = (double*) malloc(2*sizeof(double));
	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<2; j++, tok = strtok(NULL, ","))
	    {
	       if(i!=0)LL[i-1][j] = atof(tok);
		   //printf("value: %f\n",atof(tok));
	    }

        i++;
    }
	fclose(file);
	return LL;
}//end of fxn


double** read_TSSFiles(char* fileName, int fileLen,int size){
	double** tss;
	int i = 0;
	FILE *file;
	file = fopen(fileName, "r");
	
	printf("%s\n", fileName);
	
	tss = (double**) malloc(fileLen*sizeof(double*));
	
	printf("hello\n");
    char line[4098];	
	while (fgets(line, 4098, file) && (i <= fileLen))
    {
		//printf("i: %d\n",i);
		
        char* tmp = strdup(line);
		if(i!=0) tss[i-1] = (double*) malloc(size*sizeof(double));
	    int j = 0;
	    const char* tok;
		    
		for (tok = strtok(line, ","); j<size+2; j++, tok = strtok(NULL, ","))
	    {
			if((i!=0) && (j>1)){
				//printf("%s\n",tok);
				if(tok == NULL || atof(tok) == 0.0){			
					tss[i-1][j-2] = -9999;
					continue;
				}
				tss[i-1][j-2] = atof(tok);
				//printf("%f\n",tss[i-1][j-2]);
			}
			//if(i>=7496 && j<3 ){
			if(i!=0 && i<14 && j>1 && j<3 ){
				//printf("%d, %f\n",i-1,tss[i-1][j-2]);
				
			}
	    }
		
        i++;
    }
	fclose(file);
	return tss;
}//end of fxn

//function in the works
double** readSIMfiles(char* fileName,int fileLen,int col){
	//col variable is 0-based index
	
	double** UV;
	int i = 0;
	int k=0;
	FILE *file;
	file = fopen(fileName, "r");
	
	UV = (double**) malloc(fileLen*sizeof(double*));
	
    char line[4098];	
	while (fgets(line, 4098, file) && (i < fileLen))// maybe add a -1 here too?
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		//if(i!=0)UV[i-1] = (double*) malloc(3*sizeof(double));
	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<col+1; j++, tok = strtok(NULL, ","))
	    {
			if(i!=0 && j==col){
				UV[i-1][j] = atof(tok);
			}
	    }

        i++;
    }
	
	return UV;
}

double** readERAfiles(char* fileName,int fileLen){
	double** UV;
	int col = 2;
	int i = 0;
	FILE *file;
	file = fopen(fileName, "r");
	
	UV = (double**) malloc(fileLen*sizeof(double*));
	
    char line[4098];	
	while (fgets(line, 4098, file) && (i < fileLen))// maybe add a -1 here too?
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		if(i!=0)UV[i] = (double*) malloc(2*sizeof(double));
	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<4; j++, tok = strtok(NULL, ","))
	    {
			if(i!=0){
				if(j>1){
					UV[i][j-2] = atof(tok);
				}
			}
	    }

        i++;
    }
	
	return UV;
}

double** readLLDAfiles(char* fileName,int fileLen){
	double** UV;
	int col = 5;
	int i = 0,j=0;
	FILE *file;
	file = fopen(fileName, "r");
	
	UV = (double**) malloc(fileLen*sizeof(double*));
	
    char line[4098];	
	while (fgets(line, 4098, file) && (i < fileLen))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);
		//printf("i: %d\n",i);
		if(i>0)UV[i-1] = (double*) malloc(5*sizeof(double));
	    j=0;
		const char* tok;
	    for (tok = strtok(line, ","); j<5; j++, tok = strtok(NULL, ","))
	    {
			if(i>0){
				//printf("i-1: %d\n",i-1);
				UV[i-1][j] = atof(tok);
				//printf("%d,%d\t value: %f\n",i-1,j,UV[i-1][j]);
			}
	    }
		//printf("\n");

        i++;
    }
	
	printf("done reading the file...\n");
	
	/*
	for(i=0;i<fileLen;i++){
		for(j=0;j<5;j++){
			printf("%d,%d\n",i,j);
			printf("value: %f\n",UV[i][j]);
		}
		printf("\n");
	}
	*/
	
	return UV;
}


void getWISEWind(char* fileName,int fileLen,double* windSpdFnl, int* windDirFnl){
	
	FILE *fp;
	fp = fopen(fileName, "r");
	double* windSpd = (double*) malloc((fileLen-1)*sizeof(double));
	int* windDir = (int*) malloc((fileLen-1)*sizeof(int));
	char* buf;
	int i=0;
	//double* u = (double*) malloc(fileLen*sizeof(double));
	//double* v = (double*) malloc(fileLen*sizeof(double));
	
	double uComp=0;
	double vComp=0;

	char line[4098];	
	while (fgets(line, 4098, fp) && (i < fileLen+1))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);

	    int j = 0;
	    const char* tok;
		
	    for (tok = strtok(line, ","); j<5; j++, tok = strtok(NULL, ","))
	    {	
			//skip first line (header)
			if(i!=0){
				//skip first two points
				if(j==2){
					windSpd[i-1] = atof(tok);
				}
				if(j==3){
					windDir[i-1] = getWindDir(strdup(tok));
				}
			}//end of i-if
	    }

        i++;
    }//end of while
	fclose(fp);
	
	for(i=0;i<fileLen-1;i++){
		uComp += windSpd[i]*cos(windDir[i]*pi/180);
		vComp += windSpd[i]*sin(windDir[i]*pi/180);
	}
	
	uComp /= fileLen-1;
	vComp /= fileLen-1;
	
	//convertUVtoWSD(uComp,vComp,&windSpdFnl,&windDirFnl);
	
	*windSpdFnl = sqrt(pow(uComp,2) + pow(vComp,2));
	*windDirFnl = 180*atan2(-vComp,-uComp)/pi;
	
	if(*windDirFnl < 0 ) *windDirFnl += 180;
	//printf("spd: %f\tdir: \n",*windSpd,*windDir);
	
}//end of get wind from wise function

void convertUVtoWSD(double U,double V,double* windSpdFnl, int* windDirFnl){
	*windSpdFnl = sqrt(pow(U,2) + pow(V,2));
	*windDirFnl = 180*atan2(-V,-U)/pi;
	
	if(*windDirFnl < 0 ) *windDirFnl += 180;
}

void getWind(char* fileName,int fileLen,double* windSpd, int* windDir){
	FILE *fp;
	fp = fopen(fileName, "r");
	
	int i=0;
	//double* u = (double*) malloc(fileLen*sizeof(double));
	//double* v = (double*) malloc(fileLen*sizeof(double));
	
	double uComp=0;
	double vComp=0;
	
	char line[4098];	
	while (fgets(line, 4098, fp) && (i < fileLen+1))
    {
    	// double row[ssParams->nreal + 1];
        char* tmp = strdup(line);

	    int j = 0;
	    const char* tok;
	    for (tok = strtok(line, ","); j<4; j++, tok = strtok(NULL, ","))
	    {
			//skip first line (header)
			if(i!=0){
				//skip first two points
				if(j==2){
					uComp += atof(tok);
					printf("u: %f\t",atof(tok));
				}
				if(j==3){
					vComp += atof(tok);
					printf("v: %f\n",atof(tok));
				}
			}//end of i-if
	    }

        i++;
    }//end of while
	
	//average u and v
	uComp /= fileLen;
	vComp /= fileLen;
	
	printf("AVERAGE\tu: %f\tv: %f\n",uComp,vComp);
	
	*windSpd = sqrt(pow(uComp,2) + pow(vComp,2));
	*windDir = 180*atan2(-vComp,-uComp)/pi;
	if(*windDir < 0 ) *windDir += 360;
	
	printf("wind speed: %f\tdir: %d\n",*windSpd,*windDir);
	
	//begin converting u and v to spd and dir	
	
	fclose(fp);
}

char *substr(char *string, int position, int length)
{
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
   
   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(1);
   }
 
   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *(string+position-1);      
      string++;  
   }
 
   *(pointer+c) = '\0';
 
   return pointer;
}

int getLenOfFile(char* fileName){
	FILE *file;
	file = fopen(fileName, "r");
	char curChar;
	int i=0;

	while((curChar = fgetc(file)) !=  EOF){
        if(curChar == '\n'){
            i++;
        }
    }
	//printf("length: %d\n",i);
	return i;
}//end of fxn


double* extractArr(double** input,int row,int col){
	double* Arr;
	int i=0;
	Arr = (double*) malloc(col*sizeof(double));
	
	for(i=0;i<col;i++){
		//printf("data: %f\n",input[row][i]);
		Arr[i] = input[row][i];
	}//end of for
	
	return Arr;
	
}//end of fxn


