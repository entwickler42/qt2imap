#ifndef QT2IMAP_P_H
#define QT2IMAP_P_H

#include "qt2imapconnection.h"

#include <QThread>
#include <QSslSocket>
#include <QStringList>
#include <QTimerEvent>
#include <QHostAddress>

Q2_BEGIN_NAMESPACE

#define Q2_TIMER_STOPPED -1

class QtImapConnectionPrivate;

//! ...
class QtImapConnectionPrivate :
		public QObject
{
	Q_OBJECT

public:
    quint16 tcp_port;
    QHostAddress addr_server;
    QSslSocket socket;
    QtImapUnsecureCredintials* credintials;
    QString selected_mailbox;
    QString last_error_string;
    char tag_char;
    int tag_int;
    int last_error;
    int last_socket_error;
    int state;
    int timout_connect_ms;
    int timer_auto_shutdown;
    int timer_dead_request;
    bool enable_tls;

	QtImapConnectionPrivate(QtImapConnection* parent) :
		QObject(parent),
		tcp_port(143),
		addr_server("127.0.0.1"),
		socket(this),
        credintials(0),
		tag_char('a'),
		tag_int(0),
		last_error(0),
		last_socket_error(QAbstractSocket::UnknownSocketError),
		state(QtImap::ConnectionClosed),
		timout_connect_ms(5 * 1000),
		timer_auto_shutdown(Q2_TIMER_STOPPED),
        timer_dead_request(Q2_TIMER_STOPPED),
		enable_tls(false),
		i_ptr(parent)
	{
		connect(&socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(on_socket_ssl_error(QList<QSslError>)));
		connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(on_socket_error(QAbstractSocket::SocketError)));
		connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(on_socket_stateChanged(QAbstractSocket::SocketState)));
		connect(&socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

		socket.setProtocol(QSsl::AnyProtocol);
	}

	~QtImapConnectionPrivate()
	{
		disconnect(&socket, 0, 0, 0);
		if( state != QtImap::ConnectionClosed ){
			socket.disconnectFromHost();
		}
	}

public slots:
	void readyRead()
	{
		if( !socket.canReadLine() ){
			setLastError(QtImap::ReadLineError);
			return;
		}
		// read all available lines and prase imap responses
		while( socket.canReadLine() ){
			QString line = socket.readLine();
			QtImapResponse* resp = QtImapResponse::fromString(line);
			QtImapRequest* origin = 0;
            QtImapRequest* target_data = 0;
			// determine request for tagged responses
            if( resp->type() == QtImap::TaggedResponse ){
				foreach( QtImapRequest* it, outreq ){
					if( it->tag() == resp->tag() ){
                        origin = target_data = it;
                        origin->m_is_finished = true;
						break;
					}
				}
            }
            // appand data to oldest outstandig request for this kind of data
            if( !target_data ){
                foreach( QtImapRequest* it, outreq ){
                    if( it->command() == resp->commandCode() ){
                        target_data = it;
                    }
                }
            }
            if( target_data ){
                target_data->m_resp_data.append(resp->data());
            }
			/* remove origin from waiting list, update state and
 emit signals according to request and response */
			if( origin ) {
				outreq.removeAll(origin);
				switch( origin->command() )
				{
                case QtImap::IMAP_STARTTLS:
                    if( resp->commandCode() == QtImap::IMAP_OK && !socket.isEncrypted() ){
                        socket.startClientEncryption();
                    }
                    break;

				case QtImap::IMAP_LOGIN:
					if( resp->commandCode() == QtImap::IMAP_OK ){
						setState(QtImap::ConnectionAuthenticated);
					}
					break;

                case QtImap::IMAP_EXAMINE:
				case QtImap::IMAP_SELECT:
					if( resp->commandCode() == QtImap::IMAP_OK ){
						setState(QtImap::ConnectionSelected);
						selected_mailbox = origin->argv().first();
						emit i_ptr->mailboxSelected(selected_mailbox);
					}
					break;

				default: break;
				}
			}
			// further update state and emit signals according to response
			switch( resp->commandCode() )
			{
			case QtImap::IMAP_OK:
				if( state == QtImap::ConnectionAwaitingGreeting ){
					abortAutoShutdown();
					setState(QtImap::ConnectionNotAuthenticated);
				}
				break;

			case QtImap::IMAP_BYE:
				if( state != QtImap::ConnectionLogout ){
					setState(QtImap::ConnectionLogout);
				}
				disconnectFromHost();
				break;

			case QtImap::IMAP_CAPABILITY:
				break;

			case QtImap::IMAP_FLAGS:
				break;

			case QtImap::IMAP_EXISTS:
				break;

			case QtImap::IMAP_RECENT:
				break;

			case QtImap::IMAP_BAD:
				setLastError(QtImap::BadCommandError);
				break;

			case QtImap::IMAP_NO:
				setLastError(QtImap::ServerDeniedError);
				break;

			case QtImap::IMAP_UNKNOWN:
				qDebug("Invalid Response: %s", qPrintable(line.remove("\r\n")));
				setLastError(QtImap::InvalidResponseError);
				break;

			default:
                //qDebug("Unhandled Response: %s", qPrintable(line.remove("\r\n")));
				break;
			}
			// emit ready and finished signals; no slots are allowed to delete the objects!
			emit i_ptr->responseReady(resp);
			if( origin ) emit i_ptr->requestFinished(origin);
		}
	}

	// send a request to the server
	QtImapRequest* request(QtImapRequest* request)
	{
		if( state == QtImap::ConnectionClosed || state == QtImap::ConnectionLogout ){
			setLastError(QtImap::ConnectionClosedError);
			return 0;
		}
		QByteArray _req = request->encode();
		_req.push_back("\r\n");
		if( socket.write(_req) != _req.length() ){
			setLastError(QtImap::WriteLineError);
			return 0;
		}
        if( timer_dead_request == Q2_TIMER_STOPPED ){
            timer_dead_request = startTimer(0);
		}
		switch(request->command())
		{
		case QtImap::IMAP_LOGOUT:
			initAutoShutdown();
			break;

		case QtImap::IMAP_STARTTLS:
			enable_tls = true;
			break;

		default: break;
		}
		outreq.push_back(request);
		return request;
	}

	void initAutoShutdown(int msec = -1)
	{
		if( msec == -1 ) msec = timout_connect_ms;
		if( timer_auto_shutdown != -1 )
			abortAutoShutdown();
		timer_auto_shutdown = startTimer(msec);
	}

	void abortAutoShutdown()
	{
		if( timer_auto_shutdown != -1 ){
			killTimer(timer_auto_shutdown);
			timer_auto_shutdown = -1;
		}
	}
	// establish TCP/IP connection and prepare imap
	void connectToHost()
	{
		if( state == QtImap::ConnectionClosed ){
			socket.connectToHost(addr_server, tcp_port);
			initAutoShutdown();
		}else setLastError(QtImap::ConcurrentOpenError);
	}
	// gracefully shutdown IMAP
	void disconnectFromHost()
	{
		switch( state )
		{
		case QtImap::ConnectionLogout:
			abortAutoShutdown();
			socket.disconnectFromHost();
			break;

		default:
			i_ptr->request(QtImap::IMAP_LOGOUT);
			break;
		}
	}

	// get the next outgoing command tag
	QString nextTag()
	{
		if( tag_int == INT_MAX ){
			tag_int=0;
			tag_char++; // bug
		}else{
			tag_int++;
		}
		return QString("%1%2").arg(tag_char).arg(tag_int);
	}

	// set last error members according to error and emit the error signal
	void setLastError(int error)
	{
		struct error_entry{
			int code;
			const char* message;
		};
		static const struct error_entry error_text[]  = {
			{ QtImap::UnknownError,              QT_TR_NOOP("an unknown error occured") },
			{ QtImap::SocketError,               QT_TR_NOOP("socket: %1") },
			{ QtImap::WriteLineError,            QT_TR_NOOP("can't send line to IMAP server") },
			{ QtImap::ReadLineError,             QT_TR_NOOP("can't read line - propably due to incomplete input") },
			{ QtImap::UnknownDataError,          QT_TR_NOOP("received unidentified line from server") },
			{ QtImap::ConcurrentOpenError,       QT_TR_NOOP("this connection is already open") },
			{ QtImap::TimeoutError,              QT_TR_NOOP("IMAP connection timed out") },
			{ QtImap::InvalidResponseError,      QT_TR_NOOP("invalid server response received") },
			{ QtImap::ServerDeniedError,         QT_TR_NOOP("IMAP server denied (NO)") },
			{ QtImap::BadCommandError,           QT_TR_NOOP("IMAP command not supported or not available in this state (BAD)") },
			{ QtImap::ConnectionClosedError,     QT_TR_NOOP("IMAP connection is in closed state") },
		    { QtImap::HostLookupError,           QT_TR_NOOP("can't resolve hostname to ip address") },
			{ -1, 0 }
		};
		const struct error_entry* entry = 0;
		const struct error_entry* it = error_text;
		while( !(it->code == -1 && it->message == 0) ){
			if( it->code == error ) {
				entry = it;
				break;
			}
			++it;
		}
		if(entry && entry->code == QtImap::SocketError) last_error_string = tr(entry->message).arg(socket.errorString());
		else if(entry) last_error_string = tr(entry->message);
		else last_error_string = tr("unknown error (%1)").arg(error);
		last_error = error;
		emit i_ptr->error(error);
	}	
	
private slots:
	// update internal state and reemit as imap error
	void on_socket_error(QAbstractSocket::SocketError socketError)
	{
		abortAutoShutdown();
		last_socket_error = socketError;
		setLastError(QtImap::SocketError);
	}
	//! \todo delegate decition on ssl errors to user
	void on_socket_ssl_error(QList<QSslError> /*errors*/)
	{
		socket.ignoreSslErrors();
	}
	// translate socket state to imap state and reemit imap state changes
	void on_socket_stateChanged(QAbstractSocket::SocketState socketState)
	{
		int reemit; // translated connection state or -1 to ignore
		switch(socketState)
		{
		case QAbstractSocket::UnconnectedState:
            clearAllOutstaning();
			reemit = QtImap::ConnectionClosed;
			break;

		case QAbstractSocket::ConnectingState:
			reemit = QtImap::ConnectionConnecting;
			break;

		case QAbstractSocket::ConnectedState:
			reemit = QtImap::ConnectionAwaitingGreeting;
			break;

		case QAbstractSocket::HostLookupState:
		default: reemit = -1;
		}
		if( reemit != -1 ) setState(QtImap::ConnectionState(reemit));
		// else qDebug("%s: unhandled socket state (%i)", Q_FUNC_INFO, socketState);
	}

private:
    QtImapConnection* i_ptr;
    QList<QtImapRequest*> outreq;

    void timerEvent(QTimerEvent *ev)
    {
        if( ev->timerId() == timer_auto_shutdown ){
            // emit timeout error and disconnect socket
            abortAutoShutdown();
            setLastError(QtImap::TimeoutError);
            disconnectFromHost();
        }
        if( ev->timerId() == timer_dead_request ){
            // remove expired responses and emit responseTimeout()
            killTimer(timer_dead_request);
            timer_dead_request = Q2_TIMER_STOPPED;
            QList<QtImapRequest*> _outreq = outreq;
            qint64 msecepoc = QDateTime::currentMSecsSinceEpoch() - 1000000 * 60;
            foreach(QtImapRequest* it, _outreq){
                if( it->timestamp() < msecepoc ){
                    emit i_ptr->requestTimeout(it);
                    outreq.removeAll(it);
                }
            }
            if( outreq.count() > 0 ) timer_dead_request = startTimer(0);
        }
    }
    // forget about all outstanding replies and asoziated high level event buffers
    void clearAllOutstaning()
    {
        qDeleteAll(outreq);
        outreq.clear();
    }
	// update internal state and emit the stateChanged() signal
	void setState(QtImap::ConnectionState state)
	{
		this->state = state;
		emit i_ptr->stateChanged(state);
	}
};

Q2_END_NAMESPACE

#endif // QT2IMAP_P_H
