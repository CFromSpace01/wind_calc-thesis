/*
Description: Main code for running the Orbital Velocity model
*/


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<malloc.h>
#include<string.h>

#include "calc_fetch.h"
#include "basicFxns.h"
#include "modelCode.h"

int compMode = 1;
//mode 0: default simulation
//mode 1: for field data
//mode 2: wind data from file (u,v). ave. points then compute for wind spd and dir
//mode 3: read data from WISE output (parsed from JAVA code)
//mode 4: read data from ERA5. parsed and krigged using JAVA code
//mode 5: process with LLDA stations. wind from ERA5 processed with JAVA code. depth binned from LLDA
//mode 6: process with in-situ stations, depth from in-situ, wind from ERA5 processed with JAVA code
float computeKH(float,float);
int ws_max = 12;
float g = 9.81; //		m/s^2
//double pi = 3.14159265359;

int main(){
	float pi = 3.14159265359;
	
	double pt[2] = {297038,1607383};
	char wd_c[4];
	int wd = 45;//use atoi();
	char ws_c[3];
	int ws = 0;
	double windSpeed = 0;
	
	double TSS = 0,oldTSS=0;
	double sedEntr = 23; // kg/m2s
	double sedDep = 0.00003;
	double deltaT = 1; // seconds
	double Dstar = 0;//dimensionless particle size parameter
	double Dmed = 0.016;//median particle size. unit in mm(?). silt. 1/64.
	double specific_density = 2.5875;//based on rizal geology data from rizal website and some densities of minerals + rocks from some random website.
	double kinematic_viscosity = 0.801;//m2/s kinematic viscosity of water at 30 C since 30C seems to be the approximate temperature based on table of viscosities + Herrera et al.'s vertical profile of Laguna Lake.
	double comb_SS = 0;
	
	//double depth; // m
	double T_c = 9.8; // m/s2
	//double T_max;
	double multip,tetha;
	double Ts,TsMax,H,L,k;
	double orb_velocity,shear_stress,time;
	int timestep;
	
	char fileName_LBounds[22] = "dense_6_nodes_utm.csv\0";
	char fileName_bathyPts[20] = "bathy_2014_llda.csv\0";
	char baseFile[12] = "TSS\\simOut_\0";
	char fileName_outputs[70];
	int fileLen_LBounds = 0;
	int fileLen_bathyPts = 0;
	
	int fileLen = 0;
	char fileName_pts_withTSS[16] = "pts_withTSS.csv\0";
	char fileTSS_out[24] = "field_TSS_out_phd.csv\0";
	char dirBuf[4];
	
	char baseTimeStamp[18] = "baseTimeStamp.csv\0";
	char dateCFS[7] = "YYMMDD\0";
	char fileName_CFS[23] = "\0";
	char fileTSS_CFS_out[19] = "YYMMDD.csv\0";
	char CFS_base_in[13] = "CFS_sum\\CFS_\0";
	double windSpd=0;
	int windDir=0;
	
	char** fileList = NULL;
	char* fileWISE_dir = "WISE_in\\";
	char fileWISE_buf[70] = "\0";
	
	char* fileERA5_dir = "ERA5_united\\";
	
	char* LLDA_krig_dir = "LLDA_krig\\";
	
	char* insitu_depth_dir = "insitu_depth\\";
	
	double** LL;
	double** bathy;
	double** UV;
	double fetch=0;
	double depthBuf=0; // m
	int i=0,j=0,m=0,za=0,n=0;
	
	printf("BEGIN\n");
	
	fileLen_LBounds = getLenOfFile(fileName_LBounds);
	LL = read_LLshpFilePts(fileName_LBounds,fileLen_LBounds);
	
	if(compMode == 0){
		fileLen_bathyPts = getLenOfFile(fileName_bathyPts);
		bathy = read_bathyFilePTs(fileName_bathyPts,fileLen_bathyPts);
		
		for(m=0;m<1;m++){//interate over wind direction 0-315 degrees
			wd = m*45;
			printf("WD: %d\n",wd);
			ws=0;
			for(j=4;ws<ws_max-1;j++){//iterate over windspeed 2-12 m/s/ minimum value of j should be 1
				ws = j*2.0;
				//printf("ws: %d\n",ws);
				strcpy(fileName_outputs,baseFile);
				sprintf(wd_c,"%03d",wd);
				strcat(fileName_outputs,"wd\0");
				strcat(fileName_outputs,wd_c);
				sprintf(ws_c,"%02d",ws);
				strcat(fileName_outputs,"_ws");
				strcat(fileName_outputs,ws_c);
				strcat(fileName_outputs,".csv\0");
				
				FILE *fp;
				fp = fopen(fileName_outputs, "w");
				//fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time\n");
				fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,orb_velocity\n");
				
				for(i=0;i<fileLen_bathyPts;i++){
					pt[0] = bathy[i][0];
					pt[1] = bathy[i][1];
					//depth = bathy[i][2];//get depth from given pt
					runModel(fp,LL,fileLen_LBounds,(double)ws,wd,pt,bathy[i][2]);
				}
				fclose(fp);
			}
		}
		printf("End of compMode 0\n");
	}//end of compMode 0
	
	if(compMode == 1){
		printf("enter compMode == 1\n");
		//fieldComp(LL,fileLen_LBounds);
		
		FILE *fp;
		FILE *outFile;
		
		fp = fopen(fileName_pts_withTSS, "r");
		outFile = fopen(fileTSS_out,"w");
		//fprintf(outFile,"lon,lat,TSS (mg/L),shear stress (N/m2),depth,windSpeed,wd,fetch (km),wave period,TsMax,time,orbital velocity,L/2\n");
		//fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,orb_velocity\n");
		fprintf(fp,"lon,lat,shear stress,depth,TSS mg/L,OV,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,fb\n");
		
		fileLen = getLenOfFile(fileName_pts_withTSS);
		printf("fileLen: %d\n",fileLen);
		
		char line[4098];
		while (fgets(line, 4098, fp) && (i < fileLen))
		{
			// double row[ssParams->nreal + 1];
			char* tmp = strdup(line);
			int j = 0;
			const char* tok;
			for (tok = strtok(line, ","); j<12; j++, tok = strtok(NULL, ","))
			{
				//if current line != first line. //this basically skips the header
				if(i==0) break;
				if(i!=0){
					if(j==2) pt[1] = atof(tok);//lat
					if(j==3) pt[0] = atof(tok);//lon
					if(j==8) windSpeed = atof(tok);
					if(j==9){
						strcpy(dirBuf,"\0\0\0\0");
						strcpy(dirBuf,tok);
						wd = getWindDir(dirBuf);
					}
					if(j==10) depthBuf = atof(tok);
				}
			}//end of search line

			//compute TSS here
			if(i!=0){
				if(wd == -1){
					printf("uncomputable\n");
					fprintf(outFile,"%d,uncomputable\n",i);
				}
				else if(wd >-2){
					runModel(outFile,LL,fileLen_LBounds,windSpeed,wd,pt,depthBuf);
				}
				else{
					printf("BUG in fieldLib.h. wind speed error");
					exit(0);
				}//end of else
			}//end of if
			i++;
		}//end of while //end of search file
		
		fclose(outFile);
		fclose(fp);

		printf("End of compMode 1\n");
	}//end of compMode 1
	
	
	if(compMode == 2){
		fileLen = getLenOfFile(baseTimeStamp);
		FILE *fileDate = fopen(baseTimeStamp, "r");
		
		fileLen_bathyPts = getLenOfFile(fileName_bathyPts);
		bathy = read_bathyFilePTs(fileName_bathyPts,fileLen_bathyPts);
		
		i=0;
		char line[20];
		while (fgets(line, 4098, fileDate) && (i < fileLen)){
		//for(i=0;i<fileLen;i++){
			char* tmp = strdup(line);
			const char* tok;
			tok = strtok(line,"\n");
			
			strcpy(dateCFS,tok);
			//printf("%s\n",dateCFS);
			
			strcpy(fileName_CFS,CFS_base_in);
			strcat(fileName_CFS,dateCFS);
			strcat(fileName_CFS,".csv\0");
			printf("%s\n",fileName_CFS);
			
			getWind(fileName_CFS,getLenOfFile(fileName_CFS),&windSpd,&wd);
			
			//ready file for output
			strcpy(fileName_outputs,"CFS_sum_out\\TSS_CFSout_\0");
			strcat(fileName_outputs,dateCFS);
			sprintf(wd_c,"%03d",wd);
			strcat(fileName_outputs,"_wd\0");
			strcat(fileName_outputs,wd_c);
			sprintf(ws_c,"%02d",(int)windSpd);
			strcat(fileName_outputs,"_ws");
			strcat(fileName_outputs,ws_c);
			strcat(fileName_outputs,".csv\0");
			
			printf("%s\n",fileName_outputs);
			FILE *fp;
			fp = fopen(fileName_outputs, "w");
			fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time\n");
			
			//run Model here
			for(j=0;j<fileLen_bathyPts;j++){
				pt[0] = bathy[j][0];
				pt[1] = bathy[j][1];
				runModel(fp,LL,fileLen_LBounds,windSpd,wd,pt,bathy[j][2]);
			}
			
			fclose(fp);
			
			tok = strtok(NULL,",");
			i++;
		}//end of sim loop (while)
		
		fclose(fileDate);
		//outFile = fopen(fileTSS_CFS_out,"w");
		
	}//end of mode 2
	
	
	if(compMode == 3){
		printf("compMode 3...reading WISE wind\n");
		fileLen = getNumFile(fileWISE_dir);
		fileList = (char**) malloc(fileLen*sizeof(char*));
		fileList = listFiles(fileWISE_dir,fileList,fileLen);

		if(getNumFile > 0){
			fileLen_bathyPts = getLenOfFile(fileName_bathyPts);
			bathy = read_bathyFilePTs(fileName_bathyPts,fileLen_bathyPts);
		}
		
		//loop around filenames and extract wind information
		for(i=0;i<getNumFile(fileWISE_dir);i++){
			printf("processing %s\n",fileList[i]);
			strcpy(fileWISE_buf,fileWISE_dir);
			strcat(fileWISE_buf,fileList[i]);
			strcat(fileWISE_buf,"\0");
			
			fileLen = getLenOfFile(fileWISE_buf);
			
			getWISEWind(fileWISE_buf,fileLen,&windSpd,&windDir);
			
			//fileName_outputs
			strcpy(fileName_outputs,"WISE_TSS\\WISE_");
			strcat(fileName_outputs,substr(fileList[i],1,8));
			strcat(fileName_outputs,"_TSS.csv");
			
			FILE *fp;
			fp = fopen(fileName_outputs, "w");
			fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time\n");
		
			//run model here
			for(j=0;j<fileLen_bathyPts;j++){
				pt[0] = bathy[j][0];
				pt[1] = bathy[j][1];
				runModel(fp,LL,fileLen_LBounds,windSpd,windDir,pt,bathy[j][2]);
			}//end of model
			
			fclose(fp);
		}//end of iteration over WISE files
	}//end of mode 3
	
	if(compMode == 4){
		printf("compMode 4...reading ERA5 wind\n");
		fileLen = getNumFile(fileERA5_dir);
		fileList = (char**) malloc(fileLen*sizeof(char*));
		fileList = listFiles(fileERA5_dir,fileList,fileLen);

		if(getNumFile > 0){
			fileLen_bathyPts = getLenOfFile(fileName_bathyPts);
			bathy = read_bathyFilePTs(fileName_bathyPts,fileLen_bathyPts);
		}
		
		//loop around filenames and extract wind information
		for(i=0;i<getNumFile(fileERA5_dir);i++){
			printf("processing %s\n",fileList[i]);
			strcpy(fileWISE_buf,fileERA5_dir);
			strcat(fileWISE_buf,fileList[i]);
			strcat(fileWISE_buf,"\0");
			
			printf("filename_in: %s\n",fileWISE_buf);
			fileLen = getLenOfFile(fileWISE_buf);
			printf("fileLen: %d\n",fileLen);
			//getWISEWind(fileWISE_buf,fileLen,&windSpd,&windDir);
			UV = readERAfiles(fileWISE_buf,fileLen);
			
			//fileName_outputs
			strcpy(fileName_outputs,"ERA5_TSS_phd\\");
			strcat(fileName_outputs,substr(fileList[i],1,11));
			strcat(fileName_outputs,"_TSS.csv");
			printf("file outputs: %s\n\n",fileName_outputs);
			
			FILE *fp;
			fp = fopen(fileName_outputs, "w");
			fprintf(fp,"lon,lat,shear stress,depth,TSS mg/L,OV,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,fb\n");
			
			//run model here
			for(j=0;j<fileLen_bathyPts;j++){
				convertUVtoWSD(UV[j][0],UV[j][1],&windSpd,&windDir);
				
				pt[0] = bathy[j][0];
				pt[1] = bathy[j][1];
				runModel(fp,LL,fileLen_LBounds,windSpd,windDir,pt,bathy[j][2]);
			}//end of model
			fclose(fp);
			
		}//end of iteration over WISE files
	}//end of mode 4
	
	
	if(compMode == 5){
		printf("compMode 5...for LLDA points\n");
		fileLen = getNumFile(LLDA_krig_dir);
		printf("# of files: %d\n",fileLen);
		fileList = (char**) malloc(fileLen*sizeof(char*));
		fileList = listFiles2(LLDA_krig_dir,fileList,fileLen);
		
		printf("before loop\n");
		
		//loop around filenames and extract wind information
		for(i=0;i<getNumFile(LLDA_krig_dir);i++){
			printf("processing %s\n",fileList[i]);
			strcpy(fileWISE_buf,LLDA_krig_dir);
			strcat(fileWISE_buf,fileList[i]);
			strcat(fileWISE_buf,"\0");
			
			printf("filename_in: %s\n",fileWISE_buf);
			fileLen = getLenOfFile(fileWISE_buf);
			printf("fileLen: %d\n",fileLen);
			//getWISEWind(fileWISE_buf,fileLen,&windSpd,&windDir);
			
			UV = readLLDAfiles(fileWISE_buf,fileLen);
			
			//fileName_outputs
			strcpy(fileName_outputs,"LLDA_krig_out\\");
			strcat(fileName_outputs,substr(fileList[i],1,16));
			strcat(fileName_outputs,"_TSS.csv");
			printf("file outputs: %s\n\n",fileName_outputs);
			
			FILE *fp;
			fp = fopen(fileName_outputs, "w");
			fprintf(fp,"lon,lat,TSS mg/L,shear stress,depth,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,orb_velocity,fb\n");
			
			//run model here
			for(j=0;j<fileLen-1;j++){
				convertUVtoWSD(UV[j][3],UV[j][4],&windSpd,&windDir);
				
				pt[0] = UV[j][1];
				pt[1] = UV[j][0];
				
				runModel(fp,LL,fileLen_LBounds,windSpd,windDir,pt,UV[j][2]);
			}//end of model
			fclose(fp);
			
		}//end of iteration over WISE files
	}//end of mode 5
	
	if(compMode == 6){
		printf("compMode 6...for in-situ depth points\n");
		fileLen = getNumFile(insitu_depth_dir);
		printf("# of files: %d\n",fileLen);
		fileList = (char**) malloc(fileLen*sizeof(char*));
		fileList = listFiles2(insitu_depth_dir,fileList,fileLen);
		
		printf("before loop\n");
		
		//loop around filenames and extract wind information
		for(i=0;i<getNumFile(insitu_depth_dir);i++){
			printf("processing %s\n",fileList[i]);
			strcpy(fileWISE_buf,insitu_depth_dir);
			strcat(fileWISE_buf,fileList[i]);
			strcat(fileWISE_buf,"\0");
			
			printf("filename_in: %s\n",fileWISE_buf);
			fileLen = getLenOfFile(fileWISE_buf);
			printf("fileLen: %d\n",fileLen);
			//getWISEWind(fileWISE_buf,fileLen,&windSpd,&windDir);
			
			UV = readLLDAfiles(fileWISE_buf,fileLen);
			
			//fileName_outputs
			strcpy(fileName_outputs,"insitu_depth_out_phd\\");
			strcat(fileName_outputs,substr(fileList[i],1,16));
			strcat(fileName_outputs,"_TSS.csv");
			printf("file outputs: %s\n\n",fileName_outputs);
			
			FILE *fp;
			fp = fopen(fileName_outputs, "w");
			fprintf(fp,"lon,lat,shear stress,depth,TSS mg/L,orb_velocity,ws,wd,fetch (km),wave period,TsMax,time,L/2,H,fb\n");
			
			//run model here
			for(j=0;j<fileLen-1;j++){
				convertUVtoWSD(UV[j][3],UV[j][4],&windSpd,&windDir);
				
				pt[0] = UV[j][1];
				pt[1] = UV[j][0];
				
				runModel(fp,LL,fileLen_LBounds,windSpd,windDir,pt,UV[j][2]);
			}//end of model
			fclose(fp);
			
		}//end of iteration over WISE files
	}//end of mode 6
	
	
	printf("done\n");
	
}//end of main


float computeKH(float T, float depth){
	float x=0;
	float y=0;
	float t=0;
	float w=0;
	w = 2*pi/T;
	
	x = w * 2 * depth/g;
	if(x<1){
		y = sqrt(x);
	}
	else{
		y = x;
	}
	
	t = tanh(y);
	y = y- ( ( y*t -x) / (t+y *(1-pow(t,2))));
	t = tanh(y);
	y = y- ( ( y*t -x) / (t+y *(1-pow(t,2))));
	t = tanh(y);
	y = y- ( ( y*t -x) / (t+y *(1-pow(t,2))));
	return y;
}