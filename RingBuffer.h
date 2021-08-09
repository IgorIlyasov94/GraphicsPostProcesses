#pragma once

#include "stdafx.h"

namespace Graphics
{
	template<typename RingBufferType>
	class RingBufferList
	{
	private:
		using UnderlyingType = std::list<RingBufferType>;

	public:
		RingBufferList() {};
		RingBufferList(size_t bufferSize)
		{
			buffer.resize(bufferSize);
		}

		RingBufferList(typename UnderlyingType::iterator vectorBegin, typename UnderlyingType::iterator vectorEnd)
		{
			size_t bufferSize = std::distance(vectorBegin, vectorEnd);
			buffer.resize(bufferSize);
			std::copy(vectorBegin, vectorEnd, std::back_inserter(buffer));
		}

		RingBufferList(typename UnderlyingType::const_iterator vectorBegin, typename UnderlyingType::const_iterator vectorEnd)
		{
			size_t bufferSize = std::distance(vectorBegin, vectorEnd);
			buffer.resize(bufferSize);
			std::copy(vectorBegin, vectorEnd, std::back_inserter(buffer));
		}

		~RingBufferList() {};

		void PushFront(const RingBufferList& newElement)
		{
			buffer.push_front(newElement);
		}

		void PushFront(RingBufferList&& newElement)
		{
			buffer.push_front(std::forward<RingBufferType>(newElement));
		}

		void PushBack(const RingBufferList& newElement)
		{
			buffer.push_back(newElement);
		}

		void PushBack(RingBufferList&& newElement)
		{
			buffer.push_back(std::forward<RingBufferType>(newElement));
		}

		std::list<RingBufferType>& GetNative() noexcept
		{
			return buffer;
		}

		const size_t& Size() const noexcept
		{
			return buffer.size();
		}

		RingBufferType& operator[](int64_t index)
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			return *bufferIterator;
		}

		void RemoveElement(int64_t index) noexcept
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			buffer.erase(bufferIterator);
		}

		void Clear() noexcept
		{
			buffer.clear();
		}

	private:
		UnderlyingType buffer;
	};

	template<typename RingBufferType>
	class RingBufferVector
	{
	private:
		using UnderlyingType = std::vector<RingBufferType>;

	public:
		RingBufferVector() {};
		RingBufferVector(size_t bufferSize)
		{
			buffer.resize(bufferSize);
		}

		RingBufferVector(typename UnderlyingType::iterator vectorBegin, typename UnderlyingType::iterator vectorEnd)
		{
			size_t bufferSize = std::distance(vectorBegin, vectorEnd);
			buffer.resize(bufferSize);
			std::copy(vectorBegin, vectorEnd, buffer.begin());
		}

		RingBufferVector(typename UnderlyingType::const_iterator vectorBegin, typename UnderlyingType::const_iterator vectorEnd)
		{
			size_t bufferSize = std::distance(vectorBegin, vectorEnd);
			buffer.resize(bufferSize);
			std::copy(vectorBegin, vectorEnd, buffer.begin());
		}

		~RingBufferVector() {};

		void PushBack(const RingBufferVector& newElement)
		{
			buffer.push_back(newElement);
		}

		void PushBack(RingBufferVector&& newElement)
		{
			buffer.push_back(std::forward<RingBufferType>(newElement));
		}

		std::vector<RingBufferType>& GetNative() noexcept
		{
			return buffer;
		}

		size_t Size() const noexcept
		{
			return buffer.size();
		}

		RingBufferType& operator[](int64_t index)
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			return buffer[correctIndex];
		}

		void RemoveElement(int64_t index) noexcept
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			buffer.erase(bufferIterator);
		}

		void Clear() noexcept
		{
			buffer.clear();
		}

	private:
		UnderlyingType buffer;
	};
}
