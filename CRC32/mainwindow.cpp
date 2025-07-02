#include "mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QRegularExpression>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    centralWidget(new QWidget(this)),
    selectFileButton(new QPushButton("Выбрать файл", this)),
    resultLineEdit(new QLineEdit(this)),
    polynomialLineEdit(new QLineEdit(this)),
    filePathLabel(new QLabel("Путь к файлу:", this)),
    fileContentTextEdit(new QTextEdit(this))
{
    setWindowTitle("CRC32 с настраиваемым полиномом");
    resize(600, 600);
    setAcceptDrops(true);

    resultLineEdit->setReadOnly(true);
    polynomialLineEdit->setPlaceholderText("Введите полином (например: x^32 + x^26 + ... + 1)");
    fileContentTextEdit->setReadOnly(true);
    fileContentTextEdit->setPlaceholderText("Содержимое файла будет отображено здесь...");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(selectFileButton);
    layout->addWidget(filePathLabel);
    layout->addWidget(new QLabel("CRC32 файла:", this));
    layout->addWidget(resultLineEdit);
    layout->addWidget(new QLabel("Полином CRC:", this));
    layout->addWidget(polynomialLineEdit);
    layout->addWidget(new QLabel("Содержимое файла:", this));
    layout->addWidget(fileContentTextEdit);

    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(selectFileButton, &QPushButton::clicked, this, &MainWindow::selectFile);
}

MainWindow::~MainWindow() {}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        filePathLabel->setText("Файл: " + filePath);
        calculateCRC32(filePath);
    }
}

void MainWindow::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Выбрать файл");
    if (!fileName.isEmpty()) {
        filePathLabel->setText("Файл: " + fileName);
        calculateCRC32(fileName);
    }
}

void MainWindow::calculateCRC32(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
        return;
    }

    QByteArray data = file.readAll();
    fileContentTextEdit->setPlainText(QString::fromUtf8(data));

    QString polyStr = polynomialLineEdit->text();
    uint32_t poly = parsePolynomialString(polyStr);
    if (poly == 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось распознать полином.");
        return;
    }

    uint32_t crc = computeCRC32(data, poly);
    QString crcStr = QString("0x%1").arg(crc, 8, 16, QLatin1Char('0')).toUpper();
    resultLineEdit->setText(crcStr);
}

uint32_t MainWindow::parsePolynomialString(const QString &polyString) {
    QString input = polyString;
    input.remove(" ");

    if (!input.contains("x")) return 0;

    uint32_t result = 0;

    QRegularExpression regex("x\\^?(\\d*)");
    QRegularExpressionMatchIterator i = regex.globalMatch(input);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString degreeStr = match.captured(1);

        int degree = 1;
        if (!degreeStr.isEmpty())
            degree = degreeStr.toInt();

        if (degree < 0 || degree > 32) continue;
        result |= (1u << degree);
    }

    if (input.contains("+1") || input.endsWith("1"))
        result |= 1u;

    return result;
}

uint32_t MainWindow::computeCRC32(const QByteArray &data, uint32_t polynomial) {
    uint32_t table[256];

    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j)
            crc = (crc >> 1) ^ ((crc & 1) ? polynomial : 0);
        table[i] = crc;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (unsigned char byte : data)
        crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];

    return crc ^ 0xFFFFFFFF;
}
