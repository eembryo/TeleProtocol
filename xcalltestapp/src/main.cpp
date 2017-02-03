/*
 * @@@ LICENSE
 * Copyright (C) 2016-2017, LG Electronics, All Right Reserved.
 * No part of this source code may be communicated, distributed, reproduced
 * or transmitted in any form or by any means, electronic or mechanical or
 * otherwise, for any purpose, without the prior written permission of
 * LG Electronics.
 * LICENSE @@@
 *
 # Design/Author : manisha.manisha@lge.com, dhilipkumar.raman@lge.com
 # Date : Oct. 24, 2016
 *
 * XCALL TEST APP Main Implementation
 *
 */


#include "xcalltest.h"

using namespace std;

int main(int argc, char *argv[]) {
	XCallTest xcallTestObj;

	int choice = 0;

	cout << "***************************************************************"<<endl;
	cout << "********************* XCALL TEST APP ******************"<<endl;
	cout << "***************************************************************"<<endl;

	do {
		cout<<endl<<endl<<"----------------------------Main menu--------------------------"<<endl;
		cout <<"\t1. Xcall Event Test"<<endl<<"\t2. Xcall Scenario Test"<<endl<<"\t3. Change timer value"<<endl<<"\t4. Exit"<<endl;
		cout <<"Select your choice : ";
		//cin >>choice;

		if (EOF == scanf("%d", &choice)) {
			cin.clear();
			cin.ignore(INT_MAX,'\n');
			continue;
		}

		switch(choice) {
			case 0:
				cin >>choice;
				break;
			/* Coverity CID=4983016 */
			case 1: 
				xcallTestObj.TestEvent();
				break;
			case 2:
				xcallTestObj.TestScenario();
				break;
			case 3:
				xcallTestObj.ChangeTimer();
				break;
			case 4:
				exit(0);
				break;
			default:
				break;
		}

		cin.clear();
		cin.ignore(INT_MAX,'\n');
	} while(1);

	return 0;
}
