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
double getMaxBattleshipFireRange (double velocityMax)
{

	return (velocityMax * velocityMax) / gravity;

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

		if (t == 0)
		{
		
			bf->escort[i].maxVelocity = 1.0 * BVelocityMax;
			
		} else 
		{
		
			bf->escort[i].maxVelocity = getRandom(bf->escort[i].minVelocity, BVelocityMax);
		}
	}
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

void simulate(const battleField *bf, const char *outputFileName)
{

	double BRangeMax = getMaxBattleshipFireRange(bf -> battleship.maxVelocity);

	int BSunk = 0;
	int ESinkingId = -1;

	//checks escort ships can hit battleship
	for (int i = 0; i < bf->numberEscort; i++)
	{
	
		double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);

		double ERangeMax, ERangeMin;
		getMaxMinEscortshipFireRange(&bf -> escort[i], &ERangeMin, &ERangeMax);

		//check battleship in the range of escortships
		if (distance >= ERangeMin && distance <= ERangeMax)
		{
		
			BSunk = 1;
			ESinkingId = bf -> escort[i].id;
			break;
			
		}
	}

	FILE *file = fopen(outputFileName, "w");
	if (!file) return;
	
	fprintf(file, "Simulation Results\n");

	if(BSunk)
	{
	
		printf("Result:- Battleship was sunk by escort ship. Escortsip id:- %d\n", ESinkingId);
		fprintf(file, "Result: Battleship was sunk\n");
		fprintf(file, "Destroyed by escort Ship Id:- %d\n", ESinkingId);
	
	}else
	{
		int hitCount = 0;
		double totalTime = 0.0; //no reloading

		printf("Result:- Battleship Won\n\n");
		fprintf(file, "Result:- Battleship Won\n");
		fprintf(file, "Battle duration: %.2f seconds\n\n", totalTime);
		fprintf(file, "Escort ships hit by the battleship:-\n");
		fprintf(file, "%-10s %-10s %-15s\n", "Escort ship id", "Time to hit", "Distance");
	
		for (int i = 0; i < bf->numberEscort; i++)
		{
			
			double distance = getDistance(bf-> battleship.x, bf-> battleship.y, bf->escort[i].x, bf-> escort[i].y);
			if(distance <= BRangeMax)
			{
			
				hitCount++;
				printf("Hit escort ship ID %d (distance:- %.2fm)",bf->escort[i].id,distance);
				fprintf(file, "%-10d %-10.2f %-15.2f\n", bf->escort[i].id, 0.0, distance);
			
			}

		}

		printf("Total escort ships destroyed by the Battleship: %d\n", hitCount);
		fprintf(file, "\nTotal escort ships destroyed: %d\n", hitCount);
	}

	fclose(file);
	printf("simulation results saved as '%s'\n", outputFileName);
}

//start of the main funtion 
int main(void)
{

	srand((unsigned int) time (NULL));

	battleField bf;

	battlefieldSetup(&bf, 1000.0, 10, 'U', 100.0);

	savefile(&bf, "battleformation.txt");
	simulate(&bf, "results.txt");

	free(bf.escort);
	return 0;
}
//end of the main function 


