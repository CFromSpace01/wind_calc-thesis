/*
Description:
Contains calculations for the model
*/

/*#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include<malloc.h>
#include<string.h>*/


void runModel(FILE *fp,double **LL,int fileLen_LBounds, double ws,int wd,double pt[2],double depth){
	float pi = 3.14159265359;
	float g = 9.81; //		m/s^2
	
	double TSS = 0,oldTSS=0;
	double sedEntr = 35; // kg/m2s by Hawley and Lesht (1992) maybe?
	double sedDep = 0.000106;// 0.00003 m/s as observed by Bailey and Hamilton (1997) for Thompson Lake
	double deltaT = 1; // seconds
	double Dstar = 0;//dimensionless particle size parameter
	double Dmed = 0.0156;//median particle size. unit in mm(?). silt. 1/64.
	double specific_density = 2.5875;//based on rizal geology data from rizal website and some densities of minerals + rocks from some random website.
	double kinematic_viscosity = 0.801;//m2/s kinematic viscosity of water at 30 C since 30C seems to be the approximate temperature based on table of viscosities + Herrera et al.'s vertical profile of Laguna Lake.
	double comb_SS = 0;
	
	double T_c = 9.8; // m/s2
	//double T_max;
	double multip,tetha;
	double Ts,TsMax,H,L,k;
	double orb_velocity,shear_stress,time;
	double friction,ks,maxWaveAmp;
	double d50 = 0.0000156;// 7.6 um or 0.0000076 m. bottom bounds for fine silt
	int timestep;

	double fetch=0;
	int fb = 0;
	Dmed = d50;
	if(depth < 0.1) return;
	/*//USELESS code
	T_max = T_c*(sqrt(depth/g));
	//L = T_max * pow(g*depth,1/2);
	L = g*(pow(T_max,2))/(2*pi);
	fb = L/2;
	*/////////////////////////////
	
	//calculate fetch for given pt
	fetch = getFetch(LL,fileLen_LBounds,wd,pt);
	if(fetch <= 100) return;
	//parts of a function (tmp variables)
	multip = (1.2*2*pi*ws)/g;
	tetha = 0.077*pow(g*fetch/(pow(ws,2)),0.25);
	//wave period
	Ts = multip*tanh(tetha);
	//
	TsMax = 9.78*pow((depth/g),0.5);
	
	if(Ts > TsMax) Ts = TsMax;
	
	//parts of a function (tmp variables)
	multip = (0.283*pow(ws,2))/g;
	tetha = 0.0125*pow(g*fetch/pow(ws,2),0.42);
	
	//wave height
	H = multip*tanh(tetha);
	//wave length
	L = g*(pow(Ts,2))/(2*pi);
	//wave number
	k = 2*pi/L;
	if(L/2 > depth) fb = 1;
	//orbital velocity
	orb_velocity = (H*pi)/(Ts*sinh(k*depth));
	//printf("OV: %0.3f\n",orb_velocity);
	//inserted friction factor in shear stress calc 19/07/03
	//by Tanaka 1994 and was used by Suntoyo, 2016
	friction = 1.0;
	//friction = exp(-7.0) + exp(8.07 * pow((depth*2*pi)/(Ts*orb_velocity),0.1));
	//friction = 1.39 * pow((depth*2*pi)/(Ts*orb_velocity),0.1);
	
	ks = 2.5 * d50;
	maxWaveAmp = H/(2*sinh(k*depth));
	friction = exp((5.213*pow(ks/maxWaveAmp,0.194)) - 5.977);
	
	//shear stress
	shear_stress = 997*friction*pow(orb_velocity,2)/2;
	//shear_stress = friction*H*997000* pow(0.00000001*pow(2*pi/Ts,3),0.5)/(2*sinh(k*depth));
	if(shear_stress > 1000000) return;
	
	//time it takes to achieve max height(?)
	time = 77.23*pow(fetch/1000,0.67)/(pow(ws,0.34)*pow(9.81,0.33));
	/*
	//calculate TSS after 1 hr
	oldTSS = 0;
	//for(timestep=1;timestep<2;timestep++){
	for(timestep=1;timestep<3601;timestep++){
	//for(timestep=1;timestep<time;timestep++){
		TSS = (sedEntr/depth)*(shear_stress/1000)*deltaT
			+ (oldTSS * exp(-sedDep * deltaT/depth));
		oldTSS = TSS;
	}
	
	if(TSS > 400000) TSS = 400000;
	*/
	
	//TSS calculation from Lou et al. (2000)
	//comb_SS = (shear_stress - 0.0630) / shear_stress;
	comb_SS = shear_stress;
	if(comb_SS <= 0) comb_SS = 0;
	Dstar = Dmed * pow((specific_density - 1)*g/pow(kinematic_viscosity*pow(10,-6),2),1/3);
	Dstar = Dmed * pow((specific_density - 1)*g/pow(kinematic_viscosity,2),1/3);
	TSS = 0.015 * (Dmed/depth)*pow(comb_SS,1.5)/pow(Dstar,0.3);
	
	//printf("%.3f,%.3f,%.3f,%.3f,%d,%d,%.3f,%.3f\n",pt[0],pt[1],shear_stress,depth,ws,wd,fetch/1000,time,L/2);
	fprintf(fp,"%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",pt[0],pt[1],shear_stress,depth,TSS,orb_velocity,ws,wd,fetch/1000,Ts,TsMax,time,L/2,H,fb);
	//fprintf(fp,"%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",pt[0],pt[1],shear_stress,depth,TSS,orb_velocity,ws,wd,fetch/1000,Ts,TsMax,time,L/2,H,fb);
	//TSS needs to be multiplied by 1000 for unit conversion from kg/m3 to mg/L
}