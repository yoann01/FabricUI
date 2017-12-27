/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#include <QMouseEvent>
#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include "QtToKLEvent.h"
#include "TimeLineWidget.h"
#include "GLViewportWidget.h"
#include <FabricUI/DFG/DFGLogWidget.h>
#include <FabricUI/Commands/KLCommandManager.h>
#include <FabricUI/Application/FabricException.h>
#include <FabricUI/Application/FabricApplicationStates.h>

using namespace FabricUI;
using namespace Commands;
using namespace Viewports;
using namespace FabricCore;
using namespace Application;

GLViewportCaptureSequenceDialog::GLViewportCaptureSequenceDialog(QWidget *parent, QString title, const FabricUI::DFG::DFGConfig &dfgConfig)
  : FabricUI::DFG::DFGBaseDialog(parent, true, dfgConfig)
{
  setObjectName("GLViewportCaptureSequenceDialog");

  this->setWindowTitle(title);

  m_lineEditCaptureResX = new QLineEdit();
  m_lineEditCaptureResX->setMinimumWidth(50);
  m_lineEditCaptureResX->setReadOnly(true);
  m_lineEditCaptureResX->setValidator(new QIntValidator(0, 32000, this));
  addInput(m_lineEditCaptureResX, "Viewport Width");

  m_lineEditCaptureResY = new QLineEdit();
  m_lineEditCaptureResY->setMinimumWidth(50);
  m_lineEditCaptureResY->setReadOnly(true);
  m_lineEditCaptureResY->setValidator(new QIntValidator(0, 32000, this));
  addInput(m_lineEditCaptureResY, "Viewport Height");

  m_lineEditCapturePath = new QLineEdit();
  m_lineEditCapturePath->setMinimumWidth(250);
  addInput(m_lineEditCapturePath, "Path");

  m_lineEditCaptureFilename = new QLineEdit();
  m_lineEditCaptureFilename->setMinimumWidth(150);
  addInput(m_lineEditCaptureFilename, "Filename");

  m_lineEditCaptureFramePadding = new QLineEdit();
  m_lineEditCaptureFramePadding->setMinimumWidth(50);
  m_lineEditCaptureFramePadding->setValidator(new QIntValidator(0, 32000, this));
  addInput(m_lineEditCaptureFramePadding, "Frame Padding");

  m_lineEditCaptureFrameStart = new QLineEdit();
  m_lineEditCaptureFrameStart->setMinimumWidth(50);
  m_lineEditCaptureFrameStart->setValidator(new QIntValidator(-32000, 32000, this));
  addInput(m_lineEditCaptureFrameStart, "Frame Start");

  m_lineEditCaptureFrameEnd = new QLineEdit();
  m_lineEditCaptureFrameEnd->setMinimumWidth(50);
  m_lineEditCaptureFrameEnd->setValidator(new QIntValidator(-32000, 32000, this));
  addInput(m_lineEditCaptureFrameEnd, "Frame End");

  //this->adjustSize();
  //this->window()->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

GLViewportCaptureSequenceDialog::~GLViewportCaptureSequenceDialog()
{
}

int GLViewportCaptureSequenceDialog::captureResX()
{
  return m_lineEditCaptureResX->text().toInt();
}

int GLViewportCaptureSequenceDialog::captureResY()
{
  return m_lineEditCaptureResY->text().toInt();
}

QString GLViewportCaptureSequenceDialog::capturePath()
{
  return m_lineEditCapturePath->text();
}

QString GLViewportCaptureSequenceDialog::captureFilename()
{
  return m_lineEditCaptureFilename->text();
}

int GLViewportCaptureSequenceDialog::captureFramePadding()
{
  return m_lineEditCaptureFramePadding->text().toInt();
}

int GLViewportCaptureSequenceDialog::captureFrameStart()
{
  return m_lineEditCaptureFrameStart->text().toInt();
}

int GLViewportCaptureSequenceDialog::captureFrameEnd()
{
  return m_lineEditCaptureFrameEnd->text().toInt();
}

void GLViewportCaptureSequenceDialog::setCaptureResX(int resX)
{
  m_lineEditCaptureResX->setText(QString::number(resX));
}

void GLViewportCaptureSequenceDialog::setCaptureResY(int resY)
{
  m_lineEditCaptureResY->setText(QString::number(resY));
}

void GLViewportCaptureSequenceDialog::setCapturePath(QString path)
{
  m_lineEditCapturePath->setText(path);
}

void GLViewportCaptureSequenceDialog::setCaptureFilename(QString filename)
{
  m_lineEditCaptureFilename->setText(filename);
}

void GLViewportCaptureSequenceDialog::setCaptureFramePadding(int framePadding)
{
  m_lineEditCaptureFramePadding->setText(QString::number(framePadding));
}

void GLViewportCaptureSequenceDialog::setCaptureFrameStart(int frameStart)
{
  m_lineEditCaptureFrameStart->setText(QString::number(frameStart));
}

void GLViewportCaptureSequenceDialog::setCaptureFrameEnd(int frameEnd)
{
  m_lineEditCaptureFrameEnd->setText(QString::number(frameEnd));
}

GLViewportWidget::GLViewportWidget(
  QColor bgColor, 
  QGLFormat format, 
  QWidget *parent)
  : ViewportWidget(format, parent)
  , m_bgColor(bgColor)
{
  setAutoBufferSwap(false);

  m_gridVisible = true; // default value
  
  QSettings* settings = FabricApplicationStates::GetAppStates()->getSettings();
  if(settings && settings->contains("glviewport/gridVisible"))
    m_gridVisible = settings->value("glviewport/gridVisible").toBool();

  FABRIC_CATCH_BEGIN();

  Client client = FabricApplicationStates::GetAppStates()->getClient();
  client.loadExtension("Manipulation", "", false);
  client.loadExtension("InlineDrawing", "", false);
  
  FABRIC_CATCH_END("GLViewportWidget::GLViewportWidget");

  m_resizedOnce = false;
  initializeID(false /*shouldUpdateGL*/);
}

GLViewportWidget::~GLViewportWidget()
{
}

RTVal GLViewportWidget::getCamera() 
{ 
  return m_camera; 
}

bool GLViewportWidget::onEvent(
  QEvent *event)
{
  bool isAccepted = false;
  
  FABRIC_CATCH_BEGIN();

  if(!m_viewport.isValid())
    return false;

  // Now we translate the Qt events to FabricEngine events..
  RTVal klevent = QtToKLEvent(
    event, 
    m_viewport, 
    "Canvas");

  if(!klevent.isValid())
    return false;

  bool alt = QApplication::keyboardModifiers().testFlag(Qt::AltModifier);
  
  if(!alt && m_manipTool->isActive())
  {
    bool redrawRequested;
    QString manipulatedPortName;

    isAccepted = m_manipTool->onEvent(
      klevent,
      redrawRequested,
      manipulatedPortName);

    if(!event->isAccepted() && isAccepted)
      event->setAccepted(isAccepted);

    // In certain cases, the kl event is not accepted but should be.
    // We check if KL commands where added.
    if(isAccepted || event->type() == QEvent::MouseButtonRelease)
    {
      KLCommandManager *manager = qobject_cast<KLCommandManager*>(
        CommandManager::getCommandManager());

      // Check the command execution and print the exception, 
      // we don't want to crash the app if the command fails.
      try
      {
        manager->synchronizeKL();
      }

      catch(FabricException &e)
      {
        FabricException::Throw(
          "GLViewportWidget::onEvent",
          "",
          e.what(),
          FabricException::LOG);
        
        return isAccepted;
      }
    }    

    ///!!!Force the graph to update correclty
    if(isAccepted)
      emit portManipulationRequested(manipulatedPortName);    

    if(redrawRequested)
      updateGL();
  }
  
  else if(alt)
    isAccepted = manipulateCamera(klevent);
 
  FABRIC_CATCH_END("GLViewportWidget::onEvent");

  return isAccepted;
}

void GLViewportWidget::setBackgroundColor(
  QColor color)
{
  m_bgColor = color;

  if(m_viewport.isValid())
  {
    FABRIC_CATCH_BEGIN();

    Context context = FabricApplicationStates::GetAppStates()->getContext();
    RTVal bgColorVal = m_viewport.maybeGetMember("bgColor");
    bgColorVal.setMember("r", RTVal::ConstructFloat32(context, float(m_bgColor.red()) / 255.0f));
    bgColorVal.setMember("g", RTVal::ConstructFloat32(context, float(m_bgColor.green()) / 255.0f));
    bgColorVal.setMember("b", RTVal::ConstructFloat32(context, float(m_bgColor.blue()) / 255.0f));
    m_viewport.setMember("bgColor", bgColorVal);
    
    FABRIC_CATCH_END("GLViewportWidget::setBackgroundColor");
  }
}

void GLViewportWidget::clear()
{    
  if(m_viewport.isValid() && m_drawContext.isValid())
  {
    initializeID();
    initializeGL();
    resizeGL(size().width(), size().height());
  }
}

void GLViewportWidget::initializeGL()
{
  FABRIC_CATCH_BEGIN();

  m_viewport.callMethod("", "setup", 1, &m_drawContext);
  m_drawing = m_drawing.callMethod("OGLInlineDrawing", "getInstance", 0, 0);
  setGridVisible(m_gridVisible, false);
  emit initComplete();
 
  FABRIC_CATCH_END("GLViewportWidget::initializeGL");
}

void GLViewportWidget::resizeGL(
  int width, 
  int height)
{
  FABRIC_CATCH_BEGIN();

  Context context = FabricApplicationStates::GetAppStates()->getContext();

  RTVal args[3] = {
    m_drawContext,
    RTVal::ConstructUInt32(context, (unsigned int)width),
    RTVal::ConstructUInt32(context, (unsigned int)height)
  };

  m_viewport.callMethod("", "resize", 3, args);
  m_resizedOnce = true;

  FABRIC_CATCH_END("GLViewportWidget::resizeGL");
}

void GLViewportWidget::paintGL()
{
  if(!m_resizedOnce)
  {
    QSize scale = size();
    resizeGL(scale.width(), scale.height());
  }

  computeFPS();

  FABRIC_CATCH_BEGIN();

  RTVal args[2] = {
    RTVal::ConstructString(FabricApplicationStates::GetAppStates()->getContext(), "default"),
    m_drawContext
  };
  m_drawing.callMethod("", "drawViewport", 2, args);

  FABRIC_CATCH_END("GLViewportWidget::resizeGL");

  swapBuffers();

  emit redrawn();
}

void GLViewportWidget::initializeID(
  bool shouldUpdateGL)
{
  FABRIC_CATCH_BEGIN();

  Context context = FabricApplicationStates::GetAppStates()->getContext();

  // Call onNewScene (Singletons interface)
  RTVal singletonHandle = RTVal::Create( context, "SingletonHandle", 0, 0 );
  singletonHandle.callMethod( "", "onNewScene", 0, NULL );

  m_drawing = RTVal::Create(context, "OGLInlineDrawing", 0, 0);
  if(!m_drawing.isValid())
  {
    printf("[GLWidget] Error: Cannot construct OGLInlineDrawing RTVal (extension loaded?)\n");
    return;
  }
  m_drawing = m_drawing.callMethod("OGLInlineDrawing", "getInstance", 0, 0);

  m_viewport = RTVal::Create(context, "OGLStandaloneViewport", 0, 0);
  if(!m_viewport.isValid())
  {
    printf("[GLWidget] Error: Cannot construct OGLStandaloneViewport RTVal (extension loaded?)\n");
    return;
  }
  else
  {
    RTVal args[2] = {
      RTVal::ConstructString(context, "default"),
      m_viewport
    };

    m_drawing.callMethod("", "registerViewport", 2, args);

    // [pzion 20150909] No viewport overlay, at least for now
    // Client client = FabricApplicationStates::GetAppStates()->getClient();
    // bool hasCommercialLicense = client.hasCommercialLicense();
    // if(!hasCommercialLicense)
    //   m_viewport.callMethod("", "setupLicensingOverlay", 0, 0);
  }

  // Proto tool setup
  RTVal::Create(context, "Tool::InlineDrawingRender::RenderSetup", 0, 0);

  m_camera = m_viewport.maybeGetMember("camera");
  m_cameraManipulator = RTVal::Create(context, "CameraManipulator", 1, &m_camera);

  m_viewport.setMember("windowId", RTVal::ConstructUInt64(context, (uint64_t)this->winId()));

  setBackgroundColor(m_bgColor);

  m_drawContext = RTVal::Create(context, "DrawContext", 0, 0);
  if(!m_drawContext.isValid())
  {
    printf("[GLWidget] Error: Cannot construct DrawContext RTVal (extension loaded?)\n");
    return;
  }
  m_drawContext = m_drawContext.callMethod("DrawContext", "getInstance", 0, 0);
  m_drawContext.callMethod("", "resetAllViewportParams", 0, 0);
 
  FABRIC_CATCH_END("GLViewportWidget::initializeID");

  setGridVisible(m_gridVisible, shouldUpdateGL);
  emit initComplete();
}

bool GLViewportWidget::manipulateCamera(
  RTVal klevent,
  bool shouldUpdateGL)
{
  bool result = false;

  FABRIC_CATCH_BEGIN();

  // And then pass the event to the camera manipulator for handling.
  m_cameraManipulator.callMethod("", "onEvent", 1, &klevent);
  result = klevent.callMethod("Boolean", "isAccepted", 0, 0).getBoolean();
  
  FABRIC_CATCH_END("GLViewportWidget::manipulateCamera");

  if(shouldUpdateGL)
    update();

  return result;
}

bool GLViewportWidget::isGridVisible()
{
  return m_gridVisible;
}

void GLViewportWidget::setGridVisible(
  bool gridVisible, 
  bool updateView)
{
  m_gridVisible = gridVisible;

  QSettings* settings = FabricApplicationStates::GetAppStates()->getSettings();
  if(settings)
    settings->setValue("glviewport/gridVisible", m_gridVisible);

  if(!m_viewport.isValid())
    return;

  FABRIC_CATCH_BEGIN();

  Context context = FabricApplicationStates::GetAppStates()->getContext();
  RTVal gridVisibleVal = RTVal::ConstructBoolean(context, m_gridVisible);
  m_viewport.callMethod("", "setGridVisible", 1, &gridVisibleVal);

  FABRIC_CATCH_END("GLViewportWidget::setGridVisible");

  if(updateView)
    update();
}

void GLViewportWidget::resetCamera()
{
  if(!m_viewport.isValid())
    return;

  FABRIC_CATCH_BEGIN();

  Context context = FabricApplicationStates::GetAppStates()->getContext();
  
  RTVal position[3];
  position[0] = RTVal::ConstructFloat32(context, 30.0f);
  position[1] = RTVal::ConstructFloat32(context, 20.0f);
  position[2] = RTVal::ConstructFloat32(context, 40.0f);
  
  RTVal target[3];
  target[0] = RTVal::ConstructFloat32(context, 0.0f);
  target[1] = RTVal::ConstructFloat32(context, 0.0f);
  target[2] = RTVal::ConstructFloat32(context, 0.0f);
  
  RTVal args[2] = {
    RTVal::Construct(context, "Vec3", 3, position),
    RTVal::Construct(context, "Vec3", 3, target)
  };

  m_camera.callMethod("", "setFromPositionAndTarget", 2, args);
  m_cameraManipulator = RTVal::Create(context, "CameraManipulator", 1, &m_camera);
  
  FABRIC_CATCH_END("GLViewportWidget::resetCamera");

  update();
}

void GLViewportWidget::startViewportCapture()
{
  if (!m_viewport.isValid())
    return;

  FABRIC_CATCH_BEGIN();

  // get the parent widget.
  QWidget *parent = parentWidget();
  if (parent == NULL)
  {
    printf("[Viewport Capture] Error: the viewport has no parent widget\n");
    return;
  }

  // get the log widget.
  DFG::DFGLogWidget *log = (DFG::DFGLogWidget *)parent->findChild<QWidget *>("DFGLogWidget");
  if (log == NULL)
  {
    printf("[Viewport Capture] Error: unable to find log widget\n");
    return;
  }

  // get the timeline widget.
  TimeLine::TimeLineWidget *timeline = (TimeLine::TimeLineWidget *)parent->findChild<QWidget *>("DFGTimelineWidget");
  if (timeline == NULL)
  {
    log->logError("[Viewport Capture] Error: unable to find timeline widget");
    return;
  }

  // stop playback.
  if (timeline->isPlaying())
    timeline->pause();

  // memorize the timeline's current state.
  int   timelineMemFrameStart   = timeline->getRangeStart();
  int   timelineMemFrameEnd     = timeline->getRangeEnd();
  int   timelineMemCurrentFrame = timeline->getTime();
  float timelineMemFramerate    = timeline->framerate();
  int   timelineMemLoopMode     = timeline->loopMode();

  // get the capture parameters.
  Context context = FabricApplicationStates::GetAppStates()->getContext();
  int      captureResX         = this->width();
  int      captureResY         = this->height();

  FabricCore::RTVal viewportParams = m_drawContext.invokeMethod( "getCurrentViewportParams", 0, NULL );
  assert( viewportParams.hasType( "RTVal[String]" ) );
  FabricCore::RTVal captureParams = viewportParams.getDictElement(
    FabricCore::RTVal::ConstructString( m_drawContext.getContext(), "capture" ) ).getUnwrappedRTVal();
  assert( captureParams.hasType( "RTVal[String]" ) );

  FabricCore::RTVal capturePathRT = captureParams.getDictElement(
    FabricCore::RTVal::ConstructString( m_drawContext.getContext(), "path" ) ).getUnwrappedRTVal();
  assert( capturePathRT.hasType( "String" ) );
  QString capturePath = capturePathRT.getStringCString();

  FabricCore::RTVal captureFilenameRT = captureParams.getDictElement(
    FabricCore::RTVal::ConstructString( m_drawContext.getContext(), "filename" ) ).getUnwrappedRTVal();
  assert( captureFilenameRT.hasType( "String" ) );
  QString captureFilename = captureFilenameRT.getStringCString();

  FabricCore::RTVal captureFramePaddingRT = captureParams.getDictElement(
    FabricCore::RTVal::ConstructString( m_drawContext.getContext(), "framePadding" ) ).getUnwrappedRTVal();
  assert( captureFramePaddingRT.hasType( "UInt32" ) );
  uint32_t captureFramePadding = captureFramePaddingRT.getUInt32();

  int      captureFrameStart   = timelineMemFrameStart;
  int      captureFrameEnd     = timelineMemFrameEnd;

  // if the capture path is empty we default it to the Fabric user dir.
  if (capturePath.isEmpty())
    capturePath = QString(FabricCore::GetFabricUserDir()) + "/Captures";

  // capture sequence dialog.
  GLViewportCaptureSequenceDialog dialog(this, "Canvas Viewport Capture");
  dialog.setCaptureResX        (captureResX);
  dialog.setCaptureResY        (captureResY);
  dialog.setCapturePath        (capturePath);
  dialog.setCaptureFilename    (captureFilename);
  dialog.setCaptureFramePadding(captureFramePadding);
  dialog.setCaptureFrameStart  (captureFrameStart);
  dialog.setCaptureFrameEnd    (captureFrameEnd);
  if (dialog.exec() != QDialog::Accepted)
    return;
  captureResX         = dialog.captureResX();
  captureResY         = dialog.captureResY();
  capturePath         = dialog.capturePath();
  captureFilename     = dialog.captureFilename();
  captureFramePadding = dialog.captureFramePadding();
  captureFrameStart   = dialog.captureFrameStart();
  captureFrameEnd     = dialog.captureFrameEnd();

  // expand the environment variables in capturePath.
  {
    RTVal rtValCapturePath      = RTVal::ConstructString(context, capturePath.toUtf8().data());
    RTVal rtValOriginalFilePath = RTVal::Construct(context, "FilePath", 1, &rtValCapturePath);
    RTVal rtValExpandedFilePath = rtValOriginalFilePath.callMethod("FilePath", "expandEnvVars", 0, 0);
    capturePath = rtValExpandedFilePath.callMethod("String", "string", 0, 0).getStringCString();
  }

  // ensure the path exists (i.e. create folders if necessary).
  if (!QDir(capturePath).exists() && !QDir().mkpath(capturePath))
    log->logWarning("[Viewport Capture] Warning: output folder might not exist");

  // create and init the progress dialog.
  QProgressDialog progressDialog("Capturing Viewport ...", "Abort Capture", captureFrameStart, captureFrameEnd, parent);
  progressDialog.setWindowModality(Qt::ApplicationModal);
  progressDialog.setMinimumWidth(qMax(250, this->width() / 2));
  progressDialog.setAutoClose(false);

  // capture.
  timeline->setTimeRange(captureFrameStart, captureFrameEnd);
  timeline->setLoopMode(LOOP_MODE_PLAY_ONCE);
  timeline->setFrameRate(1000);
  timeline->goToEndFrame();
  progressDialog.show();
  for (int frame=captureFrameStart;frame<=captureFrameEnd;frame++)
  {
    // evalute frame.
    progressDialog.setValue(frame);
    timeline->updateTime(frame);
    QApplication::processEvents();
    if (progressDialog.wasCanceled())
      break;

    // create the output filepath.
    char paddedFrame[64];
    sprintf(paddedFrame, "%0*d", captureFramePadding, frame);
    QString filepath = QDir(capturePath + "/" + captureFilename + paddedFrame + ".png").absolutePath();
    log->logInfo(QString("[Viewport Capture] saving viewport as \"" + filepath + "\"").toUtf8().data());

    // grab the viewport.
    QImage image = grabFrameBuffer(false /* withAlpha */);
    if (image.isNull() || image.width() == 0 || image.height() == 0)
    {
      log->logError("[Viewport Capture] Error: grabFrameBuffer() failed");
      break;
    }
   
    // save image.
    if (!image.save(filepath))
    {
      log->logWarning("[Viewport Capture] Warning: write error ... trying again in 0.5 seconds");
      // [Mootz]
      // saving the image failed, but instead of aborting here
      // we wait for half a second and try again as sometimes
      // people are using a flipbbok to watch the images while
      // they are being written (e.g. I like doing that).
      const uint32_t ms = 500;
      #ifdef Q_OS_WIN
        Sleep(ms);
      #else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
      #endif
      if (!image.save(filepath))
      {
        log->logInfo(QString("[Viewport Capture] Error: failed to write \"" + filepath + "\"").toUtf8().data());
        break;
      }
    }

  }

  // close the progress dialog and restore the timeline.
  progressDialog.close();
  timeline->setTimeRange(timelineMemFrameStart, timelineMemFrameEnd);
  timeline->setLoopMode (timelineMemLoopMode);
  timeline->setFrameRate(timelineMemFramerate);
  timeline->updateTime  (timelineMemCurrentFrame);

  FABRIC_CATCH_END("GLViewportWidget::startViewportCapture");
}

void GLViewportWidget::saveViewportAs()
{
  if (!m_viewport.isValid())
  {
    printf("Error: viewport not valid\n");
    return;
  }

  FABRIC_CATCH_BEGIN();

  QImage image = grabFrameBuffer(false /* withAlpha */);
  if (image.isNull())
  {
    printf("Error: grabFrameBuffer() failed\n");
    return;
  }

  static QString filepath = "capture.png";
  filepath = QFileDialog::getSaveFileName(this, "Save Capture As", filepath, "PNG Image (*.png)");
  if (!filepath.isEmpty())
  {
    filepath = QFileInfo(filepath).absoluteFilePath();
    if (!image.save(filepath))
    {
      QString msg = "Failed to save image as\n\"" + filepath + "\"";
      QMessageBox::warning(this, "Canvas Viewport Capture", msg);
      return;
    }
  }

  FABRIC_CATCH_END("GLViewportWidget::saveViewportAs");
}
