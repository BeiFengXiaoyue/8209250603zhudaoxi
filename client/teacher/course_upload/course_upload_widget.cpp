#include "course_upload_widget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QFrame>
#include <QUrl>
#include <QDialog>

#include "../../common/network_handler.h"

// ============================================================
// 样式常量
// ============================================================
	static const QString kTagNormal =
	    "QPushButton { background-color: #F0F2F5; color: #666666; border: none; "
	    "border-radius: 12px; padding: 4px 14px; font-size: 12px; min-height: 0px; }"
	    "QPushButton:hover { background-color: #E4E6EB; }";
	
	static const QString kTagActive =
	    "QPushButton { background-color: #0071E3; color: #FFFFFF; border: none; "
	    "border-radius: 12px; padding: 4px 14px; font-size: 12px; font-weight: bold; min-height: 0px; }";

static const QString kPrimaryBtn =
    "QPushButton { background-color: #0071E3; color: white; border: none; "
    "border-radius: 20px; padding: 12px 48px; font-size: 15px; font-weight: bold; }"
    "QPushButton:hover { background-color: #0077ED; }"
    "QPushButton:disabled { background-color: #CCCCCC; }";

static const QString kInputNormal =
    "QLineEdit { background-color: #F5F7FA; border: 1px solid #E0E0E0; "
    "border-radius: 10px; padding: 12px 16px; font-size: 14px; }"
    "QLineEdit:focus { border-color: #0071E3; }";

static const QString kTextEditStyle =
    "QTextEdit { background-color: #F5F7FA; border: 1px solid #E0E0E0; "
    "border-radius: 10px; padding: 12px 16px; font-size: 14px; }"
    "QTextEdit:focus { border-color: #0071E3; }";

static const QString kScrollBarStyle =
    "QScrollBar:vertical { width: 6px; background: transparent; }"
    "QScrollBar::handle:vertical { background: #D0D0D0; border-radius: 3px; }"
    "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }";

// ============================================================
// 辅助函数
// ============================================================
static void applyShadow(QWidget *w)
{
    auto *shadow = new QGraphicsDropShadowEffect(w);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    w->setGraphicsEffect(shadow);
}

static QString formatFileSize(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    return QString::number(bytes / (1024.0 * 1024.0), 'f', 1) + " MB";
}

// ============================================================
// CourseFlowLayout 实现
// ============================================================
CourseFlowLayout::CourseFlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

CourseFlowLayout::~CourseFlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void CourseFlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int CourseFlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *CourseFlowLayout::itemAt(int index) const
{
    return m_itemList.value(index);
}

QLayoutItem *CourseFlowLayout::takeAt(int index)
{
    if (index >= 0 && index < m_itemList.size())
        return m_itemList.takeAt(index);
    return nullptr;
}

Qt::Orientations CourseFlowLayout::expandingDirections() const
{
    return {};
}

bool CourseFlowLayout::hasHeightForWidth() const
{
    return true;
}

int CourseFlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

QSize CourseFlowLayout::minimumSize() const
{
    QSize size;
    for (const auto *item : m_itemList)
        size = size.expandedTo(item->minimumSize());
    const auto margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

QSize CourseFlowLayout::sizeHint() const
{
    return minimumSize();
}

void CourseFlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

int CourseFlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    int spaceX = smartSpacing();
    int spaceY = spaceX;

    for (auto *item : m_itemList) {
        const auto widget = item->widget();
        if (widget && widget->isHidden()) continue;

        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}

int CourseFlowLayout::smartSpacing() const
{
    if (m_hSpace >= 0) return m_hSpace;
    return 8;
}

// ============================================================
// CourseDropZone 实现
// ============================================================
CourseDropZone::CourseDropZone(const QString &hintText, QWidget *parent)
    : QWidget(parent), m_hintText(hintText)
{
    setFixedHeight(180);
    setCursor(Qt::PointingHandCursor);
    setMinimumWidth(300);
}

void CourseDropZone::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 白色圆角背景
    painter.setBrush(QColor("#FFFFFF"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 15, 15);

    // 虚线边框
    QPen dashPen(QColor("#CCCCCC"), 2, Qt::DashLine);
    dashPen.setDashPattern({6, 4});
    painter.setPen(dashPen);
    int m = 12;
    painter.drawRoundedRect(rect().adjusted(m, m, -m, -m), 12, 12);

    // 居中图标（简化：一个 + 号）
    painter.setPen(QPen(QColor("#0071E3"), 3));
    int cx = width() / 2;
    int cy = height() / 2 - 15;
    painter.drawLine(cx - 12, cy, cx + 12, cy);
    painter.drawLine(cx, cy - 12, cx, cy + 12);

    // 提示文本
    painter.setPen(QColor("#999999"));
    QFont font;
    font.setPointSize(11);
    painter.setFont(font);
    QFontMetrics fm(font);
    QString text = m_hintText.isEmpty() ? "点击选择视频文件" : m_hintText;
    int tw = fm.horizontalAdvance(text);
    painter.drawText(cx - tw / 2, cy + 40, text);

    // 小字提示
    painter.setPen(QColor("#CCCCCC"));
    QFont smallFont;
    smallFont.setPointSize(9);
    painter.setFont(smallFont);
    QString hint = "支持 MP4, AVI, MOV 等常见格式";
    int hw = QFontMetrics(smallFont).horizontalAdvance(hint);
    painter.drawText(cx - hw / 2, cy + 65, hint);
}

void CourseDropZone::mousePressEvent(QMouseEvent *)
{
    emit clicked();
}

// ============================================================
// CourseUploadWidget 实现
// ============================================================
CourseUploadWidget::CourseUploadWidget(const QString &username, int classId,
                                       QWidget *parent)
    : QWidget(parent), m_username(username), m_classId(classId)
{
    m_subjectTags = {"语文","数学","英语","物理","化学","生物","政治","历史","地理","技术","其他"};
    m_functionTags = {"引入","预习","学习","复习","习题课"};
    setupUI();
    refreshData();
}

void CourseUploadWidget::setupUI()
{
    setStyleSheet("CourseUploadWidget { background-color: transparent; }");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 标题
    auto *titleLabel = new QLabel("课程上传");
    titleLabel->setStyleSheet(
        "QLabel { font-size: 24px; font-weight: bold; color: #333333; "
        "padding: 10px 0 20px 0; }");
    mainLayout->addWidget(titleLabel);

    // 可滚动的内容区
    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea { background-color: transparent; border: none; }" +
        kScrollBarStyle);

    auto *scrollContent = new QWidget();
    scrollContent->setStyleSheet("background-color: transparent;");
    auto *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(20);

    scrollLayout->addWidget(createUploadPage());
    scrollLayout->addStretch(1);

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea, 1);
}

QWidget* CourseUploadWidget::createTagGroup(const QString &title,
                                            const QStringList &tags,
                                            QPushButton *&selected)
{
    auto *card = new QWidget();
    card->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    card->setMinimumHeight(100);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 14, 20, 14);
    layout->setSpacing(8);

    // 标题行
    auto *titleRow = new QHBoxLayout();
    auto *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("QLabel { color: #333333; font-size: 14px; font-weight: bold; }");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch(1);
    auto *hintLabel = new QLabel("（最多选1项，可不选）");
    hintLabel->setStyleSheet("QLabel { color: #AAAAAA; font-size: 11px; }");
    titleRow->addWidget(hintLabel);
    layout->addLayout(titleRow);

    // 标签流式布局
    auto *flowWidget = new QWidget();
    flowWidget->setStyleSheet("background-color: transparent;");
        auto *flow = new CourseFlowLayout(flowWidget, 0, 8, 8);

    for (const auto &tag : tags) {
        auto *btn = new QPushButton(tag);
        btn->setStyleSheet(kTagNormal);
        btn->setCursor(Qt::PointingHandCursor);
        // 不设固定高度，由 padding + font-size 自然决定

        connect(btn, &QPushButton::clicked, this, [this, btn, &selected]() {
            if (selected == btn) {
                // 取消选择
                selected->setStyleSheet(kTagNormal);
                selected = nullptr;
            } else {
                // 取消之前的选择
                if (selected) selected->setStyleSheet(kTagNormal);
                // 选中当前
                btn->setStyleSheet(kTagActive);
                selected = btn;
            }
        });

        flow->addWidget(btn);
    }

    layout->addWidget(flowWidget);

    applyShadow(card);
    return card;
}

QWidget* CourseUploadWidget::createUploadPage()
{
    auto *page = new QWidget();
    page->setStyleSheet("background-color: transparent;");
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    // --- 视频名称 ---
    auto *nameCard = new QWidget();
    nameCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *nameLayout = new QVBoxLayout(nameCard);
    nameLayout->setContentsMargins(20, 16, 20, 16);
    nameLayout->setSpacing(6);
    auto *nameTitle = new QLabel("视频名称 *");
    nameTitle->setStyleSheet("QLabel { color: #333333; font-size: 14px; font-weight: bold; }");
    nameLayout->addWidget(nameTitle);
    auto *nameSubtitle = new QLabel("为你的课程视频起一个名称");
    nameSubtitle->setStyleSheet("QLabel { color: #999999; font-size: 12px; }");
    nameLayout->addWidget(nameSubtitle);
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("例：二次函数图像与性质");
    m_nameEdit->setStyleSheet(kInputNormal);
    m_nameEdit->setFixedHeight(44);
    nameLayout->addWidget(m_nameEdit);
    applyShadow(nameCard);
    layout->addWidget(nameCard);

    // --- 选择文件 ---
    auto *fileCard = new QWidget();
    fileCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *fileLayout = new QVBoxLayout(fileCard);
    fileLayout->setContentsMargins(20, 16, 20, 16);
    fileLayout->setSpacing(6);
    auto *fileTitle = new QLabel("选择视频文件 *");
    fileTitle->setStyleSheet("QLabel { color: #333333; font-size: 14px; font-weight: bold; }");
    fileLayout->addWidget(fileTitle);

    m_dropZone = new CourseDropZone("");
    connect(m_dropZone, &CourseDropZone::clicked, this, &CourseUploadWidget::onSelectFile);
    fileLayout->addWidget(m_dropZone);

    m_fileInfoLabel = new QLabel("");
    m_fileInfoLabel->setStyleSheet("QLabel { color: #666666; font-size: 12px; padding-left: 4px; }");
    m_fileInfoLabel->setVisible(false);
    fileLayout->addWidget(m_fileInfoLabel);

    applyShadow(fileCard);
    layout->addWidget(fileCard);

    // --- 科目标签 ---
    layout->addWidget(createTagGroup("科目", m_subjectTags, m_selectedSubject));

    // --- 功能标签 ---
    layout->addWidget(createTagGroup("功能", m_functionTags, m_selectedFunction));

    // --- 简介 ---
    auto *descCard = new QWidget();
    descCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *descLayout = new QVBoxLayout(descCard);
    descLayout->setContentsMargins(20, 16, 20, 16);
    descLayout->setSpacing(6);
    auto *descTitle = new QLabel("简介（选填）");
    descTitle->setStyleSheet("QLabel { color: #333333; font-size: 14px; font-weight: bold; }");
    descLayout->addWidget(descTitle);
    m_descEdit = new QTextEdit();
    m_descEdit->setPlaceholderText("简单描述一下这节课的内容...");
    m_descEdit->setStyleSheet(kTextEditStyle);
    m_descEdit->setFixedHeight(100);
    descLayout->addWidget(m_descEdit);
    applyShadow(descCard);
    layout->addWidget(descCard);

    // --- 上传按钮 ---
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 10, 0, 10);
    btnLayout->addStretch(1);
    m_uploadBtn = new QPushButton("上传课程");
    m_uploadBtn->setStyleSheet(kPrimaryBtn);
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    connect(m_uploadBtn, &QPushButton::clicked, this, &CourseUploadWidget::onUpload);
    btnLayout->addWidget(m_uploadBtn);
    btnLayout->addStretch(1);
    layout->addLayout(btnLayout);

    // --- 分隔线 ---
    auto *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("QFrame { color: #E0E0E0; max-height: 1px; margin: 10px 0; }");
    layout->addWidget(separator);

    // --- 已上传课程 ---
    auto *listTitle = new QLabel("已上传课程");
    listTitle->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: bold; color: #333333; }");
    layout->addWidget(listTitle);

    auto *tableCard = new QWidget();
    tableCard->setStyleSheet("QWidget { background-color: #FFFFFF; border-radius: 15px; }");
    auto *tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(0, 0, 0, 0);

    m_courseTable = new QTableWidget(0, 5);
    m_courseTable->setHorizontalHeaderLabels({"视频名称", "科目", "功能", "上传时间", "操作"});
    m_courseTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_courseTable->setShowGrid(false);
    m_courseTable->verticalHeader()->setVisible(false);
    m_courseTable->horizontalHeader()->setStretchLastSection(true);
    m_courseTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_courseTable->setAlternatingRowColors(true);
    m_courseTable->setStyleSheet(R"(
        QTableWidget {
            border: none;
            border-radius: 15px;
            background-color: #FFFFFF;
            alternate-background-color: #F8F9FA;
            font-size: 13px;
        }
        QTableWidget::item {
            padding: 8px 12px;
            border-bottom: 1px solid #F0F0F0;
        }
        QHeaderView::section {
            background-color: #F5F7FA;
            color: #888888;
            font-size: 12px;
            font-weight: bold;
            padding: 10px 12px;
            border: none;
            border-bottom: 1px solid #E0E0E0;
        }
    )" + kScrollBarStyle);

    tableLayout->addWidget(m_courseTable);
    applyShadow(tableCard);
    layout->addWidget(tableCard);

    return page;
}

void CourseUploadWidget::onSelectFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this, "选择视频文件", QString(),
        "视频文件 (*.mp4 *.avi *.mov *.mkv *.wmv *.flv *.webm);;所有文件 (*)");

    if (filePath.isEmpty()) return;

    m_selectedFilePath = filePath;
    QFileInfo info(filePath);

    m_fileInfoLabel->setText(QString("已选择: %1  (%2)")
                                 .arg(info.fileName())
                                 .arg(formatFileSize(info.size())));
    m_fileInfoLabel->setVisible(true);

    // 如果没有填名称，自动填入文件名（不含后缀）
    if (m_nameEdit->text().trimmed().isEmpty()) {
        m_nameEdit->setText(info.completeBaseName());
    }
}

void CourseUploadWidget::onUpload()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入视频名称");
        m_nameEdit->setFocus();
        return;
    }
    if (m_selectedFilePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择要上传的视频文件");
        return;
    }

    m_uploadBtn->setEnabled(false);
    m_uploadBtn->setText("上传中...");

    // 获取标签文本
    QString subject = m_selectedSubject ? m_selectedSubject->text() : "";
    QString function = m_selectedFunction ? m_selectedFunction->text() : "";
    QString description = m_descEdit->toPlainText().trimmed();

    // 构建 multipart 请求
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 文件部分
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QString("form-data; name=\"file\"; filename=\"%1\"")
            .arg(QFileInfo(m_selectedFilePath).fileName()));
    QFile *file = new QFile(m_selectedFilePath);
    if (!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "错误", "无法读取文件");
        m_uploadBtn->setEnabled(true);
        m_uploadBtn->setText("上传课程");
        delete file;
        delete multiPart;
        return;
    }
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    // 表单字段
    auto addTextPart = [multiPart](const QString &name, const QString &value) {
        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
            QString("form-data; name=\"%1\"").arg(name));
        textPart.setBody(value.toUtf8());
        multiPart->append(textPart);
    };

    addTextPart("name", name);
    addTextPart("teacher", m_username);
    addTextPart("class", QString::number(m_classId));
    addTextPart("subject", subject);
    addTextPart("function", function);
    addTextPart("description", description);

    // 发送请求
    QUrl url(NetworkHandler::baseUrl() + "/api/courses/upload");
    QNetworkRequest request(url);

    auto *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        reply->manager()->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::critical(this, "上传失败",
                "网络错误: " + reply->errorString());
            m_uploadBtn->setEnabled(true);
            m_uploadBtn->setText("上传课程");
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        if (obj["success"].toBool()) {
            // 显示成功对话框
            auto *dialog = new QDialog(this);
            dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            dialog->setFixedSize(300, 220);
            dialog->setStyleSheet("QDialog { background-color: #FFFFFF; border-radius: 20px; }");

            auto *dlgLayout = new QVBoxLayout(dialog);
            dlgLayout->setContentsMargins(30, 30, 30, 30);
            dlgLayout->setSpacing(15);

            // 对勾图标
            auto *iconLabel = new QLabel("✓");
            iconLabel->setAlignment(Qt::AlignCenter);
            iconLabel->setStyleSheet(
                "QLabel { color: #4CAF50; font-size: 48px; font-weight: bold; }");
            dlgLayout->addWidget(iconLabel);

            auto *msgLabel = new QLabel("课程上传成功！");
            msgLabel->setAlignment(Qt::AlignCenter);
            msgLabel->setStyleSheet("QLabel { color: #333333; font-size: 16px; font-weight: bold; }");
            dlgLayout->addWidget(msgLabel);

            dlgLayout->addStretch(1);

            auto *okBtn = new QPushButton("确定");
            okBtn->setStyleSheet(kPrimaryBtn);
            okBtn->setCursor(Qt::PointingHandCursor);
            connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
            dlgLayout->addWidget(okBtn, 0, Qt::AlignCenter);

            // 对话框阴影
            applyShadow(dialog);

            connect(dialog, &QDialog::accepted, this, [this]() {
                resetUploadForm();
                emit uploadSuccess();
                refreshData();
            });

            dialog->exec();
            dialog->deleteLater();
        } else {
            QString msg = obj["message"].toString("未知错误");
            QMessageBox::critical(this, "上传失败", msg);
        }

        m_uploadBtn->setEnabled(true);
        m_uploadBtn->setText("上传课程");
    });
}

void CourseUploadWidget::resetUploadForm()
{
    m_nameEdit->clear();
    m_selectedFilePath.clear();
    m_fileInfoLabel->setVisible(false);
    m_descEdit->clear();

    if (m_selectedSubject) {
        m_selectedSubject->setStyleSheet(kTagNormal);
        m_selectedSubject = nullptr;
    }
    if (m_selectedFunction) {
        m_selectedFunction->setStyleSheet(kTagNormal);
        m_selectedFunction = nullptr;
    }
}

void CourseUploadWidget::refreshData()
{
    QString url = NetworkHandler::baseUrl() +
                  "/api/courses/list?class=" + QString::number(m_classId);
    NetworkHandler::instance()->get(url, [this](bool ok, const QJsonObject &json) {
        if (!ok) return;
        QJsonArray arr = json["data"].toArray();
        // 缓存数据并刷新表格
        m_courseTable->setRowCount(0);
        for (const auto &val : arr) {
            QJsonObject obj = val.toObject();
            int row = m_courseTable->rowCount();
            m_courseTable->insertRow(row);

            m_courseTable->setItem(row, 0,
                new QTableWidgetItem(obj["course"].toString()));
            m_courseTable->setItem(row, 1,
                new QTableWidgetItem(obj["subject"].toString()));
            m_courseTable->setItem(row, 2,
                new QTableWidgetItem(obj["function"].toString()));
	            m_courseTable->setItem(row, 3,
	                new QTableWidgetItem(obj["time"].toString().split(" ")[0]));

            // 操作列：下载按钮
            int courseId = obj["id"].toInt();
            auto *downloadBtn = new QPushButton("下载");
            downloadBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #0071E3;
                    color: white;
                    border: none;
                    border-radius: 12px;
                    padding: 4px 16px;
                    font-size: 12px;
                }
                QPushButton:hover { background-color: #0077ED; }
            )");
            downloadBtn->setCursor(Qt::PointingHandCursor);
            connect(downloadBtn, &QPushButton::clicked, this, [this, courseId]() {
                QString fileUrl = NetworkHandler::baseUrl() +
                                  "/api/courses/" + QString::number(courseId) + "/file";
                QString savePath = QFileDialog::getSaveFileName(this, "保存视频");
                if (savePath.isEmpty()) return;

                auto *manager = new QNetworkAccessManager(this);
                QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(fileUrl)));
                connect(reply, &QNetworkReply::finished, this, [reply, savePath]() {
                    reply->deleteLater();
                    reply->manager()->deleteLater();
                    if (reply->error() != QNetworkReply::NoError) return;
                    QFile file(savePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(reply->readAll());
                        file.close();
                    }
                });
            });

            auto *btnWidget = new QWidget();
            auto *btnLayout = new QHBoxLayout(btnWidget);
            btnLayout->setContentsMargins(4, 4, 4, 4);
            btnLayout->addWidget(downloadBtn);
            btnLayout->setAlignment(Qt::AlignCenter);
            m_courseTable->setCellWidget(row, 4, btnWidget);
        }
    });
}
