/*
Description: computes for the R-squared of TSS and simulated output maps.
Input:
1. TSS raster
2. simulated files
Note: Lat-lon points must match

Output:
1. csv of R-squared values for each point in the map


*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<malloc.h>
#include<string.h>

#include "basicFxns.h"

#define points 7496
#define lonid 0
#define latid 1

//#define col_ws 5
#define col_ws 11
#define col_L2 11
#define col_ov 13
#define col_len 14

int* checkValidity(double**,int,int);
int comparePts(double,double,double,double);
double mean(double*,double);
double sd(double*,double,double);
double* svi(double*,double,double,double);
double* svi(double*,double,double,double);
double rsq_out(double*,double*,int);
	
int main(){
	char fileName_rsq[11] = "rsq_L2.csv\0";
	float pi = 3.14159265359;
	
	double **tss;
	double **ws;
	double **ov;
	double **L2;
	
	int *validCheck;//variable to store how many -9999.0 values there are in tss variable
	int compTest=0;
	
	//double **bestVar;
	double **pts_LLDA;	//lat lon 
	double pts_data [points][2];
	
	double rsq=0;
	// double rsq_ws [points];
	// double rsq_ov [points];
	// double rsq_l2 [points];

	double *tmpVar;
	double *tmpTss;
	double tmpVar_ave;
	double tmpTss_ave;
	double tmpVar_sd;
	double tmpTss_sd;
	double *tmpVar_sv;
	double *tmpTss_sv;
	
	double tmpR;

	char fileWISE_buf[70] = "\0";

	char fileName_TSS[16] = "TSS_sampled.csv\0";
	int fileLen_TSS = 0;
	
	char* fileSIMS_dir = "ERA5_TSS_fixTF\\";
	char *tmp;
	char line[4098];
	const char* tok;
	double tmpLon=0,tmpLat=0;
	
	int simFileLen=0;
	int fileLen = 0;
	char** fileList = NULL;
	
	int i=0,j=0,k=0,m=0;
	FILE *file;
	FILE *outFile;
	
	printf("begin\n");
	
	fileLen_TSS = getLenOfFile(fileName_TSS);
	//fileLen_TSS = 20;
	fileLen = getNumFile(fileSIMS_dir);
	tss = read_TSSFiles(fileName_TSS,fileLen_TSS,fileLen);//uncomment me
	printf("tss len: %d,\tsim len: %d\n",fileLen_TSS,fileLen);
	
	// for(i=7400;i<7497;i++){
		// for(j=0;j<30;j++){
			// printf("%d, %d\t %f\n",i,j,tss[i][j]);
		// }
	// }
	
	// return 0;
	// pts_LLDA = read_LLshpFilePts2(fileName_TSS,fileLen_TSS);
	pts_LLDA = read_TSSpts(fileName_TSS,fileLen_TSS);//uncomment me
	
	//read through all files in ERA5_TSS_fixTF folder
	ws = (double**) malloc((fileLen_TSS)*sizeof(double*));//probably -1
	for(i=0;i<fileLen_TSS;i++){
		ws[i] = (double*) malloc(fileLen*sizeof(double));
	}
	
	printf("reading sim files...\n");
	
	fileList = (char**) malloc(fileLen*sizeof(char*));
	fileList = listFiles(fileSIMS_dir,fileList,fileLen);
	
	printf("done reading TSS files\n");
	printf("reading sim files and extracting...\n");
	
	//for(i=0;i<1;i++){	
	for(i=0;i<fileLen;i++){		
		//printf("processing %s\n",fileList[i]);
		strcpy(fileWISE_buf,fileSIMS_dir);
		strcat(fileWISE_buf,fileList[i]);
		strcat(fileWISE_buf,"\0");
		//printf("file: %s\n",fileWISE_buf);
		simFileLen = getLenOfFile(fileWISE_buf);
		//printf("sim file len: %d\n",simFileLen);
		
		file = fopen(fileWISE_buf,"r");
		k=0;
		m=0;
		//while loop is tart of loop for 1 file
		//while(fgets(line,4098,file) && (k<3)){
		while(fgets(line,4098,file) && (k<simFileLen)){
		//for(k=0;k<simFileLen;k++){
			//printf("\ninside file...line: %d\n",k);
			
			tmp = strdup(line);
			j=0;
			for(tok = strtok(line,","); j<=col_ws;j++,tok=strtok(NULL,",")){
				if(k!=0){
					//printf("reading the columns %d\n",j);
					//segment to check where the sim files belong in the real world
					if(j==0){
						
						tmpLon = atof(tok);
						tok=strtok(NULL,",");
						tmpLat = atof(tok);
						//printf("col: %d\t %s\n",j,tok);
						
						// printf("lon: %f, lat: %f\n",tmpLon,tmpLat);
						j++;//need extra increment since I read lon and lat
						
						//loop around 
						//printf("loop again\n");
						compTest=0;
						for(;m<fileLen_TSS-1;m++){
							//printf("m: %d\t\t%d\n",m,i);
							ws[m][i] = -9999.0;
							//printf("passed m+i access\n");
							if(comparePts(pts_LLDA[m][0],pts_LLDA[m][1],tmpLon,tmpLat) == 1){
								compTest=1;
								//m++;
								break;
							}
							//else break;
						}
						//printf("lat lon comp loop end\n");
						if(compTest==0) printf("WARNING..........!!!!!!!!!!!!!!!!!\n");
					}//
					//printf("col: %d\t %s\n",j,tok);
					//read variable here
					if(j==col_ws){
						
						// printf("extracting ws...");
						// if(m==12){
							// printf("m i\t%d\t\t %d\n",m,i);
							// printf("value: %f\n",atof(tok));
						// }
						ws[m][i] = atof(tok);
						if(m==7496){
							printf("m i\t%d\t %d\t\t",m,i);
							printf("ws: %f\n",ws[m][i]);
						}
						m++;
					}
					
				}
			}//end of j-loop
			k++;
		}//end of k-loop
		fclose(file);

	}//end of i-loop or the folder loop
	printf("done printing files\n");

	validCheck = checkValidity(tss,fileLen_TSS,fileLen);
	outFile = fopen(fileName_rsq,"w");
	fprintf(outFile,"lon,lat,r2\n");
	printf("calculating r2\n");
	
	printf("%f\t%f\n",tss[7496][0],ws[7496][0]);
	i=0;
	for(j=0;j<30;j++) printf("%d,%d\tdata: %f\t%f\n",i,j,ws[7496][j],tss[7496][j]);
	
	//return 0;
	//snippet for calculating r2
	for(j=0;j<fileLen_TSS-2;j++){//loop for rows
		//printf("%d\n",j);
		// if(validCheck[j] <20){
			// fprintf(outFile,"%0.3f,%0.3f,\n",pts_LLDA[j][0],pts_LLDA[j][1]);
			// continue;
		// }
		//printf("\n");
		//for(int z=0;z<30;z++) printf("%d\tdata: %f\n",j,ws[j][z]);
		//printf("entering for loop\n");
		for(i=0;i<fileLen;i++){//loop for column
			tmpTss = extractArr(tss,j,fileLen);
			tmpVar = extractArr(ws,j,fileLen);
			
			tmpVar_ave = mean(tmpVar,fileLen);
			tmpTss_ave = mean(tmpTss,fileLen);
			
			tmpVar_sd = sd(tmpVar,tmpVar_ave,fileLen);
			tmpTss_sd = sd(tmpTss,tmpTss_ave,fileLen);
			
			tmpVar_sv = svi(tmpVar,tmpVar_ave,tmpVar_sd,fileLen);
			tmpTss_sv = svi(tmpTss,tmpTss_ave,tmpTss_sd,fileLen);
			
		}//end of i-loop / col-loop
		
		rsq = rsq_out(tmpVar_sv,tmpTss_sv,fileLen);
		
		fprintf(outFile,"%0.3f,%0.3f,%0.3f\n",pts_LLDA[j][0],pts_LLDA[j][1],rsq);
		
	}//end of j-loop / row-loop
	
	fclose(outFile);
	printf("done");
}

double rsq_out(double* svVar,double* svTss,int len){
	double rsq=0;
	int i=0;
	//printf("entered rsq_out\n");
	for(i=0;i<len;i++){
		//printf("i: %d\n",i);
		rsq += svVar[i]*svTss[i];
	}
	rsq/=len-1;
	
	rsq = pow(rsq,2);
	//printf("leaving rsq_out\n");
	return rsq;
}

double* svi(double* data, double mean, double sd, double len){
	int i=0;
	double* sv = (double*) malloc(len*sizeof(double));
	
	for(i=0;i<len;i++){
		if(data[i] == -9999.0) continue;
		sv[i] = (data[i] - mean)/sd;
	}//end of i-loop
	
	return sv;
	
}//end of fxn

double sd(double* data, double mean, double len){
	int i=0;
	double sd=0;
	int counter=0;
	
	for(i=0;i<len;i++){
		if(data[i] == -9999.0) continue;
		sd += pow(data[i] - mean,2);
		counter++;
	}//end of i-loop
	
	sd/=counter;
	sd = pow(sd,0.5);
	
	return sd;
}//end of fxn


double mean(double* data, double len){
	int i=0;
	double ave=0;
	int counter=0;
	
	for(i=0;i<len;i++){
		if(data[i] == -9999.0) continue;
		ave += data[i];
		counter++;
	}//end of i-loop
	
	return ave/counter;
	
}//end of fxn


int comparePts(double tssLon,double tssLat,double ptLon, double ptLat){
	if(tssLon == ptLon){
		if(tssLat == ptLat){
			return 1;
		}
	}
	return 0;
}

//row = fileLen_TSS. col = fileLen
int* checkValidity(double **tss,int row, int col){
	int *validCheck;
	int i=0,j=0;
	int validCounter=0;
	printf("checking validity\n");
	row--;
	validCheck = (int*) malloc(row*sizeof(int));
	// // printf("entering loop\n");
	// printf("rows: %d\n", row);
	for(i=0;i<row;i++){
		
		validCounter=0;
		for(j=0;j<col;j++){
			//printf("i: %d\t%d\t\t%f\n",i,j,tss[i][j]);
			if(tss[i][j] == -9999.0){
				validCounter++;
			}
		}// end of j
		validCheck[i] = validCounter;
	}//end of i
	printf("returning\n");
	return validCheck;
}//end of fxn

