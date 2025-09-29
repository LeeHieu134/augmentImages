#include "ImageTiler.h"
#include <opencv2/opencv.hpp>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>
#include <algorithm>

ImageTiler::ImageTiler(const QString &imagePath, const QString &labelPath)
    : m_imagePath(imagePath), m_labelPath(labelPath)
{ }

void ImageTiler::setTileSize(const QSize &size) {
    m_tileSize = size;
}

void ImageTiler::setOutputDir(const QString &dir) {
    m_outputDir = dir;
}

void ImageTiler::loadLabels() {
    m_boxes.clear();
    QFile file(m_labelPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open label file:" << m_labelPath;
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        int cls; float cx, cy, w, h;
        in >> cls >> cx >> cy >> w >> h;
        if (in.status() == QTextStream::Ok) {
            m_boxes.push_back({cls, cx, cy, w, h});
        }
    }
}

void ImageTiler::process() {
    cv::Mat img = cv::imread(m_imagePath.toStdString());
    if (img.empty()) {
        qWarning() << "Cannot read image:" << m_imagePath;
        return;
    }
    m_imgWidth = img.cols;
    m_imgHeight = img.rows;

    loadLabels();

    int tileW = m_tileSize.width();
    int tileH = m_tileSize.height();

    // 1) Nếu tile lớn hơn ảnh => bỏ qua (không tile, không copy)
    if (tileW > m_imgWidth || tileH > m_imgHeight) {
        qDebug() << "Tile size" << tileW << "x" << tileH
                 << "bigger than image" << m_imgWidth << "x" << m_imgHeight
                 << "=> skip tiling for" << m_imagePath;
        return;
    }

    // 2) Lọc bỏ các bbox lớn hơn tile (nếu bbox rộng/ cao hơn tile thì không tile cho bbox đó)
    QVector<BBox> filtered;
    for (const auto &b : m_boxes) {
        int bw = int(b.w * m_imgWidth);
        int bh = int(b.h * m_imgHeight);
        if (bw <= tileW && bh <= tileH) {
            filtered.push_back(b);
        } else {
            qDebug() << "Skip bbox (bigger than tile):" << b.cls << "bbox_px="
                     << bw << "x" << bh;
        }
    }

    if (filtered.isEmpty()) {
        qDebug() << "No bbox fits tile size => nothing to tile for" << m_imagePath;
        return;
    }

    // 3) Gom nhóm bbox gần nhau: greedy - thêm bbox vào nhóm nếu union vẫn <= tile
    auto groups = groupBBoxes(filtered);

    QVector<cv::Rect> savedTiles;
    int localIndex = 1;

    // 4) Tạo tile cho mỗi group
    for (const auto &g : groups) {
        if (g.isEmpty()) continue;
        cv::Rect u = unionGroup(g);

        // nếu hợp nhất (union) vượt tile thì bỏ (mặc dù groupBBoxes đã cố gắng tránh, vẫn kiểm tra an toàn)
        if (u.width > tileW || u.height > tileH) {
            qDebug() << "Group union larger than tile -> skip group";
            continue;
        }

        // ROI: cắt theo sát biên ảnh (không shift tile vào trong toàn bộ)
        int roiX = u.x + u.width/2 - tileW/2;   // canh giữa group theo X
        int roiY = u.y + u.height/2 - tileH/2;  // canh giữa group theo Y

        // --- dịch tile vào trong nếu vượt biên trái/trên ---
        if (roiX < 0) roiX = 0;
        if (roiY < 0) roiY = 0;

        // --- dịch tile vào trong nếu vượt biên phải/dưới ---
        if (roiX + tileW > m_imgWidth)  roiX = m_imgWidth  - tileW;
        if (roiY + tileH > m_imgHeight) roiY = m_imgHeight - tileH;

        // --- tile luôn cố định đúng size chuẩn ---
        cv::Rect roi(roiX, roiY, tileW, tileH);


        // Tránh trùng tile (IOU cao)
        bool duplicate = false;
        for (const auto &st : savedTiles) {
            if (iou(st, roi) > m_iouThresh) {
                duplicate = true;
                break;
            }
        }
        if (duplicate) continue;

        cv::Mat tile = img(roi).clone();

        // 5) Cắt bbox theo tile, giữ phần nằm trong tile (cho phép cắt 1 phần)
        QVector<BBox> newBoxes;
        for (const auto &bb : g) {
            int bx = int(bb.xc * m_imgWidth);
            int by = int(bb.yc * m_imgHeight);
            int bw = int(bb.w * m_imgWidth);
            int bh = int(bb.h * m_imgHeight);

            int xmin = bx - bw/2, ymin = by - bh/2;
            int xmax = bx + bw/2, ymax = by + bh/2;

            int nxmin = std::max(xmin, roi.x) - roi.x;
            int nymin = std::max(ymin, roi.y) - roi.y;
            int nxmax = std::min(xmax, roi.x + roi.width) - roi.x;
            int nymax = std::min(ymax, roi.y + roi.height) - roi.y;

            if (nxmin < nxmax && nymin < nymax) {
                float ncx = (nxmin + nxmax) / 2.0f / float(roi.width);
                float ncy = (nymin + nymax) / 2.0f / float(roi.height);
                float nw  = (nxmax - nxmin) / float(roi.width);
                float nh  = (nymax - nymin) / float(roi.height);
                newBoxes.push_back({bb.cls, ncx, ncy, nw, nh});
            }
        }

        if (!newBoxes.isEmpty()) {
            saveTile(tile, newBoxes, roi.x, roi.y, localIndex);
            savedTiles.push_back(roi);
            localIndex++;
        }
    }

    qDebug() << "Generated" << (localIndex - 1) << "tiles for" << m_imagePath;
}

void ImageTiler::saveTile(const cv::Mat &tile, const QVector<BBox> &boxes,
                          int tileX, int tileY, int localIndex) {
    QString baseName = QFileInfo(m_imagePath).completeBaseName();
    QString ext = QFileInfo(m_imagePath).suffix();

    QString imgName = QString("%1/%2[%3].%4")
                          .arg(m_outputDir)
                          .arg(baseName)
                          .arg(localIndex)
                          .arg(ext);

    cv::imwrite(imgName.toStdString(), tile);

    QString labelName = imgName;
    labelName.replace("." + ext, ".txt");

    QFile file(labelName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (const auto &b : boxes) {
            out << b.cls << " " << b.xc << " " << b.yc << " "
                << b.w << " " << b.h << "\n";
        }
    }
}

// --- grouping / utility implementations ---

QVector<QVector<BBox>> ImageTiler::groupBBoxes(const QVector<BBox> &boxes) const {
    QVector<QVector<BBox>> groups;
    int tileW = m_tileSize.width();
    int tileH = m_tileSize.height();

    for (const auto &b : boxes) {
        bool added = false;
        for (auto &g : groups) {
            // thử hợp nhất tạm thời
            QVector<BBox> temp = g;
            temp.push_back(b);
            cv::Rect u = unionGroup(temp);
            if (u.width <= tileW && u.height <= tileH) {
                // nếu hợp nhất vẫn vừa tile -> gộp vào nhóm đó
                g.push_back(b);
                added = true;
                break;
            }
        }
        if (!added) {
            groups.push_back(QVector<BBox>{b});
        }
    }
    return groups;
}

cv::Rect ImageTiler::unionGroup(const QVector<BBox> &group) const {
    if (group.isEmpty()) return cv::Rect();

    int xmin = INT_MAX, ymin = INT_MAX;
    int xmax = INT_MIN, ymax = INT_MIN;

    for (const auto &bb : group) {
        int bx = int(bb.xc * m_imgWidth);
        int by = int(bb.yc * m_imgHeight);
        int bw = int(bb.w * m_imgWidth);
        int bh = int(bb.h * m_imgHeight);

        int x1 = bx - bw/2;
        int y1 = by - bh/2;
        int x2 = bx + bw/2;
        int y2 = by + bh/2;

        xmin = std::min(xmin, x1);
        ymin = std::min(ymin, y1);
        xmax = std::max(xmax, x2);
        ymax = std::max(ymax, y2);
    }

    // Trả về bounding rect của group (có thể có x < 0 hoặc y < 0 - xử lý khi tạo ROI)
    return cv::Rect(xmin, ymin, xmax - xmin, ymax - ymin);
}

double ImageTiler::iou(const cv::Rect &a, const cv::Rect &b) const {
    cv::Rect inter = a & b;
    int interArea = inter.area();
    int unionArea = a.area() + b.area() - interArea;
    if (unionArea <= 0) return 0.0;
    return double(interArea) / double(unionArea);
}
