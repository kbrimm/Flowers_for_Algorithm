/*
 *  "Flowers for Algorithm"
 *  Implements shortest path and state- based artificial intelligence algorithms
 *  A rat is deposited in to a maze with randomized biological drives
 *  Various states of "need satisfied" and "filling need" for fun, health, hunger, and sleep
 *  Utilizes Dijkstra's algorithm to find shortest path to fill needs
 *  Once needs are > 50%, returns to entrance for release.
 *
 *  The MIT License (MIT)
 *  Copyright (c) 2014 K. Brimm
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 *  and associated documentation files (the "Software"), to deal in the Software without restriction, 
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 *  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all copies or 
 *  substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 *  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 *  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

const int MAX_CHAR = 100;
const int FUN_MAX = 35;
const int HEALTH_MAX = 60;
const int HUNGER_MAX = 30;
const int SLEEP_MAX = 40;
const int VERTEX_COUNT = 18;
const int INFINITY_APPROX = 42;

struct ratState{
	int fun;
	int health;
	int hunger;
	int sleep;
};

struct edgeWeight{
	char initial;
	char terminal;
	int weight;
};


// Functions for user interface
string getString(string prompt);
char getChar(string prompt);
void pause();

// Functions to drive finite state machine
void initializeDrives(ratState &drives);
char identifyState(const ratState &drives, string name);
void setPercentage(const ratState &drives, ratState &percent);
void printDrives(const ratState &percent, string name);
void printState(char biggestNeed, string name);
void updateState(ratState &drives, int travel);
void satisfyNeed(ratState &drives, char currentLocation, string name);

// Functions to drive Dijkstra's shortest path algorithm
bool loadGraph(edgeWeight baseGraph[]);
void copyGraph(const edgeWeight baseGraph[], edgeWeight newGraph[]);
int findRoute(edgeWeight newGraph[], char &currentLocation, char currentState);
void initializeNodeWeight(int nodeWeight[]);
int nodeToNumber(char nodeLetter);
char numberToNode(int nodeNumber);
void findNeighbors(const edgeWeight newGraph[], int nodeWeight[], char currentNode, int currentNum);
void removeVertex(edgeWeight newGraph[], char currentLocation);
char findLeast(int nodeWeight[]);

// And now we begin:
int main()
{
	bool graph;
	char currentLocation = 'E';
	char currentState;
	edgeWeight baseGraph[VERTEX_COUNT];
	edgeWeight newGraph[VERTEX_COUNT];
	int travel;
	ratState drives;
	string name;

	graph = loadGraph(baseGraph);
	if(!graph)
		return 1;

	cout << "~~ Flowers for Algorithm ~~" << endl << endl;
	cout << "The scientist places the rat in the vestibule of a maze." << endl
		<< "The rat is a thinly veiled metaphor for the tenuous nature of human existence." << endl;
	name = getString("What is the rat's name?");

	initializeDrives(drives);
	do
	{		
		copyGraph(baseGraph, newGraph);
		currentState = identifyState(drives, name);
		travel = findRoute(newGraph, currentLocation, currentState);
		updateState(drives, travel);
		satisfyNeed(drives, currentLocation, name);
	}
	while(currentLocation != 'E');

	cout << "The scientist removes " << name << " from the maze and jots in her notebook:" << endl
		<< "\t\'Science accomplished.\'" << endl << "THE END" << endl;

	pause();
	return 0;
}

// Basic functions for user interface
string getString(string prompt)
{
	string input;

	cout << prompt << " ";
	cin >> input;
	cin.ignore(MAX_CHAR, '\n');
	cin.clear();

	return input;
}

void pause()
{
	char reply;

	cout << "Press enter to continue." << endl;
	cin.get(reply);
}

// Functions for state machine
// Assigns random values of different weights to ratState drives
void initializeDrives(ratState &drives)
{
	srand(static_cast<int>(time(0)));

	drives.fun = rand() % FUN_MAX;
	drives.health = rand() % HEALTH_MAX;
	drives.hunger = rand() % HUNGER_MAX;
	drives.sleep = rand() % SLEEP_MAX;
}

// Compares values of ratState drives, determines current state
char identifyState(const ratState &drives, string name)	
{
	int index;
	char biggestNeed = 'M';
	ratState percent;

	setPercentage(drives, percent);
	printDrives(percent, name);

	index = percent.health;				// In case of equal drives, precedence follows in descending order
	if(percent.hunger < index)
	{
		index = percent.hunger;
		biggestNeed = 'F';
	}
	if(percent.sleep < index)
	{
		index = percent.sleep;
		biggestNeed = 'N';
	}
	if(percent.fun < index)
	{
		index = percent.fun;
		biggestNeed = 'W';
	}
	if(50 < index)
		biggestNeed = 'E';

	printState(biggestNeed, name);
	return biggestNeed;
}

// Calculates percentage of ratState drives based on weight (for ease of comparison)
void setPercentage(const ratState &drives, ratState &percent)	
{
	percent.fun = (100 * drives.fun) / FUN_MAX;
	percent.health = (100 * drives.health) / HEALTH_MAX;
	percent.hunger = (100 * drives.hunger) / HUNGER_MAX;
	percent.sleep = (100 * drives.sleep) / SLEEP_MAX;
}

void printDrives(const ratState &percent, string name)
{
	cout << name << " is currently feeling: " << endl
		<< "\t" << percent.fun << "% entertained" << endl
		<< "\t" << percent.health << "% healthy" << endl
		<< "\t" << percent.hunger << "% nourished" << endl
		<< "\t" << percent.sleep << "% rested" << endl;
	pause();
}

// Displays text about the rat's current state and destination.
void printState(char biggestNeed, string name)
{
	switch(biggestNeed){
	case 'E':
		cout << name << " is feeling satisfied and is going to the exit for release." << endl;
		break;
	case 'F':
		cout << name << " is hungry and is going to the food bowl." << endl;
		break;	
	case 'M':
		cout << name << " is feeling sick and is going to the medicine dispenser." << endl;
		break;
	case 'N':
		cout << name << " is sleepy and is going to the nest for a nap." << endl;
		break;
	case 'W':
		cout << name << " is bored and is going to the exercise wheel." << endl;
		break;
	}
}

// Decrements each need by the distance traveled
void updateState(ratState &drives, int travel)
{
	drives.fun -= travel;
	if(drives.fun < 0)
		drives.fun = 0;
	drives.health -= travel;
	if(drives.health < 0)
		drives.health = 0;
	drives.hunger -= travel;
	if(drives.hunger < 0)
		drives.hunger = 0;
	drives.sleep -= travel;
	if(drives.sleep < 0)
		drives.sleep = 0;
}

// Fills the drive for the rats current location, prints a bit of amusing text to accompany it.
void satisfyNeed(ratState &drives, char currentLocation, string name)
{
	switch(currentLocation)
	{
	case 'F':
		drives.hunger = HUNGER_MAX;
		cout << name << " has reached the food bowl." << endl <<
			name << " finds a tasty kibble to chew on. Mmmm, lab diets." << endl;
		break;	
	case 'M':
		drives.health = HEALTH_MAX;
		cout << name << " has reached the medical pod." << endl << 
			"YUCK! That medicine is disgusting, but " << name << " feels much better now." << endl;
		break;
	case 'N':
		drives.sleep = SLEEP_MAX;
		cout << name << " has reached the rat's nest." << endl <<
			"Off to dreamland!" << endl;
		cout << name << " is bright-eyed and ready to go after that refreshing nap!" << endl;
		break;
	case 'W':
		drives.fun = FUN_MAX;
		cout << name << " has reached the exercise wheel." << endl <<
			"The wheel goes squeak, squeak, squeak, squeak, squeak, squeak." << endl;
		break;
	default:
		break;
	}
}

// Functions for Dijkstra's Algorithm
// Loads graph from .txt file (for flexibility, I guess?)
bool loadGraph(edgeWeight baseGraph[])
{
	char fileName[] = "graphWeights";
	ifstream inFile;

	inFile.open(fileName);
	if(!inFile.is_open())
	{
		cout << "Failed to load graph. Program unable to continue." << endl <<
			"Check the location of graphWeights and try again." << endl;
		pause();
		return false;
	}

	for(int i = 0; i < VERTEX_COUNT; i++)
		inFile >> baseGraph[i].initial >> baseGraph[i].terminal >> baseGraph[i].weight;

	inFile.close();
	return true;
}

// Resets graph for each new origin/destination pair.
void copyGraph(const edgeWeight baseGraph[], edgeWeight newGraph[])
{
	for(int i = 0; i < VERTEX_COUNT; i++)
	{
		newGraph[i].initial = baseGraph[i].initial;
		newGraph[i].terminal = baseGraph[i].terminal;
		newGraph[i].weight = baseGraph[i].weight;
	}
}

// Implements Dijkstra's algorithm to calculate the sortest distance between where the rat currently is
//  and where it needs to go to satisfy its current need.
int findRoute(edgeWeight newGraph[], char &currentLocation, char currentState)
{
	int currentNum;
	char currentNode;
	char terminalNode;
	int nodeWeight[VERTEX_COUNT/2];
	int visited[VERTEX_COUNT/2];
	

	currentNode = currentLocation;				//Initializing all the stuff
	terminalNode= currentState;					//	|
	initializeNodeWeight(nodeWeight);			//	|
	currentNum = nodeToNumber(currentNode);		//	|
	nodeWeight[currentNum] = 0;					//	V

	while(currentNode != terminalNode)	// Keep going until you get there, little guy!
	{
		findNeighbors(newGraph, nodeWeight, currentNode, currentNum);	// Find vertices adjacent to current
		removeVertex(newGraph, currentNode);							// Remove current from index, 
		nodeWeight[currentNum] = 0;										// Mark current as visited
		currentNode = findLeast(nodeWeight);							// Identify the unvisited vertex w/least weight
		currentNum = nodeToNumber(currentNode);							//  and set that vertex as current.
		// cout << "\tAdding node " << currentNode << " to potential path, with a current weight of " << 
		// 	nodeWeight[currentNum] << "." << endl;
	}

	currentLocation = currentNode;
	cout << "\tTraveling to node " << currentNode << "." << endl;
	cout << "\tTraveled a total of " << nodeWeight[currentNum] << " distance units." << endl;
	return nodeWeight[currentNum];
}

int nodeToNumber(char nodeLetter)
{
	int nodeNumber;

	switch(nodeLetter)
	{
	case 'E':
		nodeNumber = 0;
		break;
	case 'N':
		nodeNumber = 1;
		break;
	case 'F':
		nodeNumber = 2;
		break;
	case 'A':
		nodeNumber = 3;
		break;
	case 'W':
		nodeNumber = 4;
		break;
	case 'B':
		nodeNumber = 5;
		break;
	case 'M':
		nodeNumber = 6;
		break;
	}

	return nodeNumber;
}

char numberToNode(int nodeNumber)
{
	char nodeLetter;

	switch(nodeNumber)
	{
	case 0:
		nodeLetter = 'E';
		break;
	case 1:
		nodeLetter = 'N';
		break;
	case 2:
		nodeLetter = 'F';
		break;
	case 3:
		nodeLetter = 'A';
		break;
	case 4:
		nodeLetter = 'W';
		break;
	case 5:
		nodeLetter = 'B';
		break;
	case 6:
		nodeLetter = 'M';
		break;
	}

	return nodeLetter;
}

// Sets all nodes to an infinity value (a number greater than the sum of all the edge weights in the graph).
void initializeNodeWeight(int nodeWeight[])
{
	for(int i = 0; i < VERTEX_COUNT/2; i++)
		nodeWeight[i] = INFINITY_APPROX;
}

// Identifies vertices adjacent to the current vertex, updates their unvisited weights.
void findNeighbors(const edgeWeight newGraph[], int nodeWeight[], char currentNode, int currentNum)
{
	int neighborNode;

	for(int i = 0; i < VERTEX_COUNT; i++)
	{
		if(currentNode == newGraph[i].initial)
		{
			neighborNode = nodeToNumber(newGraph[i].terminal);
			if((newGraph[i].weight + nodeWeight[currentNum]) < nodeWeight[neighborNode])
				nodeWeight[neighborNode] = newGraph[i].weight + nodeWeight[currentNum];
		}
	}
}

// Removes a specified vertex from indexing.
void removeVertex(edgeWeight newGraph[], char currentLocation)
{
	for(int i = 0; i < VERTEX_COUNT; i++)
	{
		if(newGraph[i].terminal == currentLocation)
		{
			newGraph[i].initial = 'Z';
			newGraph[i].terminal = 'Z';
		}
	}
}

// Finds the current fringe node with the least weight.
char findLeast(int nodeWeight[])
{
	int base = 42;
	int node;
	char nodeLetter;

	for(int i = 0; i < VERTEX_COUNT/2; i++)
	{
		if(nodeWeight[i] != 0 && nodeWeight[i] < base)
		{
			base = nodeWeight[i];
			node = i;
		}
	}

	nodeLetter = numberToNode(node);
	return nodeLetter;
}