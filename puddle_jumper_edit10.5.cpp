// puddle_jumper_edit10.5.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include <cstdlib> // atoi, strtod
#include <cstring> // strtok
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <limits>

using namespace std;

int calculate_minutes(int start, int end);

// forward declaration so I can use this in Airport
class Flight;

class Airport /* vertex */
{
public:
	// Constructor
	Airport(const string & code) : airport_code(code) {} // Only giving the cconstructor the airport code

	string airport_code;

	// Dijkstra algorithm variables
	bool visited;
	double lowest_to_get_here; // Used for distance, cost, and time
	vector<Flight *> flight_path; // Flight path to reach this airport

	vector<Flight *> outbound_fights;

	void print_flight_path();
};

class Flight /* edge */
{
public:
	string from_to;
	int timeLeave;
	int timeArrive;
	int minutes;
	double cost;
	int distance;
	Airport * source;
	Airport * destination;
};

class Graph
{
	vector<Airport *> airports; /* First variable in graph. Airport is vector. Holds pointer to
								all the airports. Want to iterate through all*/


								/**
								* Returns existing airport or creates new one and returns that
								* if the airport doesn't already exist
								*/
	Airport * get_airport(const string & airport_code)
	{
		Airport * airport = find(airport_code); // Tries to find airport object

												// If airport isn't found, create one and add it to the vector
		if (airport == NULL)
		{
			airport = new Airport(airport_code); // Creates the airport object
			airports.push_back(airport);
		}

		return airport;
	}

	void dijkstra_init(Airport * start);
	void dijkstra_for_distance(Airport * current_airport);
	void dijkstra_for_cost(Airport * current_airport);
	void dijkstra_for_time(Airport * current_airport);
public:

	enum searchTypes { price, distance, time };

	/* Search all airports and return the one with the same
	airport_code, or NULL if it's not found*/
	Airport * find(const string & airport_code)
	{
		for (int i = 0; i < airports.size(); i++)
			if (airports[i]->airport_code == airport_code)
				return airports[i];
		return NULL;
	}

	/* Creates a flight and adds it to the vector of outbound flights
	in the departure airport.  If an airport doesn't exist it is
	created here and added to the Graphs's airport vector.  The flight
	contains the departure and arrival airport, the departure and
	arrival time, the cost, and the distance.  For convenience it also
	has a string for the flight and the minutes.*/
	void add_flight(string depart, string arrive, // Graph normally has a list of vertex and node
		int timeLeave, int timeArrive,
		int distance, double cost)
	{

		Airport * depart_airport = get_airport(depart); // Finds the depart airport and returns it to depart airport. 
														// Will assign airport that already exists or the one we created
		Airport * arrive_airport = get_airport(arrive);

		Flight * flight = new Flight();
		flight->from_to = depart + " to " + arrive;
		flight->timeLeave = timeLeave;
		flight->timeArrive = timeArrive;
		flight->minutes = calculate_minutes(timeLeave, timeArrive);
		flight->distance = distance;
		flight->cost = cost;
		flight->source = depart_airport;
		flight->destination = arrive_airport;

		depart_airport->outbound_fights.push_back(flight); // Pushes list of outbound flights to airport
	}

	// Lists the airport codes so the user knows what codes are available
	void print_codes()
	{
		for (int i = 0; i < airports.size(); i++) // Looping through each airport and printing out each node
			cout << i + 1 << ": " << airports[i]->airport_code << endl; // airports[i] gives pointer to airport
	}


	void find_flights(const string & leave, const string & arrive, searchTypes stype);
};

int main()
{
	Graph airport_graph;

	// Try to open the file
	string flights; // Holds the line that it reads in from file
	ifstream file("puddle jumper.csv"); /*File is opened within constructor for file, constructor opens file.
										ifstream will open the file when the constructor is called*/
	if (!file.is_open()) // Makes sure the file is open
	{
		cout << "Couldn't open the file" << endl;
		return 0;
	}

	// Skip the header
	getline(file, flights);

	// process each flight in the CSV file
	while (file.peek() != EOF)
	{
		file >> flights;
		int pos = 0;
		int count = 0;
		string depart, arrive = "";
		int timeLeave, timeArrive, distance = 0;
		double price = 0;

		int delimiterPos[5];
		for (int i = 0; i < flights.length(); i++)
		{
			if (flights[i] == ',')
			{
				delimiterPos[count++] = pos;
			}
			pos++;
		}

		depart = flights.substr(0, delimiterPos[0]);
		arrive = flights.substr(delimiterPos[0] + 1, (delimiterPos[1] - delimiterPos[0] - 1));
		timeLeave = stoi(flights.substr(delimiterPos[1] + 1, (delimiterPos[2] - delimiterPos[1] - 1)));
		timeArrive = stoi(flights.substr(delimiterPos[2] + 1, (delimiterPos[3] - delimiterPos[2] - 1)));
		distance = stoi(flights.substr(delimiterPos[3] + 1, (delimiterPos[4] - delimiterPos[3] - 1)));
		price = stod(flights.substr(delimiterPos[4] + 1, (delimiterPos[5] - delimiterPos[4] - 1)));

		airport_graph.add_flight(depart, arrive, timeLeave, timeArrive, distance, price);
	}

	file.close();


	string depart;
	string arrive;

	airport_graph.print_codes(); // airport_graph is graph that knows about all the airports

	cout << "Enter Departure Airport:   ";
	cin >> depart;

	cout << "Enter Destination Airport: ";
	cin >> arrive;

	// Force both airiport codes to upper-case
	for (int i = 0; i < depart.size(); i++) depart[i] = toupper(depart[i]);
	for (int i = 0; i < arrive.size(); i++) arrive[i] = toupper(arrive[i]);

	if (!airport_graph.find(depart))
	{
		cout << "Couldn't find departure airport." << endl;
		cout << "There's no airport with code '" << depart << "'" << endl;
	}
	else if (!airport_graph.find(arrive))
	{
		cout << "Couldn't find arrival airport." << endl;
		cout << "There's no airport with code '" << arrive << "'" << endl;
	}
	else
	{
		 // Will find and print path
		airport_graph.find_flights(depart, arrive, Graph::searchTypes::price);
		airport_graph.find_flights(depart, arrive, Graph::searchTypes::distance);
		airport_graph.find_flights(depart, arrive, Graph::searchTypes::time);
	}

	system("pause");
	return 0;
}

int calculate_minutes(int start, int end)
{
	// Separate the hours and minutes from the time integers
	int start_hour = start / 100;
	int start_minute = start % 100;
	int end_hour = end / 100;
	int end_minute = end % 100;

	// If start is greater than the end time then midnight is crossed.
	if (start > end)
	{
		// Calculate minutes until midnight + minutes after midnight
		// Example:  22:20 - 1:15am
		// 24 - 22:20 = 1hour 40 minutes = 60*(24-22 hours) - 20 minutes
		//
		// Add the minutes before midnight to the time after midnight
		// 1:15 - 00:00 = 1 hours + 15 minutes = 60 * (1 hour) + 15 
		return 60 * (24 - start_hour) - start_minute + 60 * end_hour + end_minute;
	}
	else
	{
		return 60 * (end_hour - start_hour) + (end_minute - start_minute);
	}
}


void Graph::dijkstra_init(Airport * start) /* Initializes the starting point to zero, then every other node
										   gets initialized to maximum value for a double */
{
	for (int i = 0; i < airports.size(); i++) // airports.size = number of airports
	{
		// Mark all airports as unvisited
		airports[i]->visited = false;
		airports[i]->flight_path.clear(); // Flight to get there plus cost and distance

		if (airports[i] == start)
		{
			// The start airport has a cost/distance/travel time as zero 
			airports[i]->lowest_to_get_here = 0;
		}
		else
		{
			// All other airports inifinity far away.
			airports[i]->lowest_to_get_here = numeric_limits<double>::max();
		}
	}
}

void Graph::dijkstra_for_distance(Airport * current_airport)
{
	if (current_airport->visited)
	{
		return;
	}

	current_airport->visited = true;

	// Update connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Flight * flight = current_airport->outbound_fights[i];
		Airport * destination = flight->destination;

		double total_distance = current_airport->lowest_to_get_here + flight->distance;

		if (total_distance < destination->lowest_to_get_here)
		{
			destination->lowest_to_get_here = total_distance;
			destination->flight_path = current_airport->flight_path;
			destination->flight_path.push_back(flight);
		}
	}

	// Update all connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Airport * next_airport = current_airport->outbound_fights[i]->destination;
		dijkstra_for_distance(next_airport);
	}
}

void Graph::dijkstra_for_cost(Airport * current_airport)
{
	if (current_airport->visited)
	{
		return;
	}

	current_airport->visited = true;

	// Update connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Flight * flight = current_airport->outbound_fights[i];
		Airport * destination = flight->destination;

		double total_cost = current_airport->lowest_to_get_here + flight->cost;

		if (total_cost < destination->lowest_to_get_here)
		{
			destination->lowest_to_get_here = total_cost;
			destination->flight_path = current_airport->flight_path;
			destination->flight_path.push_back(flight);
		}
	}

	// Update all connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Airport * next_airport = current_airport->outbound_fights[i]->destination;
		dijkstra_for_cost(next_airport);
	}
}

void Graph::dijkstra_for_time(Airport * current_airport)
{
	if (current_airport->visited)
	{
		return;
	}

	current_airport->visited = true;

	Flight * last_flight = NULL;

	// Look at the last flight to get here for the arrival time;
	if (current_airport->flight_path.size() > 0)
	{
		size_t last_index = current_airport->flight_path.size() - 1;
		last_flight = current_airport->flight_path[last_index];
	}


	// Update connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Flight * flight = current_airport->outbound_fights[i];
		Airport * destination = flight->destination;

		int layover_minutes = 0;

		if (last_flight)
		{
			int timeArrive = last_flight->timeArrive;
			layover_minutes = calculate_minutes(timeArrive, flight->timeLeave);

		}

		double total = current_airport->lowest_to_get_here +
			layover_minutes +
			flight->minutes;

		if (total < destination->lowest_to_get_here)
		{
			destination->lowest_to_get_here = total;
			destination->flight_path = current_airport->flight_path;
			destination->flight_path.push_back(flight);
		}
	}

	// Visit all connected airports
	for (int i = 0; i < current_airport->outbound_fights.size(); i++)
	{
		Airport * next_airport = current_airport->outbound_fights[i]->destination;
		dijkstra_for_time(next_airport);
	}
}



void Graph::find_flights(const string & leave, const string & arrive, Graph::searchTypes stype)
{
	Airport * start = find(leave);
	Airport * end = find(arrive);
	dijkstra_init(start);

	switch (stype)
	{
	case Graph::searchTypes::distance:
		dijkstra_for_distance(start);
		cout << endl << "Shortest: " << end->lowest_to_get_here << " miles" << endl;
		break;

	case Graph::searchTypes::price:
		dijkstra_for_cost(start);
		cout << endl << "Cheapest: $" << end->lowest_to_get_here << endl;
		break;

	case Graph::searchTypes::time:
		dijkstra_for_time(start);
		cout << endl << "Fastest: " << end->lowest_to_get_here << " minutes" << endl;
		break;
	}

	end->print_flight_path();
}

void Airport::print_flight_path()
{
	unsigned MAX_INDEX = this->flight_path.size() - 1;

	for (int i = 0; i < this->flight_path.size(); i++)
	{
		Flight * flight = this->flight_path[i];

		cout << flight->from_to
			<< "  $" << flight->cost
			<< "   leave " << flight->timeLeave << " arrive " << flight->timeArrive
			<< " (" << flight->minutes << " minutes)   "
			<< flight->distance << " miles" << endl;

		if (i < MAX_INDEX)
		{
			Flight * next_flight = this->flight_path[i + 1];
			int minutes = calculate_minutes(flight->timeArrive, next_flight->timeLeave);

			if (minutes > 59)
			{
				int hours = minutes / 60;
				minutes = minutes % 60;
				cout << "layover: " << hours << " hours " << minutes << " minutes." << endl;
			}
			else
			{
				cout << "layover: " << minutes << " minutes." << endl;
			}

		}
	}
}