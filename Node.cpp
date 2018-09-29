#include <assert.h>
#include"Node.h"

/***************************************************************/
//Node
Node::Node()
{
	_isLeaf=false;
	_featureIndex=-1;
	_threshold=0;
	_samples=NULL;
}

Node::~Node()
{
}

static int compare_pair( const void* a, const void* b )
{
   Pair* arg1 = (Pair*) a;
   Pair* arg2 = (Pair*) b;
   if( arg1->feature < arg2->feature ) return -1;
   if( arg1->feature == arg2->feature ) return 0;
   return 1;
}

// 在 sampleId 子集中按 data 中指定 feature 由小到大排序
void Node::sortIndex(int featureId)
{
	const vector < vector <float> > &data=_samples->_dataset;
	int n_all_samples = data.size();

#if 1	// 提前把全集排好序，以便后面抽取子集
	static int **ordered_samples = NULL;
	static Pair *pairs = NULL;
	if (ordered_samples == NULL)
	{
		int n_all_features = data[0].size();
		ordered_samples = new int *[n_all_features];
		for (int feature = 0;  feature < n_all_features;  feature++)
		{
			ordered_samples[feature] = new int[n_all_samples];
		}

		pairs = new Pair[n_all_samples];
		for (int feature = 0;  feature < n_all_features;  feature++)
		{
			for (int sample = 0;  sample < n_all_samples;  sample++)
			{
				pairs[sample].feature = data[sample][feature];
				pairs[sample].id = sample;
			}

			// time consuming process
			qsort(pairs, n_all_samples, sizeof(Pair), compare_pair);

			for (int order = 0;  order < n_all_samples;  order++)
			{
				ordered_samples[feature][order] = pairs[order].id;
			}
		}
		// delete [] pairs;
	}

	static int *sample_count = NULL;
	static char *sample_set = NULL;
	static int *sample_order = NULL;
	if (sample_count == NULL)
	{
		sample_count = new int[n_all_samples];
		sample_set = new char[n_all_samples];
		sample_order = new int[n_all_samples];
	}
#endif	// This part will be executed only once

	int *sampleId = _samples->getSampleIndex();
	int n_samples = _samples->getSelectedSampleNum();

	// for many samples, search existing result
	if (n_samples > n_all_samples / 100)
	{
		memset(sample_count, 0, sizeof(*sample_count) * n_all_samples);

		for (int sample = 0;  sample < n_samples;  sample++)
		{
			sample_count[sampleId[sample]]++;
		}

		int index = 0;
		int *ordered_sample = ordered_samples[featureId];
		for (int order = 0;  index < n_samples;  order++)
		{
			int sample = ordered_sample[order];
			for (int i = sample_count[sample];  i--;)
			{
				sampleId[index++] = sample;
			}
		}
	// for less samples, just sort them
	} else {
		for (int i = 0;  i < n_samples;  ++i)
		{
			pairs[i].id=sampleId[i];
			pairs[i].feature=data[sampleId[i]][featureId];
		}
		qsort(pairs, n_samples, sizeof(Pair), compare_pair);
		for (int i = 0;  i < n_samples; ++i)
		{
			sampleId[i] = pairs[i].id;
		}
	}
}

/***************************************************************/
//ClasNode
ClasNode::ClasNode()
	:Node()
{
	_class=-1;
	_prob=0;
}

ClasNode::~ClasNode()
{
	if(_probs!=NULL)
	{
		delete[] _probs;
		_probs=NULL;
	}
}

void ClasNode::calculateParams()
{
	int i=0;
	int*sampleId=_samples->getSampleIndex();
	int sampleNum=_samples->getSelectedSampleNum();
	int classNum=_samples->getClassNum();
	float gini=0;
	_probs=new float[classNum];
	for(i=0;i<classNum;++i)
	{_probs[i]=0;}
	for(i=0;i<sampleNum;++i)
	{_probs[static_cast<int>(_samples->_labels[sampleId[i]])]++;}
	for(i=0;i<classNum;++i)
	{
		float p=_probs[i]/sampleNum;
		gini+=(p*p);
	}
	_gini=1-gini;
}

void ClasNode::calculateInfoGain(Node**nodeArray,int id,float minInfoGain)
{
	const vector < vector <float> > &data=_samples->_dataset;
	const vector <float> &labels=_samples->_labels;

	//some used variables
	int	*sampleId=_samples->getSampleIndex();
	int	*featureId=_samples->getFeatureIndex();
	int	featureNum=_samples->getSelectedFeatureNum();
	int	sampleNum=_samples->getSelectedSampleNum();
	int classNum=_samples->getClassNum();

	//the final params need to store
	int   maxFeatureId=0;
	float maxInfoGain=0;
	float maxThreshold=0;
	float maxGiniLeft=0;
	float maxGiniRight=0;
	int   maxSamplesOnLeft=0;
	float *maxProbsLeft=new float[classNum];
	float *maxProbsRight=new float[classNum];
	memset(maxProbsLeft, 0, sizeof(*maxProbsLeft) * classNum);
	memset(maxProbsRight, 0, sizeof(*maxProbsRight) * classNum);

	//the params need to store in first loop
	int   fMaxFeatureId=0;
	int   fMaxSamplesOnLeft=0;
	float fMaxinfoGain=0;
	float fMaxThreshold=0;
	float fMaxGiniLeft=0;
	float fMaxGiniRight=0;
	float *fMaxProbsLeft=new float[classNum];
	float *fMaxProbsRight=new float[classNum];

	//the temp params in inner loop
	float giniLeft=0, giniRight=0, infoGain=0;
	float *probsLeft=new float[classNum];
	float *probsRight=new float[classNum];

	for (int f_index = 0;  f_index < featureNum;  f_index++)  //for every dimension
	{
		//sort the samples according to the current feature
		//this means only exchange the position of the index
		//in sampleIndex.the trainset and labels never change
		fMaxinfoGain=0;
		fMaxFeatureId = featureId[f_index];
		fMaxGiniLeft=0;
		fMaxGiniRight=0;
		fMaxThreshold=0;
		fMaxSamplesOnLeft=0;
		for (int j = 0;  j < classNum;  j++)
		{
			fMaxProbsLeft[j]=0;
			fMaxProbsRight[j]=0;
		}

		// FIXME: time consuming process
        // sort samples by current feature
		sortIndex(fMaxFeatureId);

		//initialize the probsLeft&probsRight
		for (int j = 0;  j < classNum;  j++)
		{
			probsLeft[j]=0;
			probsRight[j]=0;
		}
		memcpy(probsRight,_probs,sizeof(float)*classNum);
		for (int j = 0;  j < sampleNum-1;  j++)
		{
			giniLeft=0;
			giniRight=0;
			infoGain=0;
			probsLeft[static_cast<int>(labels[sampleId[j]])]++;
			probsRight[static_cast<int>(labels[sampleId[j]])]--;
            //do not do calculation if the nearby samples' feature are too similar(<0.000001)
			if((data[sampleId[j+1]][fMaxFeatureId]-data[sampleId[j]][fMaxFeatureId])<0.000001)
				continue;

			float n_left = j + 1;
			float n_right = sampleNum - n_left;

			for (int k = 0;  k < classNum;  k++)
			{
				float p = probsLeft[k] / n_left;
				giniLeft += p*p;
			}
			giniLeft = 1 - giniLeft;

			for (int k = 0;  k < classNum;  k++)
			{
				float p = probsRight[k] / n_right;
				giniRight += p*p;
			}
			giniRight = 1-giniRight;

			infoGain = _gini - (n_left * giniLeft + n_right * giniRight) / sampleNum;
			if (infoGain > fMaxinfoGain)
			{
				fMaxinfoGain		= infoGain;
				fMaxGiniLeft		= giniLeft;
				fMaxGiniRight		= giniRight;
				fMaxThreshold		= (data[sampleId[j]][fMaxFeatureId] + data[sampleId[j+1]][fMaxFeatureId])/2;
				fMaxSamplesOnLeft	= j;
				memcpy(fMaxProbsLeft,probsLeft,sizeof(float)*classNum);
				memcpy(fMaxProbsRight,probsRight,sizeof(float)*classNum);
			}
		}
		if(fMaxinfoGain>maxInfoGain)
		{
			maxInfoGain=fMaxinfoGain;
			maxGiniLeft=fMaxGiniLeft;
			maxGiniRight=fMaxGiniRight;
			maxFeatureId=fMaxFeatureId;
			maxThreshold=fMaxThreshold;
			maxSamplesOnLeft=fMaxSamplesOnLeft;
			memcpy(maxProbsLeft,fMaxProbsLeft,sizeof(float)*classNum);
			memcpy(maxProbsRight,fMaxProbsRight,sizeof(float)*classNum);
		}
	}
	sortIndex(maxFeatureId);
	if (maxInfoGain < minInfoGain)
	{
		createLeaf();
	} else {
		_featureIndex=maxFeatureId;
		_threshold=maxThreshold;
		nodeArray[id*2+1]=new ClasNode();
		nodeArray[id*2+2]=new ClasNode();
		((ClasNode*)nodeArray[id*2+1])->_gini=maxGiniLeft;
		((ClasNode*)nodeArray[id*2+1])->_probs=maxProbsLeft;
		((ClasNode*)nodeArray[id*2+2])->_gini=maxGiniRight;
		((ClasNode*)nodeArray[id*2+2])->_probs=maxProbsRight;
		//assign samples to left and right
		Sample *leftSamples = new Sample(_samples, 0, maxSamplesOnLeft);
		Sample *rightSamples = new Sample(_samples, maxSamplesOnLeft+1, sampleNum-1);
		nodeArray[id*2+1]->_samples=leftSamples;
		nodeArray[id*2+2]->_samples=rightSamples;
	}
	delete[] _probs;
	_probs=NULL;
	delete[] fMaxProbsLeft;
	delete[] fMaxProbsRight;
	delete[] probsLeft;
	delete[] probsRight;
}

void ClasNode::createLeaf()
{
	_class=0;
	_prob=_probs[0];
	for(int i=1;i<_samples->getClassNum();++i)
	{
		if(_probs[i]>_prob)
		{
			_class=i;
			_prob=_probs[i];
		}
	}
	_prob/=_samples->getSelectedSampleNum();
	_isLeaf=true;
}

int ClasNode::predict(const vector <float> &data,int id)
{
	if(data[_featureIndex]<_threshold)
	{return id*2+1;}
	else
	{return id*2+2;}
}

void ClasNode::getResult(Result&r)
{
	r.label=_class;
	r.prob=_prob;
}
/***************************************************************/
//RegrNode
RegrNode::RegrNode()
	:Node()
{
	_value=0;
}

RegrNode::~RegrNode()
{}

void RegrNode::calculateParams()
{
	int i=0;
	int*labelId=_samples->getSampleIndex();
	int sampleNum=_samples->getSelectedSampleNum();
	double mean=0,variance=0;
	for(i=0;i<sampleNum;++i)
	{mean+=_samples->_labels[labelId[i]];}
	mean/=sampleNum;
	for(i=0;i<sampleNum;++i)
	{
		float diff=_samples->_labels[labelId[i]]-mean;
		variance+=diff*diff;
	}
	_mean=mean;
	_variance=variance/sampleNum;
}

void RegrNode::calculateInfoGain(Node**nodeArray,int id,float minInfoGain)
{
	//some used variables
	int i=0,j=0,k=0;
	int*sampleId=_samples->getSampleIndex();
	int*featureId=_samples->getFeatureIndex();
	const vector < vector <float> > &data=_samples->_dataset;
	const vector <float> &labels=_samples->_labels;
	int featureNum=_samples->getSelectedFeatureNum();
	int sampleNum=_samples->getSelectedSampleNum();
	//the final params need to store
	float maxInfoGain=0;
	int maxFeatureId=0;
	float maxThreshold=0;
	float maxVarLeft=0;
	float maxVarRight=0;
	int maxSamplesOnLeft=0;
	float maxMeanLeft=0;
	float maxMeanRight=0;
	//the params need to store in first loop
	float fMaxinfoGain=0;
	int fMaxFeatureId=0;
	float fMaxThreshold=0;
	float fMaxVarLeft=0;
	float fMaxVarRight=0;
	int fMaxSamplesOnLeft=0;
	float fMaxMeanLeft=0;
	float fMaxMeanRight=0;
	//the temp params in inner loop
	float infoGain=0;
	float varLeft=0,varRight=0;
	float meanLeft=0,meanRight=0;
	for(i=0;i<featureNum;++i)  //for every dimension
	{
		//sort the samples according to the current feature
		//this means only exchange the position of the index
		//in sampleIndex.the trainset and labels never change
		fMaxinfoGain=0;
		fMaxFeatureId=featureId[i];
		fMaxVarLeft=0;
		fMaxVarRight=0;
		fMaxMeanLeft=0;
		fMaxMeanRight=0;
		fMaxThreshold=0;
		fMaxSamplesOnLeft=0;
		//sort the samples by the current selected feature
		sortIndex(featureId[i]);
		//initialize the probsLeft&probsRight
		meanLeft=0;
		meanRight=_mean;
		for(j=0;j<sampleNum-1;++j)
		{
			varLeft=0;
			varRight=0;
			infoGain=0;
			//recalculate the current mean for left and right
			float n_left = j + 1;
			float n_right = sampleNum - n_left;
			meanLeft=(meanLeft*j+labels[sampleId[j]]) / n_left;
			meanRight=(meanRight*(sampleNum-j)-labels[sampleId[j]]) / n_right;
			//the difference is too tiny,ignore
			if((data[sampleId[j+1]][featureId[i]]-data[sampleId[j]][featureId[i]])<0.000001)
				continue;

			for(k=0;k<=j;++k)
			{
				float diff=labels[sampleId[k]]-meanLeft;
				varLeft+=diff*diff;
			}
			varLeft /= n_left;

			for(k=j+1;k<sampleNum;++k)
			{
				float diff=labels[sampleId[k]]-meanRight;
				varRight+=diff*diff;
			}
			varRight /= n_right;

			//calculate the infoGain to decide to update
			infoGain = _variance - (n_left * varLeft + n_right * varRight ) / sampleNum;
			if(infoGain>fMaxinfoGain)
			{
				fMaxinfoGain=infoGain;
				fMaxVarLeft=varLeft;
				fMaxVarRight=varRight;
				fMaxThreshold=(data[sampleId[j]][featureId[i]]+data[sampleId[j+1]][featureId[i]])/2;
				fMaxSamplesOnLeft=j;
				fMaxMeanLeft=meanLeft;
				fMaxMeanRight=meanRight;
			}
		}
		if(fMaxinfoGain>maxInfoGain)
		{
			maxInfoGain=fMaxinfoGain;
			maxVarLeft=fMaxVarLeft;
			maxVarRight=fMaxVarRight;
			maxFeatureId=fMaxFeatureId;
			maxThreshold=fMaxThreshold;
			maxSamplesOnLeft=fMaxSamplesOnLeft;
			maxMeanLeft=fMaxMeanLeft;
			maxMeanRight=fMaxMeanRight;
		}
	}

	if(maxInfoGain<minInfoGain)
	{createLeaf();}
	else
	{
		//sort the samples so that all the samples
		//less than the threshold will be on the left
		//and others will be on the right
		sortIndex(maxFeatureId);
		_featureIndex=maxFeatureId;
		_threshold=maxThreshold;
		nodeArray[id*2+1]=new RegrNode();
		nodeArray[id*2+2]=new RegrNode();
		((RegrNode*)nodeArray[id*2+1])->_variance=maxVarLeft;
		((RegrNode*)nodeArray[id*2+1])->_mean=maxMeanLeft;
		((RegrNode*)nodeArray[id*2+2])->_variance=maxVarRight;
		((RegrNode*)nodeArray[id*2+2])->_mean=maxMeanRight;
		//assign samples to left and right
		Sample*leftSamples=new Sample(_samples,0,maxSamplesOnLeft);
		Sample*rightSamples=new Sample(_samples,maxSamplesOnLeft+1,sampleNum-1);
		nodeArray[id*2+1]->_samples=leftSamples;
		nodeArray[id*2+2]->_samples=rightSamples;
	}
}

void RegrNode::createLeaf()
{
	_value=_mean;
	_isLeaf=true;
}

int RegrNode::predict(const vector <float> &data,int id)
{
	if(data[_featureIndex]<_threshold)
	{return id*2+1;}
	else
	{return id*2+2;}
}

void RegrNode::getResult(Result&r)
{
	r.label=0;
	r.prob=_value;
}
