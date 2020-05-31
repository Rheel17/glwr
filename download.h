/*
 * Copyright (c) 2020 Levi van Rheenen
 */
#ifndef MGL_DOWNLOAD_H
#define MGL_DOWNLOAD_H

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <rapidxml/rapidxml.hpp>

#include <iostream>
#include <string>
#include <exception>

using Json = nlohmann::json;
using XmlDocument = rapidxml::xml_document<>;

class CurlError : public std::exception {

public:
	explicit CurlError(const char* error) :
			_error(error) {}

	[[nodiscard]] const char* what() const noexcept override {
		return _error.c_str();
	}

private:
	const std::string _error;

};

class MemoryBlock {

public:
	MemoryBlock() :
			_cap(16),
			_size(0) {

		_ptr = static_cast<char*>(std::malloc(_cap));
		if (!_ptr) {
			throw CurlError("Out of memory");
		}
	}

	~MemoryBlock() noexcept {
		free(_ptr);
		_ptr = nullptr;
		_cap = 0;
	}

	MemoryBlock(const MemoryBlock&) = delete;
	MemoryBlock& operator=(const MemoryBlock&) = delete;

	MemoryBlock(MemoryBlock&& mb) noexcept:
			_ptr(mb._ptr),
			_cap(mb._cap),
			_size(mb._size) {

		mb._ptr = nullptr;
		mb._size = 0;
		mb._cap = 0;
	}

	MemoryBlock& operator=(MemoryBlock&& mb) noexcept {
		if (&mb != this) {
			std::swap(_ptr, mb._ptr);
			std::swap(_cap, mb._cap);
			std::swap(_size, mb._size);
		}

		return *this;
	}

	size_t Append(void* data, size_t bytes) {
		if (_size + bytes > _cap) {
			while (_size + bytes > _cap) {
				_cap *= 1.5;
			}

			_ptr = static_cast<char*>(std::realloc(_ptr, _cap));
			if (!_ptr) {
				throw CurlError("Out of memory");
			}
		}

		memcpy(_ptr + _size, data, bytes);
		_size += bytes;

		return bytes;
	}

	[[nodiscard]] size_t GetSize() const {
		return _size;
	}

	[[nodiscard]] const char* AsCharArray() const {
		return _ptr;
	}

	[[nodiscard]] std::string AsString() const {
		return std::string(_ptr, _size);
	}

	[[nodiscard]] Json AsJson() const {
		return Json::parse(_ptr);
	}

	[[nodiscard]] std::unique_ptr<XmlDocument> AsXml() const {
		auto doc = std::make_unique<XmlDocument>();
		doc->parse<0>(_ptr);
		return doc;
	}

private:
	char* _ptr = nullptr;
	size_t _cap = 0;
	size_t _size = 0;

};

size_t curlWriteFunction(void* contents, size_t size, size_t nmemb, void* userp) {
	auto block = static_cast<MemoryBlock*>(userp);
	return block->Append(contents, size * nmemb);
}

MemoryBlock download(const std::string& url) {
	CURL* curl = curl_easy_init();
	MemoryBlock memory;

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, false);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Build-Modern-OpenGL-Header");
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &memory);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);

	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		throw CurlError(curl_easy_strerror(res));
	}

	curl_easy_cleanup(curl);

	char zero = 0;
	memory.Append(&zero, 1);

	std::cout << "Downloaded " << url << " (" << memory.GetSize() << " byte(s))" << std::endl;

	return memory;
}


#endif
