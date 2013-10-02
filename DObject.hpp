#ifndef DOBJECT_HPP
#define DOBJECT_HPP
#include<iostream>
#include <cmath>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#define tam 5
#define FILTER_2PI double (2*3.1415926535897932384626433832795)
#define ancho 1.2
#define alto 1.3
#define MaxABC(A,B,C) ((A)>(B))? (((A)>(C))?(A):(C)):(((B)>(C))?(B):(C));
#define dW 95
#define dH 65

using namespace std;

class DObject
{
	private:
		IplImage* img;
        //Test
        IplImage *tpl;
		int W, H, Channels, step, x, y, z, *X, *Y;
		uchar *Data;
		void FindColorRGB(const IplImage* src, IplImage* dst, int B, int G, int R, int BSensibility, int GSensibility, int RSensibility);
		void FindColorHSV(const IplImage* src, IplImage* dst, int H1Min, int S1Min, int V1Min, int H1Max, int S1Max, int V1Max);
		void FindColorYCrCb(const IplImage* src, IplImage* dst, int Ymin, int Crmin, int Cbmin, int Ymax, int Crmax, int Cbmax);
		void FindColorLab(const IplImage* src, IplImage* dst, int LMin, int aMin, int bMin, int LMax, int aMax, int bMax);
		char *itoa(int val, int base);
		int WG(IplImage* src, IplImage* dst);
	public:
		DObject();
		~DObject();
		uchar* LoadImage();
		int col;
		int form;
        void SetImage(uchar *I, int w, int h, int l);
        void SetColor(int cC);
		int GetW(){return W;}
		int GetH(){return H;}
		int GetCH(){return Channels;}
		int GetX(){return x;}
		int GetY(){return y;}
		int Procesado();
        void ReleaseImage();
};

#endif
