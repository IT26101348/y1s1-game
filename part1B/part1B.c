//this is naval simulation part 1 A simple version 
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define pi 3.1415 
#define gravity 9.81 //defining the gravity value 

const double impactPowers[5] = {0.08, 0.06, 0.07, 0.05, 0.04};
const char* escortTypes[5] = {"EA", "EB", "EC", "ED", "EE"};
const double angleRange[5] = {20.0, 30.0, 25.0, 50.0, 70.0};

typedef struct 
{
	double x;
	double y;
}point;

//escortship stuct 
typedef struct {
	
	int id; //ship id
	int typeIndex; //ship type index
	char typeName[10]; //ship type name
	double impactPower;
	double minAngle;
	double angleRange;
	double maxVelocity;
	double minVelocity;
	double x; //x position 
	double y; //y position 
	int isDestroyed; //to check distroyed or not 

} escortShip;

//battleship struct 
typedef struct {

	int type; //ship type
	double maxVelocity;
	double x; //x position 
	double y; //y position 

} battleShip;

//battlefield struct 
typedef struct {

	double canvasSize;
	battleShip battleship;
	int numberEscort;
	escortShip *escort;

} battleField;

//genarate random numbers within the given parameters 
double getRandom (double min, double max)
{
	
	return min + ((double)rand() / RAND_MAX) * (max - min);

}

//finds distance between two points
double getDistance (double x1, double x2, double y1, double y2)
{

	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1) * (y2 - y1));

}

//converts degress to radians
double degToRad (double deg)
{

	return deg * (pi / 180);

}

//finds the max firing range of a given battleship
double getMaxBattleshipFireRange (double velocityMax, double thetaMin)
{
	double thetaBest = 45.0;
	if (45.0 < thetaMin)
	{
		thetaBest = thetaMin;
	
	}
	return (velocityMax * velocityMax * sin(degToRad(2.0 * thetaBest))) / gravity;

}

//find the max and min firing range of a given escortship
double getMaxMinEscortshipFireRange (const escortShip *escort, double *rangeMin, double *rangeMax )
{

	double thetaH = escort -> minAngle + escort -> angleRange;
	double thetaL = escort -> minAngle;

	*rangeMin = (escort -> minVelocity * sin( degToRad ( 2.0 * thetaL ))) / gravity; //finds minimus range 
	
	//finds which angle is the closest one to the 45
	double thetabest = 45.0;
	if ( 45.0 < thetaL )
	{
		thetabest = thetaL;
	} else if ( 45.0 > thetaH)
	{
		thetabest = thetaH;
	}

	*rangeMax = ( escort -> maxVelocity * escort -> maxVelocity * sin( degToRad(2.0 * thetabest))) / gravity;
}

//battlefiled setup 
void battlefieldSetup(battleField *bf, double canvasSize, int escortNumber, char BShipType, double BVelocityMax )
{

	bf->canvasSize = canvasSize;
	bf->numberEscort = escortNumber;
	bf->battleship.type = BShipType;
	bf->battleship.maxVelocity = BVelocityMax;
	bf->battleship.x = getRandom(0.0, canvasSize);
	bf->battleship.y = getRandom(0.0, canvasSize);

	bf->escort = (escortShip *)malloc(sizeof(escortShip) * escortNumber);

	for (int i = 0; i < escortNumber; i++)
	{
	
		int t = rand() % 5;
		bf -> escort[i].id = i;
		bf -> escort[i].typeIndex = t;
		strcpy(bf->escort[i].typeName, escortTypes[t]);
		bf->escort[i].impactPower = impactPowers[t];
		bf->escort[i].angleRange = angleRange[t];
		bf->escort[i].minAngle = getRandom(0.0, 30.0);
		bf->escort[i].minVelocity = getRandom(0.0, BVelocityMax * 0.5);
		bf->escort[i].x = getRandom(0.0, canvasSize);
		bf->escort[i].y = getRandom(0.0, canvasSize);
		bf->escort[i].isDestroyed = 0;

		if (t == 0)
		{
		
			bf->escort[i].maxVelocity = 1.0 * BVelocityMax;
			
		} else 
		{
		
			bf->escort[i].maxVelocity = getRandom(bf->escort[i].minVelocity, BVelocityMax);
		}
	}
}

void resetEscortShip(battleField *bf)
{
	for (int i = 0; i < bf -> numberEscort; i++)
	{
		bf->escort[i].isDestroyed = 0;
	}
}	

point* generatePath(int k, double canvasSize)
{
	point *path = (point *)malloc(sizeof(point)*k);

	for (int i = 0; i < k; i++)
	{
		path[i].x = getRandom(0.0, canvasSize);
		path[i].y = getRandom(0.0, canvasSize);
	}

	return path;
}
//save initial conditions on a save file 
void savefile (const battleField *bf, const char *filename)
{
	
	FILE *file = fopen(filename , "w");
	if (!file) return;

	fprintf(file, "Initial battlefield conditions \n\n");
	fprintf(file, "Canvas size:- %.2f \n", bf->canvasSize);
	fprintf(file, "Battleship type:- %c , Position:- (%.2f , %.2f) , Max shell speed:- %.2f \n\n", bf->battleship.type, bf->battleship.x, bf->battleship.y , bf->battleship.maxVelocity);
	
	fprintf(file, "%-5s %-6s %-18s %-12s %-12s %-12s %-12s %-12s\n", "ID", "Type", "Position (x,y)", "ImpactPower", "Min Angle", "Angle Range", "V_min", "V_max");

	for (int i = 0; i < bf-> numberEscort; i++)
	{
	
		escortShip *es = &bf->escort[i];
		fprintf(file, "%-5d %-6s (%.2f, %.2f)    %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f\n",
                es->id, es->typeName, es->x, es->y, es->impactPower, es->minAngle, es->angleRange, es->minVelocity, es->maxVelocity);

	}

	fclose(file);
	
}

void simulate1(battleField *bf,point *path, int k, const char *outputFileName)
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if(!file) return;

	fprintf(file, "Simulation 1 results");

	int BSunk = 0;

	for (int step = 0; step < k; step++)
	{
		bf->battleship.x = path[step].x;
		bf->battleship.y = path[step].y;

		fprintf(file, "step %d / %d. Battleship position (%.2f , %.2f) \n", step + 1, k, bf->battleship.x, bf-> battleship.y);
		
		//escortships check if they can hit the battleship
		for (int i = 0; i < bf->numberEscort; i++)
		{
			if(bf->escort[i].isDestroyed) continue;
			
			double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			double ERangeMin, ERangeMax;
			
			getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);

			if(distance >= ERangeMin & distance <= ERangeMax)
			{
				BSunk = 1;
				fprintf(file, "Result: Battleship was sunk by Escort Ship ID %d at step %d\n", bf->escort[i].id, step + 1);
				break;
			}
		}

		if(BSunk) break;
		
		//battleship attacks active escort ships in the range
		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
		int hitInStep = 0;

		for (int i = 0; i < bf->numberEscort; i++)
		{
			if(bf->escort[i].isDestroyed) continue;
			
			double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);

			if (distance <= BRangeMax)
			{
				bf->escort[i].isDestroyed = 1;
				hitInStep++;
				fprintf(file, "Hit escort ship ID %d (distance: %.2fm)\n", bf->escort[i].id, distance);
			}

		}

		if (hitInStep == 0) fprintf(file, "No Escort Ships in range at step %d.\n", step + 1);
		fprintf(file, "\n");
	}

	if(BSunk)
	{
		printf("Simulation 1:- battleship was sunk.\n");
	} else
	{
		printf("Simulation 1: Battleship survived all %d steps.\n",k);
	}

	fclose(file);
	printf("Simulation 1 results savs as '%s' \n", outputFileName);
}

//simulation 2 

void simulate2(battleField *bf, point *path, int k, int t, double minTheta, const char *outputFileName)
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if (!file) return;

	fprintf(file, "Simulation 2 results. \n");
	fprintf(file, "Jammed angle limit:- %.2fdeg\n\n", minTheta);
	int BSunk = 0;

	for (int step = 0; step < k; step++)
	{
		bf->battleship.x = path[step].x;
		bf->battleship.y = path[step].y;

		int isJammed = (step >= t);
		double activeMinTheta = isJammed ? minTheta : 0.0;

		fprintf(file, "Step %d / %d | Pos: (%.2f, %.2f) | Status: %s \n", step + 1, k, bf->battleship.x, bf->battleship.y, isJammed ? "GUN JAMMED" : "NORMAL");

		//escort ships check is the battleship in the range or not
		for (int i = 0; i < bf->numberEscort; i++)
		{
			if (bf->escort[i].isDestroyed) continue;

			double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			double ERangeMin, ERangeMax;
			getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);

			if (distance >= ERangeMin && distance <= ERangeMax)
			{
				BSunk = 1;
				fprintf(file, "Result:- Battleship was sunk by the %d escort ship at step %d\n ",bf->escort[i].id, step + 1);
				break;
			}
		}
	
		if (BSunk) break;

		//battleship attacks using remaing range
		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, activeMinTheta);
		int hitInStep = 0;

		for (int i = 0; i < bf-> numberEscort; i++)
		{
			if(bf->escort[i].isDestroyed) continue;
			double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			if (distance <= BRangeMax)
			{
				bf->escort[i].isDestroyed = 1;
				hitInStep++;
				fprintf(file, "Hit escort ship ID %d at %.2fm distance.\n",bf->escort[i].id, distance);

			}
		}
		if (hitInStep == 0 ) fprintf(file,"No escort ships ine the step of %d\n",step + 1);
		fprintf(file, "\n");

	}
	if (BSunk)
	{
		printf("Simulation 2:- battleship was sunk.\n");
	} else 
	{
		printf("Simulation 2:- Battleeship survied all %d steps.\n",k);
	}

	 fclose(file);
	 printf("Simulation 2 results saved as '%s'\n",outputFileName);
}

//start of the main funtion 
int main(void)
{

	srand((unsigned int) time (NULL));

	battleField bf;

	battlefieldSetup(&bf, 1000.0, 10, 'U', 100.0);

	int k = 5; //path points 
	int t = 2; //steps before gun jamed
	double mintheta = 25.0; //jammed angle restriction (0<minTheta<30)

	point *path = generatePath(k, bf.canvasSize);
				

	savefile(&bf, "battleformation.txt");
	simulate1(&bf, path, k, "results_simulation1.txt");
	simulate2(&bf, path, k, t, mintheta, "results_simulation2.txt");

	free(path);
	free(bf.escort);
	return 0;
}
//end of the main function 


