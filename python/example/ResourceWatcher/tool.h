#ifndef TOOL_H
#define TOOL_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class Tool {
public:
    static QJsonObject string2js(const QString &str)
    {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(str.toLocal8Bit().data());
        if( jsonDocument.isNull() ){
    //        qInfo()<< "failed "<< str;
        }

        return jsonDocument.object();
    }

    static QString js2String(const QJsonObject &json)
    {
        return QString(QJsonDocument(json).toJson());
    }

    static QJsonObject readJsonFromFile(const QString &filename)
    {
        QJsonObject js;
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly))
        {
            qInfo()<< "can not open "<< filename;
            return js;      // js.empty() == true
        }
        QByteArray ba = file.readAll();
        QJsonParseError e;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(ba, &e);
        if(e.error != QJsonParseError::NoError || jsonDoc.isNull())
        {
            qInfo()<< "to Json Failed" << filename;
            return js;
        }

        return jsonDoc.object();
    }

    static bool saveJosnToFile(const QString &filename, const QJsonObject &js)
    {
        QFile file(filename);
        if(!file.open(QIODevice::WriteOnly| QIODevice::Text))
        {
            qInfo()<< "can not open "<< filename;
            return false;
        }

        QJsonDocument json_doc(js);
        QByteArray ba = json_doc.toJson();

        file.write(ba);
        file.close();
        return true;
    }
} ;

#endif // TOOL_H
