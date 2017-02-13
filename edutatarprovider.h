#ifndef EDUTATARPROVIDER_H
#define EDUTATARPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>

class EduTatarData
{
public:
    struct subject_t {
        QString      subject;
        QList<QChar> mark;
        QString      homework;
    };

    struct day_t {
        QDate date;
        QList<subject_t> subjects;
    };

    struct week_t {
        QList<day_t> days;
    };

    QList<week_t> weeks;

public:
    EduTatarData();
    ~EduTatarData();
};


class EduTatarProvider: public QObject
{
    Q_OBJECT

    QString _login;
    QString _password;

    QNetworkAccessManager _mgr;

    EduTatarData _data;

public:
    EduTatarProvider( QObject* p );
    ~EduTatarProvider();

    void setAuth( const QString& login, const QString& pass )
    {
        _login= login;
        _password= pass;
    }

    QString login() const
        { return _login; }
    QString password() const
        { return _password; }

    bool update();

    void testXPath();

public slots:
    void onReplyMainPage();
    void onReplyLogin();
    void onReplyMainAfterLogin();
    void onReplyMainAfterLogin2();
    void onReplyDiary();
    void onReplyWiki();

    void onRedirect(QUrl url);

signals:
    void dataReceived();

};

#endif // EDUTATARPROVIDER_H
