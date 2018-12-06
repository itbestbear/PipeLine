#include "RenderingPipeline.h"

#include <QPalette>
#include <QPainter>

RenderingPipeline::RenderingPipeline(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	// 设置背景纯黑色
	QPalette pal = palette();
	pal.setColor(QPalette::Background, Qt::black);
	setAutoFillBackground(true);
	setPalette(pal);

	// 申请纹理空间
	m_textureRed = new int*[PIC_HEIGHT];
	m_textureGreen = new int*[PIC_HEIGHT];
	m_textureBlue = new int*[PIC_HEIGHT];
	for (int i = 0; i < PIC_HEIGHT; ++i) {
		m_textureRed[i] = new int[PIC_WIDTH];
		m_textureGreen[i] = new int[PIC_WIDTH];
		m_textureBlue[i] = new int[PIC_WIDTH];
	}

	// 申请深度缓存空间
	m_depthBuffer = new double*[WINDOW_HEIGHT];
	for (int i = 0; i < WINDOW_HEIGHT; ++i) {
		m_depthBuffer[i] = new double[WINDOW_WIDTH];
	}
	updateDepthBuffer();

	/******************************这里设置纹理、光线、局部中心等基本数据******************************/
	Vector4 localPos(0, 0, 0);
	m_initCube.centerPos = localPos;   // 立方体的中心

	m_lightDir.X(0);
	m_lightDir.Y(-0.3);
	m_lightDir.Z(-1);
	/*************************************************************************************************/

	/****************************这里设置需要绘制的物体****************************/
	// 注意图形必须是封闭的，起点必须首位相接
	int L = 5;
	Plane plane1;
	plane1.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane1.emplace_back(Point(L, L, L, 0, 255, 0, 0, PIC_HEIGHT));
	plane1.emplace_back(Point(L, -L, L, 0, 0, 255, 0, PIC_HEIGHT));
	plane1.emplace_back(Point(-L, -L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane1.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));

	Plane plane2;
	plane2.emplace_back(Point(L, L, L, 255, 0, 0,0, PIC_HEIGHT));
	plane2.emplace_back(Point(L, L, -L, 0, 255, 0, 0, PIC_HEIGHT));
	plane2.emplace_back(Point(L, -L, -L, 0, 0, 255, 0, PIC_HEIGHT));
	plane2.emplace_back(Point(L, -L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane2.emplace_back(Point(L, L, L, 255, 0, 0, 0, PIC_HEIGHT));

	Plane plane3;
	plane3.emplace_back(Point(-L, L, -L, 255, 0, 0, 0, PIC_HEIGHT));
	plane3.emplace_back(Point(L, L, -L, 0, 255, 0, 0, PIC_HEIGHT));
	plane3.emplace_back(Point(L, -L, -L, 0, 0, 255, 0, PIC_HEIGHT));
	plane3.emplace_back(Point(-L, -L, -L, 255, 0, 0, 0, PIC_HEIGHT));
	plane3.emplace_back(Point(-L, L, -L, 255, 0, 0, 0, PIC_HEIGHT));

	Plane plane4;
	plane4.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane4.emplace_back(Point(-L, -L, L, 0, 255, 0, 0, PIC_HEIGHT));
	plane4.emplace_back(Point(-L, -L, -L, 0, 0, 255, 0, PIC_HEIGHT));
	plane4.emplace_back(Point(-L, L, -L, 255, 0, 0, 0, PIC_HEIGHT));
	plane4.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));

	Plane plane5;
	plane5.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane5.emplace_back(Point(L, L, L, 0, 255, 0, 0, PIC_HEIGHT));
	plane5.emplace_back(Point(L, L, -L, 0, 0, 255, 0, PIC_HEIGHT));
	plane5.emplace_back(Point(-L, L, -L, 255, 0, 0, 0, PIC_HEIGHT));
	plane5.emplace_back(Point(-L, L, L, 255, 0, 0, 0, PIC_HEIGHT));

	Plane plane6;
	plane6.emplace_back(Point(-L, -L, L, 255, 0, 0, 0, PIC_HEIGHT));
	plane6.emplace_back(Point(L, -L, L, 0, 255, 0, 0, PIC_HEIGHT));
	plane6.emplace_back(Point(L, -L, -L, 0, 0, 255, 0, PIC_HEIGHT));
	plane6.emplace_back(Point(-L, -L, -L, 255, 0, 0, 0, PIC_HEIGHT));
	plane6.emplace_back(Point(-L, -L, L, 255, 0, 0, 0, PIC_HEIGHT));

	// 构造立方体的平面
	m_initCube.planes.push_back(plane1);
	m_initCube.planes.push_back(plane2);
	m_initCube.planes.push_back(plane3);
	m_initCube.planes.push_back(plane4);
	m_initCube.planes.push_back(plane5);
	m_initCube.planes.push_back(plane6);

	for (auto& plane : m_initCube.planes) {
		for (auto& p : plane) {
			p.r = p.g = p.b = 100;
		}
	}
	/********************************************************************************/


	/******************************这里设置相机参数*********************************/
	m_camPos.X(10);
	m_camPos.Y(16);
	m_camPos.Z(20);
	m_camTarget.X(0);
	m_camTarget.Y(0);
	m_camTarget.Z(0);
	m_UpVec.X(0);
	m_UpVec.Y(1);
	m_UpVec.Z(0);
	/*******************************************************************************/


	m_tranformer = new Transformer;

	m_camera = new Camera;
	m_camera->setCamera(m_camPos, m_camTarget, m_UpVec);
	m_camera->setLightDir(m_lightDir);

	m_drawPolygon = new DrawPolygon;
	m_drawPolygon->setTexture(m_textureRed, m_textureGreen, m_textureBlue);
	m_drawPolygon->setDepth(m_depthBuffer);


	m_perspectiveProjector = new PerspectiveProjector;
	m_perspectiveProjector->setProjectMat();  // 使用默认投影

	repaint();
}

RenderingPipeline::~RenderingPipeline() {
	if (m_camera != nullptr) {
		delete m_camera;
		m_camera = nullptr;
	}
	if (m_tranformer != nullptr) {
		delete m_tranformer;
	}
	if (m_drawPolygon != nullptr) {
		delete m_drawPolygon;
	}

	for (int i = 0; i < PIC_HEIGHT; ++i) {
		delete[] m_textureRed[i];
		delete[] m_textureGreen[i];
		delete[] m_textureBlue[i];
	}
	for (int i = 0; i < WINDOW_HEIGHT; ++i) {
		delete[] m_depthBuffer[i];
	}
}

void RenderingPipeline::updateDepthBuffer() {
	for (int i = 0; i < WINDOW_HEIGHT; ++i) {
		for (int j = 0; j < WINDOW_WIDTH; ++j) {
			m_depthBuffer[i][j] = INF;
		}
	}
}

void RenderingPipeline::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	updateDepthBuffer();
	if (m_showFrame) {
		m_camera->setLightMode(0);
		m_camera->transform(m_initCube, m_finalCube);
		m_perspectiveProjector->project(m_finalCube, WINDOW_WIDTH, WINDOW_HEIGHT);
		m_drawPolygon->drawFrame(this, &painter, m_finalCube);
	}

	if (m_showFill) {
		auto tmp = m_initCube;
		m_camera->setLightMode(1);
		m_camera->transform(m_initCube, m_finalCube);
		m_perspectiveProjector->project(m_finalCube, WINDOW_WIDTH, WINDOW_HEIGHT);
		m_drawPolygon->setMode(0);
		m_drawPolygon->drawFill(this, &painter, m_finalCube);
	}

	if (m_showTexture) {
		m_camera->setLightMode(1);
		m_camera->transform(m_initCube, m_finalCube);
		m_perspectiveProjector->project(m_finalCube, WINDOW_WIDTH, WINDOW_HEIGHT);
		m_drawPolygon->setMode(1);
		m_drawPolygon->drawFill(this, &painter, m_finalCube);
	}
}

void RenderingPipeline::keyPressEvent(QKeyEvent * event) {
	double x = 1;
	double y = 1;
	double z = 1;
	double angle_10 = 10 * PI / 180;
	Vector4 offsetVec(0, 0, 0);
	switch (event->key())
	{

		/******************************以下是移动物体的操作******************************/
	case Qt::Key_Q:  // 物体向上移动
		offsetVec.Y(y);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_W:  // 物体向下移动
		offsetVec.Y(-y);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_E:  // 物体向左移动
		offsetVec.X(-x);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_R:  // 物体向右移动
		offsetVec.X(x);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_T:  // 物体向前移动
		offsetVec.Z(z);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_Y:  // 物体向后移动
		offsetVec.Z(-z);
		m_tranformer->transformOffset(m_initCube, offsetVec);
		repaint();   // 立刻重绘
		break;

		/******************************以下是旋转物体的操作******************************/
	case Qt::Key_A:
		m_tranformer->transformRotationX(m_initCube, angle_10);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_S:
		m_tranformer->transformRotationX(m_initCube, -angle_10);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_D:
		m_tranformer->transformRotationY(m_initCube, angle_10);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_F:
		m_tranformer->transformRotationY(m_initCube, -angle_10);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_G:
		m_tranformer->transformRotationZ(m_initCube, angle_10);
		repaint();   // 立刻重绘
		break;
	case Qt::Key_H:
		m_tranformer->transformRotationZ(m_initCube, -angle_10);
		repaint();   // 立刻重绘
		break;


		/******************************以下是移动相机物体的操作******************************/

	default:
		break;
	}
}
