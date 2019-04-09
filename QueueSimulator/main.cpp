//
//  main.cpp
//  QueueSimulator
//
//  Created by Marshal Foster on 3/30/19.
//  Copyright © 2019 Marshal Foster. All rights reserved.
//

#include <iostream>
#include <queue>
#include <random>
#include <vector>
#include <algorithm>
#include <cmath>

class Customer {
public:
	int totalServiceTime;
	int serviceTime;
	int enterQueueTime;
	Customer(const int serviceTime_) {
		serviceTime = serviceTime_;
	}
};

class Clerk {
public:
	int startServiceTime = 0;
	int endServiceTime = 0;
	
	bool isFree(const int time, const int serviceTime) {
		return time >= endServiceTime && time < 43200;
	}
	
	bool join(const int time, const int serviceTime) {
		if (isFree(time, serviceTime)) {
			startServiceTime = time;
			endServiceTime = time + serviceTime;
			return true;
		} else {
			return false;
		}
	}
};

class Bank {
private:
	Clerk clerks[6];
public:
	std::queue<Customer> inLine;
	std::vector<int> totalWaitTimes;
	bool clerkAvalible(const int time, const int serviceTime) {
		for (int i = 0; i < 6; i++) {
			if (clerks[i].isFree(time, serviceTime))
				return true;
		}
		return false;
	}
	
	bool joinClerk(const int time, Customer &customer) {
		for (int i = 0; i < 6; i++) {
			if (clerks[i].isFree(time, customer.serviceTime))
				return clerks[i].join(time, customer.serviceTime);
		}
		return false;
	}
};

class Line {
public:
	int waitTime = 0;
	
	// returns the total wait time for the joined customer or
	// -1 if the customer cannot complete their service before store closes.
	int joinLine(const int time, Customer &customer) {
		//update waitTime
		if (waitTime <= time)
			waitTime = time + customer.serviceTime;
		else
			waitTime += customer.serviceTime;
		
		if (waitTime >= 43200) {
            waitTime = rand() % 43200; // assign new rand number with maximum mod rather than -1
			return waitTime += customer.serviceTime;
        } else
			return waitTime - time;
	}
};

class Market {
private:
	Line lines[6];
	
	int getSortestWaitLine() {
		int shortest = 0;
		for (int i = 1; i < 6; i++) {
			if (lines[i].waitTime < lines[shortest].waitTime)
				shortest = i;
		}
		return shortest;
	}
public:
	std::vector<int> totalWaitTimes;
	
	// returns the total wait time for the joined customer
	int joinShortestLine(const int time, Customer customer) {
		return lines[getSortestWaitLine()].joinLine(time, customer);
	}
};

int getServiceTime(std::default_random_engine &generator, const double maxServiceTime) {
	int maxServiceTimeSec = maxServiceTime * 60;
	std::uniform_int_distribution<int> distribution(0, maxServiceTimeSec+1);
	return distribution(generator);
}

std::queue<Customer> generateTotalCustomers(double customesPerMin, double maxServiceTime, int seed = 100) {
	// 43,200 / 60 = 720
	int maxDailyCustomers = 720 * customesPerMin;
	std::queue<Customer> ret;
	std::default_random_engine generator = std::default_random_engine(seed);
	for(int i = 0; i < maxDailyCustomers; i++) {
		ret.push(Customer(getServiceTime(generator, maxServiceTime)));
	}
	return ret;
}

// Uses RNG to determin even enterence time.
// bool enterQueue(std::default_random_engine &generator, double customeArrivalRate) {
// 	double enterOdds = customeArrivalRate/60.00;
// 	std::uniform_real_distribution<double> distribution(0.0, 1.0);
// 	double random = distribution(generator);
// 	return random <= enterOdds;
// }

double getPercentileTime(const double percentile, const std::vector<int> &data) {
	// 20th percentile: .2(N) = index of 20th percentile (starting at index 0)
	int index = percentile * data.size();
	return (double) data[index] / 60;
}

bool toEnter(double cpm, int time) {
	int val = 60 / cpm;
	return time % val == 0;
	}

struct buildings {
    Bank bank;
    Market market;
    int enteredCustomers;
    buildings(Bank bank, Market market, int customers) {
        this->bank = bank;
        this->market = market;
        this->enteredCustomers = customers;
    }
};

struct::buildings Driver(int secondsInDay, double cpm, std::queue<Customer> q) {
    Bank bank;
    Market market;
    // entering building
	int enteredCustomers = 0;
	for (int time = 0; time < secondsInDay; time++) {
		if (!q.empty() && toEnter(cpm, time)) {
			enteredCustomers++;
			// joining bank line
			bank.inLine.push(q.front());
			bank.inLine.back().enterQueueTime = time;

			//joining market
			market.totalWaitTimes.push_back(market.joinShortestLine(time, q.front()));
			q.pop();
		}
			//leaving bank line joining teller
			while (!bank.inLine.empty() && bank.clerkAvalible(time, bank.inLine.front().serviceTime)) {
				bank.joinClerk(time, bank.inLine.front());
				bank.inLine.front().totalServiceTime = (time - bank.inLine.front().enterQueueTime) + bank.inLine.front().serviceTime;
				bank.totalWaitTimes.push_back(bank.inLine.front().totalServiceTime);
				bank.inLine.pop();
			}
	}

	//Sorting service times
	std::sort(bank.totalWaitTimes.begin(), bank.totalWaitTimes.end());
    buildings _building = buildings(bank, market, enteredCustomers);
    return _building;
}

void calculate(buildings _building) {
    //Caclulating results
      // refactor removal of -1 wait times
	// market.totalWaitTimes.erase(remove( market.totalWaitTimes.begin(), market.totalWaitTimes.end(), -1 ), market.totalWaitTimes.end());
	// std::sort(market.totalWaitTimes.begin(), market.totalWaitTimes.end());
	double tenth = getPercentileTime(0.10, _building.bank.totalWaitTimes);
	double fifty = getPercentileTime(0.50, _building.bank.totalWaitTimes);
	double ninty = getPercentileTime(0.90, _building.bank.totalWaitTimes);
	double Mtenth = getPercentileTime(0.10, _building.market.totalWaitTimes);
	double Mfifty = getPercentileTime(0.50, _building.market.totalWaitTimes);
	double Mninty = getPercentileTime(0.90, _building.market.totalWaitTimes);
	std::cout << _building.enteredCustomers << " customers entered\n";
	std::cout << "Bank service times in minutes: 10th %ile " << tenth << ", 50th %ile " << fifty << ", 90th %ile " << ninty << '\n';
	std::cout << "Total customers served at the Bank: " << _building.bank.totalWaitTimes.size() << '\n';
	std::cout << "Market service times in minutes: 10th %ile " << Mtenth << ", 50th %ile " << Mfifty << ", 90th %ile " << Mninty << '\n';
	std::cout << "Total customers served at the Market: " << _building.market.totalWaitTimes.size() << '\n';
}

int main(int argc, const char * argv[]) {
	double cpm = std::stof(argv[1]);
	double maxServicetime = std::stof(argv[2]);;
	int seed = std::stoi(argv[3]);;
  // initalization
	std::queue<Customer> q = generateTotalCustomers(cpm, maxServicetime, seed);
	int secondsInDay = 43200;
	
	// generating daily customers
	int qSize = q.size();
	//std::cout << "q size: " << qSize << '\n';

    buildings _building = Driver(secondsInDay, cpm, q);
	calculate(_building);

  return 0;
}
