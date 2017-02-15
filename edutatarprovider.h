#ifndef EDUTATARPROVIDER_H
#define EDUTATARPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QDomNode>

class EduTatarData
{
public:
    struct mark_t {
        QString reason;
        QString mark;

        mark_t( const QString& m, const QString& r ) :
            reason(r), mark(m)
        {
        }
    };

    struct subject_t {
        QString       subject;
        QString       comment;
        QString       homework;
        QList<mark_t> marks;

        subject_t( const QString& s, const QString& c, const QString& hw ):
            subject(s), comment(c), homework(hw) {
        }

        void addMark( const QString& m, const QString& r ){
            marks.append( mark_t( m, r ) );
        }

        QString toString() const {
            QString s;
            s= subject + " (" + homework + ") " + comment;
            for( auto it=marks.constBegin(); it!=marks.constEnd(); it++ )
                s += " " + it->mark + " (" + it->reason + ")";
            return s;
        }

    };

    struct day_t {
        QDate date;
        QList<subject_t> subjects;

        subject_t& add( const QString& s, const QString& c, const QString& hw ){
            subjects.append( subject_t( s, c, hw ) );
            return subjects.last();
        }

        bool isValid() const {
            return ! subjects.isEmpty();
        }

        QString toString() const {
            QString s;
            for( auto it= subjects.constBegin(); it != subjects.constEnd(); it++ )
                s += it->toString() + "\n";
            return s;
        }
    };

    QMap<QDate,day_t> _days;

public:
    EduTatarData();
    ~EduTatarData();

    bool addDay( const day_t& day );
    day_t& day( const QDate& date );
};

class EduTatarProvider: public QObject
{
    Q_OBJECT

    QString _login;
    QString _password;

    QNetworkAccessManager _mgr;
    QDate _requestedDate;
    bool isLoggedIn;

    EduTatarData _data;

private:
    bool parseDairyHtml( const QString& html, EduTatarData::day_t& day );
    void testParseDairy();
    bool processLogin();
    void processDairy();

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

    bool requestDay(const QDate& day);
    EduTatarData::day_t dataForDay( const QDate& date );

public slots:
    void onReplyMainPage();
    void onReplyLogin();
    void onReplyMainAfterLogin();
    void onReplyMainAfterLogin2();
    void onReplyDiary();

    void onSslErrors( QNetworkReply *reply, const QList<QSslError>& errors );

signals:
    void dataReceived();
    void progress(const QString&);
};

#endif // EDUTATARPROVIDER_H
