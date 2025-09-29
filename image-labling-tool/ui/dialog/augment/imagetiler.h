#ifndef IMAGETILER_H
#define IMAGETILER_H

#include <opencv2/opencv.hpp>
#include <QString>
#include <QSize>
#include <QVector>

struct BBox {
    int cls;
    float xc, yc, w, h; // YOLO normalized
};

class ImageTiler
{
public:
    ImageTiler(const QString &imagePath, const QString &labelPath);

    void setTileSize(const QSize &size);
    void setOutputDir(const QString &dir);
    void process();

private:
    void loadLabels();
    void saveTile(const cv::Mat &tile, const QVector<BBox> &boxes,
                  int tileX, int tileY, int localIndex);

    // grouping / utils
    QVector<QVector<BBox>> groupBBoxes(const QVector<BBox> &boxes) const;
    cv::Rect unionGroup(const QVector<BBox> &group) const;
    double iou(const cv::Rect &a, const cv::Rect &b) const;

private:
    QString m_imagePath;
    QString m_labelPath;
    QString m_outputDir;
    QSize m_tileSize;
    QVector<BBox> m_boxes;
    int m_imgWidth {0};
    int m_imgHeight {0};

    double m_iouThresh {0.3}; // ngưỡng tránh tile trùng
};

#endif // IMAGETILER_H
