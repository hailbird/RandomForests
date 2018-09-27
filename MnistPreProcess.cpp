#include <arpa/inet.h>
#include <assert.h>
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
	unsigned char element;
	FILE* dataFile=fopen(dataPath,"rb");
	int mbs = _read_int(dataFile);
	assert(mbs == 0x0803);
	int number = _read_int(dataFile);
	int row = _read_int(dataFile);
	int col = _read_int(dataFile);

	dataset.resize(number);
	for (int i = 0;  i < number;  i++)
	{
		dataset[i].resize(row * col);
		for (int j = 0;  j < row * col;  j++)
		{
			fread(&element, sizeof(element), 1, dataFile);
			dataset[i][j] = static_cast<float>(element);
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
		fread(&element, sizeof(element), 1, labelFile);
		if (should_count)
		{
			n_classes = max(n_classes, (int) element + 1);
		} else {
			assert((int)element < n_classes);
		}
		labels[i] = static_cast<float>(element);
	}
	fclose(labelFile);
};
