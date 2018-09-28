#include <arpa/inet.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include"MnistPreProcess.h"

using namespace std;


static int
_read_int(FILE *fp)
{
	int	tmp;

	assert(fread(&tmp, sizeof(tmp), 1, fp) == 1);
	return ntohl(tmp);
}


void readData(
		vector< vector< float > > &dataset,
		vector< float > &labels,
		int &n_classes,
		const char *dataPath,
		const char *labelPath)
{
	FILE* dataFile=fopen(dataPath,"rb");
	int mbs = _read_int(dataFile);
	assert(mbs == 0x0803 || mbs == 0xfe03);
	int number = _read_int(dataFile);
	int row = _read_int(dataFile);
	int col = _read_int(dataFile);

	dataset.resize(number);
	for (int i = 0;  i < number;  i++)
	{
		dataset[i].resize(row * col);
		for (int j = 0;  j < row * col;  j++)
		{
		    if (mbs == 0x0803)
		    {
	            unsigned char byte_element;
			    fread(&byte_element, sizeof(byte_element), 1, dataFile);
			    dataset[i][j] = static_cast<float>(byte_element);
			} else {
	            float   float_element;
			    fread(&float_element, sizeof(float_element), 1, dataFile);
			    dataset[i][j] = static_cast<float>(float_element);
			}
		}
	}
	fclose(dataFile);

	FILE* labelFile=fopen(labelPath,"rb");
	bool should_count = (n_classes == 0);
	mbs = _read_int(labelFile);
	assert(mbs == 0x801);
	assert(_read_int(labelFile) == number);
	labels.resize(number);
	for (int i = 0;  i < number;  i++)
	{
	    unsigned char class_id;
		fread(&class_id, sizeof(class_id), 1, labelFile);
		if (should_count)
		{
			n_classes = max(n_classes, (int) class_id + 1);
		} else {
			assert((int)class_id < n_classes);
		}
		labels[i] = static_cast<float>(class_id);
	}
	fclose(labelFile);
};


static float
calculate_weight(
	const vector < vector <float> > &trainset,
	int feature)
{
	double sum = 0, sum2 = 0;
	int n_samples = trainset.size();

	for (int i = 0;  i < n_samples;  i++)
	{
		const float &v = trainset[i][feature];
		sum += v;
		sum2 += v * v;
	}

	return sum2 < 1e-5 ? 0 : sqrt(sum2 - sum * sum / n_samples);
}


void
init_weight_map(
	const vector < vector <float> > &trainset,
	map <float, int> &weight_map)
{
	vector <float> weight;
	int n_samples = trainset[0].size();

	weight.resize(n_samples);
	weight_map.clear();

	for (int i = 0;  i < n_samples;  i++)
	{
		weight[i] = calculate_weight(trainset, i);
	}

	double total_weight = 0;
	for (int i = 0;  i < n_samples;  i++)
	{
		total_weight += weight[i];
		weight_map[total_weight] = i;
	}
}
