#ifndef QTIMAPCONNECTION_H
#define QTIMAPCONNECTION_H

#include "qt2imap_global.h"
#include <QString>

Q2_BEGIN_NAMESPACE

//! QtImap...
class QT2IMAPSHARED_EXPORT QtImap
{
    public:
	enum MessageFlags{
		MessageSeen      = 0x01, //! Message has been read
		MessageAnswered  = 0x02, //! Message has been answered
		MessageFlagged   = 0x04, //! Message is "flagged" for urgent/special attention
		MessageDeleted   = 0x08, //! Message is "deleted" for removal by later EXPUNGE
		MessageDraft     = 0x10, //! Message has not completed composition
		MessageRecent    = 0x20  //! Message is "recently" arrived in this mailbox
	};

	enum ConnectionState{
		ConnectionClosed            = 1, //! Connection is closed
		ConnectionConnecting        = 2, //! Connection is about to be established
		ConnectionAwaitingGreeting  = 3, //! TCP connection was established successfully and QtImap is waiting for the greeting message
		ConnectionNotAuthenticated  = 4, //! In the not authenticated state, the client MUST supply authentication credentials before most commands will be permitted
		ConnectionAuthenticated     = 5, //! In the authenticated state, the client is authenticated and MUST select a mailbox to access before commands that affect messages will be permitted
		ConnectionSelected          = 6, //! In a selected state, a mailbox has been selected to access
		ConnectionLogout            = 7  //! In the logout state, the connection is being terminated
	};

	enum ErrorCode
	{
		UnknownError               = 1,
		UnknownDataError           = 2,
		InvalidResponseError       = 3,
		SocketError                = 4,
		TimeoutError               = 5,
		ReadLineError              = 6,
		WriteLineError             = 7,
		ConcurrentOpenError        = 8,
		ServerDeniedError          = 9,
        ConnectionClosedError      = 10,
        BadCommandError            = 11,
		HostLookupError            = 12
	};

	enum ResponseType
	{
		InvalidResponse,
		TaggedResponse,
		UntaggedResponse,
		ContinuationResponse
	};

	enum CommandCode
	{
		IMAP_OK,
		IMAP_NO,
		IMAP_BAD,
		IMAP_BYE,
		IMAP_NOOP,
		IMAP_FLAGS,
		IMAP_LOGIN,
		IMAP_LOGOUT,
		IMAP_SELECT,
        IMAP_LIST,
        IMAP_EXAMINE,          //! The EXAMINE command is identical to SELECT and returns the same output; however, the selected mailbox is identified as read-only
        IMAP_CREATE,           //! The CREATE command creates a mailbox with the given name
        IMAP_RENAME,           //! The RENAME command changes the name of a mailbox
        IMAP_DELETE,           //! The DELETE command permanently removes the mailbox with the given name
        IMAP_SUBSCRIBE,        //! The SUBSCRIBE command adds the specified mailbox name to the server's set of "active" or "subscribed" mailboxes
        IMAP_UNSUBSCRIBE,      //! The UNSUBSCRIBE command removes the specified mailbox name from the server's set of "active" or "subscribed" mailboxes as returned by the LSUB command
        IMAP_LSUB,             //! The LSUB command returns a subset of names from the set of names that the user has declared as being "active" or "subscribed"
        IMAP_STATUS,           //! The STATUS command requests the status of the indicated mailbox
        IMAP_CHECK,            //! The CHECK command requests a checkpoint of the currently selected
        IMAP_EXPUNGE,          //! The EXPUNGE command permanently removes all messages that have the \Deleted flag set from the currently selected mailbox
        IMAP_SEARCH,           //! The SEARCH command searches the mailbox for messages that match the given searching criteria
        IMAP_CLOSE,
        IMAP_APPEND,
		IMAP_EXISTS,
		IMAP_RECENT,
		IMAP_STARTTLS,
		IMAP_CAPABILITY,
		IMAP_UNKNOWN
	};
};

struct command_code { QtImap::CommandCode id; QString key; };

static struct command_code command_code_table[] = {
    { QtImap::IMAP_OK,          "OK"           },
    { QtImap::IMAP_NO,          "NO"           },
    { QtImap::IMAP_BAD,         "BAD"          },
    { QtImap::IMAP_BYE,         "BYE"          },
    { QtImap::IMAP_NOOP,        "NOOP"         },
    { QtImap::IMAP_FLAGS,       "FLAGS"        },
    { QtImap::IMAP_LOGIN,       "LOGIN"        },
    { QtImap::IMAP_LOGOUT,      "LOGOUT"       },
    { QtImap::IMAP_SELECT,      "SELECT"       },
    { QtImap::IMAP_LIST,        "LIST"         },
    { QtImap::IMAP_EXAMINE,     "EXAMINE"      },
    { QtImap::IMAP_CREATE,      "CREATE"       },
    { QtImap::IMAP_DELETE,      "DELETE"       },
    { QtImap::IMAP_RENAME,      "RENAME"       },
    { QtImap::IMAP_LSUB,        "LSUB"         },
    { QtImap::IMAP_SUBSCRIBE,   "SUBSCRIBE"    },
    { QtImap::IMAP_UNSUBSCRIBE, "UNSUBSCRIBE"  },
    { QtImap::IMAP_STATUS,      "STATUS"       },
    { QtImap::IMAP_CHECK,       "CHECK"        },
    { QtImap::IMAP_EXISTS,      "EXISTS"       },
    { QtImap::IMAP_EXPUNGE,     "EXPURGE"      },
    { QtImap::IMAP_SEARCH,      "SEARCH"       },
    { QtImap::IMAP_CLOSE,       "CLOSE"        },
    { QtImap::IMAP_APPEND,      "APPEND"       },
    { QtImap::IMAP_RECENT,      "RECENT"       },
    { QtImap::IMAP_STARTTLS,    "STARTTLS"     },
    { QtImap::IMAP_CAPABILITY,  "CAPABILITY"   },
    { QtImap::IMAP_UNKNOWN,     QString()      }
};


Q2_END_NAMESPACE


#endif // QTIMAPCONNECTION_H
