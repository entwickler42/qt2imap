#include "qt2imapconnection_p.h"
#include <QHostInfo>


QtImapConnection::QtImapConnection(QObject* parent) :
	QObject(parent),
	d(new QtImapConnectionPrivate(this))
{}

QtImapConnection::~QtImapConnection()
{}

void QtImapConnection::setCredintials(QtImapUnsecureCredintials* credintials)
{
    d->credintials = credintials;
    emit credintialsChanged(credintials);
}

QtImapCredintials* QtImapConnection::credintials() const
{
    return d->credintials;
}

const QHostAddress& QtImapConnection::serverAddress() const
{
	return d->addr_server;
}

quint16 QtImapConnection::serverPort() const
{
	return d->tcp_port;
}

QString QtImapConnection::errorString() const
{
	return d->last_error_string;
}

QString QtImapConnection::socketErrorString() const
{
	return d->socket.errorString();
}

int QtImapConnection::lastError() const
{
	return d->last_error;
}

int QtImapConnection::lastSocketError() const
{
	return d->last_socket_error;
}

void QtImapConnection::setServerAddress(const QString& addr)
{
	QHostInfo dns = QHostInfo::fromName(addr);
	if( dns.error() == QHostInfo::NoError ){
		d->addr_server = dns.addresses().first();
	}else{
		d->setLastError(QtImap::HostLookupError);
	}
}

void QtImapConnection::setServerAddress(const QHostAddress& addr)
{
	d->addr_server = addr;
}

void QtImapConnection::setServerPort(quint16 port)
{
	d->tcp_port = port;
}


int QtImapConnection::state() const
{
	return d->state;
}

const QString& QtImapConnection::selectedMailbox() const
{
	return d->selected_mailbox;
}

void QtImapConnection::open()
{
	d->connectToHost();
}

QtImapRequest* QtImapConnection::capability()
{
    return request(QtImap::IMAP_CAPABILITY);
}

QtImapRequest* QtImapConnection::noop()
{
    return request(QtImap::IMAP_NOOP);
}

QtImapRequest* QtImapConnection::startTls()
{
    return request(QtImap::IMAP_STARTTLS);
}

QtImapRequest* QtImapConnection::logout()
{
	d->disconnectFromHost();
    return 0;
}

QtImapRequest* QtImapConnection::login()
{
    if( d->credintials == 0 ) return 0;

	return request(QtImap::IMAP_LOGIN, QStringList()
                   << d->credintials->username()
                   << QString(d->credintials->password()));
}

QtImapRequest* QtImapConnection::select(const QString& mailbox)
{
    return request(QtImap::IMAP_SELECT, QStringList() << mailbox);
}

QtImapRequest* QtImapConnection::examine(const QString& mailbox)
{
    return request(QtImap::IMAP_EXAMINE, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::create(const QString& mailbox)
{
    return request(QtImap::IMAP_CREATE, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::deleteMailbox(const QString& mailbox)
{
    return request(QtImap::IMAP_DELETE, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::lsub(const QString& mailbox)
{
    return request(QtImap::IMAP_LSUB, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::status(const QString& mailbox)
{
    return request(QtImap::IMAP_STATUS, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::append(const QString& mailbox, const QByteArray& literal, const QString& flag, const QString& datetime)
{
    QStringList argv;
    argv << mailbox;
    if( !flag.isEmpty() ) argv << QString("(\\%1)").arg(datetime);
    if( !datetime.isEmpty() ) argv << QString("{%1}").arg(datetime);
    return request(QtImap::IMAP_APPEND, literal, argv);
}

QtImapRequest* QtImapConnection::check()
{
    return request(QtImap::IMAP_CHECK);
}

QtImapRequest* QtImapConnection::close()
{
    return request(QtImap::IMAP_CLOSE);
}

QtImapRequest* QtImapConnection::expunge()
{
    return request(QtImap::IMAP_EXPUNGE);
}

QtImapRequest* QtImapConnection::subscribe(const QString& mailbox)
{
    return request(QtImap::IMAP_SUBSCRIBE, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::unsubscribe(const QString& mailbox)
{
    return request(QtImap::IMAP_UNSUBSCRIBE, QStringList() << mailbox );
}

QtImapRequest* QtImapConnection::list(const QString& reference, const QString& mailbox)
{
    return request(QtImap::IMAP_LIST, QStringList() << QString("\"%1\"").arg(reference) << QString("\"%1\"").arg(mailbox));
}

QtImapRequest* QtImapConnection::rename(const QString& mailbox, const QString& newMailboxName)
{
    return request(QtImap::IMAP_RENAME, QStringList() << mailbox << newMailboxName);
}

QtImapRequest* QtImapConnection::search(const QString& searchCriteria, const QString& charset)
{
    QStringList argv;
    if( !charset.isEmpty() ) argv.push_back(charset);
    argv.push_back(searchCriteria);
    return request(QtImap::IMAP_SEARCH, argv);
}

QtImapRequest* QtImapConnection::request(QtImap::CommandCode cmd, const QByteArray& literal, const QStringList& argv)
{
    return request(cmd, argv);
}

QtImapRequest* QtImapConnection::request(QtImap::CommandCode cmd, const QStringList& argv)
{
	QtImapRequest* req = new QtImapRequest(this, d->nextTag(), cmd, argv);
	if( d->request(req) == 0 ) {
		delete req;
		req = 0;
	}else emit requestSend(req);
	return req;
}
