#ifndef PDF_WRITER_H
#define PDF_WRITER_H

#include <QString>
#include <QImage>

class pdf_util
{
public:
    void init(QString pathname);
    QString new_pathname();
    void merge_pdf();

private:
    int file_index;
    QString path;
    QStringList inputs;
    QString output_pathname;
};

class QFile;
class QPdfWriter;
class pdfwriter
{
public:
    static pdf_util util;
    pdfwriter();

    void append_page(QImage img);

    ~pdfwriter();

    QFile *pdf_file;
    QPdfWriter *pdf_writer;
    QPainter *pdf_painter;
    int append_count = 0;
};

#endif // PDF_WRITER_H
