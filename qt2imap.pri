QT += network

INCLUDEPATH += $$PWD

SOURCES += \
	 $$PWD/qt2imapconnection.cpp

HEADERS += $$PWD/qt2imap_global.h \
	 $$PWD/qt2imap.h \
	 $$PWD/qt2imapconnection_p.h \
	 ../qt2imap/qt2imapresponse.h \
	 ../qt2imap/qt2imapconnection.h \
    ../qt2imap/qt2imaprequest.h

DEFINES += QT2IMAP_LIBRARY

OTHER_FILES += $$PWD/GPLv2 \
	$$PWD/LICENSE \
	$$PWD/README \
	$$PWD/rfc3501 \
	$$PWD/rfc2822
