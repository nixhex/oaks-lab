#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QString>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QClipboard>
#include <QApplication>
#include <QRegularExpression>

#include "LCRNG.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnGenerate_clicked()
{
    // Clear previous results
    ui->tableResults->clearContents();
    ui->tableResults->setRowCount(0);

    // Parse 32-bit seeds
    QStringList seedLines32 = ui->txt32bitSeeds->toPlainText().split(QRegularExpression("[\\r\\n]+"),Qt::SkipEmptyParts);
    std::vector<SEED_32> seeds32;
    for (const QString &line : seedLines32)
    {
        bool ok;
        uint32_t val = line.trimmed().toUInt(&ok, 16);
        if(ok)
        {
            seeds32.push_back(val);
        }
    }

    // Parse 16-bit seeds
    QStringList seedLines16 = ui->txt16bitFilter->toPlainText().split(QRegularExpression("[\\r\\n]+"),Qt::SkipEmptyParts);
    std::vector<SEED_16> filter16;
    for(const QString &line : seedLines16)
    {
        bool ok;
        SEED_16 val = line.trimmed().toUShort(&ok, 16);
        if (ok)
        {
            filter16.push_back(val);
        }
    }

    int maxFrames = ui->spinMaxFrames->value();

    auto matches = LCRNG::SeekGBASeeds(seeds32, filter16, maxFrames);
    if (matches.empty())
    {
        QMessageBox::information(this, "No Seeds Found", "No matching seeds were found.");
        return;
    }

    // Populate results table
    ui->tableResults->setRowCount(static_cast<int>(matches.size()));
    for (int i = 0; i < matches.size(); i++)
    {
        const auto &entry = matches[i];

        QTableWidgetItem *item32 = new QTableWidgetItem(QString("%1").arg(entry.seed32, 8, 16, QLatin1Char('0')).toUpper());
        QTableWidgetItem *item16 = new QTableWidgetItem(QString("%1").arg(entry.seed16, 4, 16, QLatin1Char('0')).toUpper());
        QTableWidgetItem *itemFrame = new QTableWidgetItem(QString::number(entry.frames));
        ui->tableResults->setItem(i, 0, item32);
        ui->tableResults->setItem(i, 1, item16);
        ui->tableResults->setItem(i, 2, itemFrame);
    }
}

