#include "ladybugwidget.h"

//=======================================================
// Constructor
//=======================================================
LadybugWidget::LadybugWidget(QWidget *parent) :
    QWidget(parent), iFrameFrom(0), iFrameTo(0),
    iOutputImageWidth(1024), iOutputImageHeight(512)
{
    //=== Label Image ====//
    this->setMinimumSize(1100, 470);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    labelImage = new QLabel(this);
    lv = new QVBoxLayout;
    layout = new QHBoxLayout;
    //==== Select Action =============================//
    se = new QAction("Seleccionar", this);
    se->setIcon(QIcon(QPixmap(":/LWidget/select.png")));
    se->setCheckable(true);
    se->setDisabled(true); //Enable when streaming starts
    //==== Open PGR Action =============================//
    ao = new QAction("Abrir archivo PGR", this);
    ao->setIcon(QIcon(QPixmap(":/LWidget/open.png")));
    ao->setEnabled(true); //Enable when streaming starts
    //==== LRM Logo =============================//
    lrm = new QLabel(this);
    lrm->setPixmap(QPixmap(":/LWidget/LRM.png").scaled(185,36));
    separator = new QWidget(this);
    separator->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //==== Play PGR Action =============================//
    aplay = new QAction("Reproducir", this);
    aplay->setIcon(QIcon(QPixmap(":/LWidget/play.png")));
    aplay->setDisabled(true); //Enable when streaming starts
    //==== Pause PGR Action =============================//
    apause = new QAction("Pause", this);
    apause->setIcon(QIcon(QPixmap(":/LWidget/pause.png")));
    apause->setCheckable(true);
    apause->setDisabled(true); //Enable when streaming starts
    //==== Stop PGR Action =============================//
    astop = new QAction("Stop", this);
    astop->setIcon(QIcon(QPixmap(":/LWidget/stop.png")));
    astop->setDisabled(true); //Enable when streaming starts
    //======= Progress Bar    =============================//
    bprog = new QSlider(Qt::Horizontal, this);
    bprog->setTickInterval(1);
    bprog->setTickPosition(QSlider::TicksBelow);
    bprog->setToolTip("<b>Barra de progreso</b>");
    bprog->setDisabled(true); //Enable when streaming starts
    //=========== Top Toolbar ============================//
    tbt = new QToolBar("Menu", this);
    tbt->setIconSize(QSize(36,36));
    tbt->addAction(se);
    tbt->addAction(ao);
    tbt->addWidget(separator);
    tbt->addSeparator();
    tbt->addWidget(lrm);
    //=========== Bottom Toolbar ============================//
    tbb = new QToolBar("Controls", this);
    tbb->setIconSize(QSize(16,16));
    tbb->addAction(aplay);
    tbb->addAction(apause);
    tbb->addAction(astop);
    tbb->addWidget(bprog);
    //============ Map ===================================//
    mc = new MapControl(QSize(300, 450));
    // create layer
    /*MapAdapter* mapadapter = new OSMMapAdapter();
    Layer* l = new MapLayer("Custom Layer", mapadapter);*/

    // create MapAdapter to get maps from
    TileMapAdapter* mapadapter = new TileMapAdapter("mt1.google.com", "/vt/lyrs=m@120&hl=de&x=%2&y=%3&z=%1&s=", 256, 0, 25);
    // create a layer with the mapadapter and type MapLayer
    l = new Layer("Custom Layer", mapadapter, Layer::MapLayer);

    mc->addLayer(l);
    linepen = new QPen(QColor(0, 0, 255, 100));
    linepen->setWidth(5);

    ////////////// ================= Testing GPS ==================////////////////
    // create buttons as controls for zoom
    QPushButton* zoomin = new QPushButton("+");
    QPushButton* zoomout = new QPushButton("-");
    gpsposition = new QLabel();
    zoomin->setMaximumWidth(50);
    zoomout->setMaximumWidth(50);
    gpsposition->setFont(QFont("Arial", 10));
    connect(zoomin, SIGNAL(clicked(bool)),
              mc, SLOT(zoomIn()));
    connect(zoomout, SIGNAL(clicked(bool)),
              mc, SLOT(zoomOut()));
    // add zoom buttons to the layout of the MapControl
    QVBoxLayout* innerlayout = new QVBoxLayout;
    innerlayout->addWidget(zoomin);
    innerlayout->addWidget(zoomout);
    innerlayout->addWidget(gpsposition);
    mc->setZoom(17);
    mc->setLayout(innerlayout);
    //========= Layout Sorting ===========================//
    lv->addWidget(tbt);
    lv->addWidget(labelImage);
    lv->addWidget(tbb);
    layout->addLayout(lv,1);
    layout->addWidget(mc,1);
    this->setLayout(layout->layout());
    //=== Action Connection ===//
    connect(se, SIGNAL(triggered(bool)), SLOT(SelectI(bool)));
    connect(ao, SIGNAL(triggered()), SLOT(SetFileName()));
    connect(aplay, SIGNAL(triggered()), SLOT(StarCamera()));
    connect(apause, SIGNAL(toggled(bool)), SLOT(Pause(bool)));
    connect(astop, SIGNAL(triggered()), SLOT(Stop()));
    sel = false;
    //=== Variable initialization ===//
    outputImageType = LADYBUG_SPHERICAL;//LADYBUG_SPHERICAL;
    outputImageFormat = LADYBUG_FILEFORMAT_JPG;
    colorProcessingMethod = LADYBUG_HQLINEAR_GPU;
    iBlendingWidth = 100;
    fFalloffCorrectionValue = 1.0f;
    bFalloffCorrectionFlagOn = false;
    bEnableAntiAliasing = true;
    bEnableSoftwareRendering = false;
    bEnableStabilization = false;
    stabilizationParams = {6, 100, 0.95};
    fFOV = 60.0f;
    fRotX = 0.0f;
    fRotY = 0.0f;
    fRotZ = 30.0f;
    scale = 0.0f;
    iBitRate = 4000; // in kbps
    processH264 = false;
    pszConfigFile = (char*)"ladybug11501086.cal";
    iFrame = 0;
    flagmove = false;
    //Current pos
    Cpos = 0;
    t = new QTimer(this);
}
//=======================================================
// Destructor
//=======================================================
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

//=======================================================
// Clean Up
//=======================================================
void LadybugWidget::cleanUp()
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
//=======================================================
// Ladybug Image Depth bit
//=======================================================
bool LadybugWidget::isHighBitDepth( LadybugDataFormat format)
{
    return (format == LADYBUG_DATAFORMAT_RAW12 ||
        format == LADYBUG_DATAFORMAT_HALF_HEIGHT_RAW12 ||
        format == LADYBUG_DATAFORMAT_COLOR_SEP_JPEG12 ||
        format == LADYBUG_DATAFORMAT_COLOR_SEP_HALF_HEIGHT_JPEG12 ||
        format == LADYBUG_DATAFORMAT_RAW16 ||
        format == LADYBUG_DATAFORMAT_HALF_HEIGHT_RAW16);
}
//=======================================================
// Ladybug Initialization
//=======================================================
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
        strcpy( pszTempPath, "temp.cal");
        error = ladybugGetStreamConfigFile( readContext , pszTempPath);
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

    qDebug( "--- Stream Information ---\n");
    qDebug( "Stream version : %ld\n", streamHeaderInfo.ulLadybugStreamVersion);
    qDebug( "Base S/N: %ld\n", streamHeaderInfo.serialBase);
    qDebug( "Head S/N: %ld\n", streamHeaderInfo.serialHead);
    qDebug( "Frame rate : %3.2f\n", frameRateToUse);
    qDebug( "--------------------------\n");

    //
    // Set color processing method.
    //
    qDebug("Setting color processing method...\n" );
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
    qDebug( "Initializing alpha masks (this may take some time)...\n" );
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
    qDebug( "Configure output images in Ladybug library...\n" );
    error = ladybugConfigureOutputImages(
        context,
        outputImageType );
    _CHECK_ERROR;

    qDebug("Set off-screen panoramic image size:%dx%d image.\n", iOutputImageWidth, iOutputImageHeight );
    error = ladybugSetOffScreenImageSize(
        context,
        outputImageType,
        iOutputImageWidth,
        iOutputImageHeight );
    _CHECK_ERROR;

    error = ladybugGetStreamNumOfImages( readContext, &totalFrames);

    return LADYBUG_OK;
}

//=====================================================
// Image Display Func
//=====================================================
void LadybugWidget::paintImage(int pos)
{
    error = ladybugGoToImage( readContext, pos);

    error = ladybugSetSphericalViewParams(
        context,
        fFOV,
        fRotX * 3.14159265f / 180.0f,
        fRotY * 3.14159265f / 180.0f,
        fRotZ * 3.14159265f / 180.0f,
        scale /*0.0f init*/,
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

    //
    // GPS NMEA Code from Image
    //
    error = ladybugGetGPSNMEADataFromImage( &image, "GPGGA", &gpsData);
    if ( error == LADYBUG_OK && gpsData.bValidData)
    {
        qDebug( "GPS INFO: LAT %lf, LONG %lf\n", gpsData.dGGALatitude, gpsData.dGGALongitude);
        mc->setView(QPointF(gpsData.dGGALongitude, gpsData.dGGALatitude));
        points.append(new Point(gpsData.dGGALongitude, gpsData.dGGALatitude, "Posición "+QString::number(pos)));
        LineString* ls = new LineString(points, "Ruta", linepen);
        // Add the LineString to the layer
        l->addGeometry(ls);
    }

    //Processing here!
    /*for(int i= 0; i< processedImage.uiCols*processedImage.uiRows*3; i++)
    {
        processedImage.pData[i] = 255 - processedImage.pData[i];
    }*/

    /*Im.SetImage(processedImage.pData,processedImage.uiCols,processedImage.uiRows,3);
    Im.SetColor(5);
    Im.Procesado();
    Im.ReleaseImage();*/
    //
    //Display image in QLabel
    //
    QImage I(processedImage.pData, processedImage.uiCols, processedImage.uiRows,
             QImage::Format_RGB888);
    Pori = QPixmap::fromImage(I.rgbSwapped());
    Pix = QPixmap::fromImage(I.scaled(labelImage->width(),
                       labelImage->height()).rgbSwapped());
    labelImage->setPixmap(Pix);
    labelImage->update();

    //iFrame++;
}

//===========================================================
// Update streaming
//===========================================================
void LadybugWidget::updateStream()
{
    Cpos = (Cpos>totalFrames)?Cpos:Cpos+1;
    emit CurrentPos(Cpos);
}

void LadybugWidget::SetFrame(int fr)
{
    paintImage(fr);
    bprog->setValue(fr);
    Cpos = (unsigned int)fr;
}
//=======================================================
// Open PGR File
//=======================================================
void LadybugWidget::SetFileName()
{
    name1 = QFileDialog::getOpenFileName(this, "abrir PRG", ".", tr("*.pgr"));
    if(name1.isEmpty())
    {
        QMessageBox::warning(this, "Error de apertura", "No existe el archivo");
        //this->close();
    }
    else
        aplay->setEnabled(true);
}

//=======================================================
// Start Streaming File
//=======================================================
void LadybugWidget::StarCamera()
{
    /// Camera initialization ///
    error = initializeLadybug();
    error = ladybugGetStreamNumOfImages( readContext, &totalFrames);
    // fast-forward to the first frame to process in the stream
    error = ladybugGoToImage( readContext, iFrameFrom);
    // === Enable Actions  =======///
    se->setEnabled(true);
    ao->setDisabled(true);
    apause->setEnabled(true);
    aplay->setDisabled(true);
    astop->setEnabled(true);
    bprog->setEnabled(true);
    bprog->setRange(0,totalFrames);
    //fps rate connection
    t->start(1);
    connect(t, SIGNAL(timeout()), SLOT(updateStream()));
    connect(this,SIGNAL(CurrentPos(int)), bprog, SLOT(setValue(int)));
    connect(bprog, SIGNAL(valueChanged(int)), SLOT(SetFrame(int)));

}

//=======================================================
// Mouse Press Event -- Click to drag a position
//=======================================================
void LadybugWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton)
    {
        if(sel)
        {
            initp = e->pos();
            flagmove = false;
        }
        else
        {
            flagmove = true;
            ppos = QPoint(0,0);
            // Map  coordinates (kRawX, kRawY) via ladybug library functionality into ladybug global coorindates.
            /*LadybugPoint3d point3d = libraryTranslation( context, 0, e->pos().x(), e->pos().y() );
            qDebug("Ladybug global coordinates (library) = (%lf, %lf, %lf)\n", point3d.fX, point3d.fY, point3d.fZ );*/

            /*double dLadybugX, dLadybugY, dLadybugZ;
            manualTranslation( context, 0,  e->pos().x(), e->pos().y(), 20.0, dLadybugX, dLadybugY, dLadybugZ );
            qDebug("Ladybug global coordinates (manual) = (%lf, %lf, %lf)\n", dLadybugX, dLadybugY, dLadybugZ  );*/
        }
    }
    else
        flagmove = false;
    e->accept();
}
//=======================================================
// Mouse Release Event -- Select Region when release
//=======================================================
void LadybugWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button()==Qt::LeftButton)
    {
        if(sel)
        {
            endp = e->pos();
            QPixmap psq(labelImage->width(), labelImage->height());
            QPainter p(&psq);
            p.setPen(QPen(Qt::red, 3));
            QRectF rectangle((float)initp.x()-labelImage->pos().x(), (float)initp.y()-labelImage->pos().y(),
                             (float)endp.x()-(float)initp.x(), (float)endp.y()-(float)initp.y());
            p.drawPixmap(0,0, Pix);
            p.drawRect(rectangle);
            labelImage->setPixmap(psq);
            QString nfile = QFileDialog::getSaveFileName(this, "Guardar Captura", ".", tr("IMAGEN(*.png *.jpg)"));
            QSizeF scalepi((float)processedImage.uiCols/(float)labelImage->width(),
                           (float)processedImage.uiRows/(float)labelImage->height());
            GetRealDimRectangle(QRectF(rectangle.x()*scalepi.width(), rectangle.y()*scalepi.height(),
                                       rectangle.width()*scalepi.width(), rectangle.height()*scalepi.height()));
            if(!nfile.isEmpty())
                //Save snipping area
                Pori.copy(rectangle.x()*scalepi.width(), rectangle.y()*scalepi.height(),
                          rectangle.width()*scalepi.width(), rectangle.height()*scalepi.height()).save(nfile);
        }
    }
    e->accept();
}
//=====================================================================
// Mose move Event -- click to move perspective camera or select region
//=====================================================================
void LadybugWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(flagmove)
    {
        oldpos = ppos;
        ppos = e->pos();
        //if((ppos.x()-oldpos.x())>0)
            fRotZ+=(ppos.x()-oldpos.x())/50.0;
            fRotY+=(ppos.y()-oldpos.y())/50.0;
        /*else
            fRotZ-=0.5;

        if((ppos.y()-oldpos.y())>0)
            fRotY-=0.4;
        else
            fRotY+=0.4;*/
        qDebug()<<"rotz: "<<fRotZ<<" roty:"<<fRotY<<endl;
        paintImage(Cpos);
    }
    if(sel)
    {   
        QPixmap psq(labelImage->width(), labelImage->height());
        QPainter p(&psq);
        p.setPen(QPen(Qt::red, 3));
        float xi = (float)initp.x()-labelImage->pos().x();
        float yi = (float)initp.y()-labelImage->pos().y();
        float wt = (float)e->pos().x()-(float)initp.x();
        float ht = (float)e->pos().y()-(float)initp.y();
        QVector<QRectF> vrect;
        //Rectangle grid
        vrect.push_back(QRectF(xi, yi, wt/2.0, ht/2.0));
        vrect.push_back(QRectF(xi+wt/2.0, yi, wt/2.0, ht/2.0));
        vrect.push_back(QRectF(xi, yi+ht/2.0, wt/2.0, ht/2.0));
        vrect.push_back(QRectF(xi+wt/2.0, yi+ht/2.0, wt/2.0, ht/2.0));
        p.drawPixmap(0,0,Pix);
        p.drawRects(vrect);
        labelImage->setPixmap(psq);
    }
    e->accept();
}

//=======================================================
// Wheel Event -- Zoom Event
//=======================================================
void LadybugWidget::wheelEvent(QWheelEvent *e)
{
    scale -= e->delta()/15/8;
    paintImage(Cpos);
    e->accept();
}

//=======================================================
// Get Dimensions from a Rectangle
//=======================================================
void LadybugWidget::GetRealDimRectangle(QRectF r)
{
    double dLadybugX0, dLadybugY0, dLadybugZ0;
    double dLadybugX1, dLadybugY1, dLadybugZ1;
    manualTranslation( context, 0,  r.x(), r.y(), 20.0, dLadybugX0, dLadybugY0, dLadybugZ0 );
    manualTranslation( context, 0,  r.x()+r.width(), r.y()+r.height(), 20.0,
                       dLadybugX1, dLadybugY1, dLadybugZ1 );
    double x0 = dLadybugX0/dLadybugZ0;
    double y0 = dLadybugY0/dLadybugZ0;
    double x1 = dLadybugX1/dLadybugZ1;
    double y1 = dLadybugY1/dLadybugZ1;
    qDebug("W: %lf, H: %lf, z0: %lf, z1: %lf\n",fabs(y1-y0), fabs(x1-x0), dLadybugZ0, dLadybugZ1);/*dLadybugY1-dLadybugY0, dLadybugZ1-dLadybugZ0);*/
}

//=======================================================
// Resize Event
//=======================================================
void LadybugWidget::resizeEvent(QResizeEvent *e)
{
    mc->resize(QSize(this->width()-labelImage->width(), this->height()-20));
    e->accept();
}

//=======================================================
// Ladybug Translation 2D to 3D
//=======================================================
LadybugPoint3d LadybugWidget::libraryTranslation( LadybugContext context, const int kCamera, const int kRawX, const int kRawY )
{
    int srcCols, srcRows;
    srcCols = 1616;
    srcRows = 1232;
    const int kGridCols = srcCols;
    const int kGridRows = srcRows;
    const LadybugImage3d* pImage3d;
    error = ladybugGet3dMap( context, kCamera, kGridCols, kGridRows, srcCols, srcRows, false, &pImage3d);
    if (error != LADYBUG_OK) { qDebug("Error ladybugGet3dMap - %s", ladybugErrorToString(error)); this->close(); }
    // obtaining the grid point that is one of the closest ones.
    const int gridX = kRawX * kGridCols / srcCols;
    const int gridY = kRawY * kGridRows / srcRows;
    return pImage3d->ppoints[ gridY * kGridCols + gridX];
}
//=======================================================
// Pause streaming file
//=======================================================
void LadybugWidget::Pause(bool p)
{
    if(p)
    {
        t->stop();
    }
    else t->start(10);
}

//=======================================================
// Stop streaming file
//=======================================================
void LadybugWidget::Stop()
{
    t->stop();
    aplay->setEnabled(true);
    apause->setDisabled(true);
    astop->setDisabled(true);
    labelImage->clear();
    Cpos = 0;
    ao->setEnabled(true);
    //cleanUp();
}

//=======================================================
// Select Action
//=======================================================
void LadybugWidget::SelectI(bool p)
{
    sel = p;
}
//=============================================
//Manual transformation whit Craig's Matrix
//=============================================
void LadybugWidget::makeTransformation( const double rotX, const double rotY, const double rotZ, const double transX, const double transY, const double transZ,
                             double matrix[4][4] /*out*/ )
{
    double cosX, sinX, cosY, sinY, cosZ, sinZ;

    cosX = cos( rotX );		sinX = sin( rotX );
    cosY = cos( rotY );		sinY = sin( rotY );
    cosZ = cos( rotZ );		sinZ = sin( rotZ );

    // translation portion of transform
    matrix[0][3] = transX;
    matrix[1][3] = transY;
    matrix[2][3] = transZ;

    // cz*cy;
    matrix[0][0] = cosZ * cosY;
    // cz*sy*sx - sz*cx;
    matrix[0][1] = cosZ * sinY * sinX - sinZ * cosX;
    // cz*sy*cx + sz*sx;
    matrix[0][2] = cosZ * sinY * cosX + sinZ * sinX;

    // sz*cy;
    matrix[1][0] = sinZ * cosY;
    // sz*sy*sx + cz*cx;
    matrix[1][1] = sinZ * sinY * sinX + cosZ * cosX;
    // sz*sy*cx - cz*sx;
    matrix[1][2] = sinZ * sinY * cosX - cosZ * sinX;

    //-sy;
    matrix[2][0] = -sinY;
    //cy*sx;
    matrix[2][1] = cosY * sinX;
    //cy*cx;
    matrix[2][2] = cosY * cosX;

    // bottom row, always the same
    matrix[3][0] = 0.0;
    matrix[3][1] = 0.0;
    matrix[3][2] = 0.0;
    matrix[3][3] = 1.0;
}
//=======================================================
// Euler Transformation
//=======================================================
void LadybugWidget::applyTransformation(const double matrix[4][4], double & x, double & y, double & z)
{
    double outPoint[3];
    for ( int r = 0; r < 3; r++ )
    {
        outPoint[r] = matrix[r][3]
                    + matrix[r][0] * x
                    + matrix[r][1] * y
                    + matrix[r][2] * z ;
    }
    x = outPoint[0];
    y = outPoint[1];
    z = outPoint[2];
}
//=======================================================
// Manual Translation 2D to 3D
//=======================================================
void LadybugWidget::manualTranslation( LadybugContext context, const int kCamera, const int kRawX, const int kRawY, const double kSphereSize,
                        double & dLadybugX /*out*/, double & dLadybugY /*out*/, double & dLadybugZ /*out*/ )
{
    // Offscreen image size needs to be set before using ladybugGetCameraUnitFocalLength and ladybugGetCameraUnitImageCenter
    const int kRectImageWidth = iOutputImageWidth;//400; // this doesn't affect the result
    const int kRectImageHeight = iOutputImageHeight;//300; // this doesn't affect the result
    LadybugError error = ladybugSetOffScreenImageSize(context, LADYBUG_SPHERICAL, kRectImageWidth, kRectImageHeight);
    if (error != LADYBUG_OK) { qDebug( "Error ladybugSetOffScreenImageSize - %s", ladybugErrorToString(error)); this->close(); }

    // Read in information about the camera.
    double dFocalLen = 0.0;
    error = ladybugGetCameraUnitFocalLength(context, kCamera, &dFocalLen);
    if (error != LADYBUG_OK) { qDebug( "Error ladybugGetCameraUnitFocalLength - %s", ladybugErrorToString(error)); this->close(); }

    double dCameraCenterX = 0.0;
    double dCameraCenterY = 0.0;
    error = ladybugGetCameraUnitImageCenter(context, kCamera, &dCameraCenterX, &dCameraCenterY);
    if (error != LADYBUG_OK) { qDebug( "Error ladybugGetCameraUnitImageCenter - %s", ladybugErrorToString(error)); this->close(); }

    double dExtrinsics[6] = {0.0};
    error = ladybugGetCameraUnitExtrinsics(context, kCamera, dExtrinsics);
    if (error != LADYBUG_OK) { qDebug( "Error ladybugGetCameraUnitExtrinsics - %s", ladybugErrorToString(error)); this->close(); }

    qDebug()<<"Extrinsic parameters: "<<dExtrinsics[0]<<","<<dExtrinsics[1]<<","
           <<dExtrinsics[2]<<","<<dExtrinsics[3]<<","<<dExtrinsics[4]<<","<<dExtrinsics[5]<<"\n";

    // Map the raw coordinate to a rectified coordinate
    double dRectifiedX = 0.0;
    double dRectifiedY = 0.0;
    error = ladybugRectifyPixel( context, kCamera, kRawY, kRawX, &dRectifiedY, &dRectifiedX );
    if (error != LADYBUG_OK) { qDebug( "Error ladybugRectifyPixel - %s", ladybugErrorToString(error)); this->close(); }

    qDebug( "Raw image coordinate = (%d, %d)\n", kRawX, kRawY );
    qDebug( "Rectified image coordinate = (%lf, %lf)\n", dRectifiedX, dRectifiedY );

    // Map the rectified coordinate to camera-local 3D coordinate (dLocalX, dLocalY, dLocalZ)
    //
    // Here, we solve the following equations:
    //
    //  (Pin-hole camera model)
    //  kRectifiedX = ( dLocalX / dLocalZ) * dFocalLen + dCameraCenterX
    //  kRectifiedY = ( dLocalY / dLocalZ) * dFocalLen + dCameraCenterY
    //
    //  (Set the constraint that the point is on a sphere with the radius of kSphereSize
    //  dLocalX^2 + dLocalY^2 + dLocalZ^2 = kSphereSize^2
    const double kx = (dRectifiedX - dCameraCenterX) / dFocalLen;
    const double ky = (dRectifiedY - dCameraCenterY) / dFocalLen;
    const double dLocalZ = kSphereSize / sqrt(kx * kx + ky * ky + 1.0);
    const double dLocalX = kx * dLocalZ;
    const double dLocalY = ky * dLocalZ;
    qDebug( "Camera local coordinate = (%lf, %lf, %lf)\n", dLocalX, dLocalY, dLocalZ);

    // Map the camera-local 3D coordinate (dLocalX, dLocalY, dLocalZ) to
    // ladybug (global) coordinate (dLadybugX, dLadybugY, dLadybugZ).
    // We create an explicit representation of Craig's Matrix as described in
    // ladybuggeom.h:ladybugGetCameraUnitExtrinsics().
    const double rotX = dExtrinsics[0];
    const double rotY = dExtrinsics[1];
    const double rotZ = dExtrinsics[2];
    const double transX = dExtrinsics[3];
    const double transY = dExtrinsics[4];
    const double transZ = dExtrinsics[5];

    double toGlobalCoords[4][4]; // Craig's Matrix
    makeTransformation( rotX, rotY, rotZ, transX, transY, transZ, toGlobalCoords );
    dLadybugX = dLocalX;
    dLadybugY = dLocalY;
    dLadybugZ = dLocalZ;
    applyTransformation( toGlobalCoords, dLadybugX, dLadybugY, dLadybugZ );

    // scale the point so that it is on the sphere
    const double dLen = sqrt(dLadybugX * dLadybugX + dLadybugY * dLadybugY + dLadybugZ * dLadybugZ);
    dLadybugX = dLadybugX / dLen * kSphereSize;
    dLadybugY = dLadybugY / dLen * kSphereSize;
    dLadybugZ = dLadybugZ / dLen * kSphereSize;

    qDebug( "Ladybug global coordinates (no rot)  = (%lf, %lf, %lf)\n", dLadybugX, dLadybugY, dLadybugZ );

    // Apply rotation to camera 0 aligned global coordinates.
    double dRx, dRy, dRz;
    error = ladybugGet3dMapRotation( context, & dRx, & dRy, & dRz );
    if (error != LADYBUG_OK) { qDebug( "Error ladybugGet3dMapRotation - %s", ladybugErrorToString(error)); this->close(); }

    double cam0rotation[4][4];
    makeTransformation( dRx, dRy, dRz, 0.0, 0.0, 0.0, cam0rotation );
    applyTransformation( cam0rotation, dLadybugX, dLadybugY, dLadybugZ );
}

//================================================================================
// Reverse Translation -- 3D Point to 2D Point
//================================================================================
void LadybugWidget::reverseTranslation( LadybugContext context, const int kCamera, const double x, const double y, const double z,
                             double & unrectifiedX /*out*/, double & unrectifiedY /*out*/ )
{
    unrectifiedX = -1.0;
    unrectifiedY = -1.0;

    double calculatedX = 0.0;
    double calculatedY = 0.0;
    error = ladybugXYZtoRC(context, x, y, z, kCamera, &calculatedY, &calculatedX, NULL);
    if (error != LADYBUG_OK) { qDebug( "Error ladybugXYZtoRC - %s", ladybugErrorToString(error)); this->close(); }

    if (calculatedX > 0.0 && calculatedY > 0.0)
    {
        ladybugUnrectifyPixel(context, kCamera, calculatedY, calculatedX, &unrectifiedY, &unrectifiedX);
        if (error != LADYBUG_OK) { qDebug("Error ladybugUnrectifyPixel - %s", ladybugErrorToString(error)); this->close(); }
    }
}

void LadybugWidget::GPS_PositonMap(QByteArray line)
{
    //QByteArray line("$GPGGA,155115.000,0437.6047,N,07403.8467,W,1,05,3.3,2692.6,M,3.4,M,,0000*49");
    QList<QByteArray> elems = line.split(',');
    float time = QString(elems.at(1)).toFloat();
    float latitude = elems.at(2).toFloat()/100;
    QString latitude_dir = elems.at(3);
    float longitude = elems.at(4).toFloat()/100;
    QString longitude_dir = elems.at(5);
    //=== Converting to GPS Position  ====//
    GPS_Position pos = GPS_Position(time, longitude, longitude_dir, latitude, latitude_dir);
    gpsposition->setText(QString::number(time) + " / " + QString::number(pos.longitude) + " / " + QString::number(pos.latitude));
    mc->setView(QPointF(pos.longitude, pos.latitude));
}
