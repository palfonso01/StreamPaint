#include "ladybugwidget.h"

LadybugWidget::LadybugWidget(QWidget *parent) :
    QWidget(parent), iFrameFrom(0), iFrameTo(0),
    iOutputImageWidth(2048), iOutputImageHeight(1024)
{
    //=== Label Image ====//
    this->setMinimumSize(800, 470);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    labelImage = new QLabel(this);
    layout = new QHBoxLayout(this);
    layout->addWidget(labelImage);
    this->setLayout(layout->layout());

    //=== Variable initialization ===//
    outputImageType = LADYBUG_SPHERICAL;
    outputImageFormat = LADYBUG_FILEFORMAT_JPG;
    colorProcessingMethod = LADYBUG_HQLINEAR_GPU;
    iBlendingWidth = 100;
    fFalloffCorrectionValue = 1.0f;
    bFalloffCorrectionFlagOn = false;
    bEnableAntiAliasing = false;
    bEnableSoftwareRendering = false;
    bEnableStabilization = false;
    stabilizationParams = {6, 100, 0.95};
    fFOV = 60.0f;
    fRotX = 0.0f;
    fRotY = 0.0f;
    fRotZ = 30.0f;
    iBitRate = 4000; // in kbps
    processH264 = false;
    iFrame = 0;
    flagmove = false;
    t = new QTimer(this);
    t->start(10);
}

LadybugWidget::~LadybugWidget()
{
    ladybugDestroyStreamContext( &readContext);
    ladybugDestroyContext( &context);
    for( int i = 0; i < LADYBUG_NUM_CAMERAS; i++)
    {
        if ( arpTextureBuffers[ i ] != NULL )
        {
            delete arpTextureBuffers[ i ];
            arpTextureBuffers[ i ] = NULL;
        }
    }
}

bool LadybugWidget::isHighBitDepth( LadybugDataFormat format)
{
    return (format == LADYBUG_DATAFORMAT_RAW12 ||
        format == LADYBUG_DATAFORMAT_HALF_HEIGHT_RAW12 ||
        format == LADYBUG_DATAFORMAT_COLOR_SEP_JPEG12 ||
        format == LADYBUG_DATAFORMAT_COLOR_SEP_HALF_HEIGHT_JPEG12 ||
        format == LADYBUG_DATAFORMAT_RAW16 ||
        format == LADYBUG_DATAFORMAT_HALF_HEIGHT_RAW16);
}


LadybugError LadybugWidget::initializeLadybug( void )
{
    char pszTempPath[_MAX_PATH] = {0};
    //
    // Create contexts and prepare stream for reading
    //
    error = ladybugCreateContext( &context);
    _CHECK_ERROR;

    error = ladybugCreateStreamContext( &readContext);
    _CHECK_ERROR;

    error = ladybugInitializeStreamForReading( readContext, name1.toStdString().data(), true );
    _CHECK_ERROR;

    // Is configuration file specified by the command line option?
    if ( strlen( pszConfigFile) == 0){
#if _MSC_VER >= 1400 // Is this newer than Visual C++ 8.0?
        errno_t err = tmpnam_s( pszTempPath, sizeof ( pszTempPath));
        if ( err)
        {
            printf( "Error creating temporary file name.\n");
            return LADYBUG_FAILED;
        }
#else
        strcpy( pszTempPath, "temp.cal");
#endif

        error = ladybugGetStreamConfigFile( readContext , pszTempPath );
        _CHECK_ERROR;

        strncpy(pszConfigFile, pszTempPath, strlen( pszTempPath) );
    }

    //
    // Load configuration file
    //
    error = ladybugLoadConfig( context, pszConfigFile );
    _CHECK_ERROR;

    if ( strlen( pszTempPath) != 0 )
    {
        // Remove the temporary configuration file
        remove ( pszTempPath );
    }

    //
    // Get and display the the stream information
    //
    error = ladybugGetStreamHeader( readContext, &streamHeaderInfo );
    _CHECK_ERROR;

    const float frameRateToUse = streamHeaderInfo.ulLadybugStreamVersion < 7 ? (float)streamHeaderInfo.ulFrameRate : streamHeaderInfo.frameRate;

    printf( "--- Stream Information ---\n");
    printf( "Stream version : %ld\n", streamHeaderInfo.ulLadybugStreamVersion);
    printf( "Base S/N: %ld\n", streamHeaderInfo.serialBase);
    printf( "Head S/N: %ld\n", streamHeaderInfo.serialHead);
    printf( "Frame rate : %3.2f\n", frameRateToUse);
    printf( "--------------------------\n");

    //
    // Set color processing method.
    //
    printf("Setting color processing method...\n" );
    error = ladybugSetColorProcessingMethod( context, colorProcessingMethod);
    _CHECK_ERROR;

    //
    // Set falloff correction value and flag
    //
    error = ladybugSetFalloffCorrectionAttenuation( context, fFalloffCorrectionValue );
    _CHECK_ERROR;
    error = ladybugSetFalloffCorrectionFlag( context, bFalloffCorrectionFlagOn );
    _CHECK_ERROR;

    //
    // read one image from the stream
    //
    error = ladybugReadImageFromStream( readContext, &image);
    _CHECK_ERROR;

    //
    // Allocate the texture buffers that hold the color-processed images for all cameras
    //
    if ( colorProcessingMethod == LADYBUG_DOWNSAMPLE4 || colorProcessingMethod == LADYBUG_MONO)
    {
        iTextureWidth = image.uiCols / 2;
        iTextureHeight = image.uiRows / 2;
    }
    else
    {
        iTextureWidth = image.uiCols;
        iTextureHeight = image.uiRows;
    }

    const unsigned int outputBytesPerPixel = isHighBitDepth(streamHeaderInfo.dataFormat) ? 2 : 1;
    for( int i = 0; i < LADYBUG_NUM_CAMERAS; i++)
    {
        arpTextureBuffers[ i ] = new unsigned char[ iTextureWidth * iTextureHeight * 4 * outputBytesPerPixel];
    }

    //
    // Set blending width
    //
    error = ladybugSetBlendingParams( context, iBlendingWidth );
    _CHECK_ERROR;

    //
    // Initialize alpha mask size - this can take a long time if the
    // masks are not present in the current directory.
    //
    printf( "Initializing alpha masks (this may take some time)...\n" );
    error = ladybugInitializeAlphaMasks( context, iTextureWidth, iTextureHeight );
    _CHECK_ERROR;

    //
    // Make the rendering engine use the alpha mask
    //
    error = ladybugSetAlphaMasking( context, true );
    _CHECK_ERROR;

    //
    // Enable image sampling anti-aliasing
    //
    if ( bEnableAntiAliasing )
    {
        error = ladybugSetAntiAliasing( context, true );
        _CHECK_ERROR;
    }

    //
    // Use ladybugEnableSoftwareRendering() to enable
    // Ladybug library to render the off-screen image using a bitmap buffer
    // in system memory. The image rendering process will not be hardware
    // accelerated.
    //
    if ( bEnableSoftwareRendering )
    {
        error = ladybugEnableSoftwareRendering( context, true );
        _CHECK_ERROR;
    }

    if ( bEnableStabilization )
    {
        error = ladybugEnableImageStabilization(
            context, bEnableStabilization, &stabilizationParams);
        _CHECK_ERROR;
    }

    //
    // Configure output images in Ladybug liabrary
    //
    printf( "Configure output images in Ladybug library...\n" );
    error = ladybugConfigureOutputImages(
        context,
        outputImageType );
    _CHECK_ERROR;

    printf("Set off-screen panoramic image size:%dx%d image.\n", iOutputImageWidth, iOutputImageHeight );
    error = ladybugSetOffScreenImageSize(
        context,
        outputImageType,
        iOutputImageWidth,
        iOutputImageHeight );
    _CHECK_ERROR;

    return LADYBUG_OK;
}

void LadybugWidget::paintImage()
{
    error = ladybugSetSphericalViewParams(
        context,
        fFOV,
        fRotX * 3.14159265f / 180.0f,
        fRotY * 3.14159265f / 180.0f,
        fRotZ * 3.14159265f / 180.0f,
        0.0f,
        0.0f,
        0.0f);

    error = ladybugReadImageFromStream( readContext, &image);
    //
    // Convert the image to BGRU format texture buffers
    //
    error = ladybugConvertImage( context, &image, arpTextureBuffers, isHighBitDepth(streamHeaderInfo.dataFormat) ? LADYBUG_BGRU16 : LADYBUG_BGRU);

    //
    // Update the textures on graphics card
    //
    error = ladybugUpdateTextures(
        context, LADYBUG_NUM_CAMERAS, (const unsigned char**)arpTextureBuffers, isHighBitDepth(streamHeaderInfo.dataFormat) ? LADYBUG_BGRU16 : LADYBUG_BGRU);

    //
    // Render and obtain the image in off-screen buffer
    //
    error = ladybugRenderOffScreenImage(
        context, outputImageType, LADYBUG_BGR, &processedImage);

    //Processing here!
    /*for(int i= 0; i< processedImage.uiCols*processedImage.uiRows*3; i++)
    {
        processedImage.pData[i] = 255 - processedImage.pData[i];
    }*/

    //
    //Display image in QLabel
    //
    QImage I(processedImage.pData, processedImage.uiCols, processedImage.uiRows,
             QImage::Format_RGB888);
    labelImage->setPixmap(QPixmap::fromImage(
                          I.scaled(labelImage->width(),
                          labelImage->height()).rgbSwapped()));
    labelImage->update();

    iFrame++;
}


void LadybugWidget::SetFileName()
{
    name1 = QFileDialog::getOpenFileName(this, "abrir PRG", ".", tr("*.pgr"));
    if(name1.isEmpty())
    {
        QMessageBox::warning(this, "Error de apertura", "No existe el archivo");
        this->close();
    }
}

void LadybugWidget::StarCamera()
{
    /// Camera initialization ///
    error = initializeLadybug();
    error = ladybugGetStreamNumOfImages( readContext, &totalFrames);
    // fast-forward to the first frame to process in the stream
    error = ladybugGoToImage( readContext, iFrameFrom);

    //fps rate connection
    connect(t, SIGNAL(timeout()), SLOT(paintImage()));

}


void LadybugWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton)
    {
        flagmove = true;
    }
    else
        flagmove = false;
    e->accept();
}
void LadybugWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(flagmove)
    {
        if(e->posF().x()>(this->width()/2.0))
            fRotZ+=5.0;
        else
            fRotZ-=5.0;

        if(e->posF().y()>(this->height()/2.0))
            fRotY-=3.0;
        else
            fRotY+=3.0;
    }
    e->accept();
}
