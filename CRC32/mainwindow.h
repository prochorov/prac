#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <QLabel>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QTextEdit>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void selectFile();
    void calculateCRC32(const QString& filePath);

private:
    QWidget *centralWidget;
    QPushButton *selectFileButton;
    QLineEdit *resultLineEdit;
    QLineEdit *polynomialLineEdit;
    QLabel *filePathLabel;
    QTextEdit *fileContentTextEdit;

    uint32_t computeCRC32(const QByteArray &data, uint32_t poly);
    uint32_t parsePolynomialString(const QString &polyString);
};

#endif // MAINWINDOW_H
