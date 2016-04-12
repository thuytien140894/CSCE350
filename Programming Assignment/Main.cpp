/*************************************************************************
* Program to recognize human facial action units by computing the cosine
* similarity between the query and template facial images. These images
* are represented as vectors of doubles read from the provided query and
* template files. For each query data, 10 nearest neighbors from its
* corresponding template data are outputed.
*
* Authors: Tien Ho and Brandon Hostetter
* Date: 4 December 2015
*
**/

#include "Main.h"

// Global constants
const int NUMBER_OF_ELEMENTS_PER_VECTOR = 5632;
const int NUMBER_OF_VECTORS_PER_TEMPLATE = 138;
const int NUMBER_OF_TEMPLATES = 47;
const int NUMBER_OF_QUERIES = 141;
const string TEMPLATE_NAME = "template";
const string FIRST_QUERY_NAME = "_AU01_query";
const string SECOND_QUERY_NAME = "_AU12_query";
const string THIRD_QUERY_NAME = "_AU17_query";

// Global variables
vector<vector<vector<double> > > template_set(NUMBER_OF_TEMPLATES);
vector<vector<double> > query_set(NUMBER_OF_QUERIES);
ofstream outputFile;

/**
* This method implements the quicksort algorithm to sort the file
* names lexicographically.
*
* Inputs:
* 		vector<string> containing the data to be sorted
*		int left to indicate the beginning index of a vector
*		int right to indicate the end index of a vector
**/
void quickSort(vector<string>& data, int left, int right) {
	if (left >= right) {
		return;
	}

	int middle = left + (right - left) / 2;
	swap(data[middle], data[left]);
	int tempLeft = left;

	for (int i = tempLeft; i < right; ++i) {
		if (data[i] <= data[left]) {
			swap(data[i], data[tempLeft]);
			tempLeft++;
		}
	}

	int pivot = tempLeft - 1;
	swap(data[left], data[pivot]);
	quickSort(data, left, pivot);
	quickSort(data, pivot + 1, right);
} // void quickSort

/**
* This method reads the data from the query and template
* files and stores them in vectors of doubles. Each query file
* contains a vector of 5,632 data points, and each template file
* contains 138 vectors of 5,632 data points each.
**/
void readFile() {
	DIR* dir = opendir("data");
	dirent* pdir;
	vector<string> files;
	double next;
	string fileName = "";
	int templateCounter = 0;
	int queryCounter = 0;

	// Read the file names from the directory
	while ((pdir = readdir(dir))) {
		files.push_back(pdir->d_name);
	}

	// Sort the file names lexicographically so that 
	// the query and template files with the same subject 
	// ID are grouped together.
	quickSort(files, 0, files.size());

	// The first two file names are . and .. which 
	// indicates the current and previous directories
	// and therefore are ignored.
	for (unsigned int i = 2; i < files.size(); i++) {
		fileName = "data/" + files[i];
		ifstream file(fileName.c_str());

		if (file.is_open()) {
			if (fileName.find(TEMPLATE_NAME) != string::npos) { // Parse template files
				vector<vector<double> > template_image_set(NUMBER_OF_VECTORS_PER_TEMPLATE);
				for (int j = 0; j < NUMBER_OF_VECTORS_PER_TEMPLATE; j++) {
					vector<double> template_image(NUMBER_OF_ELEMENTS_PER_VECTOR);
					for (int k = 0; k < NUMBER_OF_ELEMENTS_PER_VECTOR; k++) {
						file >> next;
						template_image.at(k) = next;
					}

					template_image_set.at(j) = template_image;
				}

				template_set.at(templateCounter) = template_image_set;
				templateCounter++;
			}
			else { // Parse query data
				vector<double> query_image(NUMBER_OF_ELEMENTS_PER_VECTOR);
				for (int i = 0; i < NUMBER_OF_ELEMENTS_PER_VECTOR; i++) {
					file >> next;
					query_image.at(i) = next;
				}

				query_set.at(queryCounter) = query_image;
				queryCounter++;
			}
		}
		else {
			cout << "Unable to open file" << endl;
		}

		file.close();
	}
} // void readFile

/**
* Implements the cosine simlarity algorithm to compute the similarity value
* between the query and template file passed in.
*
* Inputs:
* 		vector<double> containing the data for a single query file
*		vector<double> containing the data for a single row from a template file
*
* Returns:
* 		double similarity value
**/
double computeSimilarity(vector<double>& query_data, vector<double>& template_data) {
	double numerator = 0;
	double denomQ = 0;
	double denomT = 0;

	for (int i = 0; i < NUMBER_OF_ELEMENTS_PER_VECTOR; i++) {
		numerator += (query_data[i] * template_data[i]);
		denomQ += (query_data[i] * query_data[i]);
		denomT += (template_data[i] * template_data[i]);
	}

	return (numerator / (sqrt(denomQ) * sqrt(denomT)));
} // double computeSimilarity

/**
* The similarity value between the query and template files is calculated for each template file
* and the value is pushed onto the multimap neighbors. Neighbors is sorted in ascending order so
* we erase the first 128 values of map. This leaves us with the the last 10 elements in the
* map which are the neighbors with the highest similarity values.
*
* Inputs:
*		vector<double> containing the data for a single query file
* 		vector<vector<double> > containing the data for a single template file
*
* Returns:
* 		multimap<double, int> - containing the 10 nearest neighbors
**/
multimap<double, int> find10NearestNeighbors(vector<double>& query_data, vector<vector<double> >& template_data) {
	multimap<double, int> neighbors;

	for (int i = 0; i < NUMBER_OF_VECTORS_PER_TEMPLATE; i++) {
		double similarityValue = computeSimilarity(query_data, template_data[i]);
		neighbors.insert(pair<double, int >(similarityValue, i + 1));
	}

	// Retrieve the last 10 elements of the multimap, which have 
	// the largest similarity values
	multimap<double, int>::iterator iter = neighbors.begin();
	for (int i = 0; i < (NUMBER_OF_VECTORS_PER_TEMPLATE - 10); i++) {
		neighbors.erase(iter);
		advance(iter, 1);
	}

	return neighbors;
} // multimap<double, int> find10NearestNeighbors

/**
* This method prints the values in the multimap image_set to the outputFile.
* The multimap is sorted in ascending order so a reverse iterator is used
* to arrange the data from highest similiarty value to lowest similarity value.
*
* Inputs:
*		multimap<double, int> containing the similarity values and indexes of the
10 closest neighbors
*		int index is the value of the subject ID, from 1 to 47
* 		string fileName is the name of the query file
**/
void print(multimap<double, int> image_set, int index, string fileName) {
	outputFile << index << fileName << "		";

	// Map is in ascending order; reverse to get correct order
	multimap<double, int>::reverse_iterator iter;
	for (iter = image_set.rbegin(); iter != image_set.rend(); ++iter) {
		outputFile << iter->second << " ";
	}

	outputFile << endl;
} // void print

/**
* The main method calls the readFile method to read the data from all of the
* query and template files. Next, the find10NearestNeighbors method is called
* for each query file and the results are outputted to the log file by calling
* the print method. Finally, the total execution time for the program is
* calculated and appended at the bottom of the output log file.
**/
int main(int argc, char *argv[]) {
	// Start the time of execution
	std::clock_t start = std::clock();

	outputFile.open("log.txt");

	readFile();

	int index = 0;
	int counter = 0;

	// Find the 10-nearest neighbors for each of the queries using its corresponding template
	// and output them to the log file
	for (unsigned int i = 0; i < template_set.size(); i++) {
		counter = i + 1;
		index = i * 3; // each template is used for three queries
		multimap<double, int> query_AU01 = find10NearestNeighbors(query_set[index], template_set[i]);
		print(query_AU01, counter, FIRST_QUERY_NAME);
		multimap<double, int> query_AU12 = find10NearestNeighbors(query_set[index + 1], template_set[i]);
		print(query_AU12, counter, SECOND_QUERY_NAME);
		multimap<double, int> query_AU17 = find10NearestNeighbors(query_set[index + 2], template_set[i]);
		print(query_AU17, counter, THIRD_QUERY_NAME);
	}

	// End the time of execution
	double totalTime = (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000);
	cout << "Time: " << totalTime << " ms" << endl;
	outputFile << endl << "Time: " << totalTime << " ms" << endl;

	outputFile.close();

	return 0;
} // int main
