//this is naval simulation part 2A  
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
	double health;
	double startx;
	double starty;
	double cumulativeImpact; // Total damage percentage 
	double reloadTime; //reloading time

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
double getDistance (double x1, double y1, double x2, double y2)
{

	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1) * (y2 - y1));

}

//converts degress to radians
double degToRad (double deg)
{

	return deg * (pi / 180);

}

double getMaxBattleshipFireRange(double velocityMax, double thetaMin)
{
	double thetaBest = 45.0;
	if (45.0 < thetaMin) thetaBest = thetaMin;
	return (velocityMax * velocityMax * sin(degToRad(2.0 * thetaBest))) / gravity;
}


//find the max and min firing range of a given escortship
void getMaxMinEscortshipFireRange (const escortShip *escort, double *rangeMin, double *rangeMax )
{

	double thetaH = escort -> minAngle + escort -> angleRange;
	double thetaL = escort -> minAngle;

	*rangeMin = (escort -> minVelocity * escort->minVelocity * sin( degToRad ( 2.0 * thetaL ))) / gravity; //finds minimus range 
	
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
void battlefieldSetup(battleField *bf, double canvasSize, int escortNumber, char BShipType, double BVelocityMax, double reloadTime )
{

	bf->canvasSize = canvasSize;
	bf->numberEscort = escortNumber;
	bf->battleship.type = BShipType;
	bf->battleship.maxVelocity = BVelocityMax;
	bf->battleship.x = getRandom(0.0, canvasSize);
	bf->battleship.y = getRandom(0.0, canvasSize);
	bf->battleship.startx = bf->battleship.x;
	bf->battleship.starty = bf->battleship.y;
	bf->battleship.health = 1.0;
	bf->battleship.cumulativeImpact = 0.0;
	bf->battleship.reloadTime = reloadTime;

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
		
			bf->escort[i].maxVelocity = 1.2 * BVelocityMax;
			
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
	bf->battleship.x = bf->battleship.startx; // Restore position
    	bf->battleship.y = bf->battleship.starty;
	bf->battleship.health = 1.0;
	bf->battleship.cumulativeImpact = 0.0;
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
	fprintf(file, "Battleship type:- %c , Position:- (%.2f , %.2f) , Max shell speed:- %.2f, Reload time: %.2fs \n\n", bf->battleship.type, bf->battleship.x, bf->battleship.y , bf->battleship.maxVelocity, bf->battleship.reloadTime);
	
	fprintf(file, "%-5s %-6s %-18s %-12s %-12s %-12s %-12s %-12s\n", "ID", "Type", "Position (x,y)", "ImpactPower", "Min Angle", "Angle Range", "V_min", "V_max");

	for (int i = 0; i < bf-> numberEscort; i++)
	{
	
		escortShip *es = &bf->escort[i];
		fprintf(file, "%-5d %-6s (%.2f, %.2f)    %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f\n",
                es->id, es->typeName, es->x, es->y, es->impactPower, es->minAngle, es->angleRange, es->minVelocity, es->maxVelocity);

	}

	fclose(file);
	
}

int getTargetAttackOrder(const battleField *bf, double BRangeMax, int *targetIndices)
{
	int count = 0;
	for (int i = 0; i < bf->numberEscort; i++)
	{
		if (!bf->escort[i].isDestroyed)
		{
			double dist = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			if (dist <= BRangeMax)
			{
				targetIndices[count++] = i;
			}
		}
	}


	// Sort targets by highest impactPower first to minimize damage to Battleship
    	for (int i = 0; i < count - 1; i++) 
	{
        	for (int j = i + 1; j < count; j++) 
		{
            		if (bf->escort[targetIndices[i]].impactPower < bf->escort[targetIndices[j]].impactPower) 
			{
                		int temp = targetIndices[i];
                		targetIndices[i] = targetIndices[j];
                		targetIndices[j] = temp;
            		}
       	 	}
    	}
	return count;
}

void simulatePart1A (battleField *bf, const char *outputFileName)
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if(!file) return;

	fprintf(file, "Simulation 1 results (Stationary, Single Hit Sinks Battleship)\n");
	fprintf(file, "Battleship Position: (%.2f, %.2f)\n\n", bf->battleship.x, bf->battleship.y);
	fprintf(file, "Battleship Reload Time (T_B^q): %.2fs\n\n", bf->battleship.reloadTime);

	int BSunk = 0;
	
	//escort try to hit battleships
	for (int i = 0; i < bf->numberEscort; i++)
	{
		double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
		double ERangeMin, ERangeMax;
		getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);

		if (distance >= ERangeMin && distance <= ERangeMax)
		{
			BSunk = 1;
			fprintf(file, "Result: Battleship SUNK by Escort Ship ID %d (%s)!\n", bf->escort[i].id, bf->escort[i].typeName);
			break;
		
		}
	}

	//battleship counter attack
	if (!BSunk)
	{
		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
		int targets[100];
		int targetCount = getTargetAttackOrder(bf, BRangeMax, targets);
		double elapsedTime = 0.0;


		fprintf(file, "Target Attack Order Sequence: [");
		for (int i = 0; i < targetCount; i++) 
		{
    			fprintf(file, "%d%s", targets[i], (i == targetCount - 1) ? "" : ", ");
		}
		fprintf(file, "]\n");

		fprintf(file, "Battleship Strategy Counter-Attack (%d targets in range):\n", targetCount);
		for (int i = 0; i < targetCount; i++)
		{
			int idx = targets[i];
			bf->escort[idx].isDestroyed = 1;
			elapsedTime += bf->battleship.reloadTime;
			fprintf(file, "Hit Escort ID %d (%s) | Impact Power: %.2f | Elapsed Time: %.2fs\n", bf->escort[idx].id, bf->escort[idx].typeName, bf->escort[idx].impactPower, elapsedTime);
		}
		fprintf(file, "Result: Battleship SURVIVED | Total Firing Time: %.2fs\n", elapsedTime);
	}
	
	fprintf(file, "Strategy Used: Priority Target Selection (Highest Impact Power First)\n");

	fclose(file);
	printf("Simulation 1 results savs as '%s' \n", outputFileName);
}

void simulatePart1B_Sim1(battleField *bf, point *path, int k, const char *outputFileName)
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if (!file) return;

	fprintf(file, "Part 1B Simulation 1 Results (Path Movement, Single Hit Sinks Battleship) \n\n");

	int BSunk = 0;
	double elapsedTime = 0.0;

	for (int step = 0; step < k; step++)
	{
		bf->battleship.x = path[step].x;
		bf->battleship.y = path[step].y;

		fprintf(file, "Step %d / %d , Battleship Position: (%.2f, %.2f)\n", step + 1, k, bf->battleship.x, bf->battleship.y);

		for (int i = 0; i < bf->numberEscort; i++)
		{
			if (bf->escort[i].isDestroyed) continue;
			double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			double ERangeMin, ERangeMax;
			getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);

			if (distance >= ERangeMin && distance <= ERangeMax)
			{
				BSunk = 1;
				fprintf(file, "Result: Battleship sunk at step %d by escort Ship ID %d (%s)!\n", step + 1, bf->escort[i].id, bf->escort[i].typeName);
				break;
			}
		}
		
		if (BSunk) break;

		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
		int targets[100];
		int targetCount = getTargetAttackOrder(bf, BRangeMax, targets);
		
		fprintf(file, "Target Attack Order Sequence: [");
		for (int i = 0; i < targetCount; i++) 
		{
    			fprintf(file, "%d%s", targets[i], (i == targetCount - 1) ? "" : ", ");
		}
		fprintf(file, "]\n");

		for (int i = 0; i < targetCount; i++)
		{
			int idx = targets[i];
			bf->escort[idx].isDestroyed = 1;
			elapsedTime += bf->battleship.reloadTime;
			fprintf(file, "Hit escort ID %d (%s) ,Total time: %.2fs\n", bf->escort[idx].id, bf->escort[idx].typeName, elapsedTime);
		}

		if (targetCount == 0) fprintf(file, "No escorts in range.\n");
		fprintf(file, "\n");
	}
		if (!BSunk)
		{
			fprintf(file, "Final outcome:- Battleship survived all steps ,Total Firing Time: %.2fs\n", elapsedTime);
		}

	
	fprintf(file, "Strategy Used: Priority Target Selection (Highest Impact Power First)\n");
	fclose(file);
	printf("Part 1B simulation 1 results saves to '%s'\n",outputFileName);
}

void simulatePart1B_Sim2(battleField *bf, point *path, int k, int tjam, double minTheta, const char *outputFileName)
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if (!file) return;

	fprintf(file, "Part 1B Simulation 2 Results (Gun Jammed at Step %d, minTheta: %.2f deg) \n\n", tjam, minTheta);
	int BSunk = 0;
	double elapsedTime = 0.0;

	for (int step = 0; step < k; step++)
	{
		bf->battleship.x = path[step].x;
		bf->battleship.y = path[step].y;

		int isJammed = (step >= tjam);
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
		int targets[100];
		int targetCount = getTargetAttackOrder(bf, BRangeMax, targets);


		fprintf(file, "Target Attack Order Sequence: [");
		for (int i = 0; i < targetCount; i++) 
		{
    			fprintf(file, "%d%s", targets[i], (i == targetCount - 1) ? "" : ", ");
		}
		fprintf(file, "]\n");

		for (int i = 0; i < targetCount; i++)
		{
			int idx = targets[i];
			bf->escort[idx].isDestroyed = 1;
			elapsedTime += bf->battleship.reloadTime;
			fprintf(file, "Hit escort ID %d (%s) , Total Time: %.2fs\n", bf->escort[idx].id, bf->escort[idx].typeName, elapsedTime);
		}
		if (targetCount == 0) fprintf(file, "  No Escorts in range.\n");
		fprintf(file, "\n");
	}
	if (!BSunk)
	{
		fprintf(file, "Final Outcome:- Battleship survived all steps , Total Firing Time: %.2fs\n", elapsedTime);
	}
	fprintf(file, "Strategy Used: Priority Target Selection (Highest Impact Power First)\n");
	fclose(file);
	printf("Part1B imulation 2 results saved as '%s'\n",outputFileName);
}


void simulatePart1C_A(battleField *bf, const char *outputFileName) 
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if (!file) return;

	fprintf(file, "PART 1C_A SIMULATION (Stationary, Cumulative Damage)\n");
	fprintf(file, "Reload Time (T_B^q):- %.2fs\n\n", bf->battleship.reloadTime);

	for (int i = 0; i < bf->numberEscort; i++) 
	{
		double distance = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
		double ERangeMin, ERangeMax;
		getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);
		
		if (distance >= ERangeMin && distance <= ERangeMax) 
		{
			bf->battleship.cumulativeImpact += bf->escort[i].impactPower; 
			bf->battleship.health -= bf->escort[i].impactPower;
			fprintf(file, "Hit by escort ID %d , +%.2f%% damage , Total Damage: %.2f%%\n", bf->escort[i].id, bf->escort[i].impactPower * 100.0, bf->battleship.cumulativeImpact * 100.0);
		}
	}
	
	if (bf->battleship.health <= 0.0) 
	{
		bf->battleship.health = 0.0;
		fprintf(file, "\nResult:- Battleship sunk in stationary stage\n");
	} else 
	{
		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
		int targets[100];
		int targetCount = getTargetAttackOrder(bf, BRangeMax, targets);
		double elapsedTime = 0.0;

		fprintf(file, "Target Attack Order Sequence: [");
		for (int i = 0; i < targetCount; i++) 
		{
    			fprintf(file, "%d%s", targets[i], (i == targetCount - 1) ? "" : ", ");
		}
		fprintf(file, "]\n");


		fprintf(file, "\nBattleship Counter-Attack:\n");
		for (int i = 0; i < targetCount; i++) 
		{
			int idx = targets[i];
			bf->escort[idx].isDestroyed = 1;
			elapsedTime += bf->battleship.reloadTime;
			fprintf(file, "Destroyed escort ID %d (%s) , Time: %.2fs\n", bf->escort[idx].id, bf->escort[idx].typeName, elapsedTime);
		}
		fprintf(file, "\nFinal Status:- survived , Health: %.2f%% , Cumulative Impact: %.2f%%\n", bf->battleship.health * 100.0, bf->battleship.cumulativeImpact * 100.0);
	}
	fprintf(file, "Strategy Used: Priority Target Selection (Highest Impact Power First)\n");
	fclose(file);
	printf("Part 1-C_A simulation saved to '%s'\n", outputFileName);
}

void simulatePart1C_B(battleField *bf, point *path, int k, int tJam, double minTheta, const char *outputFileName) 
{
	resetEscortShip(bf);
	FILE *file = fopen(outputFileName, "w");
	if (!file) return;

	fprintf(file, "PART 1C_B SIMULATION (Moving Path, Cumulative Damage)\n");
	fprintf(file, "Gun Jammed Step: %d , Jammed minTheta: %.2f deg , Reload Time: %.2fs\n\n", tJam, minTheta, bf->battleship.reloadTime);

	double elapsedTime = 0.0;

	for (int step = 0; step < k; step++) 
	{
		bf->battleship.x = path[step].x;
		bf->battleship.y = path[step].y;

		int isJammed = (step >= tJam);
		double activeMinTheta = isJammed ? minTheta : 0.0;

		fprintf(file, "Step %d / %d , Pos: (%.2f, %.2f) , Status: %s\n", step + 1, k, bf->battleship.x, bf->battleship.y, isJammed ? "GUN JAMMED" : "NORMAL");

		for (int i = 0; i < bf->numberEscort; i++) 
		{
			if (bf->escort[i].isDestroyed) continue;

			double dist = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[i].x, bf->escort[i].y);
			double ERangeMin, ERangeMax;
			getMaxMinEscortshipFireRange(&bf->escort[i], &ERangeMin, &ERangeMax);

			if (dist >= ERangeMin && dist <= ERangeMax) 
			{
				bf->battleship.cumulativeImpact += bf->escort[i].impactPower;
				bf->battleship.health -= bf->escort[i].impactPower;
				fprintf(file, "Hit by Escort ID %d , +%.2f%% damage , Total Damage: %.2f%%\n", bf->escort[i].id, bf->escort[i].impactPower * 100.0, bf->battleship.cumulativeImpact * 100.0);
            		}
        	}

		if (bf->battleship.health <= 0.0) 
		{
			bf->battleship.health = 0.0;
			fprintf(file, "\nResult:- Battleship sunk at step %d!\n", step + 1);
			break;
		}

		double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, activeMinTheta);
		int targets[100];
		int targetCount = getTargetAttackOrder(bf, BRangeMax, targets);


		fprintf(file, "Target Attack Order Sequence: [");
		for (int i = 0; i < targetCount; i++) 
		{
    			fprintf(file, "%d%s", targets[i], (i == targetCount - 1) ? "" : ", ");
		}
		fprintf(file, "]\n");

		for (int i = 0; i < targetCount; i++) 
		{
			int idx = targets[i];
			bf->escort[idx].isDestroyed = 1;
			elapsedTime += bf->battleship.reloadTime;
			fprintf(file, "Destroyed Escort ID %d (%s) , Time: %.2fs\n", bf->escort[idx].id, bf->escort[idx].typeName, elapsedTime);
		}
		fprintf(file, "\n");
	}

	if (bf->battleship.health > 0.0) 
	{
		fprintf(file, "Final Status: survived , Health: %.2f%% , Cumulative Impact: %.2f%% , Total Time: %.2fs\n", bf->battleship.health * 100.0, bf->battleship.cumulativeImpact * 100.0, elapsedTime);
	}
	fprintf(file, "Strategy Used: Priority Target Selection (Highest Impact Power First)\n");
	fclose(file);
	printf("Part 1C_B simulation saved to '%s'\n", outputFileName);
}


//start of the main funtion 
int main(void)
{

	srand((unsigned int) time (NULL));

	battleField bf;
	double canvasSize = 1000.0;
	int escortCount = 10;
	char bType = 'U';
    	double bVMax = 100.0;
    	double bReloadTime = 5.0; // T_B^q reload time delay for Part 2A

	int k = 5; //path points 
	int tJam = 2; //steps before gun jamed
	double mintheta = 25.0; //jammed angle restriction (0<minTheta<30)

	battlefieldSetup(&bf, canvasSize, escortCount, bType, bVMax, bReloadTime);
	point *path = generatePath(k, bf.canvasSize);
				
	savefile(&bf, "battleformation.txt");

	simulatePart1A(&bf, "task1A.txt");
	simulatePart1B_Sim1(&bf, path, k, "task1B_sim1.txt");
	simulatePart1B_Sim2(&bf, path, k, tJam, mintheta, "task1B_sim2.txt");
	simulatePart1C_A(&bf, "task1C_A.txt");
	simulatePart1C_B(&bf, path, k, tJam, mintheta, "task1C_B.txt");

	free(path);
	free(bf.escort);
	return 0;
}
//end of the main function 


