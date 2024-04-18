#include "pdfwriter.h"

#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QPdfWriter>
#include <QPainter>
#include <QDateTime>

pdf_util pdfwriter::util = pdf_util();

pdfwriter::pdfwriter()
{
    pdf_file = new QFile(util.new_pathname());
    if (!pdf_file->open(QIODevice::WriteOnly)) {
        qInfo() << pdf_file << "open failed";
        return;
    }

    pdf_writer = new QPdfWriter(pdf_file);
    pdf_writer->setResolution(150);
    pdf_writer->setPageMargins(QMarginsF(0.0, 0.0, 0.0, 0.0));
    // 添加文字（复制时用到）
    // pdf_writer->setPageSize(QPageSize(QPageSize::A4, QPageSize::Inch, QString(),
    // QPageSize::ExactMatch));
    pdf_painter = new QPainter(pdf_writer);
}

void pdfwriter::append_page(QImage img)
{
    if (!pdf_file || !pdf_file->isOpen()) {
        qDebug() << "PDF file is not open.";
        return;
    }

    append_count++;
    if (append_count > 1) {
        pdf_writer->newPage();
    }
    pdf_painter->drawImage(0, 0, img, 0, 0, -1, -1);
    // 双层pdf，计算文字的大小，坐标，并绘制
    // pdf_painter->drawText();
}

pdfwriter::~pdfwriter()
{
    delete pdf_painter;
    delete pdf_writer;
    if (pdf_file->isOpen())
        pdf_file->close();
    delete pdf_file;
}

void pdf_util::init(QString pathname)
{
    path = QCoreApplication::applicationDirPath() + "/tmp/export";
    QDir dir;
    dir.mkpath(path);
    qInfo() << path;

    output_pathname = pathname;
    inputs.clear();
    file_index = 0;
}

QString pdf_util::new_pathname()
{
    QString name = QString("%1_export_%2.pdf")
                       .arg(QFileInfo(output_pathname).baseName())
                       .arg(QString::number(++file_index));
    inputs.append(path + '/' + name);
    return inputs.last();
}

void pdf_util::merge_pdf()
{
    auto merge = [](QString output, QStringList inputs) {
        qint64 time = QDateTime::currentMSecsSinceEpoch();
        QProcess process;
        QString program = "./qpdf";
        QStringList arguments;
        arguments << "--empty";
        arguments << "--pages";

        for (const QString e : inputs) {
            arguments << e;
        }

        arguments << "--";
        arguments << output;

        process.start(program, arguments);
        process.waitForFinished(300 * 1000);
        qInfo() << QString("merge, use time: %1s")
                       .arg((QDateTime::currentMSecsSinceEpoch() - time) / 1000.0);
    };

    if (inputs.size() > 1) {
        merge(output_pathname, inputs);
    }
    else {
        QFile::copy(inputs.first(), output_pathname);
    }

    qInfo() << "delete tmp file";
    for (auto e : inputs) {
        QFile::remove(e);
    }
}
