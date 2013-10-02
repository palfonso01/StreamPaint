#include"DObject.hpp"
int PrintFeat(DObject A);


int main(void)
{
	DObject I; 
	uchar *Ima;
	Ima=I.LoadImage();
	I.Procesado();
	PrintFeat(I);
	return 0;
}

int PrintFeat(DObject A)
{
	cout<<"W= "<<A.GetW()<<"\nH= "<<A.GetH()<<"\nChannels="<<A.GetCH()<<endl;
	return 0;
}


