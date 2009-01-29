/****************************************************************************
* RTIViewer                                                         o o     *
* Single and Multi-View Reflectance Transformation Image Viewer   o     o   *
*                                                                _   O  _   *
* Copyright(C) 2008                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
****************************************************************************/


#ifndef RTI_H
#define RTI_H

#include "renderingmode.h"
#include "util.h"

#include <vcg/space/point3.h>

#include <QTextStream>
#include <QRect>
#include <QBuffer>

//! RTI image abstract class.
/*!
  Abstract class to manage the RTI image.
*/
class Rti
{

// private data member 
protected:

	int h; /*!< Height. */
	int w; /*!< Width. */
	QString filename; /*!< Filename. */
	QString type; /*!< Type of image. */

	int currentRendering; /*!< Current rendering mode applied to the image. */
	QVector<RenderingMode*>* list; /*!< List of rendering modes to apply to the image. */

	bool remote; /*!< Holds whether the image is loaded by remote disk. */
	int maxRemoteResolution; /*!< Maximun level of resolution if the image is remote. */
	int minRemoteResolution; /*!< Minimum level of resolution if the image is remote. */

	unsigned int* tiles; /*!< Info about the tiles loaded from the remote server. */


//public method
public:

	//! Constructor.
	Rti():
		maxRemoteResolution(0),
		minRemoteResolution(0),
		tiles(NULL)
	{ };


	//! Deconstructor.
	virtual ~Rti()
	{
		for (int i = 0; i < list->size(); i++)
			delete list->at(i);
		delete list;
		if (tiles)
			delete tiles;
	};


	/*!
	  Loads the image from the disk.
	  \param cb callback to update the progress bar.
	  \return returns 0 if the image was successfully loaded, returns -1 otherwise.
	*/
	virtual int load(CallBackPos * cb = 0) = 0;
	
	
	/*!
	  Loads the image from the disk.
	  \param str path to file.
	  \param cb callback to update the progress bar.
	  \return returns 0 if the image was successfully loaded, returns -1 otherwise.
	*/
	virtual int load(QString str, CallBackPos * cb = 0) = 0;

	
	/*!
	  Saves the image.
	  \param str path to file.
	  \return returns 0 if the image was successfully loaded, returns -1 otherwise.
	*/
	virtual int save(QString str) = 0;

	/*!
	  Creates the texture to display in the browser.
	  \param buffer pointer to the buffer of the texture.
	  \param width reference for the width of the output texture.
	  \param height reference for the height of the output texture.
	  \param light light vector.
	  \param sub sub-image displayed in the browser.
	  \param level mip-mapping level to use.
	  \param mode special rendering mode to apply.
	  \return returns 0.
	*/
	virtual int createImage(unsigned char** buffer, int& width, int& height, const vcg::Point3f& light, const QRectF& sub, int level = 0, int mode = 0) = 0;
	

	/*!
	  Creates a preview of the image.
	  \param width waiting width of the image.
	  \param height waiting height of the image.
	  \return pointer to the preview.
	*/
	virtual QImage* createPreview(int width, int height) = 0;


	/*!
	  Allocates the data for a remote RTI image.
	  \param width width of the image.
	  \param height height of the image.
	  \param maxResLevel maximum level of resolution.
	  \return returns 0.
	*/
	virtual int allocateRemoteImage(int width, int height, int maxResLevel) = 0;
	
	
	/*!
	  Loads the data of a tile.
	  \param b pointer to the buffer with the tile data.
	  \param xinf, yinf, xsup, ysup sub-image coordinates of the tile.
	  \param level mip-mapping level.
	  \return returns 0.
	*/
	virtual int loadCompressedHttp(QBuffer* b, int xinf, int yinf, int xsup, int ysup, int level) = 0; 


public:

	/*!
		Returns the height of the image.
	*/
	int height(){return h;}

	/*!
	  Sets the height of the image.
	*/
	void setHeight(int height){h = height;}

	/*!
	  Returns the width of the image.
	*/
	int width(){return w;}

	/*!
	  Sets the width of the image.
	*/
	void setWidth(int width){w = width;}

	/*!
	  Sets the file path of the image.
	*/
	void setFileName(QString s) {filename = s;}

	/*!
	  Sets the rendering mode to apply to the image.
	*/
	void setRenderingMode(int a) {currentRendering = a;}

	/*!
	  Returns the index of the current rendering mode applied to the image.
	*/
	int getCurrentRendering() {return currentRendering;}

	/*!
	  Retruns the list of rendering modes supported by the image.
	*/
	QVector<RenderingMode*>* getSupportedRendering() {return list;}

	/*!
	  Returns the maximum level of resolution for a remote RTI image.
	*/
	int maxResolution(){return maxRemoteResolution;}

	/*!
	  Returns the minimum level of resolution for a remote RTI image.
	*/
	int minResolution(){return minRemoteResolution;}


	/*!
	  Checks if the tile is already received.
	  \param zindex tile index.
	  \param level mip-mapping level.
	*/
	bool isTilePresent(int zindex, int level)
	{
		if (tiles)
			return tiles[zindex] & (1 << level);
		else
			return false;
	}


	/*!
	  Sets the tile as received.
	  \param zindex tile index.
	  \param level mip-mapping level.
	*/
	void setTileLevel(int zindex, int level)
	{
		if (tiles)
			tiles[zindex] |= (1 << level);
	}

	/*!
	  Return the type of the image.
	*/
	QString typeFormat(){return type;}

	/*!
	  Reset the flag for the remote RTI image.
	  The method must be invoked when all tiles are received. 
	*/
	void resetRemote(){remote = false;}
};


#endif  /* RTI_H */
