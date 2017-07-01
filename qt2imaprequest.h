#ifndef QT2IMAPREQUEST_H
#define QT2IMAPREQUEST_H

#include "qt2imap.h"
#include <QDateTime>
#include <QStringList>

Q2_BEGIN_NAMESPACE

class QtImapConnection;

class QT2IMAPSHARED_EXPORT QtImapRequest
{
public:
    typedef QList<QString> ResponseData;

	const QString& tag() const { return m_tag; }
	QtImap::CommandCode command() const { return m_cmd; }
	const QStringList& argv() const { return m_argv; }
	qint64 timestamp() const { return msepoc; }
	QString toString() const
	{
		QString rval = m_tag;
		bool haskey = false;
		const struct command_code* it = command_code_table;
		while( !(it->id == QtImap::IMAP_UNKNOWN && it->key.isNull()) ){
			if( it->id == m_cmd ) {
				rval.append(' ');
				rval.append(it->key);
				haskey = true;
				break;
			}
			++it;
		}
		if( !haskey ) qWarning("!!! missing command_code_table entry for: %i", (int)m_cmd);
		foreach(const QString& it, m_argv){
			rval.append(' ');
			rval.append(it);
		}
		return rval;
	}
	QByteArray encode() const
	{
		QByteArray rval = m_tag.toAscii();
		bool haskey = false;
		const struct command_code* it = command_code_table;
		while( !(it->id == QtImap::IMAP_UNKNOWN && it->key.isNull()) ){
			if( it->id == m_cmd ) {
				rval.append(' ');
				rval.append(it->key.toAscii());
				haskey = true;
				break;
			}
			++it;
		}
		if( !haskey ) qWarning("!!! missing command_code_table entry for: %i", (int)m_cmd);
		foreach(const QString& it, m_argv){
			rval.append(' ');
			rval.append(it);
		}
		return rval;
	}
    const ResponseData& responseData() const { return m_resp_data; }
    bool isFinished() const { return m_is_finished; }

private:
	friend class QtImapConnection;
	friend class QtImapConnectionPrivate;

	QtImapRequest(QtImapConnection* connection, const QString& tag, QtImap::CommandCode cmd, const QStringList& argv) :
		m_con(connection),
		m_cmd(cmd),
		m_argv(argv),
		m_tag(tag),
        msepoc(QDateTime::currentMSecsSinceEpoch()),
        m_is_finished(false)
    {}

	QtImapConnection* m_con;
	QtImap::CommandCode m_cmd;
	QStringList m_argv;
	QString m_tag;
	qint64 msepoc;
    bool m_is_finished;
    ResponseData m_resp_data;
};

Q2_END_NAMESPACE

#endif // QT2IMAPREQUEST_H
