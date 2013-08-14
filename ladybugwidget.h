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
#include <QMouseEvent>
#include <QPointF>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>
//=============================================================================
// PGR Includes
//=============================================================================
#include <ladybug.h>
#include <ladybugrenderer.h>
#include <ladybuggeom.h>
#include <ladybugstream.h>
#include <ladybuggps.h>
#include <ladybugvideo.h>

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
public slots:
    void paintImage();
    void SetFileName();
    void StarCamera();
private:
    QLabel *labelImage;
    QHBoxLayout *layout;
    bool flagmove;
    QPointF oldp, newp;
    QTimer *t;
    QString name1;
    unsigned int iFrameFrom;
    unsigned int iFrameTo;
    char pszConfigFile[ _MAX_PATH];
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
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
};

#endif // LADYBUGWIDGET_H
