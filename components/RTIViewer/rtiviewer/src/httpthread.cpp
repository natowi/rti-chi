/****************************************************************************
* RTIViewer                                                         o o     *
* Single and Multi-View Reflectance Transformation Image Viewer   o     o   *
*                                                                _   O  _   *
* Copyright(C) 2008                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
****************************************************************************/


#include "httpthread.h"
#include "zorder.h"

#include <QMessageBox>


HttpWorker::HttpWorker(QMutex& m, QWaitCondition& w, QObject* parent): QObject(parent),
	http(NULL),
	image(NULL),
	mutex(m),
	infoReady(w),
	httpError(false),
	url(NULL),
	syncRequest(false),
	idReq(-1),
	received(false),
	aborted(false)
{
	// Sets the timer to send the tile requests.
	timer = new QTimer(this);
	timer->setInterval(200);
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), this, SLOT(sendRequestTiles()));
}


HttpWorker::~HttpWorker()
{
	if (url)
		delete url;
	if (http)
		delete http;
	if (timer) 
		delete timer;
	if (current)
		delete current;
	if (next)
		delete next;
}


bool HttpWorker::errorState()
{
	return httpError;
}


void HttpWorker::httpRequestFinished(int id, bool error)
{
	if (aborted) return;
	else if (id == idReq)
	{
		if (syncRequest)
		{
			// The request is synchronized with the GUI thread.
			if (error)
			{
				httpError = true;
				emit errorOccurred(tr("Connection failed:\n%1.").arg(http->errorString()));
			}
			mutex.lock();
			infoReady.wakeAll();
			mutex.unlock();
		}
		else
		{
			// The request is asynchronous
			current->nTry++;
			if (httpError || error)
			{
				// A error occurred
				if(current->nTry == 3)
				{
					// If the last three request failed, the thread send the next request. 
					if (next)
					{
						int row = next->row;
						int col = next->col;
						int temp = current->level;
						int level = next->level;
						delete current;
						current = next;
						if (nextTile(row, col, level))
						{
							next = new TileRequest(row, col, ZOrder::ZIndex(row, col, level), level);
							if (level > temp || level == image->minResolution())
								emit imageUpdated();
						}
						else
						{
							next = NULL;
							emit imageUpdated();
						}
					}
					else
					{
						// The download finisched.
						emit imageUpdated();
						image = NULL;
						timer->stop();
						emit downloadFinished();
					}
				}
			}
			else
			{
				// Loads the data of the tile and send the next request.
				int size = 1 << current->level;
				float deltaH = static_cast<float>(image->height()) / static_cast<float>(size);
				float deltaW = static_cast<float>(image->width()) / static_cast<float>(size);
				int x1 = static_cast<int>(deltaW * current->col);
				int x2 = static_cast<int>(deltaW * (current->col + 1)); 
				int y1 = static_cast<int>(deltaH * current->row);
				int y2 = static_cast<int>(deltaH * (current->row + 1)); 
				current->buffer->close();
				image->loadCompressedHttp(current->buffer, x1, y1, x2, y2, image->maxResolution() - current->level); 
				int n = 1 << ((image->maxResolution() - current->level)*2);
				int limit = (current->zIndex + 1) * n;
				int i = current->zIndex * n;
				while (i < limit)
					image->setTileLevel(i++, image->maxResolution() - current->level);
				if (next)
				{
					int row = next->row;
					int col = next->col;
					int temp = current->level;
					int level = next->level;
					delete current;
					current = next;
					if (nextTile(row, col, level))
					{
						next = new TileRequest(row, col, ZOrder::ZIndex(row, col, level), level);
						if (level > temp || level == image->minResolution())
							emit imageUpdated();
					}
					else
					{
						next = NULL;
						emit imageUpdated();
					}
				}
				else
				{
					//The download finisched.
					emit imageUpdated();
					image = NULL;
					timer->stop();
					emit downloadFinished();
				}
			}
			received = true;

		}
	}
	
}


void HttpWorker::readResponseHeader(const QHttpResponseHeader & responseHeader)
{
	if (responseHeader.statusCode() != 200)
	{
		httpError = true;
		if (syncRequest)
			emit errorOccurred(tr("Connection failed:\n%1 %2.").arg(responseHeader.statusCode()).arg(responseHeader.reasonPhrase()));
	}
}


void HttpWorker::sendRequestTiles()
{
	syncRequest = false;
	if (received)
	{
		received = false;
		QString tilename = tr("./tile_lvl%1_%2.ptm").arg(current->level).arg(current->zIndex);
		QUrl relativeUrl(tilename);
		idReq = http->get(url->resolved(relativeUrl).path(), current->buffer);
	}
}

void HttpWorker::setUrl(QUrl* u)
{
	if (u)
	{
		aborted = false;
		syncRequest = true;
		if (url)
			delete url;
		url = u;
		if (http)
			delete http;
		http = new QHttp(this);
		connect(http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
		connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(readResponseHeader(const QHttpResponseHeader &)));
		http->setHost(url->host());
		if(!url->userName().isEmpty())
			http->setUser(url->userName(), url->password());
	}
}


void HttpWorker::setRti(Rti *rti)
{
	image = rti;
}


void HttpWorker::requestInfo(QBuffer* b)
{
	if (url)
	{
		syncRequest = true;
		QUrl relativeUrl("./info.xml");
		idReq = http->get(url->resolved(relativeUrl).path(), b);
	}

}


void HttpWorker::requestThumb(QBuffer* b)
{
	if (url)
	{
		syncRequest = true;
		QUrl relativeUrl("./thumb.jpg");
		idReq = http->get(url->resolved(relativeUrl).path(), b);
	}
}


void HttpWorker::abort()
{
	if (image)
	{
		timer->stop();
		aborted = true;
		http->abort();
	}
}


void HttpWorker::startRequestTiles()
{
	if (image)
	{
		int level = image->minResolution();
		startRow = 0;
		startCol = 0;
		endRow = (1 << level) - 1;
		endCol = (1 << level) - 1;
		current = new TileRequest(startRow, startCol, ZOrder::ZIndex(startRow, startCol, level), level);
		next = new TileRequest(startRow, startCol + 1, ZOrder::ZIndex(startRow, startCol + 1, level), level);
		timer->start();
		subImg = QRect(0, 0, image->width(), image->height());
		received = true;
	}

}


void HttpWorker::updateView(QRectF r)
{
	if (image)
	{
		subImg = r.toAlignedRect();
		if (next)
			delete next;
		int level = image->minResolution();
		int row, col;
		firstTile(row, col, 0, level);
		int zIndex = ZOrder::ZIndex(row, col, level);
		zIndex = zIndex * (1 << (image->maxResolution() - level)*2);
		if (image->isTilePresent(zIndex, image->maxResolution() - level))
		{
			if (nextTile(row, col, level))
			{
				if (row == current->row && col == current->row && level == current->level)
				{	
					if(nextTile(row, col, level))
						next = new TileRequest(row, col, ZOrder::ZIndex(row, col, level), level);
					else
						next = NULL;
				}
				else
					next = new TileRequest(row, col, ZOrder::ZIndex(row, col, level), level);
			}
			else
				next = NULL;
		}
		else
			next = new TileRequest(row, col, ZOrder::ZIndex(row, col, level), level);
	}

}


bool HttpWorker::nextTile(int& row, int& col, int& level)
{
	int i = 0;
	int max = image->maxResolution();
	int limit = (1 << max) - 1;
	bool found = false;
	while(!found)
	{
		if (col == limit && row == limit && startCol == 0 && startRow == 0) 
			break;
		if (col == endCol && row == endRow && level == max)
		{
			i++;
			level = image->minResolution();
			firstTile(row, col, i, level);
		}
		else if (col == endCol && row == endRow)
		{
			level++;
			firstTile(row, col, i, level);
		}
		else if (col == endCol && row < endRow)
		{
			col = startCol;
			row++;
		}
		else if (col < endCol)
			col++;
		int zIndex = ZOrder::ZIndex(row, col, level);
		zIndex = zIndex * (1 << (max - level)*2);
		if (!image->isTilePresent(zIndex, max - level))
			found = true;
	}
	return found;
}

void HttpWorker::firstTile(int& row, int& col, int i, int level)
{
	int size = 1 << level;
	float deltaH = static_cast<float>(image->height()) / static_cast<float>(size);
	float deltaW = static_cast<float>(image->width()) / static_cast<float>(size);
	startRow = subImg.y() / deltaH;
	startRow = startRow > i? startRow - i: 0;
	startCol = subImg.x() / deltaW;
	startCol = startCol > i? startCol - i: 0;
	endRow = subImg.bottom() / deltaH;
	endRow = endRow < size - i ? endRow + i: size - 1;
	endCol = subImg.right() / deltaW;
	endCol = endCol < size - i ? endCol + i: size - 1;
	row = startRow;
	col = startCol;
}


HttpThread::HttpThread(QMutex& m, QWaitCondition& w):
	QThread()
{
	worker = new HttpWorker(m, w);
	worker->moveToThread(this);
	connect(this, SIGNAL(urlChanged(QUrl*)), worker, SLOT(setUrl(QUrl*)), Qt::QueuedConnection);
	connect(this, SIGNAL(rtiChanged(Rti*)), worker, SLOT(setRti(Rti*)), Qt::QueuedConnection);
	connect(this, SIGNAL(sendRequestInfo(QBuffer*)), worker, SLOT(requestInfo(QBuffer*)), Qt::QueuedConnection);
	connect(this, SIGNAL(sendRequestThumb(QBuffer*)), worker, SLOT(requestThumb(QBuffer*)), Qt::QueuedConnection);
	connect(this, SIGNAL(sendRequestTiles()), worker, SLOT(startRequestTiles()), Qt::QueuedConnection);
	connect(this, SIGNAL(abort()), worker, SLOT(abort()), Qt::QueuedConnection);
	connect(worker, SIGNAL(errorOccurred(QString)), this, SIGNAL(errorOccurred(QString)));
	connect(worker, SIGNAL(imageUpdated()), this, SIGNAL(imageUpdated()));
	connect(worker, SIGNAL(downloadFinished()), this, SIGNAL(downloadFinished()));
	connect(this, SIGNAL(viewUpdated(QRectF)), worker, SLOT(updateView(QRectF)), Qt::QueuedConnection);
}

void HttpThread::run()
{
	exec();
}
