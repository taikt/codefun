// server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <fstream>
#include "Reactor.h"
#include "AlprServer.h"
#include "DeadlineTimer.h"
#include <iomanip>
using namespace std;

#define TRACE_INTERVAL 10000

int main()
{
    AlprServer* svc = new AlprServer();
  
    svc->initiate();
    
    // start tracing query
    CreateDirectory(L"server_tracking", NULL);

    std::ofstream outfile;
    outfile.open("server_tracking\\average_query.txt");

    outfile << setw(30) << left << "Timestamp" << setw(12) << left << "User" << setw(25) << left << "Average query/sec" << setw(30) << left << "Total(All Users)" << "\n";
    outfile.close();

    outfile.open("server_tracking\\partial_match.txt");
    outfile << setw(30) << left << "Timestamp" << setw(12) << left << "User" << setw(25) << left << "Partial match" << setw(30) << left << "Total(All User)" << "\n";
    outfile.close();

    outfile.open("server_tracking\\no_match.txt");
    outfile << setw(30) << left << "Timestamp" << setw(12) << left << "User" << setw(25) << left << "No match" << setw(30) << left << "Total(All User)" << "\n";
    outfile.close();

    deadline_timer query_timer;
    query_timer.start(TRACE_INTERVAL, std::bind((&AlprServer::track_average_query)));
    query_timer.start(TRACE_INTERVAL, std::bind((&AlprServer::track_partial_match)));
    query_timer.start(TRACE_INTERVAL, std::bind((&AlprServer::track_no_partial_match)));

    // start reactor
    Reactor::getInstance()->startRunning();
}



