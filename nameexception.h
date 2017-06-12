#ifndef NAMEEXCEPTION_H
#define NAMEEXCEPTION_H

#include <exception>
#include <QByteArray>
#include <QString>

class NameConflictException : public std::exception {
    QByteArray _name;
    QString    unicodeName;
public:
    NameConflictException( QString name ) throw()
        : _name( name.toLocal8Bit() ), unicodeName(name)   {}
    ~NameConflictException() throw()                       {}
    QString name() const                                   { return unicodeName; }
    virtual const char* what() const throw()               { return (const char*)_name; }
};

#endif // NAMEEXCEPTION_H
