#include "augmentdialog.h"
#include "ui_augmentdialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include "imagetiler.h"
#include <QRegularExpression>

#include <QFile>
#include <QTextStream>
#include <QImage>
#include <QDebug>
#include <algorithm>
#include <QMessageBox>

AugmentDialog::AugmentDialog(QWidget *parent, DataSource *dataSrc)
    : QDialog(parent), _dataSrc(dataSrc), ui(new Ui::AugmentDialog)
{
    ui->setupUi(this);
    ui->imageTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->imageTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->imageTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    this->setFixedSize(this->size());
    loadImageList(_dataSrc->sourceDir());
    ui->tileDimensionWidget->setVisible(false);

    connect(ui->imageTableWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AugmentDialog::updateSelectionCount);

    connect(ui->openFileButton, &QPushButton::clicked,
            this, &AugmentDialog::openFileDialog);

    connect(ui->augmentationMethodComboBox, &QComboBox::currentTextChanged,
            this, [=](const QString &text) {
                ui->tileDimensionWidget->setVisible(text.contains("Tile"));
            });

    connect(ui->fileNameFilterLineEdit, &QLineEdit::textChanged,
            this, &AugmentDialog::applyFilter);

    connect(ui->classFilterComboBox, &QComboBox::currentTextChanged,
            this, &AugmentDialog::applyFilter);

    connect(ui->augmentedOnlyCheckBox, &QCheckBox::toggled,
            this, &AugmentDialog::applyFilter);
}

AugmentDialog::~AugmentDialog()
{
    delete ui;
}

void AugmentDialog::openFileDialog()
{
    QString folder = QFileDialog::getExistingDirectory(this, tr("Select Image Folder"));
    if (folder.isEmpty())
        return;

    ui->imageFolderPathLineEdit->setText(folder);
    qDebug() << "Selected folder:" << folder;

    loadImageList(folder);
}

void AugmentDialog::on_closePushButton_clicked()
{
    qDebug() << "Cancel clicked!";
    close();
}

void AugmentDialog::loadImageList(const QString &folder)
{
    if (folder.isEmpty()) return;

    ui->imageFolderPathLineEdit->setText(folder);

    QDir dir(folder);
    QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.bmp"};
    _allFiles = dir.entryInfoList(filters, QDir::Files);   //  lưu danh sách file gốc

    applyFilter(); // hiển thị theo filter hiện tại
}

void AugmentDialog::applyFilter()
{
    ui->imageTableWidget->setRowCount(0);

    QString nameFilter   = ui->fileNameFilterLineEdit->text().trimmed();
    QString classFilter  = ui->classFilterComboBox->currentText();
    bool showAugmented   = ui->augmentedOnlyCheckBox->isChecked();

    for (const QFileInfo &imgFile : _allFiles) {
        QString baseName = imgFile.completeBaseName();
        QString labelPath = imgFile.absolutePath() + "/" + baseName + ".txt";

        // --- lọc theo tên ---
        if (!nameFilter.isEmpty() && !imgFile.fileName().contains(nameFilter, Qt::CaseInsensitive))
            continue;

        // --- lọc theo Labeled/Unlabeled ---
        bool hasLabel = false;
        if (QFile::exists(labelPath)) {
            QFile labelFile(labelPath);
            if (labelFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                hasLabel = !labelFile.readAll().trimmed().isEmpty(); // true nếu file có nội dung
            }
        }

        if (classFilter == "Labelled" && !hasLabel) continue;
        if (classFilter == "Unlabelled" && hasLabel) continue;

        // --- lọc Augmented Only ---
        if (showAugmented) {
            QRegularExpression rx("(_FH|_FV|_R90|_R-90|\\[\\d+\\])$");
            if (rx.match(baseName).hasMatch()) continue;
        }

        // thêm row
        int row = ui->imageTableWidget->rowCount();
        ui->imageTableWidget->insertRow(row);
        addRowFromFile(imgFile, row);
    }

    ui->countLabel->setText(QString("%1 images").arg(ui->imageTableWidget->rowCount()));

    ui->imageTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);           // Image chiếm hết khoảng trống
    ui->imageTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Largest Object
    ui->imageTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Smallest Object
    ui->imageTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Tile

}

void AugmentDialog::addRowFromFile(const QFileInfo &imgFile, int row)
{
    QString largestInfo = "0 x 0";
    QString smallestInfo = "0 x 0";

    QImage img(imgFile.absoluteFilePath());
    QString labelPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + ".txt";

    if (!img.isNull() && QFile::exists(labelPath)) {
        int imgW = img.width();
        int imgH = img.height();

        QFile labelFile(labelPath);
        if (labelFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&labelFile);
            QList<BBox> boxes;

            while (!in.atEnd()) {
                BBox b;
                QString line = in.readLine().trimmed();
                if (line.isEmpty()) continue;

                QTextStream ls(&line, QIODevice::ReadOnly);
                ls >> b.cls >> b.xc >> b.yc >> b.w >> b.h;
                if (ls.status() != QTextStream::Ok) continue;

                if (b.w > 0 && b.h > 0 && b.w <= 1 && b.h <= 1) {
                    boxes.append(b);
                }
            }

            if (!boxes.isEmpty()) {
                auto largest = std::max_element(boxes.begin(), boxes.end(),
                                                [imgW, imgH](const BBox &a, const BBox &b) {
                                                    return (a.w * imgW) * (a.h * imgH) < (b.w * imgW) * (b.h * imgH);
                                                });
                auto smallest = std::min_element(boxes.begin(), boxes.end(),
                                                 [imgW, imgH](const BBox &a, const BBox &b) {
                                                     return (a.w * imgW) * (a.h * imgH) < (b.w * imgW) * (b.h * imgH);
                                                 });

                int lw = int(largest->w * imgW);
                int lh = int(largest->h * imgH);
                int sw = int(smallest->w * imgW);
                int sh = int(smallest->h * imgH);

                largestInfo = QString("%1 x %2").arg(lw).arg(lh);
                smallestInfo = QString("%1 x %2").arg(sw).arg(sh);
            }
        }
    }

    // Thêm row vào bảng
    QTableWidgetItem *item0 = new QTableWidgetItem(imgFile.fileName());
    item0->setData(Qt::UserRole, imgFile.absoluteFilePath());
    item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
    ui->imageTableWidget->setItem(row, 0, item0);

    ui->imageTableWidget->setItem(row, 1, new QTableWidgetItem(largestInfo));
    ui->imageTableWidget->setItem(row, 2, new QTableWidgetItem(smallestInfo));
}


void AugmentDialog::on_generatePushButton_clicked()
{
    QString method = ui->augmentationMethodComboBox->currentText();

    // lấy các hàng được chọn
    QModelIndexList selectedRows = ui->imageTableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        qDebug() << "No image selected in table!";
        return;
    }

    for (const QModelIndex &index : selectedRows) {
        int row = index.row();
        QTableWidgetItem *item = ui->imageTableWidget->item(row, 0);
        if (!item) continue;

        QString imgPath = item->data(Qt::UserRole).toString();
        QFileInfo imgFile(imgPath);
        QString labelPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + ".txt";

        if (!QFile::exists(labelPath)) {
            qWarning() << "No label file for" << imgFile.fileName() << "-> skipped!";
            continue;
        }

        if (method.contains("Tile")) {
            // ---- Tile ----
            QString sizeText = ui->tileSizeComboBox->currentText().trimmed();
            QStringList parts = sizeText.split(QRegularExpression("\\s*x\\s*"));
            if (parts.size() != 2) continue;

            int tileW = parts[0].trimmed().toInt();
            int tileH = parts[1].trimmed().toInt();

            ImageTiler tiler(imgPath, labelPath);
            tiler.setTileSize(QSize(tileW, tileH));
            tiler.setOutputDir(imgFile.absolutePath());
            tiler.process();
        }
        else if (method.contains("Rotate 90")) {
            // ---- Rotate 90 ----
            QImage img(imgPath);
            QTransform transform;
            transform.rotate(90);
            QImage rotated = img.transformed(transform);

            QString newImgPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_R90." + imgFile.suffix();
            rotated.save(newImgPath);

            // Xử lý bbox
            qDebug() << "Rotated 90 saved to:" << newImgPath;
        }
        else if (method.contains("Rotate -90")) {
            // ---- Rotate -90 ----
            QImage img(imgPath);
            QTransform transform;
            transform.rotate(-90);
            QImage rotated = img.transformed(transform);

            QString newImgPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_R-90." + imgFile.suffix();
            rotated.save(newImgPath);

            // xử lý bbox
            qDebug() << "Rotated -90 saved to:" << newImgPath;
        }
        else if (method.contains("Flip Vertical")) {
            // ---- Flip Vertical ----
            QImage img(imgPath);
            QImage flipped = img.mirrored(false, true); // flip vertical

            QString newImgPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_FV." + imgFile.suffix();
            flipped.save(newImgPath);

            // xử lý label
            QFile labelFile(labelPath);
            if (labelFile.open(QIODevice::ReadOnly)) {
                QList<QString> newLines;
                QTextStream in(&labelFile);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (line.isEmpty()) continue;

                    QTextStream ls(&line, QIODevice::ReadOnly);
                    int cls; double xc, yc, w, h;
                    ls >> cls >> xc >> yc >> w >> h;

                    yc = 1.0 - yc; // flip theo trục ngang

                    newLines.append(QString("%1 %2 %3 %4 %5")
                                        .arg(cls).arg(xc).arg(yc).arg(w).arg(h));
                }
                labelFile.close();

                QString newLabelPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_FV.txt";
                QFile outFile(newLabelPath);
                if (outFile.open(QIODevice::WriteOnly)) {
                    QTextStream out(&outFile);
                    for (const QString &l : newLines) out << l << "\n";
                }
            }

            qDebug() << "Flipped Vertical saved to:" << newImgPath;
        }
        else if (method.contains("Flip Horizontal")) {
            // ---- Flip Horizontal ----
            QImage img(imgPath);
            QImage flipped = img.mirrored(true, false); // flip horizontal

            QString newImgPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_FH." + imgFile.suffix();
            flipped.save(newImgPath);

            // xử lý label
            QFile labelFile(labelPath);
            if (labelFile.open(QIODevice::ReadOnly)) {
                QList<QString> newLines;
                QTextStream in(&labelFile);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (line.isEmpty()) continue;

                    QTextStream ls(&line, QIODevice::ReadOnly);
                    int cls; double xc, yc, w, h;
                    ls >> cls >> xc >> yc >> w >> h;

                    xc = 1.0 - xc; // flip theo trục dọc

                    newLines.append(QString("%1 %2 %3 %4 %5")
                                        .arg(cls).arg(xc).arg(yc).arg(w).arg(h));
                }
                labelFile.close();

                QString newLabelPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + "_FH.txt";
                QFile outFile(newLabelPath);
                if (outFile.open(QIODevice::WriteOnly)) {
                    QTextStream out(&outFile);
                    for (const QString &l : newLines) out << l << "\n";
                }
            }

            qDebug() << "Flipped Horizontal saved to:" << newImgPath;
        }
    }

    qDebug() << "Augmentation done!";
    loadImageList(_dataSrc->sourceDir());   // reload bảng
}


void AugmentDialog::on_deletePushButton_clicked()
{
    QModelIndexList selectedRows = ui->imageTableWidget->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        qDebug() << "No image selected to delete!";
        return;
    }

    if (QMessageBox::question(this, tr("Delete Images"),
                              tr("Are you sure you want to delete the selected images?"))
        != QMessageBox::Yes) {
        return;
    }

    for (const QModelIndex &index : selectedRows) {
        int row = index.row();
        QTableWidgetItem *item = ui->imageTableWidget->item(row, 0);
        if (!item) continue;

        QString imgPath = item->data(Qt::UserRole).toString();
        QFileInfo imgFile(imgPath);
        QString labelPath = imgFile.absolutePath() + "/" + imgFile.completeBaseName() + ".txt";

        if (QFile::exists(imgPath)) {
            QFile::remove(imgPath);
            qDebug() << "Deleted image:" << imgPath;
        }
        if (QFile::exists(labelPath)) {
            QFile::remove(labelPath);
            qDebug() << "Deleted label:" << labelPath;
        }
        loadImageList(_dataSrc->sourceDir());
    }
}

void AugmentDialog::updateSelectionCount()
{
    int selected = ui->imageTableWidget->selectionModel()->selectedRows().count();

    ui->generatePushButton->setText(
        QString("Generate (%1 selected)").arg(selected));

    ui->deletePushButton->setText(
        QString("Delete (%1 selected)").arg(selected));
}

