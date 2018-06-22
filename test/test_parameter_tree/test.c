#include <stdio.h>

int CalcSum (int s, int i){

	return s + i;

}

int main(){

	int sum = 0 ;

	int i = 0;

	while(i < 10){

		sum = CalcSum(sum , i) ;

		i = i + 1;

	}

	return 0;

}
