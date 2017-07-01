#ifndef QT2IMAPCONNECTION_H
#define QT2IMAPCONNECTION_H

#include "qt2imap.h"
#include "qt2imaprequest.h"
#include "qt2imapresponse.h"

class QHostAddress;

Q2_BEGIN_NAMESPACE

class QtImapConnectionPrivate;

class QT2IMAPSHARED_EXPORT QtImapCredintials
{
public:
    virtual ~QtImapCredintials()
    {}
    virtual const QByteArray& username() const = 0;
    virtual const QByteArray& password() const = 0;
};

class QT2IMAPSHARED_EXPORT QtImapUnsecureCredintials
        : public QtImapCredintials
{
public:
    QtImapUnsecureCredintials(const QByteArray& user = QByteArray(), const QByteArray& passwd = QByteArray()) :
		m_user(user),
		m_passwd(passwd)
	{}

    const QByteArray& username() const { return m_user; }
	const QByteArray& password() const { return m_passwd; }

	QByteArray m_user;
	QByteArray m_passwd;
};

//! QtImapConnection ...
class QT2IMAPSHARED_EXPORT QtImapConnection :
		public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString mailbox READ selectedMailbox WRITE select NOTIFY mailboxSelected )
    Q_PROPERTY(QString errorString READ errorString NOTIFY error)

	friend class QtImapConnectionPrivate;

signals:
	void error(int error);
	void stateChanged(int state);
    void mailboxSelected(const QString& mailbox);
    void credintialsChanged(QtImapCredintials* credintials);

	void requestSend(QtImapRequest* request);
	void requestFinished(QtImapRequest* request);
    void requestTimeout(QtImapRequest* request);
	void responseReady(QtImapResponse* response);

public:
	QtImapConnection(QObject* parent = 0);
	virtual ~QtImapConnection();

	const QHostAddress& serverAddress() const;
	quint16 serverPort() const;

	QString errorString() const;
	QString socketErrorString() const;
	
	int lastError() const;
	int lastSocketError() const;
	
	int state() const;

    QtImapCredintials* credintials() const;
	const QString& selectedMailbox() const;

public slots:
    //! set server address by hostname or ip address and
	void setServerAddress(const QString& addr);
    //! set server address
	void setServerAddress(const QHostAddress& addr);
    //! set server TCP port
	void setServerPort(quint16 port);
    //! set authenticator
    void setCredintials(QtImapUnsecureCredintials* credintials);
	//! establish imap connection
    void open();

    /* Client Commands - Any State */
    QtImapRequest* capability();
    QtImapRequest* noop();
    //! close imap socket and reset the connection object
    QtImapRequest* logout();
    QtImapRequest* startTls();
    /* MISSING: AUTHENTICATE */
    QtImapRequest* login();
    /* lient Commands - Authenticated State */
    QtImapRequest* select(const QString& mailbox);
    QtImapRequest* examine(const QString& mailbox);
    QtImapRequest* create(const QString& mailbox);
    QtImapRequest* deleteMailbox(const QString& mailbox);
    QtImapRequest* rename(const QString& mailbox, const QString& newMailboxName);
    QtImapRequest* subscribe(const QString& mailbox);
    QtImapRequest* unsubscribe(const QString& mailbox);
    QtImapRequest* list(const QString& reference, const QString& mailbox);
    QtImapRequest* lsub(const QString& mailbox);
    QtImapRequest* status(const QString& mailbox);
    QtImapRequest* append(const QString& mailbox, const QByteArray& literal, const QString& flag = QString(), const QString& datetime = QString());
    /* Client Commands - Selected State */
    QtImapRequest* check();
    QtImapRequest* close();
    QtImapRequest* expunge();
    QtImapRequest* search(const QString& searchCriteria, const QString& charset = QString());
    /* MISSING: FETCH */
    /* MISSING: STORE */
    /* MISSING: COPY */
    /* MISSING: UUID */
    /* Client Commands - Experimental/Expansion */
    /* MISSING: X<atom> */

protected:
    //! execute an imap command and ensure proper command handling
    QtImapRequest* request(QtImap::CommandCode cmd, const QByteArray& literal, const QStringList& argv = QStringList());
	QtImapRequest* request(QtImap::CommandCode cmd, const QStringList& argv = QStringList());

private:
	QtImapConnectionPrivate* d;
};


Q2_END_NAMESPACE

#endif // QT2IMAPCONNECTION_H
