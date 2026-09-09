// Naval Simulation - Part 2B (Range-Based Target Lock & Simultaneous Firing Strategy)
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define pi 3.14159265358979323846
#define gravity 9.81 // defining the gravity value 

// Base attributes for the 5 escort ship types
const double impactPowers[5] = {0.08, 0.06, 0.07, 0.05, 0.04};
const char* escortTypes[5] = {"EA", "EB", "EC", "ED", "EE"};
const double angleRange[5] = {20.0, 30.0, 25.0, 50.0, 70.0};
const double escortReloadTimes[5] = {3.0, 4.0, 3.5, 5.0, 6.0}; // T_E^p reload times per escort category

typedef struct
{
	double x;
	double y;
} point;

// escortship struct 
typedef struct
{
	int id;               // ship id
	int typeIndex;        // ship type index
	char typeName[10];    // ship type name
	double impactPower;
	double minAngle;
    	double angleRange;
	double maxVelocity;
	double minVelocity;
	double reloadTime;    // T_E^p: time between continuous gun firings
	double x;             // x position 
	double y;             // y position 	
	int isDestroyed;      // to check destroyed status
} escortShip;

// battleship struct 
typedef struct
{
	int type;             // ship type
	double maxVelocity;
	double x;             // x position 
	double y;             // y position 
	double health;
	double startx;
	double starty;
	double cumulativeImpact; // Total damage percentage 
	double reloadTime;       // T_B^q: reload time between firings
} battleShip;

// battlefield struct 
typedef struct
{
	double canvasSize;
	battleShip battleship;
	int numberEscort;
	escortShip *escort;
} battleField;

// Clears input stream buffer
void clearInputBuffer(void)
{
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}

// generate random numbers within the given parameters 
double getRandom(double min, double max)
{
	return min + ((double)rand() / RAND_MAX) * (max - min);
}

// finds distance between two points
double getDistance(double x1, double y1, double x2, double y2)
{
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

// converts degrees to radians
double degToRad(double deg)
{
	return deg * (pi / 180.0);
}

double getMaxBattleshipFireRange(double velocityMax, double thetaMin)
{
	double thetaBest = 45.0;
	if (45.0 < thetaMin)
    	{
        	thetaBest = thetaMin;
    	}
    	return (velocityMax * velocityMax * sin(degToRad(2.0 * thetaBest))) / gravity;
}

// finds the max and min firing range of a given escortship
void getMaxMinEscortshipFireRange(const escortShip *escort, double *rangeMin, double *rangeMax)
{
    	double thetaH = escort->minAngle + escort->angleRange;
    	double thetaL = escort->minAngle;

    	*rangeMin = (escort->minVelocity * escort->minVelocity * sin(degToRad(2.0 * thetaL))) / gravity;
	
    	double thetabest = 45.0;
    	if (45.0 < thetaL)
    	{
        	thetabest = thetaL;
    	}
    	else if (45.0 > thetaH)
    	{
        	thetabest = thetaH;
    	}

    	*rangeMax = (escort->maxVelocity * escort->maxVelocity * sin(degToRad(2.0 * thetabest))) / gravity;
}

// battlefield setup 
void battlefieldSetup(battleField *bf, double canvasSize, int escortNumber, char BShipType, double BVelocityMax, double BReloadTime)
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
    	bf->battleship.reloadTime = BReloadTime;

    	bf->escort = (escortShip *)malloc(sizeof(escortShip) * escortNumber);

    	for (int i = 0; i < escortNumber; i++)
    	{
        	int t = rand() % 5;
        	bf->escort[i].id = i;
        	bf->escort[i].typeIndex = t;
        	strcpy(bf->escort[i].typeName, escortTypes[t]);
        	bf->escort[i].impactPower = impactPowers[t];
        	bf->escort[i].angleRange = angleRange[t];
        	bf->escort[i].minAngle = getRandom(0.0, 30.0);
        	bf->escort[i].minVelocity = getRandom(0.0, BVelocityMax * 0.5);
        	bf->escort[i].reloadTime = escortReloadTimes[t]; // T_E^p unique to escort type
        	bf->escort[i].x = getRandom(0.0, canvasSize);
        	bf->escort[i].y = getRandom(0.0, canvasSize);
        	bf->escort[i].isDestroyed = 0;

        	if (t == 0)
        	{
            	bf->escort[i].maxVelocity = 1.2 * BVelocityMax;
        	}	
        	else
        	{
            		bf->escort[i].maxVelocity = getRandom(bf->escort[i].minVelocity, BVelocityMax);
        	}
    	}
}

void resetEscortShip(battleField *bf)
{
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
    	    	bf->escort[i].isDestroyed = 0;
    	}
    	bf->battleship.x = bf->battleship.startx;
    	bf->battleship.y = bf->battleship.starty;
    	bf->battleship.health = 1.0;
    	bf->battleship.cumulativeImpact = 0.0;
}	

point* generatePath(int k, double canvasSize)
{
    	point *path = (point *)malloc(sizeof(point) * k);
    	for (int i = 0; i < k; i++)
    	{
        	path[i].x = getRandom(0.0, canvasSize);
        	path[i].y = getRandom(0.0, canvasSize);
    	}
    	return path;
}

// save initial conditions on a save file 
void savefile(const battleField *bf, const char *filename)
{
    	FILE *file = fopen(filename, "w");
    	if (!file)
    	{
        	return;
    	}

    	fprintf(file, "Part 2B Initial Battlefield Conditions\n\n");
    	fprintf(file, "Canvas size: %.2f\n", bf->canvasSize);
    	fprintf(file, "Battleship Type: %c | Position: (%.2f, %.2f) | Max Velocity: %.2f | Reload Time T_B^q: %.2fs\n\n", bf->battleship.type, bf->battleship.x, bf->battleship.y, bf->battleship.maxVelocity, bf->battleship.reloadTime);
	
    	fprintf(file, "%-5s %-6s %-18s %-12s %-10s %-12s %-12s %-10s %-10s\n","ID", "Type", "Position (x,y)", "ImpactPower", "T_E (s)", "Min Angle", "Angle Range", "V_min", "V_max");

    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortShip *es = &bf->escort[i];
        	fprintf(file, "%-5d %-6s (%.2f, %.2f)    %-12.2f %-10.2f %-12.2f %-12.2f %-10.2f %-10.2f\n", es->id, es->typeName, es->x, es->y, es->impactPower, es->reloadTime, es->minAngle, es->angleRange, es->minVelocity, es->maxVelocity);
    	}

    	fclose(file);
}

// Check if Battleship has target lock on escort
int isEscortInBattleshipRange(const battleField *bf, int escortIdx, double BRangeMax)
{
    	if (bf->escort[escortIdx].isDestroyed)
    	{	
       		return 0;
    	}
    	double dist = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[escortIdx].x, bf->escort[escortIdx].y);
    	return (dist <= BRangeMax);
}

// Check if Escort has target lock on Battleship
int isBattleshipInEscortRange(const battleField *bf, int escortIdx)
{
    	if (bf->escort[escortIdx].isDestroyed)
    	{
        	return 0;
    	}
    	double dist = getDistance(bf->battleship.x, bf->battleship.y, bf->escort[escortIdx].x, bf->escort[escortIdx].y);
    	double ERangeMin, ERangeMax;
    	getMaxMinEscortshipFireRange(&bf->escort[escortIdx], &ERangeMin, &ERangeMax);
    	return (dist >= ERangeMin && dist <= ERangeMax);
}

// Target selection strategy for Battleship: Sort locked targets by Threat Level / DPS = (Impact Power / T_E)
int getTargetAttackOrder2B(const battleField *bf, double BRangeMax, int *targetIndices)
{
    	int count = 0;
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	if (isEscortInBattleshipRange(bf, i, BRangeMax))
        	{
            		targetIndices[count++] = i;
        	}
    	}

    	// Sort targets by highest threat (DPS = impactPower / reloadTime)
    	for (int i = 0; i < count - 1; i++)
    	{
        	for (int j = i + 1; j < count; j++)
        	{
            		double threatI = bf->escort[targetIndices[i]].impactPower / bf->escort[targetIndices[i]].reloadTime;
            		double threatJ = bf->escort[targetIndices[j]].impactPower / bf->escort[targetIndices[j]].reloadTime;

            		if (threatI < threatJ) //compare the highes threat
            		{
                		int temp = targetIndices[i];
                		targetIndices[i] = targetIndices[j];
                		targetIndices[j] = temp;
            		}				
        	}
    	}
    	return count;
}

// Simultaneous Timeline Engagement Engine
// Battleship and Escorts fire chronologically whenever they have a valid target lock in range.
int processTimelineEngagement(battleField *bf, double BRangeMax, double *tCurrent, double *escortNextFire, double *bNextFire, FILE *file, int isSingleHitKill)
{
    	int targets[100];
    	int targetCount = getTargetAttackOrder2B(bf, BRangeMax, targets); //store targets 

    	while (1)
    	{
        	//Identify the earliest next available firing event among locked ships
        	double earliestTime = 1e9;//which can do first
        	int earliestEscortIdx = -1; //which e ready to fire 
        	int bReadyToFire = 0;

        	// Check if Battleship has lock on any target and when it can fire
        	if (targetCount > 0 && *bNextFire < earliestTime)
        	{
            		earliestTime = *bNextFire;
            		bReadyToFire = 1;
        	}

        	// Check if any Escort has lock on Battleship and when it can fire
        	for (int i = 0; i < bf->numberEscort; i++)
        	{
            		if (isBattleshipInEscortRange(bf, i))
            		{
                		if (escortNextFire[i] < earliestTime)
                		{
                    			earliestTime = escortNextFire[i];
                    			earliestEscortIdx = i;
                    			bReadyToFire = 0; // Escort ready earlier than battleship
                		}
            		}	
        	}

        	// If no ship has a lock or can fire, break engagement
        	if (earliestTime >= 1e8)
        	{
            		break;
        	}
	
        	*tCurrent = earliestTime;

        	// Process Battleship Firing Event
        	if (bReadyToFire)
        	{
            		int targetIdx = targets[0]; // Target highest threat locked escort
            		bf->escort[targetIdx].isDestroyed = 1;
            		*bNextFire = *tCurrent + bf->battleship.reloadTime;

           		 if (file)
            		{
                		fprintf(file, "  [t = %.2fs] Battleship LOCKED & FIRED -> DESTROYED Escort ID %d (%s)\n", *tCurrent, bf->escort[targetIdx].id, bf->escort[targetIdx].typeName);
            		}

            		// Refresh target list after destroying escort
            		targetCount = getTargetAttackOrder2B(bf, BRangeMax, targets);
        	}
        	//Process Escort Firing Event
        	else if (earliestEscortIdx != -1)
        	{
            		escortShip *es = &bf->escort[earliestEscortIdx];
            		bf->battleship.cumulativeImpact += es->impactPower;
            		bf->battleship.health -= es->impactPower;
            		escortNextFire[earliestEscortIdx] = *tCurrent + es->reloadTime;

            		if (file)
            		{
                		fprintf(file, "  [t = %.2fs] Escort ID %d (%s) LOCKED & FIRED -> Hit Battleship | +%.2f%% damage | Health: %.2f%%\n", *tCurrent, es->id, es->typeName, es->impactPower * 100.0, (bf->battleship.health < 0 ? 0 : bf->battleship.health) * 100.0);
            		}

            		if (isSingleHitKill || bf->battleship.health <= 0.0)
            		{
                		bf->battleship.health = 0.0;
                		if (file)
                		{
                    		fprintf(file, "Result: Battleship SUNK at t = %.2fs!\n", *tCurrent);
                		}
                		return 1; // Battleship sunk
            		}
        	}

        	// Stop engagement if no active escorts remain in range
        	if (targetCount == 0)
        	{
            		int escortsInRange = 0;
            		for (int i = 0; i < bf->numberEscort; i++)
            		{
                		if (isBattleshipInEscortRange(bf, i))
                		{
                    			escortsInRange = 1;
                    			break;
                		}
            		}
            		if (!escortsInRange)
            		{
                		break;
            		}
        	}
    	}
    	return 0; // Battleship survived
}

void simulatePart2B_1A(battleField *bf, const char *outputFileName)
{
    	resetEscortShip(bf);
    	FILE *file = fopen(outputFileName, "w");
    	if (!file)
    	{
        	return;
    	}

    	fprintf(file, "Part 2B Simulation 1A Results (Stationary, Range-Lock Target Engagement)\n");
    	fprintf(file, "Battleship Position: (%.2f, %.2f)\n", bf->battleship.x, bf->battleship.y);
    	fprintf(file, "Battleship Reload Time (T_B^q): %.2fs\n\n", bf->battleship.reloadTime);

    	double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
    	double tCurrent = 0.0;
    	double bNextFire = 0.0;
    	double escortNextFire[100];
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortNextFire[i] = 0.0;
   	}

    	int BSunk = processTimelineEngagement(bf, BRangeMax, &tCurrent, escortNextFire, &bNextFire, file, 1);

    	if (!BSunk)
    	{
       		fprintf(file, "Result: Battleship SURVIVED | Total Engagement Time: %.2fs\n", tCurrent);
    	}

    	fprintf(file, "Strategy Used: Simultaneous Range-Lock Engagement (Highest DPS Target First)\n");
    	fclose(file);
    	printf("Part 2B Simulation 1A results saved to '%s'\n", outputFileName);
}

void simulatePart2B_1B_Sim1(battleField *bf, point *path, int k, const char *outputFileName)
{
    	resetEscortShip(bf);
    	FILE *file = fopen(outputFileName, "w");
    	if (!file)
   	{
        	return;
    	}

    	fprintf(file, "Part 2B Simulation 1B-Sim1 Results (Path Movement, Single Hit Sinks Battleship, Range-Lock Engagement)\n\n");

    	int BSunk = 0;
    	double tCurrent = 0.0;
    	double bNextFire = 0.0;
    	double escortNextFire[100];
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortNextFire[i] = 0.0;
    	}

    	for (int step = 0; step < k; step++)
    	{		
    	    	bf->battleship.x = path[step].x;
       	 	bf->battleship.y = path[step].y;

        	fprintf(file, "Step %d / %d | Position: (%.2f, %.2f)\n", step + 1, k, bf->battleship.x, bf->battleship.y);

        	double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);
        	BSunk = processTimelineEngagement(bf, BRangeMax, &tCurrent, escortNextFire, &bNextFire, file, 1);

        	if (BSunk)
        	{
            		break;
       		}
        	fprintf(file, "\n");
    	}

    	if (!BSunk)
    	{
        	fprintf(file, "Final Outcome: Battleship survived all steps | Total Time: %.2fs\n", tCurrent);
    	}
	
    	fprintf(file, "Strategy Used: Simultaneous Range-Lock Engagement (Highest DPS Target First)\n");
    	fclose(file);
    	printf("Part 2B Simulation 1B-Sim1 results saved to '%s'\n", outputFileName);
}

void simulatePart2B_1B_Sim2(battleField *bf, point *path, int k, int tjam, double minTheta, const char *outputFileName)
{
    	resetEscortShip(bf);
    	FILE *file = fopen(outputFileName, "w");
    	if (!file)
    	{
        	return;
   	}

    	fprintf(file, "Part 2B Simulation 1B-Sim2 Results (Gun Jammed at Step %d, minTheta: %.2f deg, Range-Lock Engagement)\n\n", tjam, minTheta);
    	int BSunk = 0;
    	double tCurrent = 0.0;
    	double bNextFire = 0.0;
    	double escortNextFire[100];
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortNextFire[i] = 0.0;
    	}

    	for (int step = 0; step < k; step++)
    	{
        	bf->battleship.x = path[step].x;
        	bf->battleship.y = path[step].y;

        	int isJammed = (step >= tjam);
        	double activeMinTheta = isJammed ? minTheta : 0.0;

        	fprintf(file, "Step %d / %d | Pos: (%.2f, %.2f) | Status: %s\n", step + 1, k, bf->battleship.x, bf->battleship.y, isJammed ? "GUN JAMMED" : "NORMAL");

        	double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, activeMinTheta);
        	BSunk = processTimelineEngagement(bf, BRangeMax, &tCurrent, escortNextFire, &bNextFire, file, 1);

        	if (BSunk)
        	{
            		break;
        	}
        	fprintf(file, "\n");
    	}

    	if (!BSunk)
    	{
        	fprintf(file, "Final Outcome: Battleship survived all steps | Total Time: %.2fs\n", tCurrent);
    	}

    	fprintf(file, "Strategy Used: Simultaneous Range-Lock Engagement (Highest DPS Target First)\n");
    	fclose(file);
    	printf("Part 2B Simulation 1B-Sim2 results saved to '%s'\n", outputFileName);
}

void simulatePart2B_1C_A(battleField *bf, const char *outputFileName)
{
    	resetEscortShip(bf);
    	FILE *file = fopen(outputFileName, "w");
    	if (!file)
    	{
        	return;
    	}

    	fprintf(file, "PART 2B SIMULATION 1C_A (Stationary, Continuous Firing, Cumulative Damage)\n");
    	fprintf(file, "Battleship Reload Time (T_B^q): %.2fs\n\n", bf->battleship.reloadTime);

    	double escortNextFire[100];
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortNextFire[i] = 0.0;
    	}

   	 double tCurrent = 0.0;
    	double bNextFire = 0.0;
    	double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, 0.0);

    	int BSunk = processTimelineEngagement(bf, BRangeMax, &tCurrent, escortNextFire, &bNextFire, file, 0);

    	if (!BSunk)
    	{
        	fprintf(file, "\nFinal Status: SURVIVED | Health: %.2f%% | Cumulative Impact: %.2f%% | Total Time: %.2fs\n", bf->battleship.health * 100.0, bf->battleship.cumulativeImpact * 100.0, tCurrent);
    	}

    	fprintf(file, "Strategy Used: Simultaneous Range-Lock Engagement (Highest DPS Target First)\n");
    	fclose(file);
    	printf("Part 2B Simulation 1C_A results saved to '%s'\n", outputFileName);
}

void simulatePart2B_1C_B(battleField *bf, point *path, int k, int tJam, double minTheta, const char *outputFileName)
{
    	resetEscortShip(bf);
    	FILE *file = fopen(outputFileName, "w");
    	if (!file)
    	{
        	return;
    	}

    	fprintf(file, "PART 2B SIMULATION 1C_B (Moving Path, Continuous Firing, Cumulative Damage)\n");
    	fprintf(file, "Gun Jammed Step: %d | Jammed minTheta: %.2f deg | Battleship Reload Time: %.2fs\n\n", tJam, minTheta, bf->battleship.reloadTime);

    	double tCurrent = 0.0;
    	double bNextFire = 0.0;
    	double escortNextFire[100];
    	for (int i = 0; i < bf->numberEscort; i++)
    	{
        	escortNextFire[i] = 0.0;
    	}

    	int BSunk = 0;

    	for (int step = 0; step < k; step++)
    	{
        	bf->battleship.x = path[step].x;
        	bf->battleship.y = path[step].y;

        	int isJammed = (step >= tJam);
        	double activeMinTheta = isJammed ? minTheta : 0.0;

        	fprintf(file, "Step %d / %d | Pos: (%.2f, %.2f) | Status: %s\n", step + 1, k, bf->battleship.x, bf->battleship.y, isJammed ? "GUN JAMMED" : "NORMAL");

        	double BRangeMax = getMaxBattleshipFireRange(bf->battleship.maxVelocity, activeMinTheta);
        	BSunk = processTimelineEngagement(bf, BRangeMax, &tCurrent, escortNextFire, &bNextFire, file, 0);

        	if (BSunk)
        	{
           		 break;
        	}
        	fprintf(file, "\n");
    	}

    	if (!BSunk)
    	{
        	fprintf(file, "Final Status: SURVIVED | Health: %.2f%% | Cumulative Impact: %.2f%% | Total Time: %.2fs\n", bf->battleship.health * 100.0, bf->battleship.cumulativeImpact * 100.0, tCurrent);
    	}

    	fprintf(file, "Strategy Used: Simultaneous Range-Lock Engagement (Highest DPS Target First)\n");
    	fclose(file);
    	printf("Part 2B Simulation 1C_B results saved to '%s'\n", outputFileName);
}


// Display contents of a text file
void displayFileContent(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (!file)
	{
		printf("\n[!] File '%s' not found. Run the simulation first!\n", filename);
		return;
	}	
	printf("\n           CONTENT OF: %s\n", filename);
	char line[256];
	while (fgets(line, sizeof(line), file))
	{
		printf("%s", line);
	}
	fclose(file);
}

// Display Instruction menu
void viewInstructions(void)
{
	printf("\n             SIMULATOR INSTRUCTIONS & HELP             \n");
	
	printf("1. Overview:\n");
	printf("   - This program simulates a 2D Naval Battlefield.\n");
	printf("   - A Battleship (B) engages multiple Escort Ships (E) protecting cargo vessels.\n\n");
	printf("2. Controls & Configuration:\n");
	printf("   - Access 'Setup' under 'Start Simulation' menu to configure battlefield size,\n");
	printf("     number of escort ships, battleship type, max velocity, reload times, and random seed.\n\n");
	printf("3. Projectile Motion Equations:\n");
	printf("   - Range R = (V^2 * sin(2 * theta)) / g\n");
	printf("   - Max Range determined by maximum initial shell velocity and angle parameters.\n\n");
	printf("4. Computer Strategy:\n");
	printf("   - The Battleship prioritizes escort targets in range based on Threat Level (DPS = Impact Power / Reload Time).\n");
	
}

// Simulation Statistics Menu
void showSimulationStatistics(void)
{
	int choice;
	do
	{
		printf("\n               SIMULATION STATISTICS MENU             \n");
		
		printf("1. View Initial Battlefield Formation (battleformation_2B.txt)\n");
		printf("2. View Task 2B 1A Simulation Results (task2B_1A.txt)\n");
		printf("3. View Task 2B 1B Sim1 Results (task2B_1B_sim1.txt)\n");
		printf("4. View Task 2B 1B Sim2 Results (task2B_1B_sim2.txt)\n");
		printf("5. View Task 2B 1C_A Results (task2B_1C_A.txt)\n");
		printf("6. View Task 2B 1C_B Results (task2B_1C_B.txt)\n");
		printf("7. Return to Main Menu\n");
		printf("Enter choice (1-7): ");

		if (scanf("%d", &choice) != 1)
		{
			clearInputBuffer();
			printf("Invalid input. Try again.\n");
			continue;
		}

		switch (choice)
		{
			case 1: displayFileContent("battleformation_2B.txt"); break;
			case 2: displayFileContent("task2B_1A.txt"); break;
			case 3: displayFileContent("task2B_1B_sim1.txt"); break;
			case 4: displayFileContent("task2B_1B_sim2.txt"); break;
			case 5: displayFileContent("task2B_1C_A.txt"); break;
			case 6: displayFileContent("task2B_1C_B.txt"); break;
			case 7: printf("Returning to Main Menu...\n"); break;
			default: printf("Invalid choice. Enter 1-7.\n");
		}
	} while (choice != 7);
}

// Setup Submenu
void setupSubmenu(battleField *bf, double *canvasSize, int *escortCount, char *bType, double *bVMax, double *bReloadTime, unsigned int *seedValue, int *k, int *tJam, double *mintheta)
{
	int choice;
	do
	{
		printf("\n------------------------------------------------------\n");
		printf("                    SETUP SUBMENU                      \n");
		printf("------------------------------------------------------\n");
		printf("1. Battleship Properties (Type: %c, Vmax: %.2f, Reload T_B: %.2fs)\n", *bType, *bVMax, *bReloadTime);
		printf("2. Escort Ships Settings (Count: %d, Canvas Size: %.2f)\n", *escortCount, *canvasSize);
		printf("3. Path & Jamming Settings (K points: %d, Jamming Step: %d, Min Angle: %.2f deg)\n", *k, *tJam, *mintheta);
		printf("4. Seed Value (Current Seed: %u)\n", *seedValue);
		printf("5. Return to Simulation Submenu\n");
		printf("Enter choice (1-5): ");

		if (scanf("%d", &choice) != 1)
		{
			clearInputBuffer();
			printf("Invalid input!\n");
			continue;
		}

		switch (choice)
		{
			case 1:
				printf("\n--- Battleship Properties ---\n");
				printf("Select Notation / Type [U = USS Iowa, M = MS King George V, R = Richelieu, S = Sovetsky Soyuz]: ");
				clearInputBuffer();
				scanf("%c", bType);
				printf("Enter Max Velocity (Vmax) of Battleship: ");
				scanf("%lf", bVMax);
				printf("Enter Battleship Reload Time (T_B^q) in seconds: ");
				scanf("%lf", bReloadTime);
				printf("[+] Battleship properties updated!\n");
				break;

			case 2:
				printf("\n--- Escort Ships Settings ---\n");
				printf("Enter Number of Escort Ships (N): ");
				scanf("%d", escortCount);
				if (*escortCount > 100) *escortCount = 100;
				if (*escortCount < 1) *escortCount = 1;
				printf("Enter Battlefield Canvas Size (D): ");
				scanf("%lf", canvasSize);
				printf("[+] Escort settings updated!\n");
				break;

			case 3:
				printf("\n--- Path & Jamming Settings ---\n");
				printf("Enter number of path points (k): ");
				scanf("%d", k);
				printf("Enter gun jamming step index (t_jam): ");
				scanf("%d", tJam);
				printf("Enter minimum angle when jammed (theta_min): ");
				scanf("%lf", mintheta);
				printf("[+] Path & Jamming settings updated!\n");
				break;

			case 4:
				printf("\n--- Seed Value ---\n");
				printf("Enter Random Generator Seed Value (integer): ");
				scanf("%u", seedValue);
				srand(*seedValue);
				printf("[+] Seed updated to %u!\n", *seedValue);
				break;

			case 5:
				printf("Returning...\n");
				break;

			default:
				printf("Invalid selection!\n");
		}
	} while (choice != 5);
}

// Start Simulation Submenu
void startSimulationSubmenu(battleField *bf, double *canvasSize, int *escortCount, char *bType, double *bVMax, double *bReloadTime, unsigned int *seedValue, int *k, int *tJam, double *mintheta, int *isConfigured)
{
	int choice;
	do
	{
		printf("\n======================================================\n");
		printf("               START SIMULATION SUBMENU               \n");
		printf("======================================================\n");
		printf("1. Setup Variables\n");
		printf("2. Show Simulation (Run Simulation and Output Statistics)\n");
		printf("3. Return to Main Menu\n");
		printf("Enter choice (1-3): ");

		if (scanf("%d", &choice) != 1)
		{
			clearInputBuffer();
			printf("Invalid input. Try again.\n");
			continue;
		}

		switch (choice)
		{
			case 1:
				setupSubmenu(bf, canvasSize, escortCount, bType, bVMax, bReloadTime, seedValue, k, tJam, mintheta);
				*isConfigured = 0; // Requires re-initialization
				break;

			case 2:
				printf("\n[+] Initializing Battlefield and executing simulations...\n");
				srand(*seedValue);
				battlefieldSetup(bf, *canvasSize, *escortCount, *bType, *bVMax, *bReloadTime);
				point *path = generatePath(*k, bf->canvasSize);
				int TJam = 99999;
				savefile(bf, "battleformation_2B.txt");

				// Run Part 2B Simulation Suite
				simulatePart2B_1A(bf, "task2B_1A.txt");
				simulatePart2B_1B_Sim1(bf, path, *k, "task2B_1B_sim1.txt");
				simulatePart2B_1B_Sim2(bf, path, *k, *tJam, *mintheta, "task2B_1B_sim2.txt");
				simulatePart2B_1C_A(bf, "task2B_1C_A.txt");
				simulatePart2B_1C_B(bf, path, *k, *tJam, *mintheta, "task2B_1C_B_jamed.txt");
				simulatePart2B_1C_B(bf, path, *k, TJam, *mintheta, "task2B_1C_B_normal.txt");

				free(path);
				*isConfigured = 1;
				printf("\n[+] All simulations executed successfully! Output files generated.\n");
				break;

			case 3:
				printf("Returning to Main Menu...\n");
				break;

			default:
				printf("Invalid selection!\n");
		}
	} while (choice != 3);
}

// Main Function
int main(void)
{
    	battleField bf = {0};
	bf.escort = NULL;

	// Default battlefield & simulation parameters
	double canvasSize = 10000.0;
	int escortCount = 10;
	char bType = 'U';
	double bVMax = 100.0;
	double bReloadTime = 5.0; // T_B^q reload time delay

	int k = 5;               // Path points 
	int tJam = 2;            // Step index before gun jammed
	double mintheta = 25.0;  // Jammed angle restriction
	unsigned int seedValue = (unsigned int)time(NULL);
	int isConfigured = 0;

	int mainChoice;
	do
	{
		printf("\n======================================================\n");
		printf("       ADVANCED NAVAL BATTLE SIMULATOR (PART 2B)      \n");
		printf("======================================================\n");
		printf("1. Start Simulation\n");
		printf("2. View Instructions\n");
		printf("3. Simulation Statistics\n");
		printf("4. Exit\n");
		printf("Enter option (1-4): ");

		if (scanf("%d", &mainChoice) != 1)
		{
			clearInputBuffer();
			printf("Invalid input. Please enter a valid number.\n");
			continue;
		}

		switch (mainChoice)
		{
			case 1:
				startSimulationSubmenu(&bf, &canvasSize, &escortCount, &bType, &bVMax, &bReloadTime, &seedValue, &k, &tJam, &mintheta, &isConfigured);
				break;

			case 2:
				viewInstructions();
				break;

			case 3:
				showSimulationStatistics();
				break;

			case 4:
				printf("\nAre you sure you want to exit? (y/n): ");
				clearInputBuffer();
				char confirm;
				scanf("%c", &confirm);
				if (confirm == 'y' || confirm == 'Y')
				{
					printf("Exiting Naval Battle Simulator. Goodbye!\n");
				}
				else
				{
					mainChoice = 0; // Cancel exit
				}
				break;

			default:
				printf("Invalid choice! Please select 1, 2, 3, or 4.\n");
		}
	} while (mainChoice != 4);

	if (bf.escort != NULL)
	{
		free(bf.escort);
	}

	return 0;
}
