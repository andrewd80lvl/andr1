#include "edutatarprovider.h"
#include <QUrlQuery>
#include <QXmlQuery>
#include <QRegularExpression>
#include <time.h>

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

void testDairy()
{
    QString html(
"<html> <head> <script type=\"text/javascript\"> if (isOpenedInFrame()) redirectToBlank(); </script> </head> <body> <div id=\"wrapper\" class=\"inner without_menu\"> <div class=\"row banners hidden-print\"> <div class=\"col-md-12\"> <div class=\"carousel slide vertical\" data-ride=\"carousel\"> <div class=\"carousel-inner\"> <div class=\"item active\" data-expire=\"2017-02-02\"> <a href=\"http://mon.tatarstan.ru/rus/obschee-obrazovanie.htm\" target=\"_blank\"> <img src=\"/design/images/banners/ban_mon_reg_ege.jpg\" alt=\"Регистрация на ЕГЭ\"> </a> </div> <div class=\"item\" data-expire=\"2017-01-27\"> <a href=\"http://onlyege.ru/\" target=\"_blank\"> <img src=\"/design/images/banners/ban_onlyege.jpg\" alt=\"Образовательный ресурс «onlyege.ru»\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-03\"> <a href=\"https://portal.kai.ru/web/abiturientu/events/event?id=5677018\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kai_ege_physics.jpg\" alt=\"Республиканское тестирование по физике\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-07\"> <a href=\"http://kpfu.ru/law/struktura/strukturnye-podrazdeleniya/centr-pravovoj-informacii/zimnyaya-profilnaya-shkolapo-obschestvoznaniju\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kfu_zsh_uf.jpg\" alt=\"КФУ - зимняя профильная школа по обществознанию\"> </a> </div> <div class=\"item\" data-expire=\"2017-01-30\"> <a href=\"http://www.kai.ru/univer/liceum/\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kai_inzh_dod_17_1.jpg\" alt=\"День открытых дверей в Инженерном лицее КНИТУ-КАИ\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-15\"> <a href=\"http://kpfu.ru/do/struktura-i-funkcii/otdel-organizacii-i-soprovozhdeniya/nauchnaya-konferenciya-uchaschihsya-imeni\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kfu_conf_lob_17.jpg\" alt=\"Научная конференция учащихся имени Н.И.Лобачевского\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-03\"> <a href=\"http://intellekt.kai.ru/\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kai_oip_17.jpg\" alt=\"Олимпиада по информатике и программированию\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-16\"> <a href=\"http://technoklass.org\" target=\"_blank\"> <img src=\"/design/images/banners/ban_techclass_17.jpg\" alt=\"Клуб робототехники и изобретательства Технокласс\"> </a> </div> <div class=\"item\" data-expire=\"2017-01-21\"> <a href=\"http://baytik-kazan.ru/zapis191/razrabotka_igr_na_Action_Script_30\" target=\"_blank\"> <img src=\"/design/images/banners/ban_baitik_17_as3.png\" alt=\"Программирование игр на Action Script 3.0\"> </a> </div> <div class=\"item\" data-expire=\"2017-02-27\"> <a href=\"http://kpfu.ru/distancionnyj-konkurs-elektronnyh-otkrytok-263192.html\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kfu_konkurs_pv.jpg\" alt=\"Конкурс 'Поздравляю Вас'\"> </a> </div> <div class=\"item\" data-expire=\"2017-01-13\"> <a href=\"http://www.hi-future.ru/\" target=\"_blank\"> <img src=\"/design/images/banners/ban_mon_hi-future.jpg\" alt=\"Новогодняя интерактивная выставка для всей семьи\"> </a> </div> <div class=\"item\" data-expire=\"2017-03-01\"> <a href=\"http://kpfu.ru/konkurs-39kazanskij-universitet-zazhigaet.html\" target=\"_blank\"> <img src=\"/design/images/banners/ban_kfu_konkurs.jpg\" alt=\"Конкурс сочинений «Казанский университет зажигает звезды»\"> </a> </div> </div> </div> <ul class=\"list-unstyled\" style=\"margin-top:15px;\"\"> </ul> </div> </div> <div class=\"toggle-button-wrapper\"> </div> <div id=\"top\"> <h1><a href=\"/\">Электронное образование в Республике Татарстан</a></h1> <div id=\"inner_navy\"> <ul> <li><a href=\"/index.htm\" title=\"Организации\" class=\"btn_1\">Организации</a></li> <li><a href=\"/student\" title=\"Ученику\" class=\"btn_2\">Ученику</a></li> <li><a href=\"/teacher\" title=\"Учителю\" class=\"btn_3\">Учителю</a></li> <li><a href=\"/user/remote-learning/index\" title=\"Дистанционное образование\" class=\"btn_4\">Дистанционное образование</a></li> </ul> </div>            </div> <div id=\"container\" class=\"clearfix\"> <div class=\"crumbs\"> <ul class=\"clearfix\"> <li><a href=\"/\">Главная</a></li> <li><a href=\"/\">Личный кабинет</a></li> <li class=\"active\"><span class=\"l\">Мой дневник</span></li>                </ul>    </div> <div id=\"content_holder\"> <div id=\"content\"> <div class=\"search header hidden-print\"> <h2></h2> <div class=\"extra\"> <a href=\"http://help.edu.tatar.ru/\" class=\"xhelp\">Помощь</a> <a href=\"/user/anketa\" class=\"cabinet\">Личный кабинет</a> <!--a href=\"/user/news\" class=\"news_b\">Новости</a--> <a href=\"/logoff\" class=\"log_off\">Выход</a> </div> <div class=\"form\"> <input type=\"text\" name=\"q\" class=\"q\" value=\"\"> <input type=\"button\" name=\"qr\" class=\"qr\" value=\"\"> </div> </div> <div class=\"r_block\"> <div class=\"h\"> <!--link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"/font/stylesheet.css\"--> <!--h1></h1--> <div class=\"xdiary\"> <div class=\"calendar-type\"> <a href=\"/user/diary/day\"><img onmouseover=\"this.src='/design/images/diary/day_a.gif'\" onmouseout=\"this.src='/design/images/diary/day.gif'\" src=\"/design/images/diary/day_a.gif\"></a> <a href=\"/user/diary\"><img onmouseover=\"this.src='/design/images/diary/week_a.gif'\" onmouseout=\"this.src='/design/images/diary/week.gif'\" src=\"/design/images/diary/week.gif\"></a> <a href=\"/user/diary/month\"><img onmouseover=\"this.src='/design/images/diary/month_a.gif'\" onmouseout=\"this.src='/design/images/diary/month.gif'\" src=\"/design/images/diary/month.gif\"></a> <span class=\"tabel\"><a href=\"/user/diary/term\"><img onmouseover=\"this.src='/design/images/diary/tabel_a.gif'\" onmouseout=\"this.src='/design/images/diary/tabel.gif'\" src=\"/design/images/diary/tabel.gif\"></a></span> </div> <div class=\"dsw\"> <table> <tr> <td> <span class=\"prevd\"><a href=\"https://edu.tatar.ru/user/diary/day?for=1485378000\"><img src=\"/design/images/diary/prev_s.gif\"></a></span> </td> <td class=\"d-date\">        <span>27 Января 2017 г.</span> </td> <td> <span class=\"nextd\"><a href=\"https://edu.tatar.ru/user/diary/day?for=1485550800\"><img src=\"/design/images/diary/next_s.gif\"></a></span> </td> </tr> </table> </div> <div class=\"d-table\"> <table class=\"main\"> <thead> <tr style=\"text-align: center;\"> <td>Время</td> <td>Предмет</td> <td>Что задано</td> <td>Комментарий</td> <td>Оценка</td> </tr> </thead> <tbody> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">8:00<br/>&mdash;<br/>8:45</td> <td style=\"vertical-align: middle;\">История</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>урок повторения</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> <table class=\"marks\"> <tr> <td title=\"Практическая работа\"><div>4</div></td> </tr> </table> </td> </tr> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">8:50<br/>&mdash;<br/>9:35</td> <td style=\"vertical-align: middle;\">Математика</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>У: стр. 144-145 – читать; № 509, 510, 511, 512(а-в, по 2 первых примера).</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> </td> </tr> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">9:50<br/>&mdash;<br/>10:35</td> <td style=\"vertical-align: middle;\">Музыка</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>Выучить записи из тетради.</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> </td> </tr> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">10:50<br/>&mdash;<br/>11:35</td> <td style=\"vertical-align: middle;\">Татарский язык</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>3нче к?нег?(74 бит), 5нче к?нег?(75 бит).</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> </td> </tr> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">11:45<br/>&mdash;<br/>12:30</td> <td style=\"vertical-align: middle;\">Русский язык</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>Подготовиться к словарному диктанту по словам из раздела &quot;Пиши правильно&quot; на стр.172, начиная с Н</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> </td> </tr> <tr style=\"text-align: center;\"> <td style=\"vertical-align: middle;\">12:35<br/>&mdash;<br/>13:20</td> <td style=\"vertical-align: middle;\">Математика</td> <td style=\"text-align: left; font-size: 12px; vertical-align: middle;\"> <p>У: стр. 147, пример 7  –  читать; № 519(в, г), 520(б), 522(в); <br /> З: № 313(в), 314(в), 321(в, г), 325(в).</p> </td> <td style=\"vertical-align: middle;\"><i> </i></td> <td style=\"vertical-align: middle;\"> </td> </tr> </tbody> </table> </div> </div><br> <p style=\"font-size: 11px; color: #999\">Чтобы увидеть за какой вид работ поставлена оценка - наведите на нее курсор мыши.</p>                        </div>                            </div>                        </div> </div> </div> <style>.text-muted { color: #979797; }</style> <div id=\"footer\"> <div style=\"float: right; padding-top: 1.4em\"> Для того, чтобы обратиться в техническую поддержку, перейдите по <a style=\"color: #979797\" href=\"/fhelp\">ссылке</a>            </div> <p>&copy; 2009&ndash;2017 &laquo;Электронное образование в Республике Татарстан&raquo;<br> Все права защищены. <a class=\"text-muted\" href=\"/policies/terms\">Общие условия использования</a></p> </div> </div> <script language=\"JavaScript\" src=\"/js/moment-2.6.0-lang-ru.min.js\" type=\"text/javascript\"></script> <script language=\"JavaScript\" src=\"/js/moment-2.6.0.min.js\" type=\"text/javascript\"></script> <!-- google analytics --> <script type=\"text/javascript\"> var imagesTimer; function net_ok() { clearTimeout(imagesTimer); $J.ajax({ type: \"GET\", url: \"/start/external-available?state=1\" }); } function net_failed() { $J.ajax({ type: \"GET\", url: \"/start/external-available?state=0\", success: function(d){ $J(\"#nchimg\").attr(\"src\", '/design/images/0.gif'); } }); } $J(document).ready(function() { imagesTimer = setTimeout(\"net_failed()\", 2000); var ya = '<img id=\"nchimg\" height=\"0\" style=\"visibility:hidden;\" onload=\"net_ok()\" src=\"https://google.com/favicon.ico?nocache=' +(new Date()).getTime()+ '\">'; var b = document.getElementsByTagName('body')[0]; b.innerHTML += ya; }); </script> </body> </html>");

    QXmlQuery xp(QXmlQuery::XPath20);
    xp.setFocus( html );
    xp.setQuery( ".//text()" );

    QString res;
    xp.evaluateTo( &res );
    qDebug() << "resul is: " << res;
}

void EduTatarProvider::testXPath()
{
    QString t="asdf<!asd\nf>qwert\n<!  sfw\n  ffd>aaaaaa <style>sssss \n sss</style> fffff";
    qDebug() << "html:" << t;
//    t.remove( QRegularExpression( "<![^>]*>" ) );
    t.remove( QRegularExpression( "<style>.*</style>" ) );
    qDebug() << "html2:" << t;

    return;


    QNetworkRequest req;
    req.setUrl( QUrl("https://www.wikipedia.org/") );

    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyWiki() ) );
}

void EduTatarProvider::onReplyWiki()
{
    qDebug( "reply from wikipedia" );

    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    if ( reply->error() != QNetworkReply::NoError ) {
        qDebug() << "Failure" << reply->errorString();
        return;
    }

    QString html= reply->readAll();
    qDebug() << "html:\n" << html << "\n";
    html.remove( QRegularExpression( "<![^>]*>" ) );
    qDebug() << "\nhtml:\n" << html << "\n";
    printf( "html3:%s\n", html.toLatin1().data() );

    QXmlQuery xp(QXmlQuery::XPath20);
    xp.setFocus( html );
    xp.setQuery( ".//h1/strong/text()" );

    QString res;
    xp.evaluateTo( &res );
    qDebug() << "resul is: " << res;
}

/******************************************************************************/
EduTatarProvider::EduTatarProvider( QObject* p )
    : QObject( p ),
      _mgr( p )
{
//    testDairy();
    testXPath();
}

EduTatarProvider::~EduTatarProvider()
{

}

bool EduTatarProvider::update()
{
    QNetworkRequest req;
    req.setUrl( g_url_login );
    req.setHeader( QNetworkRequest::UserAgentHeader, g_userAgent );
    req.setRawHeader( "Host", "edu.tatar.ru" );
    req.setRawHeader( "Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8" );
    req.setRawHeader( "Connection", "keep-alive" );
    qDebug() << "hdrs: "  << req.rawHeaderList();

    QNetworkReply *pr= _mgr.get( req );
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyMainPage() ) );
    return true;
}

void EduTatarProvider::onRedirect(QUrl url)
{
    qDebug() << "in onRedirect: " << url;
}

void EduTatarProvider::onReplyMainPage()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainPage() ) );

    qDebug( "received main page" );
    qDebug() << reply;

    if ( reply->error() != QNetworkReply::NoError ) {
        qDebug() << "Failure" << reply->errorString();
        return;
    }

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
    qDebug() << reply;

    if( reply->rawHeader("Location") != "/start/logon-process" ){
        qDebug() << "* logon failure";
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
    connect( pr, SIGNAL( finished() ), this, SLOT( onReplyMainAfterLogin() ) );
}

void EduTatarProvider::onReplyMainAfterLogin()
{
    QNetworkReply *reply= qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT( reply != NULL );

    reply->deleteLater();
    disconnect( reply, SIGNAL( finished()), this, SLOT( onReplyMainAfterLogin() ) );

    qDebug( "received main page after login" );
    qDebug() << reply;

    if( reply->rawHeader("Location") != "/" ){
        qDebug() << "* logon failure";
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
    qDebug() << reply;
    qDebug() << "html[" << html.length() << "]: ";
    qDebug() << html;

    QNetworkRequest req;
    req.setUrl( QUrl( g_url_diary.arg( time(NULL) ) ) );
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
    qDebug() << reply;
    qDebug() << "html[" << html.length() << "]: ";
    qDebug() << html;


}

/******************************************************************************/
EduTatarData::EduTatarData()
{

}

EduTatarData::~EduTatarData()
{

}
