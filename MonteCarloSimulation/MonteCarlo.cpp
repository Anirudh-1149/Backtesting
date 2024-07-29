#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <boost/tokenizer.hpp>
#include <eigen3/Eigen/Dense>
#include <mutex>
#include <thread>
#include <map>

using namespace std;

mutex initialize_mtx;

vector<string> stocks = {
    "AAPL", "AMGN", "AXP", "BA", "CAT", "CRM", "CSCO", "CVX", "DIS", "DOW",
    "GS", "HD", "HON", "IBM", "INTC", "JNJ", "JPM", "KO", "MCD", "MMM",
    "MRK", "MSFT", "NKE", "PG", "TRV", "UNH", "V", "VZ", "WBA", "WMT"
};

const int numStocks = stocks.size();
const int numSimulations = 100000;
const int numDays = 128;

vector<double> means(numStocks);
vector<double> variances(numStocks);
vector<vector<double>> result(30,vector<double>(100000));
vector<double> dwt(numDays);
vector<double> relativeVariance(numDays);
vector<vector<double>> stockReturns(numStocks);
vector<double> correlation(numStocks);
random_device rd;
mt19937 gen(rd());
normal_distribution<> dis(0.0, 1.0);
vector<vector<double>> stockPrices(numStocks);


string getFilePath(string stock_symbol)
{
    string file_path = "Data/" + stock_symbol + "_data.csv";
    return file_path;
}

double calculateMean(const vector<double>& data) {
    double sum = 0;
    for (double value : data) {
        sum += value;
    }
    return sum / data.size();
}

double calculateVariance(const vector<double>& data, double mean) {
    double sum = 0;
    for (double value : data) {
        double diff = value - mean;
        sum += diff * diff;
    }
    return sum / (data.size() - 1);
}

void readCSV(const string& filename, int ind) {
    string file_path = getFilePath(filename);
    ifstream file(file_path);
    string line;
    vector<double> prices;
    boost::char_separator<char> sep(",");
    getline(file, line);

    while (getline(file, line)) {
        boost::tokenizer<boost::char_separator<char>> tok(line, sep);
        auto it = tok.begin();
        advance(it, 5);
        prices.push_back(stod(*it));
    }

    int n = prices.size();
    double mean = calculateMean(prices);
    double variance = calculateVariance(prices, mean);
    for(int i = 0;i<numSimulations;i++)
        result[ind][i] = prices[n-1];
    means[ind] = mean;
    variances[ind] = variance;
    stockPrices[ind] = prices;
}

void buildW0()
{
    for(int i = 0;i<numDays;i++)
    {
        dwt[i] = dis(gen);
    }
}

void initiazeData()
{
    vector<thread> threads;
    for(int i = 0;i<numStocks;i++)
    {
        threads.emplace_back(readCSV,stocks[i],i);
    }
    threads.emplace_back(buildW0);

    for(thread& t : threads)
        t.join();
}

void computeCovarianceMatrix() {
    int n = stockPrices[0].size();
    for (int i = 0; i < numStocks; ++i) 
    {
        double cov = 0;
        for(int j = 0;j<n;j++)
        {
            cov += (stockPrices[i][j] - means[i]) * (stockPrices[0][j] - means[0]);
        }
        cov/=n;
        correlation[i] = cov/sqrt(variances[i]*variances[0]);
    }
}

void silutionPerStock(int ind)
{
    for(int i =0;i<numDays;i++)
    {
        for(int j = 0;j<numSimulations;j++)
        {
            int rm = dis(gen);
            int dw = correlation[ind] * dwt[i] * sqrt(1-pow(correlation[ind],2)) * rm;
            result[ind][j] = result[ind][j] * exp(means[ind] - variances[ind]/2 + dw * sqrt(variances[ind]));
        }
    }
}

void simulate()
{
    vector<thread> threads;
    for(int i = 0;i<numStocks;i++)
        threads.emplace_back(silutionPerStock, i);
    
    for(thread &t: threads)
        t.join();
}
int main()
{
    initiazeData();
    computeCovarianceMatrix();

    // removing extra memory
    stockPrices.clear();
    simulate();

    return 0;
}