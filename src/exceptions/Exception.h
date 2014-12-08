#pragma once
#include "../SharedHeaders.h"

#define EXCEPTION_INFO __FUNCTION__,__LINE__

namespace br {
	class Exception : public std::exception {
	public:
		Exception(std::string func, uint32_t line, std::string reason) : func(func), line(line), reason(reason) {}
		~Exception() = default;

		virtual std::string msg() const {
			return func + "(" + std::to_string(line) + "): " + what() + " " + reason;
		};

	protected:
		std::string func;
		uint32_t line;
		std::string reason;
	};

	class NativeWindowException : public Exception {
	public:
		NativeWindowException(std::string func, uint32_t line, std::string reason)
			: Exception(func, line, reason) {
		}
		~NativeWindowException() = default;

		const char* what() const override {
			return "NativeWindowException";
		}
	};

	class EglException : public Exception {
	public:
		EglException(std::string func, uint32_t line, std::string reason)
			: Exception(func, line, reason) {
		}
		~EglException() = default;

		const char* what() const override {
			return "EglException";
		}
	};

	class ShaderException : public Exception {
	public:
		ShaderException(std::string func, uint32_t line, std::string reason)
			: Exception(func, line, reason) {
		}
		~ShaderException() = default;

		const char* what() const override {
			return "ShaderException";
		}
	};

	class InvalidObjectIdException : public Exception {
	public:
		InvalidObjectIdException(std::string func, uint32_t line, uint32_t objId)
			: Exception(func, line, "id: " + std::to_string(objId)) {
		}
		~InvalidObjectIdException() = default;

		const char* what() const override {
			return "InvalidObjectIdException";
		}
	};

	class Model3dException : public Exception {
	public:
		Model3dException(std::string func, uint32_t line, std::string path, std::string reason)
			: Exception(func, line, "path: " + path + " " + reason) {
		}
		~Model3dException() = default;

		const char* what() const override {
			return "Model3dException";
		}
	};

	class GpuException : public Exception {
	public:
		GpuException(std::string func, uint32_t line, std::string reason)
			: Exception(func, line, reason) {
		}
		~GpuException() = default;

		const char* what() const override {
			return "GpuException";
		}
	};
}