/****************************************************************
 * Header file for Main.cpp
 *
 * Authors: Tien Ho and Brandon Hostetter
 * Date: 4 December 2015
 *
 **/

#include <map>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <ctime>

using namespace std;

class Main {
public:
	double computeSimilarity(vector<double>& query_data, vector<double>& template_data);
	multimap<double, int> find10NearestNeighbors(vector<double>& query_data, vector<vector<double> > template_data);
	void quickSort(vector<string>& data, int left, int right);
	void print(multimap<double, int> image_set);

	void readFile();
	int main();
	virtual ~Main();
};