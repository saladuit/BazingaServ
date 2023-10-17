#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

// PARSE_HTTP_REQUEST -- request struct?		Status:
// 1) parse request line						done
// 		a) Set method type -- enum				done
// 		b) Set path								done
//		c) Set version							done
// 2) parse headers --> map						done
// 3) parse body --> ??							done (as string)

// INCLUDE
#include <iostream>
#include <map>

// DEFINES

#ifndef HTTP_READ_SIZE
#define HTTP_READ_SIZE 1024
#endif

// ENUM
enum class HTTPMethod
{
	GET,
	POST,
	DELETE,
	UNKNOWN,
};

// CLASS
class HTTPRequest
{
  private:
	HTTPMethod _methodType;
	std::string _path;
	std::string _version;
	std::map<std::string, std::string> _headers;
	std::string _body;

  public:
	// canonical form: constructors, operator and destructor
	HTTPRequest();
	HTTPRequest(const HTTPRequest &other);
	HTTPRequest &operator=(const HTTPRequest &other);
	~HTTPRequest();

	// methodType methods
	void setMethodType(const std::string &requestLine);
	HTTPMethod getMethodType(void) const;

	// path methods
	void setPath(const std::string &requestLine);
	const std::string &getPath(void) const;

	// version methods
	void setVersion(const std::string &requestLine);
	const std::string &getVersion(void) const;

	// header methods
	void setHeader(const std::string &headerLine);
	std::string &getValue(const std::string &key);

	// body methods
	void setBody(const std::string &body);
	const std::string &getBody(void) const;

	// parser
	void parse(void);

	// public variables -- just for now
	std::string _http_request_str;
	// size_t try out
	int _content_length;
	int _content_length_cpy;
	bool _post_method;
	size_t _pos;
};

#endif
