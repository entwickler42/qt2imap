#ifndef QT2IMAPRESPONSE_H
#define QT2IMAPRESPONSE_H

#include "qt2imap.h"

#include <QMetaType>
#include <QDateTime>

Q2_BEGIN_NAMESPACE

// QtImapResponsePrivate...
class QtImapResponsePrivate
{
	public:
	QtImapResponsePrivate() :
		msepoc(0)
	{}

	QString prefix;
	QString tag;
	QString command;
	QString data;
	qint64 msepoc;
};

//! QtImapResponse...
class QT2IMAPSHARED_EXPORT QtImapResponse
{
public:
	QtImapResponse() :
		d(new QtImapResponsePrivate)
	{}

	~QtImapResponse()
	{
		delete d;
	}

	QString toString() const
	{
		QString me = QString("%1 %2 %3")
				.arg(d->prefix.isEmpty() ? d->tag : d->prefix)
				.arg(d->command)
				.arg(d->data);
		return me;
	}

	static QtImapResponse* fromString(const QString& line)
	{
		QByteArray ascii = line.toAscii();
		QtImapResponse* resp = new QtImapResponse;
		quint8 token = 0;
		bool quote_double = false;
		bool quote_single = false;
		const char* pch = ascii.constData();
		while( !(*pch == '\r' || *pch == '\n' || *pch == '\0') ){
			if( *pch == '\'' ) quote_single = !quote_single;
			if( *pch == '"' ) quote_double = !quote_double;
			if( *pch == ' ' && token < 3 && !(quote_double || quote_single) ) {
				++token; ++pch;
				continue;
			}
			switch( token )
			{
			case 0:
				if( *pch == '*' ){
					resp->d->prefix = QString(*pch);
					break;
				}else if( *pch == '+' ){
					resp->d->prefix = QString(*pch);
					break;
				}else ++token;

			case 1:
				if( resp->d->prefix != "*" ){
					resp->d->tag.append(QChar(*pch));
					break;
				}else ++token;

			case 2:
				resp->d->command.append(QChar(*pch).toUpper());
				break;

			default:
				resp->d->data.append(QChar(*pch));
				break;
			}
			++pch;
		}
		// juggle some numeric responses to look like usual responses
		if( resp->d->data.compare("EXISTS") == 0 ){
			resp->d->data = resp->d->command;
			resp->d->command = "EXISTS";
		}else if( resp->d->data.compare("RECENT") == 0 ){
			resp->d->data = resp->d->command;
			resp->d->command = "RECENT";
		}else if( resp->d->data.compare("EXPUNGE") == 0 ){
			resp->d->data = resp->d->command;
			resp->d->command = "EXPUNGE";
		}

		resp->d->msepoc = QDateTime::currentMSecsSinceEpoch();
		return resp;
	}

	const QString& prefix() { return d->prefix; }
	const QString& tag() { return d->tag; }
	const QString& command() { return d->command; }
	const QString& data() { return d->data; }

	QtImap::ResponseType type()
	{
		if( d->prefix == "+" ) return QtImap::ContinuationResponse;
		else if( d->prefix == "*" ) return QtImap::UntaggedResponse;
		else if( !d->tag.isEmpty() ) return QtImap::TaggedResponse;
		return QtImap::InvalidResponse;
	}

	QtImap::CommandCode commandCode()
	{
		const struct command_code* it = command_code_table;
		while( !(it->id == QtImap::IMAP_UNKNOWN && it->key.isNull()) ){
			if( d->command.compare(it->key, Qt::CaseInsensitive) == 0 ){
				return it->id;
			}
			++it;
		}
		return QtImap::IMAP_UNKNOWN;
	}

	bool isNull() const { return d->command.isEmpty(); }

	qint64 timestamp() const { return d->msepoc; }

private:
	QtImapResponsePrivate* d;
};

Q2_END_NAMESPACE

Q_DECLARE_METATYPE(CuteCube::QtImapResponse)

static int g_QtImapResponse_MetaType =
qRegisterMetaType<CuteCube::QtImapResponse>("QtImapResponse");

#endif // QT2IMAPRESPONSE_H
