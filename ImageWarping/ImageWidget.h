#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);

protected:
	void paintEvent(QPaintEvent* paintevent);

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Restore();												// Restore image to origin
	void AddPointPairs();
	void IDWMethod();
	void RBFMethod();

public:
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);

private:
	void ClearPointXY();
	QImage* ptr_image_;				// image 
	QImage* ptr_image_backup_;
	std::vector<QPoint> PointX;
	std::vector<QPoint> PointY;
	bool execAddPoints;
	bool isNewPointIn;
	size_t numOfPointsOnImage;
};

