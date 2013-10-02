#include "DObject.hpp"

DObject::DObject()
{
    img=0; W=0, H=0, Channels=0; step=0;
}


DObject::~DObject()
{

}

void DObject::ReleaseImage()
{
    cvReleaseImage(&img);
    //Test
    cvReleaseImage( &tpl );
}

void DObject::SetImage(uchar *I, int w, int h, int l)
{
    img = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,l);
    H    	= img->height;
    W    	= img->width;
    step      = img->widthStep;
    Channels  = img->nChannels;
    img->imageData = (char *)I;
    //test
    /* load template image */
    tpl = cvLoadImage( "temp.png", CV_LOAD_IMAGE_COLOR );
}

void DObject::SetColor(int cC)
{
    col = cC;
}

uchar* DObject::LoadImage()
{
    char s[80];
    cout<<"Name of image: ";
    cin>>s;
    cout<<"Color: ";
    cin>>col;
    cout<<"Forma: ";
    cin>>form;
    // load an image
    img=cvLoadImage(s);
    if(!img){
    cout<<"Could not load image file: "<<s<<endl;
    return 0;
    }
    // get the image data
    H    	= img->height;
    W    	= img->width;
    step      = img->widthStep;
    Channels  = img->nChannels;
    Data      = (uchar *)img->imageData;
    return Data;
}

int DObject::Procesado()
{
    /*int i,j;
    uchar *data2;
    bool draw = false;
    IplImage *img2, *img3, *img4, *img0;
    img0=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    img2=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    img3=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    img4=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    data2= (uchar*) img2->imageData;

    //detectando color
    cvSmooth(img,img0,CV_GAUSSIAN,5,5);
    cvCvtColor(img0, img3 , CV_BGR2HSV);

    //Signals
    switch(col)
    {
    case 1:
        FindColorHSV(img3, img4, 200, 60, 35, 240, 100, 100); //Azul_Signals
        break;
    case 2:
        FindColorHSV(img3, img4, 140, 60, 0, 190, 100, 100); //Verde_Signals
        break;
    case 3:
        FindColorHSV(img3, img4, 330, 50, 10, 6, 100, 100); //Rojo_Signals
        break;
    case 4:
        FindColorHSV(img3, img4, 20, 80, 30, 80, 90, 80); //Amarillo_Signals
        break;
    default:
        FindColorHSV(img3, img4, 0, 15, 60, 0, 30, 90); //Blanco_Signals
        break;
    }

    int sx = 0, sy = 0, np = 0;
    for(i=0;i<H;i++)
    {
        for(j=0;j<W;j++)
        {
            CvScalar s = cvGet2D(img4, i, j);
            if(s.val[0]==255 && s.val[1]==255 && s.val[2]==255)
            {
                sx+=i; sy+=j;
                np++;
                draw = true;
            }
        }
    }
    if(draw)
    {
        if(col == 3)
            cvCircle( img, cvPoint(sy/np, sx/np), 50, CV_RGB(0,255,0), 5);
        else
            cvRectangle(img, cvPoint(sy/np - dW , sx/np-dH ), cvPoint(sy/np + dW, sx/np + dH), CV_RGB(0,255,0), 5);
    }

    cvReleaseImage(&img4);
    cvReleaseImage(&img3);
    cvReleaseImage(&img2);
    cvReleaseImage(&img0);
    return 0;*/

    IplImage *res, *imgg, *tplg;
    CvPoint minloc, maxloc;
    double minval, maxval;
    int img_width, img_height;
    int tpl_width, tpl_height;
    int res_width, res_height;
    /* get image's properties */
    img_width = img->width;
    img_height = img->height;
    tpl_width = tpl->width;
    tpl_height = tpl->height;
    res_width = img_width - tpl_width + 1;
    res_height = img_height - tpl_height + 1;

    /* create new image for template matching computation */
    res = cvCreateImage( cvSize( res_width, res_height ), IPL_DEPTH_32F, 1 );
    imgg = cvCreateImage( cvSize( img_width, img_height ), IPL_DEPTH_8U, 1 );
    tplg = cvCreateImage( cvSize( tpl_width, tpl_height ), IPL_DEPTH_8U, 1 );

    float threshold = 20;
    cvCvtColor(img, imgg, CV_BGR2GRAY);
    cvCvtColor(tpl, tplg, CV_BGR2GRAY);
    cvThreshold(imgg,imgg, threshold, 255, CV_THRESH_BINARY);
    cvThreshold(tplg,tplg, threshold, 255, CV_THRESH_BINARY);

    cvSaveImage("tempg.jpg",tplg);
    cvSaveImage("env.jpg",imgg);

    /* choose template matching method to be used */
    cvMatchTemplate( imgg, tplg, res, CV_TM_SQDIFF);

    cvMinMaxLoc( res, &minval, &maxval, &minloc, &maxloc, 0 );

    if(minval>0 &&  maxval>0)
    {
     //draw red rectangle
    cvRectangle( img,
    cvPoint( minloc.x, minloc.y ),
    cvPoint( minloc.x + tpl_width, minloc.y + tpl_height ),
    cvScalar( 0, 0, 255, 0 ), 4, 0, 0 );
    }

    /* free memory */
    cvReleaseImage( &res );
    cvReleaseImage( &imgg );
    cvReleaseImage( &tplg );
    return 0;
}


void DObject::FindColorRGB(const IplImage* src, IplImage* dst, int B, int G, int R,
        int BSensibility, int GSensibility, int RSensibility) {
int x, y;
    for (x=0; x<src->height; x++)
        for (y=0; y<src->width; y++) {
            CvScalar s = cvGet2D(src, x, y);
            if ((s.val[0]>(B-BSensibility) && s.val[0]<(B+BSensibility)) &&
                (s.val[1]>(G-GSensibility) && s.val[1]<(G+GSensibility)) &&
                (s.val[2]>(R-RSensibility) && s.val[2]<(R+RSensibility))) {
                s.val[0]=255;
                s.val[1]=255;
                s.val[2]=255;
                cvSet2D(dst, x, y, s);
            } else {
                s.val[0]=0;
                s.val[1]=0;
                s.val[2]=0;
                cvSet2D(dst, x, y, s);
            }
        }
}


void DObject::FindColorHSV(const IplImage* src, IplImage* dst, int H1Min, int S1Min, int V1Min, int H1Max, int S1Max, int V1Max) {
    int x, y;
    bool h;
    for (x=0; x<src->height; x++)
        for (y=0; y<src->width; y++) {
            CvScalar s = cvGet2D(src, x, y);
            if (H1Max > H1Min)
                h = (s.val[0]*2 <= H1Max) && (s.val[0]*2 >= H1Min);
            else
                h = (s.val[0]*2 <= H1Max) || (s.val[0]*2 >= H1Min);
            if (h &&
                (s.val[1]<=((S1Max*255.0)/100.0) && s.val[1]>=((S1Min*255.0)/100.0)) &&
                (s.val[2]<=((V1Max*255.0)/100.0) && s.val[2]>=((V1Min*255.0)/100.0))) {
                s.val[0]=255;
                s.val[1]=255;
                s.val[2]=255;
                cvSet2D(dst, x, y, s);
            } else {
                s.val[0]=0;
                s.val[1]=0;
                s.val[2]=0;
                cvSet2D(dst, x, y, s);
            }
        }
}

void DObject::FindColorLab(const IplImage* src, IplImage* dst, int LMin, int aMin, int bMin, int LMax, int aMax, int bMax) {
    int x, y;
    for (x=0; x<src->height; x++)
        for (y=0; y<src->width; y++) {
            CvScalar s = cvGet2D(src, x, y);
            if ((s.val[0]>=(LMin) && s.val[0]<=(LMax))&& (s.val[1]>=(aMin) && s.val[1]<=(aMax))&& (s.val[2]>=(bMin) && s.val[2]<=(bMax))) {
                s.val[0]=255;
                s.val[1]=255;
                s.val[2]=255;
                cvSet2D(dst, x, y, s);
            } else {
                s.val[0]=0;
                s.val[1]=0;
                s.val[2]=0;
                cvSet2D(dst, x, y, s);
            }
        }
}
void DObject::FindColorYCrCb(const IplImage* src, IplImage* dst, int Ymin, int Crmin, int Cbmin,
        int Ymax, int Crmax, int Cbmax) {
    int x, y;
    for (x=0; x<src->height; x++)
        for (y=0; y<src->width; y++) {
            CvScalar s = cvGet2D(src, x, y);
            if ((s.val[0]>(Ymin) && s.val[0]<(Ymax)) &&/////////////////////////////
                (s.val[1]>(Crmin) && s.val[1]<(Crmax)) &&
                (s.val[2]>(Cbmin) && s.val[2]<(Cbmax))) {
                s.val[0]=255;
                s.val[1]=255;
                s.val[2]=255;
                cvSet2D(dst, x, y, s);
            } else {
                s.val[0]=0;
                s.val[1]=0;
                s.val[2]=0;
                cvSet2D(dst, x, y, s);
            }
        }
}

char* DObject::itoa(int val, int base){

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)

        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];

}

int DObject::WG(IplImage* src, IplImage* dst)
{
    double Rmean=0, Gmean=0, Bmean=0, GrayM=0, FactorR=0, FactorG=0, FactorB=0;
    unsigned char *Rout=NULL,*Gout=NULL,*Bout=NULL, *R=NULL, *G=NULL,*B=NULL ;
    float maxL, maxRed, maxGre, maxBlue, fac;
    int l, h=0;
    IplImage* dst1, *dst2;
    dst1=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    dst2=cvCreateImage(cvSize(W,H),IPL_DEPTH_8U,3);
    uchar *Data1      = (uchar *)src->imageData;
    uchar *data2      = (uchar *)dst->imageData;
    uchar *data3      = (uchar *)dst1->imageData;
    uchar *data4      = (uchar *)dst2->imageData;
    l=W*H*3;
    R = new unsigned char[l/3];
    G = new unsigned char[l/3];
    B = new unsigned char[l/3];
    Rout = new unsigned char[l/3];
    Gout = new unsigned char[l/3];
    Bout = new unsigned char[l/3];

    for(register int i=0;i<l;i=i+3)
    {
        R[h]=Data1[i+2];
        G[h]=Data1[i+1];
        B[h]=Data1[i];
        h++;
    }

    for(register int i=0;i<l/3;i++)
    {
        Rmean=Rmean+(int)R[i];
        Gmean=Gmean+(int)G[i];
        Bmean=Bmean+(int)B[i];
    }
    Rmean=Rmean/(l/3);
    cout<<"\n Rmean="<<Rmean;
    Gmean=Gmean/(l/3);
    cout<<"\n Gmean="<<Gmean;
    Bmean=Bmean/(l/3);
    cout<<"\n Bmean="<<Bmean;
///Promedio de Gris
    GrayM=(Rmean+Gmean+Bmean)/3.0;
    cout<<"\n GrayM="<<GrayM;
    FactorR=(GrayM/Rmean);
    if(FactorR>1.0) FactorR=1.0;
    FactorG=(GrayM/Gmean);
    if(FactorG>1.0) FactorG=1.0;
    FactorB=(GrayM/Bmean);
    if(FactorB>1.0) FactorB=1.0;
    cout<<"\n Fred="<<FactorR;
    cout<<"\n Fgreen="<<FactorG;
    cout<<"\n Fblue="<<FactorB;
if(Rmean==0)
    {
        Rout=R;
    }
    else
    {
        for (register int j=0;j<l/3;j++)
        {
            Rout[j]=(unsigned char)((int)R[j]*FactorR);
        }
    }

if(Gmean==0)
    {
        Gout=G;
    }
    else
    {
        for (register int j=0;j<l/3;j++)
        {
            Gout[j]=(unsigned char)((int)G[j]*FactorG);
        }
    }
if(Bmean==0)
    {
        Bout=B;
    }
    else
    {
        for (register int j=0;j<l/3;j++)
        {
            Bout[j]=(unsigned char)((int)B[j]*FactorB);
        }
    }

    h=0;
for(register int i=0;i<l;i=i+3)
    {
        data3[i+2]=Rout[h];
        data3[i+1]=Gout[h];
        data3[i]=Bout[h];
        h++;
    }


cvCvtColor(dst1, dst2 , CV_BGR2Lab);

for(register int k=0;k<W*H;k++)
    {
        maxL=data4[k];
        maxRed=Rout[k]/maxL;
        maxGre=Gout[k]/maxL;
        maxBlue=Bout[k]/maxL;

        fac=MaxABC(maxRed, maxGre, maxBlue);

        Rout[k] = Rout[k]*(maxRed/fac);
        Gout[k] = Gout[k]*(maxGre/fac);
        Bout[k] = Bout[k]*(maxBlue/fac);
        //Conjuntando colores
        data2[k*3]=(unsigned char)Bout[k];;
        data2[k*3+1]=(unsigned char)Gout[k];
        data2[k*3+2]=(unsigned char)Rout[k];
    }
    cvReleaseImage(&dst1);
    cvReleaseImage(&dst2);
    delete []Rout;
    delete []Gout;
    delete []Bout;
    delete []R;
    delete []G;
    delete []B;
    return 0;
}

