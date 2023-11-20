#include <QCoreApplication>

#include "pdfwriter.h"

#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDir dir(u8"D:\\006");
    QStringList image_paths = dir.entryList(QDir::Files);
    QString pdf_path = "D:\\test.pdf";

    pdfwriter::util.init(pdf_path);
    pdfwriter* pdf = new pdfwriter();

    for (QString path : image_paths) {
        if (path.split('.').last() != "png")
            continue;
        if (pdf->append_count == 10) {

             delete pdf;
             pdf = new pdfwriter();
        }

        pdf->append_page(QImage(dir.absoluteFilePath(path)));
    }

    delete pdf;
    pdfwriter::util.merge_pdf();

    qInfo() << "over";

    return a.exec();
}
