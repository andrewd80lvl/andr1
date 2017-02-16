#include "edutatarprovider.h"

#include <QUrlQuery>
#include <QRegularExpression>
#include <QDomDocument>
#include <QLoggingCategory>

#include <time.h>

Q_LOGGING_CATEGORY(fcHttp, "http");

const QString g_url= "https://edu.tatar.ru";
const QString g_url_login= g_url + "/logon";
const QString g_url_diary= g_url + "/user/diary/day?for=%1";    // %1 - seconds from 1 Jan 1970 to needed day

const QString g_userAgent= "Mozilla/5.0 (X11; Linux x86_64; rv:50.0) Gecko/20100101 Firefox/50.0";

/******************************************************************************/
QDebug operator<< (QDebug d, const QNetworkReply *reply)
{
    if ( reply->error() != QNetworkReply::NoError )
        d << "failure: " << reply->errorString();

    foreach (QNetworkReply::RawHeaderPair p, reply->rawHeaderPairs() ) {
        d << p.first <<  ":" <<  p.second << "\n";
    }
    return d;
}

/******************************************************************************/
EduTatarProvider::EduTatarProvider( QObject* p )
    : QObject( p ),
      _mgr( p ),
      isLoggedIn(false)
{
    connect( &_mgr, SIGNAL(sslErrors(QNetworkReply*, QList<QSslError>)),
              this, SLOT(onSslErrors(QNetworkReply*, QList<QSslError>)));
}

EduTatarProvider::~EduTatarProvider()
{
}

void EduTatarProvider::onSslErrors( QNetworkReply *reply, const QList<QSslError>& errors )
{
    reply->ignoreSslErrors(errors);
    qDebug() << "ssl errors" << errors;
}

bool EduTatarProvider::requestDay(const QDate& day)
{
    _requestedDate= day;
    EduTatarData::day_t d= _data.day( day );
    if( d.isValid() )
        emit dataReceived();
    else {
        if( isLoggedIn == false )
            processLogin();
        else
            processDairy();
    }

    return true;
}

bool EduTatarProvider::processLogin()
{
    QNetworkRequest req;
    req.setUrl( g_url_login );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    qCDebug(fcHttp) << "hdrs: "  << req.rawHeaderList();

    emit progress("connect to edu.tatar.ru");
    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyMainPage() ) );
    return true;
}

void EduTatarProvider::onReplyMainPage()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainPage() ) );

    qDebug( "received main page" );
    qCDebug(fcHttp) << reply;

    if ( reply->error() != QNetworkReply::NoError ) {
        qDebug() << "Failure" << reply->errorString();
        emit error("server error");
        return;
    }

    emit progress("login");

    // post login
    QNetworkRequest req;
    req.setUrl( QUrl( QString( g_url_login ) ) );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    req.setRawHeader( "Referer", "https://edu.tatar.ru/logon" );
    req.setHeader( QNetworkRequest::ContentTypeHeader,
                   "application/x-www-form-urlencoded" );

    QByteArray loginData;
    QUrlQuery loginParams;
    loginParams.addQueryItem( "main_login", _login );
    loginParams.addQueryItem( "main_password", _password );
    loginData.append( loginParams.toString() );

    req.setRawHeader( QByteArray("Content-Length"), QString::number( loginData.size() ).toLatin1() );

    reply = _mgr.post( req, loginData );
    connect( reply, SIGNAL( finished()), this, SLOT( onReplyLogin() ) );
}

void EduTatarProvider::onReplyLogin()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyLogin() ) );

    qDebug( "received login page" );
    qCDebug(fcHttp) << reply;

    if( reply->rawHeader("Location") != "/start/logon-process" ){
        qDebug() << "* logon failure";
        emit error("server error");
        return;
    }

    emit progress("get profile page");

    QNetworkRequest req;
    req.setUrl( QUrl( QString( g_url ) + reply->rawHeader("Location") ) );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    req.setRawHeader( "Referer", "https://edu.tatar.ru/logon" );

    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyMainAfterLogin() ) );
}

void EduTatarProvider::onReplyMainAfterLogin()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainAfterLogin() ) );

    qDebug( "received main page after login" );
    qCDebug(fcHttp) << reply;

    if( reply->rawHeader("Location") != "/" ){
        qDebug() << "* logon failure";
        emit error("server error");
        return;
    }

    QNetworkRequest req;
    req.setUrl( QUrl( QString( g_url ) + reply->rawHeader("Location") ) );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    req.setRawHeader( "Referer", "https://edu.tatar.ru/logon" );

    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyMainAfterLogin2() ) );
}

void EduTatarProvider::onReplyMainAfterLogin2()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainAfterLogin() ) );

    QString html= reply->readAll();

    qDebug( "received main page after login 2" );
    qCDebug(fcHttp) << reply;
    qCDebug(fcHttp) << "html[" << html.length() << "]: ";
    qCDebug(fcHttp) << html;

    isLoggedIn= true;

    processDairy();
}

void EduTatarProvider::processDairy()
{
    emit progress("get dairy page for " + _requestedDate.toString("dd.MM.yyyy"));
    qDebug() << "get dairy page for " << _requestedDate.toString("dd.MM.yyyy");

    QNetworkRequest req;
    QUrl u= QUrl( g_url_diary.arg( QDateTime(_requestedDate).toTime_t() ) );

    emit progress( u.toString() );

    req.setUrl( u );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    req.setRawHeader( "Referer", "https://edu.tatar.ru/logon" );

    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyDiary() ) );
}

void EduTatarProvider::onReplyDiary()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainAfterLogin() ) );

    QString html= reply->readAll();

    qDebug( "received diary page" );
    qCDebug(fcHttp) << reply;
    qCDebug(fcHttp) << "\n" << "dairy html[" << html.length() << "]: ";
    qCDebug(fcHttp) << html;

    EduTatarData::day_t day;
    day.date= _requestedDate;
    if( parseDairyHtml( html, day ) == false ){
        qCritical("can't parse dairy html");
        emit error("can't parse dairy html");
        return;
    }
    _data.addDay( day );
    emit dataReceived();
}

void EduTatarProvider::testParseDairy()
{
    QString html= "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"\n    \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head>\n    <title>Электронное образование в Республике Татарстан</title>\n    <link href=\"/design/css/common.css\" type=\"text/css\" rel=\"stylesheet\">\n                <link rel=\"stylesheet\" type=\"text/css\" href=\"/assets/bower_components/font-awesome/css/font-awesome.min.css\" />\n    \n\n    <link rel=\"stylesheet\" href=\"/design/carousel/classic.css\" type=\"text/css\" media=\"screen\">\n    <!--[if IE]><link rel=\"stylesheet\" href=\"/design/css/ie.css\" type=\"text/css\" media=\"screen\" /><![endif]-->\n    \t    <script type=\"text/javascript\" src=\"/javascript/prototype.js\"></script>\n\t\t<script type=\"text/javascript\" src=\"/js/jquery-1.8.1.min.js\"></script>\n\n\t\t<script type=\"text/javascript\">\n        var $J = jQuery.noConflict();\n                var eduConfig = {\"serverTime\":1487072380,\"isProd\":true}    </script>\n\t            <script type=\"text/javascript\" src=\"/javascript/lib.js?v=4\"></script>\n\n\t<script type=\"text/javascript\">\n\tif (isOpenedInFrame()) redirectToBlank();\n\t</script>\n</head>\n<body>\n    <div id=\"wrapper\" class=\"inner without_menu\">\n                <div class=\"row banners hidden-print\">\n            <div class=\"col-md-12\">\n                \n<div class=\"carousel slide vertical\" data-ride=\"carousel\">\n    <div class=\"carousel-inner\">\n\n\n\n         <div class=\"item active\" data-expire=\"2017-03-22\">\n            <a href=\"https://edu.tatar.ru/n_chelny/sch78/page2871122.htm\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_nch_pch_17.jpg\" alt=\"II Всероссийский Открытый творческий конкурс «Пушкинские чтения»\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-19\">\n            <a href=\"http://baytik-kazan.ru/zapis211/zimnie_olimpiadnye_sbory_v_baytike\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_baitik_zim_ol.jpg\" alt=\"Зимние олимпиадные сборы в Байтике\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-26\">\n            <a href=\"http://admissions.kpfu.ru/comission-news?field_kategoria_novosti_target_id[14]=14&field_kategoria_novosti_target_id[12]=12&field_kategoria_novosti_target_id[9]=9&field_kategoria_novosti_target_id[15]=15&field_kategoria_novosti_target_id[18]=18&field_kategoria_novosti_target_id[13]=13&field_kategoria_novosti_target_id[11]=11&field_kategoria_novosti_target_id[17]=17&field_kategoria_novosti_target_id[16]=16&field_kategoria_novosti_target_id[8]=8\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kfu_dod_2017.jpg\" alt=\"КФУ - День открытых дверей\">\n            </a>\n         </div>\n\n\n         <div class=\"item\" data-expire=\"2017-02-19\">\n            <a href=\"https://portal.kai.ru/web/abiturientu/events/event?id=5649497\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kai_dod_17.jpg\" alt=\"День открытых дверей КНИТУ - КАИ\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-03-01\">\n            <a href=\"http://c-zarnitza.ru/uslugi/#us-6\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_zarnica_cm_17.jpg\" alt=\"КибернетикУМ в Зарнице\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-14\">\n            <a href=\"http://camp.kpfu.ru/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kfu_lag_discover.jpg\" alt=\"Лагерь английского языка 'DISCOVER KFU'\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-03-13\">\n            <a href=\"http://selet.biz/news/yay-summer-campaign-has-started/?clear_cache=Y\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_selet_17.png\" alt=\"С?л?т 2017\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-28\">\n            <a href=\"http://allkzn.com/rasti/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_ll_rasti_s_it_17.jpg\" alt=\"Расти с IT\">\n            </a>\n         </div>\n\n\n\n         <div class=\"item\" data-expire=\"2017-02-04\">\n            <a href=\"http://www.kai.ru/univer/liceum/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kai_inzh_dod_17_2.jpg\" alt=\"День открытых дверей в Инженерном лицее КНИТУ-КАИ\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-15\">\n            <a href=\"https://vk.com/technoclass.club\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_techclass_olymp_17.png\" alt=\"Первая открытая олимпиада по робототехнике\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-03-15\">\n            <a href=\"http://ieml.ru/faculties/psychologic/konkurs-nauchno-issledovatelskikh-rabot/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kiu_kp.jpg\" alt=\"Республиканский конкурс по психологии\">\n            </a>\n         </div>\n\n\n         <div class=\"item\" data-expire=\"2017-02-03\">\n            <a href=\"http://intellekt.kai.ru/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kai_oip_17.jpg\" alt=\"Олимпиада по информатике и программированию\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-16\">\n            <a href=\"http://technoklass.org\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_techclass_17.jpg\" alt=\"Клуб робототехники и изобретательства Технокласс\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-02-27\">\n            <a href=\"http://kpfu.ru/distancionnyj-konkurs-elektronnyh-otkrytok-263192.html\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_kfu_konkurs_pv.jpg\" alt=\"Конкурс 'Поздравляю Вас'\">\n            </a>\n         </div>\n\n         <div class=\"item\" data-expire=\"2017-01-13\">\n            <a href=\"http://www.hi-future.ru/\" target=\"_blank\">\n                <img src=\"/design/images/banners/ban_mon_hi-future.jpg\" alt=\"Новогодняя интерактивная выставка для всей семьи\">\n            </a>\n         </div>\n\n    </div>\n</div>\n\n\n<ul class=\"list-unstyled\" style=\"margin-top:15px;\"\">\n\n        \n    \n   \n    \n    \n    \n    \n    \n    \n    \n</ul>\n\n            </div>\n        </div>\n\n\t        \n        <div class=\"toggle-button-wrapper\">\n        </div>\n        <div id=\"top\">\n            <h1><a href=\"/\">Электронное образование в Республике Татарстан</a></h1>\n            <div id=\"inner_navy\">\n                <ul>\n                    <li><a href=\"/index.htm\" title=\"Организации\" class=\"btn_1\">Организации</a></li>\n                    <li><a href=\"/student\" title=\"Ученику\" class=\"btn_2\">Ученику</a></li>\n                                        <li><a href=\"/teacher\" title=\"Учителю\" class=\"btn_3\">Учителю</a></li>\n                                                                <li><a href=\"/user/remote-learning/index\" title=\"Дистанционное образование\" class=\"btn_4\">Дистанционное образование</a></li>\n                                    </ul>\n            </div>\t\t\t\n        </div>\n\n                <div id=\"container\" class=\"clearfix\">\r\n\r\n            <div class=\"crumbs\">\r\n                <ul class=\"clearfix\">\r\n                \t<li><a href=\"/\">Главная</a></li>\n<li><a href=\"/\">Личный кабинет</a></li>\n<li class=\"active\"><span class=\"l\">Мой дневник</span></li>                </ul>\t\r\n            </div>\r\n\r\n            <div id=\"content_holder\">\r\n                <div id=\"content\">\r\n          \t\t\t\r\n\r\n                    <div class=\"search header hidden-print\">\r\n                    \t<h2></h2>\r\n                        <div class=\"extra\">\r\n                        \t                            <a href=\"http://help.edu.tatar.ru/\" class=\"xhelp\">Помощь</a>\r\n                        \t                            <a href=\"/user/anketa\" class=\"cabinet\">Личный кабинет</a>\r\n                            <!--a href=\"/user/news\" class=\"news_b\">Новости</a-->\r\n                            <a href=\"/logoff\" class=\"log_off\">Выход</a>\r\n                        </div>\r\n                        <div class=\"form\">\r\n\t                        <input type=\"text\" name=\"q\" class=\"q\" value=\"\">\r\n    \t                    <input type=\"button\" name=\"qr\" class=\"qr\" value=\"\">\r\n\t\t\t\t\t\t</div>\r\n                    </div>\r\n\r\n                    <div class=\"r_block\">\r\n                        <div class=\"h\">\r\n                        \t                            <!--link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"/font/stylesheet.css\"-->\r\n<!--h1></h1-->\r\n\r\n<div class=\"xdiary\">\r\n\t<div class=\"calendar-type\">\r\n\t\t<a href=\"/user/diary/day\"><img onmouseover=\"this.src='/design/images/diary/day_a.gif'\" onmouseout=\"this.src='/design/images/diary/day.gif'\" src=\"/design/images/diary/day_a.gif\"></a>\r\n\t\t<a href=\"/user/diary\"><img onmouseover=\"this.src='/design/images/diary/week_a.gif'\" onmouseout=\"this.src='/design/images/diary/week.gif'\" src=\"/design/images/diary/week.gif\"></a>\r\n\t\t<a href=\"/user/diary/month\"><img onmouseover=\"this.src='/design/images/diary/month_a.gif'\" onmouseout=\"this.src='/design/images/diary/month.gif'\" src=\"/design/images/diary/month.gif\"></a>\r\n\t\t<span class=\"tabel\"><a href=\"/user/diary/term\"><img onmouseover=\"this.src='/design/images/diary/tabel_a.gif'\" onmouseout=\"this.src='/design/images/diary/tabel.gif'\" src=\"/design/images/diary/tabel.gif\"></a></span>\r\n\t</div>\r\n\t<div class=\"dsw\">\r\n\t\t<table>\r\n\t\t\t<tr>\r\n\t\t\t\t<td>\r\n\t\t\t\t\t<span class=\"prevd\"><a href=\"https://edu.tatar.ru/user/diary/day?for=1486933200\"><img src=\"/design/images/diary/prev_s.gif\"></a></span>\r\n\t\t\t\t</td>\r\n\t\t\t\t<td class=\"d-date\">\t\t\r\n\t\t\t\t\t<span>14 Февраля 2017 г.</span>\r\n\t\t\t\t</td>\r\n\t\t\t\t<td>\r\n\t\t\t\t\t<span class=\"nextd\"><a href=\"https://edu.tatar.ru/user/diary/day?for=1487106000\"><img src=\"/design/images/diary/next_s.gif\"></a></span>\r\n\t\t\t\t</td>\r\n\t\t\t</tr>\r\n\t\t</table>\r\n\t</div>\r\n\t<div class=\"d-table\">\r\n\t\t<table class=\"main\">\r\n\t\t\t<thead>\r\n\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td>Время</td>\r\n\t\t\t\t\t<td>Предмет</td>\r\n\t\t\t\t\t<td>Что задано</td>\r\n\t\t\t\t\t<td>Комментарий</td>\r\n\t\t\t\t\t<td>Оценка</td>\r\n\t\t\t\t</tr>\r\n\t\t\t</thead>\r\n\t\t\t<tbody>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">8:00<br/>&mdash;<br/>8:45</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">Русский язык</td>\r\n\t\t\t\t\t<td style=\"text-align: left; font-size: 12px; vertical-align: middle;\">\r\n\t\t\t\t\t\t<p>1)Выучить памятку на стр.28, 30;<br />\r\n2)Упр. 444 на стр.31 (вставленные буквы подчеркнуть)</p>\r\n\t\t\t\t\t</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\"><i>\r\n\t\t\t\t\t\t\t</i></td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">\r\n\t\t\t\t\t\t\t</td>\r\n\t\t\t\t</tr>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">8:50<br/>&mdash;<br/>9:35</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">Физическая культура</td>\r\n\t\t\t\t\t<td style=\"text-align: left; font-size: 12px; vertical-align: middle;\">\r\n\t\t\t\t\t\t<p></p>\r\n\t\t\t\t\t</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\"><i>\r\n\t\t\t\t\t\t\t</i></td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">\r\n\t\t\t\t\t\t\t</td>\r\n\t\t\t\t</tr>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">9:50<br/>&mdash;<br/>10:35</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">Обществознание</td>\r\n\t\t\t\t\t<td style=\"text-align: left; font-size: 12px; vertical-align: middle;\">\r\n\t\t\t\t\t\t<p>параграф 9, пересказ пункт № 1-2, ответить на вопрос № 1, в рубрике проверим себя</p>\r\n\t\t\t\t\t</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\"><i>\r\n\t\tНе был\t\t\t\t\t</i></td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">\r\n\t\t\t\t\t\t\t\t<table class=\"marks\">\r\n\t\t\t\t\t\t\t<tr>\r\n\t\t\t\t\t\t\t\t\t</tr>\r\n\t\t\t\t\t\t</table>\r\n\t\t\t\t\t\t\t</td>\r\n\t\t\t\t</tr>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">10:50<br/>&mdash;<br/>11:35</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">Английский</td>\r\n\t\t\t\t\t<td style=\"text-align: left; font-size: 12px; vertical-align: middle;\">\r\n\t\t\t\t\t\t<p>p.80 ex.5 (написать рассказ о своих выходных в Present Continuous, пример ex.2)</p>\r\n\t\t\t\t\t</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\"><i>\r\n\t\tБолел\t\t\t\t\t</i></td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">\r\n\t\t\t\t\t\t\t\t<table class=\"marks\">\r\n\t\t\t\t\t\t\t<tr>\r\n\t\t\t\t\t\t\t\t\t</tr>\r\n\t\t\t\t\t\t</table>\r\n\t\t\t\t\t\t\t</td>\r\n\t\t\t\t</tr>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t<tr style=\"text-align: center;\">\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">11:45<br/>&mdash;<br/>12:30</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">Математика</td>\r\n\t\t\t\t\t<td style=\"text-align: left; font-size: 12px; vertical-align: middle;\">\r\n\t\t\t\t\t\t<p>У: стр. 161, пример 4 – читать; № 579(б, г, е), 580(б, г), 581(б, г), 582(б, г), 583(б, в).</p>\r\n\t\t\t\t\t</td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\"><i>\r\n\t\t\t\t\t\t\t</i></td>\r\n\t\t\t\t\t<td style=\"vertical-align: middle;\">\r\n\t\t\t\t\t\t\t\
            <table class=\"marks\">\
                                        <tbody><tr>\
                                                    <td title=\"Домашняя работа\"><div>4</div></td>\
                                                    <td title=\"Практическая работа\"><div>4</div></td>\
                                                </tr>\
                                    </tbody></table></td>\r\n\t\t\t\t</tr>\r\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t</tbody>\r\n\t\t</table>\r\n\t</div>\r\n</div><br>\r\n<p style=\"font-size: 11px; color: #999\">Чтобы увидеть за какой вид работ поставлена оценка - наведите на нее курсор мыши.</p>                        </div>                            \r\n                    </div>                        \r\n                </div>\r\n            </div>\r\n        </div>\r\n<style>.text-muted { color: #979797; }</style>\n        <div id=\"footer\">\n            <div style=\"float: right; padding-top: 1.4em\">\n                Для того, чтобы обратиться в техническую поддержку, перейдите по <a style=\"color: #979797\" href=\"/fhelp\">ссылке</a>            </div>\n            <p>&copy; 2009&ndash;2017 &laquo;Электронное образование в Республике Татарстан&raquo;<br>\n            Все права защищены. <a class=\"text-muted\" href=\"/policies/terms\">Общие условия использования</a></p>\n        </div>\n    </div>\n\n<script language=\"JavaScript\" src=\"/js/moment-2.6.0-lang-ru.min.js\" type=\"text/javascript\"></script>\n<script language=\"JavaScript\" src=\"/js/moment-2.6.0.min.js\" type=\"text/javascript\"></script>\n    \n<!-- google analytics -->\n\n<script type=\"text/javascript\">\n\t\t\t\n\t\t\n\t\t\t\tvar imagesTimer;\n\n\t\t\t\tfunction net_ok() {\n\t\t\t\t\tclearTimeout(imagesTimer);\n\t\t\t\t\t\t\t\t\n\t\t\t\t\t$J.ajax({\n\t\t\t\t\t\ttype: \"GET\",\n\t\t\t\t\t\turl: \"/start/external-available?state=1\"\n\t\t\t\t\t});\n\t\t\t\t}\n\n\t\t\t\tfunction net_failed() {\n\t\t\t\t\t\n\t\t\t\t\t$J.ajax({\n\t\t\t\t\t\ttype: \"GET\",\n\t\t\t\t\t\turl: \"/start/external-available?state=0\",\n\t\t\t\t\t\tsuccess: function(d){\n                            $J(\"#nchimg\").attr(\"src\", '/design/images/0.gif');\n\t\t\t\t\t\t}\n\t\t\t\t\t});\n\t\t\t\t}\n\n\t\t\t\t$J(document).ready(function() {\n\t\t\t\t\timagesTimer = setTimeout(\"net_failed()\", 2000);\n\t\t\t\t\tvar ya = '<img id=\"nchimg\" height=\"0\" style=\"visibility:hidden;\" onload=\"net_ok()\" src=\"https://google.com/favicon.ico?nocache=' +(new Date()).getTime()+ '\">';\n\t\t\t\t\tvar b = document.getElementsByTagName('body')[0];\n\t\t\t\t\tb.innerHTML += ya;\n\t\t\t\t});\n\t\t\t\t\n\t\t    </script>\n</body>\n</html>";
    EduTatarData::day_t day;
    if( parseDairyHtml( html, day ) == false ){
        qCritical("can't parse dairy html");
        return;
    }
    _data.addDay( day );
    emit dataReceived();
}

bool EduTatarProvider::parseDairyHtml( const QString& html, EduTatarData::day_t& day )
{
    QString h= html;
    h.remove( QRegExp("^.*<table class=\"main\">") );
    h.insert( 0, "<table class=\"main\">" );

    QString tableEndStr= "</table>";
    int tableEndIdx= h.lastIndexOf( tableEndStr );
    if( tableEndIdx < 0 )
        return false;
    h.remove( tableEndIdx + tableEndStr.length(), h.length() );

    qCDebug(fcHttp) << "\nxml1[" << h << "]";

    QDomDocument doc;
    QString errStr;
    int errl, errc;
    doc.setContent( h, false, &errStr, &errl, &errc );
    if( ! errStr.isEmpty() ){
        qDebug() << "setContent error:" << errStr << " l,c:" << errl << "," << errc;
        return false;
    }

    // search <table class="main">
    QDomNodeList nlist= doc.elementsByTagName( "tbody" );

    if( nlist.size() < 1 ){
        qCritical("can't find <tbody>");
        return false;
    }

    QDomElement elCurday= nlist.at(0).toElement();
    for( QDomElement e= elCurday.firstChildElement("tr"); ! e.isNull(); e= e.nextSiblingElement("tr") ){
        QDomElement el= e.firstChildElement("td").nextSiblingElement("td");
        QString subj= el.text();
        qDebug() << "Subject: " << subj;

        el= el.nextSiblingElement("td");
        QString hw= el.firstChildElement("p").text();
        qDebug() << "Homework: " << hw;

        el= el.nextSiblingElement("td");
        QString com= el.firstChildElement("i").text().trimmed();
        qDebug() << "comment: " << com;

        EduTatarData::subject_t &s= day.add( subj, com, hw );

        // marks
        el= el.nextSiblingElement("td").firstChildElement("table");
        if( ! el.isNull() ){
            QDomNodeList ml= el.elementsByTagName( "td" );
            if( ml.size() > 0 ){
                qDebug() << "marks num: " << ml.size();
                for( int i=0; i<ml.size(); i++ ){
                    // mark target
                    QDomNode dn= ml.at(i);
                    QDomNode attr= dn.toElement().attributes().namedItem("title");
                    if( ! attr.isNull() ){
                        QString reason= attr.toAttr().value();
                        QString mark= dn.firstChildElement("div").text();
                        s.addMark( mark, reason );
                        qDebug() << "mark: " << reason << " : " << mark;
                    }
                }
            }
        }
    }

    return true;
}

EduTatarData::day_t EduTatarProvider::dataForDay( const QDate& date )
{
    return _data.day(date);
}

/******************************************************************************/
EduTatarData::EduTatarData()
{

}

EduTatarData::~EduTatarData()
{

}

bool EduTatarData::addDay( const EduTatarData::day_t& day )
{
    _days[day.date]= day;
    return true;
}

EduTatarData::day_t& EduTatarData::day( const QDate& date )
{
    return _days[date];
}
