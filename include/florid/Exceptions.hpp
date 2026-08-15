#ifndef FLORID_EXCEPTIONS_HPP
#define FLORID_EXCEPTIONS_HPP

#include <stdexcept>

namespace florid {

class Exception : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class NetworkException : public Exception {
public:
    using Exception::Exception;
};

class ProtocolException : public Exception {
public:
    using Exception::Exception;
};

class IncompatibleVersionException : public Exception {
public:
    using Exception::Exception;
};

class ControlException : public Exception {
public:
    using Exception::Exception;
};

class RealtimeException : public Exception {
public:
    using Exception::Exception;
};

class InvalidOperationException : public Exception {
public:
    using Exception::Exception;
};

class CommandException : public Exception {
public:
    using Exception::Exception;
};

} // namespace florid

#endif // FLORID_EXCEPTIONS_HPP
