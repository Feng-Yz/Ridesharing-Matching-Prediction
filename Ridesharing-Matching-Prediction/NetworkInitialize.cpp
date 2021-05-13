//
//  NetworkInitialize.cpp
//  Ridesharing-Matching-Prediction
//
//  Created by YzFENG on 2021/5/6.
//

#include "Network.hpp"
#include <random>
#include <algorithm>

Network::Network(int m, int n,
                 double pickupTime,
                 double maxDetourTime,
                 double searchRadius,
                 double speed)
{
    network = vector<vector<int>>(m, vector<int>(n)); // initialize the network vector
    size = pair<int, int>{m, n}; // initialize the size of the network
    odPairs = vector<OriginDestinationPair>(); // initialize the od pairs vector
    _pickupTime = pickupTime; // initialize the pick up time avg{t^{pk}}
    _maxDetourTime = maxDetourTime; // initialize the max detour time avg{D}
    _searchRadius = searchRadius; // initialize the search radius avg{R}
    _speed = speed; // initialize the speed
}

pair<int, int> Network::positionIndexToPair(int randInt,
                                   int xmin, int ymin, int xmax, int ymax)
{
    int xRange = xmax - xmin + 1;
    int y = ymin + randInt % xRange;
    int x = xmin + randInt / xRange;
    return pair<int, int>{x, y};
}

void Network::sortSeekerTaker()
{
    for (int i = 0; i < seekerTaker.size(); i++) {
//        sort(seekerTaker[i].begin(), seekerTaker[i].end(), compareMatchPriority);
        sort(seekerTaker[i].begin(), seekerTaker[i].end(), [] (Match m1, Match m2)
             {
            return m1.getPriority() > m2.getPriority();
        });
    }
}

void Network::generateODPairs(int number, double lambda,
                     int xmin, int ymin, int xmax, int ymax)
{
    int xRange = xmax - xmin + 1;
    int yRange = ymax - ymin + 1;
    int destUniformRandNumRange = xRange * yRange;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> destDistribution(0, destUniformRandNumRange - 1);
    std::uniform_int_distribution<int> originDistribution(0, (size.first * size.second) - 1);
    for (int i = 0; i < number; i++) {
        odPairs.push_back(OriginDestinationPair
                          (positionIndexToPair(originDistribution(generator),
                                               0, 0, size.first-1, size.second-1),
                           positionIndexToPair(destDistribution(generator),
                                               xmin, ymin, xmax, ymax),
                           lambda));
    }
    generateSeekerStates(); // generate all the seeker states
    generateTakerStates(); // generate all the taker states
    generateMatches();
}

void Network::generateSeekerStates()
{
    seekerStates = vector<SeekerState>();
    for (int i = 0; i < odPairs.size(); i++) {
        seekerStates.push_back(SeekerState(odPairs.at(i)));
    }
}

void Network::generateTakerStates()
{
    takerStates = vector<vector<TakerState>>(odPairs.size());
    for (int i = 0; i < odPairs.size(); i++) {
        auto odLinks = odPairs.at(i).generateLinks();
        for (int j = 0; j < odLinks.size(); j++) {
            takerStates[i].push_back(TakerState(odLinks.at(j), odPairs.at(i), _speed, _pickupTime));
        }
    }
}

void Network::generateMatches()
{
    matches = vector<Match>();
    seekerTaker = vector<vector<Match>>(seekerStates.size());
    takerSeeker = vector<vector<Match>>(takerStates.size());
    for (int i = 0; i < seekerStates.size(); i++) {
        for (int j = 0; j < takerStates.size(); j++) {
            for (int k = 0; k < takerStates[j].size(); k++) {
                auto detourShareDistance =
                takerStates.at(j).at(k).detourShareDistanceCal(seekerStates.at(i));
                auto pickupDistance = takerStates.at(j).at(k).currentDistanceCal(seekerStates.at(i));
                if (pickupDistance <= _searchRadius) {
                    if (std::get<1>(detourShareDistance) < _maxDetourTime * _speed) {
                        Match newMatch = Match(i, j, k,
                                               std::get<2>(detourShareDistance),
                                               std::get<1>(detourShareDistance),
                                               pickupDistance);
                        matches.push_back(newMatch);
                        seekerTaker.at(i).push_back(newMatch);
                        takerSeeker.at(j).push_back(newMatch);
                    }
                }
            }
        }
    }
    sortSeekerTaker();
}

void Network::printPairs()
{
    for (int i = 0; i < odPairs.size(); i++) {
        printf("The %dth pair: ", i + 1);
        odPairs.at(i).odPairPrint();
        printf("\n");
    }
}

void Network::printStates()
{
    printf("The seeker states: \n");
    for (int i = 0; i < seekerStates.size(); i++) {
        seekerStates.at(i).printState();
        printf("\n");
    }
    printf("The taker states: \n");
    for (int i = 0; i < takerStates.size(); i++) {
        printf("The %dth OD pair: \n", i);
        for (int j = 0; j < takerStates.at(i).size(); j++) {
            takerStates.at(i).at(j).printState();
            printf("\n");
        }
    }
}

void Network::printMatches()
{
    for (int i = 0; i < seekerTaker.size(); i++) {
        for (int j = 0; j < seekerTaker.at(i).size(); j++) {
            seekerTaker.at(i).at(j).print();
        }
    }
}