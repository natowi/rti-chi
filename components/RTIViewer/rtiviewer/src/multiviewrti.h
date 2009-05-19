/****************************************************************************
* RTIViewer                                                         o o     *
* Single and Multi-View Reflectance Transformation Image Viewer   o     o   *
*                                                                _   O  _   *
* Copyright(C) 2008                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
****************************************************************************/

#ifndef MULTIVIEWRTI_H
#define MULTIVIEWRTI_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#include <cmath>
#endif

// Local headers
#include "rti.h"
#include "renderingmode.h"
#include "universalrti.h" 

// Qt headers
#include <QFile>
#include <QImage>
#include <QVector>
#include <QSlider>
#include <QCheckBox>


#include <vcg/math/matrix.h>



//! Widget to change the viewpoint.
/*!
  The class defines the widget to change the viewpoint of the RTI.
*/
class ViewpointControl: public QWidget
{
	Q_OBJECT

private:
		
	QSlider* viewpointSlider; /*!< Slider to set the viewpoint. */
	QCheckBox* snapNearest; /*!< Checkbox to anable the use of the optical flow data. */
	int maxViewX; /*!< Max number of view on X-axis. */
	bool useFlow;

	QSlider* vertical;
public:

	//! Constructor
	/*!
	  \param initValueX value for the x-position.
	  \param nViewX max number of view on x-axix.
	  \param enableFlow enable the use of optical flow.
	  \param useFlow state of the optical flow data.
	  \param parent
	*/
	ViewpointControl(int initValueX, int nViewX, bool enableFlow, bool useFlow, QWidget *parent = 0);

signals:

	/*!
	  Emitted when the user changes the viewpoint.
	*/
	void viewpointChanged(int value);

	/*!
	  Emitted when the user changes the use of the optical flow.
	*/
	void snapModeChanged(int value);

private slots:

	/*!
	  Update the viewpoint position.
	*/
	void updateSlider(int value)
	{
		useFlow = (value != Qt::Checked);
		if (!useFlow)
		{
			viewpointSlider->setPageStep(100);
			viewpointSlider->setSingleStep(100);
			int rest = viewpointSlider->value() % 100;
			int div = viewpointSlider->value() / 100;
			if (rest < 50)
				viewpointSlider->setValue(div*100);
			else
				viewpointSlider->setValue((div + 1)*100);
		}
		else
		{
			viewpointSlider->setPageStep(10);
			viewpointSlider->setSingleStep(5);
		}
	}


	void sliderReleased()
	{
		if (!useFlow)
		{
			int rest = viewpointSlider->value() % 100;
			int div = viewpointSlider->value() / 100;
			if (rest < 50)
				viewpointSlider->setValue(div*100);
			else
				viewpointSlider->setValue((div + 1)*100);
		}
	}
};


//! Defaut Rending for Multiview RTI image.
/*!
  The class defines the default rendering for Multiview RTI image.
*/
class DefaultMRti : public QObject, public RenderingMode
{
	Q_OBJECT

private:
	float currentPosX, currentPosY;
	int maxViewX, maxViewY;
	bool useFlow;
	bool enableFlow;

public:

	DefaultMRti(float posX, float posY, int nViewX, int nViewY, bool enable, bool use) :
	  currentPosX(posX - 1),
	  currentPosY(posY - 1),
	  maxViewX(nViewX),
	  maxViewY(nViewY),
	  enableFlow(enable),
	  useFlow(use)
	  {}
	
	QString getTitle() {return "Default";}

	QWidget* getControl(QWidget* parent)
	{
		ViewpointControl* control = new ViewpointControl(currentPosX, maxViewX, enableFlow, useFlow, parent) ;
		connect(control, SIGNAL(viewpointChanged(int)), this, SLOT(setPosX(int)));
		disconnect(this, SIGNAL(refreshImage()), 0, 0);
		connect(this, SIGNAL(refreshImage()), parent, SIGNAL(updateImage()));
		return control;
	}

	bool isLightInteractive() {return true;}

	bool supportRemoteView()  {return true;}
	
	bool enabledLighting() {return true;}
	
	void applyPtmLRGB(const PyramidCoeff& coeff, const PyramidRGB& rgb, const QSize* mipMapSize, const PyramidNormals& normals, const RenderingInfo& info, unsigned char* buffer)
	{
		
	}


	void applyPtmRGB(const PyramidCoeff& redCoeff, const PyramidCoeff& greenCoeff, const PyramidCoeff& blueCoeff, const QSize* mipMapSize, const PyramidNormals& normals, const RenderingInfo& info, unsigned char* buffer)
	{

	}


	void applyHSH(const PyramidCoeffF& redCoeff, const PyramidCoeffF& greenCoeff, const PyramidCoeffF& blueCoeff, const QSize* mipMapSize, const PyramidNormals& normals, const RenderingInfo& info, unsigned char* buffer)
	{
	
	}

	float getCurrentPosX() {return currentPosX;}
	float getCurrentPosY() {return currentPosY;}
	
public slots:

	/*!
	  Sets the x-position of the viewpoint.
	*/
	void setPosX(int value)
	{
		if (enableFlow)
			currentPosX = float(value)/100.0;
		else
			currentPosX = value;
		emit refreshImage();
	}


	/*!
	  Sets the y-position of the viewpoint.
	*/
	void setPosY(int value)
	{
		if (enableFlow)
			currentPosY = float(value)/100.0;
		else
			currentPosY = value;
		emit refreshImage();
	}


signals:

	/*!
	  Emitted to refresh the image in the browser.
	*/
	void refreshImage();
};


struct OpticalFlowData
{
	std::vector<float>* up;
	std::vector<float>* down;
	std::vector<float>* left;
	std::vector<float>* right;
};


class ViewpointImage
{
public:
	bool valid;
	unsigned char* buffer;
	float* hFlow;
	float* vFlow;

	ViewpointImage()
	{
		valid = false;
		buffer = NULL;
		hFlow = NULL;
		vFlow = NULL;
	}
	
	~ViewpointImage()
	{
 		if (buffer)
			delete[] buffer;
		if (hFlow)
			delete[] hFlow;
		if (vFlow)
			delete[] vFlow;
	}

};


//! Multiview RTI class
class MultiviewRti : public Rti
{
// private data member
protected:
	
	QString version; /*!< Version. */
 	
	int maxViewX, maxViewY, nViewpoint;
	int startX, startY;
	float separationX, separationY;
	bool useFlow;
	vcg::ndim::Matrix<int>* viewpointLayout;
	std::vector<UniversalRti*> images;
	std::vector<OpticalFlowData> flow;

	float posX, posY;
	
	ViewpointImage leftImage, rightImage, leftUpImage, rightUpImage;


public:

	//! Constructor.
	MultiviewRti();

	//! Deconstructor.
	virtual ~MultiviewRti();

	// protected methods
protected:

	
	
public:

	virtual int load(CallBackPos * cb = 0);
	virtual int load(QString name, CallBackPos * cb = 0);
	virtual int save(QString name);
	virtual int loadCompressed();
	virtual int loadCompressed(QString name);
	virtual int loadCompressed(int xinf, int yinf, int xsup, int ysup, QString name);
	virtual int saveCompressed(QString name);
	virtual int saveCompressed(int xinf, int yinf, int xsup, int ysup, int reslevel, QString name);
	virtual int createImage(unsigned char** buffer, int& width, int& height, const vcg::Point3f& light, const QRectF& rect, int level = 0, int mode = 0);
	virtual QImage* createPreview(int width, int height);
	virtual int allocateRemoteImage(QBuffer* b);  
	virtual int loadCompressedHttp(QBuffer* b, int xinf, int yinf, int xsup, int ysup, int level); 
	virtual int loadData(FILE* file, int width, int height, int basisTerm, bool urti, CallBackPos * cb = 0, QString& text = QString());
	virtual void saveRemoteDescr(QString& filename, int level);

private:

	int loadFlowData(const QString& path, std::vector<float>** output);

	void applyOpticalFlow(const unsigned char* image, const std::vector<float>& flowData, float dist, unsigned char* outImg, float* outFlow); 

};

#endif //MULTIVIEWRTI_H
