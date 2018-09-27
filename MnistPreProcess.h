/************************************************
 *Random Forest Program
 *Function:		read mnist dataset and do preprocess
 *Author:		handspeaker@163.com
 *CreateTime:	2014.7.10
 *Version:		V0.1
 *************************************************/
#ifndef MNISTPREPROCESS_H
#define MNISTPREPROCESS_H

#include <stdio.h>
#include <vector>

using namespace std;

void readData(
		vector< vector< float > > &dataset,
		vector< float > &labels,
		int &n_classes,
		const char *dataPath,
		const char *labelPath);

#endif//MNISTPREPROCESS_H
