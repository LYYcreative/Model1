#include<iostream>
#include<vector>
#include<stack>
#include<time.h>
using namespace std;

void qk(int L,int R,int a[]){
    stack<int> s;
    s.push(R),s.push(L);
	while(!s.empty()){
		int l=s.top();s.pop();
		int r=s.top();s.pop();
		if(l<r){

            int i = l; int j = r;
            int x= a[l];

            while(i<j&&a[j]>x) j--;
	        if(i<j) a[i++]=a[j];
	        while(i<j&&a[i]<x) i++;
	        if(i<j) a[j--]=a[i];
	        a[i]=x;

			int pos = i;
            s.push(pos-1),s.push(l);
            s.push(r),s.push(pos+1);
		}
	}
}

void vectorAdd(int vector[], int size)
{
	int num = 0;
	for(int i = 0; i < size; i++)
	{
		num = i;
		// to create a set of numbers that is neither increasing nor decrementing
		if(num %2 == 0)
		{
			
			vector[i] = num -1;
		}
		else
		{
			vector[i] = num+1;
		}
	}
}

int main()
{

    int size = 8;
	//initial vector v1
    int *v1;
	v1 = (int *) malloc(size * sizeof(int *));
	vectorAdd(v1,size);

    cout << "origenal vector: ";
    for(int i = 0; i<size; i++)
    {
        cout << v1[i] <<" ";
    }
    cout << "\n";

    qk(0,size-1,v1);

    cout << "final vector: ";
    for(int i = 0; i<size; i++)
    {
        cout << v1[i] <<" ";
    }

    return 0;


}