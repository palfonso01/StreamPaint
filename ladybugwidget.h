#ifndef LADYBUGWIDGET_H
#define LADYBUGWIDGET_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <QWidget>
#include <QDebug>
#include <QPixmap>
#include <QImage>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPointF>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
#include <qmath.h>
// === Selection Menu ===//
#include <QToolBar>
#include <QAction>
#include <QPainter>
#include <QPixmap>
#include <QSlider>
#include <QLabel>
// === image processing ===//
#include "DObject.hpp"
//=============================================================================
// PGR Includes
//=============================================================================
#include <ladybug.h>
#include <ladybugrenderer.h>
#include <ladybuggeom.h>
#include <ladybugstream.h>
#include <ladybuggps.h>
#include <ladybugvideo.h>

//=============================================================================
// MapControl
//=============================================================================
#include "qmapcontrol.h"
using namespace qmapcontrol;
//=======

#define _CHECK_ERROR \
    if( error != LADYBUG_OK ) \
{ \
    return error; \
} \

class LadybugWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LadybugWidget(QWidget *parent = 0);
    ~LadybugWidget();
signals:
    void CurrentPos(int a);
public slots:
    void updateStream();
    void paintImage(int pos);
    void SetFileName();
    void StarCamera();
    void Pause(bool p);
    void Stop();
    void SelectI(bool p);
    void SetFrame(int fr);
    void GetRealDimRectangle(QRectF r);
    // Update Map //
    void GPS_PositonMap(QByteArray line);
private:
    QLabel *labelImage;
    QVBoxLayout *lv;
    QHBoxLayout *layout;
    bool flagmove;
    QPointF initp, endp;
    QPoint ppos, oldpos;
    QTimer *t;
    QString name1;
    // Original Image from stream file
    QPixmap Pori;
    // Scale
    double scale;
    // Current position
    unsigned int Cpos;
    unsigned int iFrameFrom;
    unsigned int iFrameTo;
    char *pszConfigFile;//[ _MAX_PATH];
    char pszOutputFilePrefix[ _MAX_PATH ];
    int iOutputImageWidth;
    int iOutputImageHeight;
    LadybugOutputImage outputImageType;
    LadybugSaveFileFormat outputImageFormat;
    LadybugColorProcessingMethod colorProcessingMethod;
    int iBlendingWidth;
    float fFalloffCorrectionValue;
    bool bFalloffCorrectionFlagOn;
    bool bEnableAntiAliasing;
    bool bEnableSoftwareRendering;
    bool bEnableStabilization;
    LadybugStabilizationParams stabilizationParams;
    LadybugContext context;
    LadybugStreamContext readContext;
    LadybugStreamHeadInfo streamHeaderInfo;
    unsigned int iTextureWidth, iTextureHeight;
    unsigned char* arpTextureBuffers[ LADYBUG_NUM_CAMERAS];
    float fFOV;
    float fRotX;
    float fRotY;
    float fRotZ;
    int iBitRate; // in kbps
    bool processH264;
    LadybugProcessedImage processedImage;
    unsigned int totalFrames;
    LadybugError error;
    LadybugImage image;
    unsigned int iFrame;
    //====== Functions ========//
    bool isHighBitDepth( LadybugDataFormat format);
    LadybugError initializeLadybug( void );
    void cleanUp();
    //====== 3D coordinates =====//
    LadybugPoint3d libraryTranslation( LadybugContext context, const int kCamera, const int kRawX, const int kRawY );
    void makeTransformation( const double rotX, const double rotY, const double rotZ, const double transX, const double transY, const double transZ,
                                 double matrix[4][4] /*out*/ );
    void applyTransformation(const double matrix[4][4], double & x, double & y, double & z);
    void manualTranslation( LadybugContext context, const int kCamera, const int kRawX, const int kRawY, const double kSphereSize,
                            double & dLadybugX /*out*/, double & dLadybugY /*out*/, double & dLadybugZ /*out*/ );
    void reverseTranslation( LadybugContext context, const int kCamera, const double x, const double y, const double z,
                             double & unrectifiedX /*out*/, double & unrectifiedY /*out*/ );
    //======= Mouse Events ==========//
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    //======= size Events ===========//
    virtual void resizeEvent(QResizeEvent *e);
    // == ToolBar ==//
    QAction *se, *ao, *aplay, *apause, *astop;
    QToolBar *tbt, *tbb;
    QSlider *bprog;
    QLabel *lrm;
    QWidget *separator;
    QPixmap Pix;
    bool sel;
    // Map
    MapControl* mc;
    Layer* l;
    QLabel *gpsposition;
    LadybugNMEAGPGGA gpsData;
    QList<Point*> points;
    QPen* linepen;

    DObject Im;
};

#endif // LADYBUGWIDGET_H
