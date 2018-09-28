#include <set>
#include"Sample.h"

using namespace std;

Sample::Sample(
	const vector < vector <float> > &dataset,
	const vector <float> &labels,
	int classNum,
	int sampleNum,
	int featureNum)
	: _dataset(dataset),
	  _labels(labels),
	  _sampleNum(sampleNum),
	  _featureNum(featureNum),
	  _classNum(classNum),
	  _selectedSampleNum(sampleNum),
	  _selectedFeatureNum(featureNum),
	  _sampleIndex(NULL),
	  _featureIndex(NULL)
{
}

Sample::Sample(Sample* samples)
	: _dataset(samples->_dataset),
	  _labels(samples->_labels),
	  _classNum(samples->getClassNum()),
	  _featureNum(samples->getFeatureNum()),
	  _sampleNum(samples->getSampleNum()),
	  _selectedSampleNum(samples->getSelectedSampleNum()),
	  _selectedFeatureNum(samples->getSelectedFeatureNum()),
	  _sampleIndex(samples->getSampleIndex()),
	  _featureIndex(samples->getFeatureIndex())
{
}

Sample::Sample(Sample* samples,int start,int end)
	: _dataset(samples->_dataset),
	  _labels(samples->_labels),
	  _classNum(samples->getClassNum()),
	  _featureNum(samples->getFeatureNum()),
	  _sampleNum(samples->getSampleNum()),
	  _selectedSampleNum(end-start+1),
	  _selectedFeatureNum(samples->getSelectedFeatureNum()),
	  _featureIndex(NULL)
{
	_sampleIndex = new int[_selectedSampleNum];
	memcpy(_sampleIndex,samples->getSampleIndex()+start,sizeof(float)*_selectedSampleNum);
}

Sample::~Sample()
{
	_sampleIndex=NULL;
	_featureIndex=NULL;
}

void Sample::randomSelectSample(int*sampleIndex,int SampleNum,int selectedSampleNum)
{
	_sampleNum=SampleNum;
	_selectedSampleNum=selectedSampleNum;
	if(_sampleIndex!=NULL)
	{delete[] _sampleIndex;}
	_sampleIndex=sampleIndex;

	//sampling trainset with replacement
	for(int i = 0;  i < selectedSampleNum;  i++)
	{
		_sampleIndex[i] = rand() % SampleNum;
	}
}
void Sample::randomSelectFeature(int*featureIndex, int featureNum, int selectedFeatureNum)
{
	_featureNum=featureNum;
	_selectedFeatureNum=selectedFeatureNum;
	_featureIndex=featureIndex;

	//sampling feature without replacement
#if 0   // baoh: I think this method unfairly emphasizes the bottom line.
	for (int i = 0, j = featureNum - selectedFeatureNum;  j < featureNum; j++, i++)
	{
	    int k, index = (j == 0) ? 0 : rand() % j;
		for(k = 0;  k < i;  k++)
		{
			if(_featureIndex[k]==index)
				break;
		}
		_featureIndex[i] = (k < i) ? j : index;
	}
#else
    set <int> selected;
    static const int row = 28;
    static const int col = 28;
    int i, this_row, this_col;
#if 1
    for (i = 0;  i < selectedFeatureNum * (1 - 0.618);  i++)
    {
        do
        {
            do
            {
                this_row = 5 + rand() % 18;
                this_col = 5 + rand() % 18;
            }
            while (this_row > 10 && this_row < 17 && this_col > 10 && this_col < 17);
            _featureIndex[i] = row * this_row + this_col;
        }
        while (selected.find(_featureIndex[i]) != selected.end());
        selected.insert(_featureIndex[i]);
    }
#else
    for (i = 0;  i < selectedFeatureNum / 2;  i++)
    {
        do
        {
            do
            {
                this_row = 4 + rand() % 20;
                this_col = 4 + rand() % 20;
            }
            while (this_row > 10 && this_row < 17 && this_col > 10 && this_col < 17);
            _featureIndex[i] = row * this_row + this_col;
        }
        while (selected.find(_featureIndex[i]) != selected.end());
        selected.insert(_featureIndex[i]);
    }
#endif
    for (;  i < selectedFeatureNum;  i++)
    {
        do
        {
            _featureIndex[i] = rand() % featureNum;
        } while (selected.find(_featureIndex[i]) != selected.end());
        selected.insert(_featureIndex[i]);
    }
#endif
}
