#include <vector>
#include <numeric>
#include"RandomForest.h"
#include"MnistPreProcess.h"

using namespace std;

int main(int argc, const char * argv[])
{
    // 1. prepare data
	vector< vector<float> > trainset;
	vector< vector<float> > testset;
	vector< float > trainlabels;
	vector< float > testlabels;
	map< float, int > weight_map;
	int	n_classes = 0;

    readData(trainset, trainlabels, n_classes, argv[1], argv[2]);
    readData(testset, testlabels, n_classes, argv[3], argv[4]);
    init_weight_map(trainset, weight_map);

    //2. create RandomForest class and set some parameters
	RandomForest randomForest(100,10,10,0);

	//3. start to train RandomForest
	//	 randomForest.train(trainset,trainlabels,TRAIN_NUM,FEATURE,10,true,56);//regression
    randomForest.train(trainset, trainlabels, weight_map, n_classes, false);//classification

    //restore model from file and save model to file
//	randomForest.saveModel("E:\\RandomForest2.Model");
//	randomForest.readModel("E:\\RandomForest.Model");
//	RandomForest randomForest("E:\\RandomForest2.Model");

    //predict single sample
//  float resopnse;
//	randomForest.predict(testset[0],resopnse);

    //predict a list of samples
    vector <float> resopnses;
    resopnses.resize(testset.size());
	randomForest.predict(testset, resopnses);
	float errorRate=0;
	for(int i=0;i<testset.size();++i)
	{
        if(resopnses[i]!=testlabels[i])
        {
            errorRate+=1.0f;
        }
        //for regression
//		float diff=abs(resopnses[i]-testlabels[i]);
//		errorRate+=diff;
	}
	errorRate /= testset.size();
	printf("the total error rate is:%f\n",errorRate);

	return 0;
};
